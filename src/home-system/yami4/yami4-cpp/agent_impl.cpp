// Copyright Maciej Sobczak 2008-2012.
// This file is part of YAMI4.
//
// YAMI4 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// YAMI4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with YAMI4.  If not, see <http://www.gnu.org/licenses/>.

#include "agent_impl.h"
#include "details.h"
#include "errors.h"
#include "event_callback.h"
#include "incoming_message.h"
#include "incoming_message_dispatcher_base.h"
#include "mutex_lock.h"
#include "outgoing_message_info.h"
#include "parameters.h"
#include "raw_buffer_data_source.h"
#include <yami4-core/channel_descriptor.h>
#include <yami4-core/fatal_errors.h>
#include <yami4-core/parameters.h>

// selected per platform
#include <delay.h>
#include <start_thread.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctime>
#include <vector>

using namespace yami;
using namespace details;

namespace // unnamed
{

// helper worker function that is called from a dedicated separate thread
// this worker function is responsible for processing I/O events
void worker(void * arg)
{
    agent_impl * this_agent = static_cast<agent_impl *>(arg);

    this_agent->do_event_loop();
}

// helper aggregate for dispatcher parameters
struct dispatcher_params
{
    agent_impl * agent;
    std::size_t dispatcher_index;
};

// dispatcher
void message_dispatcher(void * arg)
{
    dispatcher_params * params = static_cast<dispatcher_params *>(arg);

    agent_impl * this_agent = params->agent;
    std::size_t dispatcher_index = params->dispatcher_index;

    delete params;

    this_agent->do_message_dispatching(dispatcher_index);
}

// helper callback for incoming messages
extern "C" void incoming_message_callback(
    void * hint,
    const char * source,
    const char * header_buffers[],
    std::size_t header_buffer_sizes[],
    std::size_t num_of_header_buffers,
    const char * body_buffers[],
    std::size_t body_buffer_sizes[],
    std::size_t num_of_body_buffers)
{
    agent_impl * this_agent = static_cast<agent_impl *>(hint);

    // deserialize the message header and body

    core::parameters header;
    core::result res = header.deserialize(
        header_buffers, header_buffer_sizes, num_of_header_buffers);

    if (res != core::ok)
    {
        return;
    }

    const char * type_ptr;
    std::size_t type_length;
    res = header.get_string(
        "type", type_ptr, type_length);
    if (res != core::ok)
    {
        return;
    }

    const std::string message_type(type_ptr, type_length);
    if (message_type == "message")
    {
        // this is a regular (request) message

        const char * object_name_ptr;
        std::size_t object_name_length;
        res = header.get_string(
            "object_name", object_name_ptr, object_name_length);
        if (res != core::ok)
        {
            return;
        }

        const char * message_name_ptr;
        std::size_t message_name_length;
        res = header.get_string(
            "message_name", message_name_ptr, message_name_length);
        if (res != core::ok)
        {
            return;
        }

        long long message_id;
        res = header.get_long_long("message_id", message_id);
        if (res != core::ok)
        {
            return;
        }

        std::auto_ptr<incoming_message_info> im_info(
            new incoming_message_info());
        im_info->source = source;
        im_info->object_name.assign(object_name_ptr, object_name_length);
        im_info->message_name.assign(message_name_ptr, message_name_length);
        im_info->message_id = message_id;

        std::auto_ptr<std::vector<char> > raw_buffer;
        std::auto_ptr<parameters> body;

        if (this_agent->options_.deliver_as_raw_binary)
        {
            // do not deserialize the content buffer, copy it as raw

            raw_buffer_data_source body_wrapper(
                body_buffers, body_buffer_sizes, num_of_body_buffers);

            const std::size_t total_body_buffer_size =
                body_wrapper.serialize_buffer_size();
            raw_buffer.reset(new std::vector<char>(total_body_buffer_size));

            // repackage body,
            // so that it can be owned by incoming message info

            char * target_buffers[1];
            target_buffers[0] = &(*raw_buffer)[0];
            std::size_t target_buffer_sizes[1];
            target_buffer_sizes[0] = raw_buffer->size();
            std::size_t num_of_target_buffers = 1;

            body_wrapper.serialize(
                target_buffers, target_buffer_sizes, num_of_target_buffers);

            im_info->raw_buffer = raw_buffer.get();
        }
        else
        {
            // deserialize content into parameters object

            body.reset(new parameters());
            body->deserialize(
                body_buffers, body_buffer_sizes, num_of_body_buffers);
            im_info->body = body.release();
        }

        // store this new message in the queue
        // so that it can be picked by the dispatcher thread

        this_agent->queue_incoming(im_info);

        // at this point the queue takes responsibility of the info values,
        // including the body parameters or its raw buffer

        raw_buffer.release();
        body.release();
    }
    else if (message_type == "reply")
    {
        // this is a response to the message that was sent from this agent

        long long message_id;
        res = header.get_long_long("message_id", message_id);
        if (res != core::ok)
        {
            return;
        }

        if (this_agent->options_.deliver_as_raw_binary)
        {
            // do not deserialize the content buffer, copy it as raw

            raw_buffer_data_source body_wrapper(
                body_buffers, body_buffer_sizes, num_of_body_buffers);

            const std::size_t total_body_buffer_size =
                body_wrapper.serialize_buffer_size();
            std::auto_ptr<std::vector<char> > raw_buffer(
                new std::vector<char>(total_body_buffer_size));

            // repackage body,
            // so that it can be owned by incoming message info

            char * target_buffers[1];
            target_buffers[0] = &(*raw_buffer)[0];
            std::size_t target_buffer_sizes[1];
            target_buffer_sizes[0] = raw_buffer->size();
            std::size_t num_of_target_buffers = 1;

            body_wrapper.serialize(
                target_buffers, target_buffer_sizes, num_of_target_buffers);

            this_agent->report_replied(message_id, raw_buffer);
        }
        else
        {
            // deserialize content into parameters object

            std::auto_ptr<parameters> body(new parameters());
            body->deserialize(
                body_buffers, body_buffer_sizes, num_of_body_buffers);

            this_agent->report_replied(message_id, body);
        }
    }
    else if (message_type == "exception")
    {
        // this is a rejection/exception for the message
        // that was sent from this agent

        long long message_id;
        res = header.get_long_long("message_id", message_id);
        if (res != core::ok)
        {
            return;
        }

        const char * exception_reason_ptr;
        std::size_t exception_reason_length;
        res = header.get_string(
            "reason", exception_reason_ptr, exception_reason_length);
        if (res != core::ok && res != core::no_such_name)
        {
            return;
        }

        std::string reason;
        if (res == core::ok)
        {
            reason.assign(exception_reason_ptr, exception_reason_length);
        }

        this_agent->report_rejected(message_id, reason);
    }
    else
    {
        // unknown message type - ignore it as network junk
    }
}

// helper callback for reporting message transmission progress
extern "C" void outgoing_message_progress_callback(
    void * hint,
    std::size_t sent_bytes,
    std::size_t total_byte_count)
{
    outgoing_message_info * info = static_cast<outgoing_message_info *>(hint);

    bool last_notification = false;
    bool should_be_removed = false;
    {
        mutex_lock lock(info->mtx);

        info->sent_bytes = sent_bytes;
        info->total_byte_count = total_byte_count;

        if (sent_bytes == total_byte_count)
        {
            // there will be no more progress notifications for this message
            last_notification = true;

            if (sent_bytes != 0)
            {
                // the transmission of the whole message was successful

                if (info->state == posted)
                {
                    info->state = transmitted;
                    info->transmitted.notify();
                    info->process_callback();
                }
            }
            else
            {
                // the message was abandoned before it was fully transmitted

                if (info->state != posted)
                {
                    fatal_failure(__FILE__, __LINE__);
                }

                info->state = abandoned;

                info->transmitted.notify();
                info->completed.notify();
                info->process_callback();

                // there will be no more interaction with this message
                should_be_removed = true;
            }
        }
    }

    if (last_notification)
    {
        // the message is treated as leaving the output queue
        info->agent->decrease_outgoing_flow();
        
        if (should_be_removed)
        {
            info->agent->clean_outgoing_message_callback(info->message_id);
        }

        info->dec_ref_count();
    }
}

// helper callback for reporting message transmission progress
// for one-way messages
extern "C" void one_way_message_progress_callback(
    void * hint,
    std::size_t sent_bytes,
    std::size_t total_byte_count)
{
    agent_impl * this_agent = static_cast<agent_impl *>(hint);

    if (sent_bytes == total_byte_count)
    {
        // this is the last progress notification for *some*
        // one-way message - the transmission of this message
        // is either finished or abandoned and therefore
        // can be treated as leaving the output queue

        this_agent->decrease_outgoing_flow();
    }
}

extern "C" void new_incoming_connection_callback(
    void * hint,
    const char * source,
    std::size_t /* index */, std::size_t /* sequence_number */)
{
    agent_impl * this_agent = static_cast<agent_impl *>(hint);

    this_agent->report_connection_event(source, new_incoming_connection);
}

extern "C" void closed_connection_callback(
    void * hint,
    const char * name, core::result /* reason */)
{
    agent_impl * this_agent = static_cast<agent_impl *>(hint);

    this_agent->report_connection_event(name, connection_closed);
}

const char failover_prefix[] = "failover:(";
const std::size_t failover_prefix_length = sizeof(failover_prefix) - 1;
const char failover_suffix[] = ")";
const std::size_t failover_suffix_length = sizeof(failover_suffix) - 1;
const char failover_separator[] = "|";
const std::size_t failover_separator_length = sizeof(failover_separator) - 1;

bool is_target_failover(const std::string & target)
{
    return std::strncmp(target.c_str(), failover_prefix,
        failover_prefix_length) == 0 &&
        std::strcmp(target.c_str() + target.size() - failover_suffix_length,
            failover_suffix) == 0;
}

std::vector<std::string> split_failover_targets(const std::string & target)
{
    const std::string failover_content =
        target.substr(failover_prefix_length,
            target.size() - failover_prefix_length - failover_suffix_length);

    std::vector<std::string> result;
    std::string::size_type search_pos = 0;
    std::string::size_type found_pos = 0;
    while (found_pos != std::string::npos)
    {
        found_pos = failover_content.find(failover_separator, search_pos);

        std::string single_target;
        if (found_pos != std::string::npos)
        {
            single_target =
                failover_content.substr(search_pos, found_pos - search_pos);

            search_pos = found_pos + failover_separator_length;
        }
        else
        {
            single_target = failover_content.substr(search_pos);
        }

        if (single_target.empty() == false)
        {
            result.push_back(single_target);
        }
    }

    return result;
}

extern "C" void event_notification_callback(
    void * hint,
    core::event_notification e,
    const char * str,
    std::size_t size)
{
    event_callback * callback = reinterpret_cast<event_callback *>(hint);
    switch (e)
    {
    case core::agent_closed:
        callback->agent_closed();
        break;
    case core::listener_added:
        callback->listener_added(str);
        break;
    case core::listener_removed:
        callback->listener_removed(str);
        break;
    case core::incoming_connection_open:
        callback->incoming_connection_open(str);
        break;
    case core::outgoing_connection_open:
        callback->outgoing_connection_open(str);
        break;
    case core::connection_closed:
        callback->connection_closed(str);
        break;
    case core::connection_error:
        callback->connection_error(str);
        break;
    case core::message_sent:
        callback->message_sent(str, size);
        break;
    case core::message_received:
        callback->message_received(str, size);
        break;
    default:
        assert(false);
    }
}

extern "C" void io_error_callback(
    void * hint,
    int error_code,
    const char * description)
{
    io_error_dispatcher_base * callback =
        reinterpret_cast<io_error_dispatcher_base *>(hint);

    callback->dispatch(error_code, description);
}

} // unnamed namespace

void agent_impl::init(const parameters & options)
{
    options_.init(options);

    event_listener_ = NULL;

    translate_result_to_exception(
        agent_.init(options.get_core_object(),
            &incoming_message_callback, this,
            &closed_connection_callback, this));

    // set up the flow managers

    outgoing_flow_manager_.set_limits(
        options_.outgoing_high_water_mark, options_.outgoing_low_water_mark);

    outgoing_flow_allowed_.notify();

    incoming_flow_manager_.set_limits(
        options_.incoming_high_water_mark, options_.incoming_low_water_mark);

    allow_incoming_traffic_ = true;
    allow_incoming_traffic_mtx_.init();

    // start the worker thread

    worker_stop_request_ = false;
    worker_stop_mtx_.init();

    start_thread(&worker, this);

    // note: the initialization is not yet complete, but there is a thread
    // already running and accessing various objects
    // -> if the rest of initialization fails, the worker thread
    // (and all dispatcher threads that will be created) have to be stopped

    std::size_t i = 0;
    try
    {
        // start the dispatcher threads

        const std::size_t dispatcher_threads = options_.dispatcher_threads;
        dispatcher_stopped_flags_ = NULL;
        if (dispatcher_threads != 0)
        {
            dispatcher_stopped_flags_ = new flag[dispatcher_threads];
        }

        for (i = 0; i != options_.dispatcher_threads; ++i)
        {
            std::auto_ptr<dispatcher_params> params(new dispatcher_params);
            params->agent = this;
            params->dispatcher_index = i;

            start_thread(&message_dispatcher, params.get());

            params.release();
        }
    }
    catch (...)
    {
        // stop the worker thread
        stop_worker_thread();

        // stop those dispatchers that managed to be already created
        stop_dispatcher_threads(i);

        allow_incoming_traffic_mtx_.clean();

        throw;
    }
}

agent_impl::agent_impl(const parameters & options)
{
    init(options);
}

agent_impl::agent_impl(const parameters & options,
    event_callback & event_listener)
{
    init(options);

    agent_.install_event_notifications(
        event_notification_callback,
        &event_listener);

    event_listener_ = &event_listener;

    try
    {
        event_listener.agent_created();
    }
    catch (...)
    {
        // ignore errors from user callback
    }
}

void agent_impl::stop_worker_thread()
{
    {
        mutex_lock lock(worker_stop_mtx_);
        worker_stop_request_ = true;
    }

    (void) agent_.interrupt_work_waiter();

    worker_stopped_.wait();
    worker_stop_mtx_.clean();
}

void agent_impl::stop_dispatcher_threads(std::size_t num_of_threads)
{
    if (dispatcher_stopped_flags_ != NULL)
    {
        // insert termination values to the dispatcher queue

        for (std::size_t i = 0; i != num_of_threads; ++i)
        {
            incoming_queue_.push(std::auto_ptr<incoming_message_info>(NULL));
        }

        // wait for all the dispatchers to finish

        for (std::size_t i = 0; i != num_of_threads; ++i)
        {
            dispatcher_stopped_flags_[i].wait();
        }

        delete [] dispatcher_stopped_flags_;
    }
}

void agent_impl::clean()
{
    // stop the worker thread and allow the agent to clean up

    stop_worker_thread();

    // stop the dispatcher threads, if they exist

    stop_dispatcher_threads(options_.dispatcher_threads);

    // tear down the agent - during the cleanup all pending messages
    // are abandoned and their progress callbacks are triggered,
    // so the context for these callbacks should be retained here

    agent_.clean();

    // at that point no more low-level activity is possible

    allow_incoming_traffic_mtx_.clean();
}

agent_impl::~agent_impl()
{
    clean();
}

std::string agent_impl::add_listener(const std::string & listener)
{
    const char * resolved_target;
    translate_result_to_exception(
        agent_.add_listener(listener.c_str(),
            new_incoming_connection_callback, this,
            &resolved_target));

    return resolved_target;
}

void agent_impl::remove_listener(const std::string & listener)
{
    translate_result_to_exception(agent_.remove_listener(listener.c_str()));
}

void agent_impl::open_connection(const std::string & target)
{
    bool auto_connect = true;
    (void) make_sure_channel_exists(target, auto_connect);
}

void agent_impl::open_connection(const std::string & target,
    const parameters & options)
{
    bool auto_connect = true;
    (void) make_sure_channel_exists(target, auto_connect, &options);
}

std::auto_ptr<outgoing_message> agent_impl::send(
    const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect)
{
    return do_send(target,
        object_name, message_name, content, priority, auto_connect, false,
        std::auto_ptr<outgoing_message_dispatcher_base>(NULL),
        NULL, create_new);
}

void agent_impl::send(
    outgoing_message & message,
    const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect)
{
    (void) do_send(target,
        object_name, message_name, content, priority, auto_connect, false,
        std::auto_ptr<outgoing_message_dispatcher_base>(NULL),
        &message, replace);
}

long long agent_impl::send(
    std::auto_ptr<outgoing_message_dispatcher_base>
        outgoing_message_callback,
    const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect)
{
    long long message_id;
    
    (void) do_send(target,
        object_name, message_name, content, priority, auto_connect, false,
        outgoing_message_callback, NULL, keep_for_callback, &message_id);
        
    return message_id;
}

void agent_impl::clean_outgoing_message_callback(long long id)
{
    outgoing_manager_.remove(id);
}

void agent_impl::send_one_way(const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect)
{
    // do_send returns NULL pointer which can be ignored
    (void) do_send(target,
        object_name, message_name, content, priority, auto_connect, true,
        std::auto_ptr<outgoing_message_dispatcher_base>(NULL),
        NULL, none);
}

std::auto_ptr<outgoing_message> agent_impl::do_send(
    const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect,
    bool one_way,
    std::auto_ptr<outgoing_message_dispatcher_base>
        outgoing_message_callback,
    outgoing_message * message,
    message_creation_policy message_create,
    long long * out_message_id)
{
    core::parameters header;

    translate_result_to_exception(
        header.set_string("type", "message"));
    translate_result_to_exception(
        header.set_string("object_name", object_name.c_str()));
    translate_result_to_exception(
        header.set_string("message_name", message_name.c_str()));

    const long long message_id = id_gen_.get_next_id();
    translate_result_to_exception(
        header.set_long_long("message_id", message_id));

    bool wait_for_transmission = false;
    bool wait_for_reply_or_reject = false;

    std::auto_ptr<outgoing_message> out_msg;
    std::string last_exception;

    if (is_target_failover(target))
    {
        // the given target represents a group of separate destinations
        // that should be tried in random order until success

        std::vector<std::string> targets = split_failover_targets(target);
        if (targets.empty())
        {
            throw yami_logic_error("Empty failover group is not allowed.");
        }

        std::srand(std::time(NULL) + std::clock());
        std::random_shuffle(targets.begin(), targets.end());

        if (one_way)
        {
            // with failover the one-way messages are implicitly
            // forced to wait for transmission

            message_create = helper;
            wait_for_transmission = true;
        }
        else
        {
            // with failover the two-way messages are implicitly
            // forced to wait for completion

            wait_for_reply_or_reject = true;
        }

        // try failover targets one by one until success
        std::vector<std::string>::const_iterator it = targets.begin();
        const std::vector<std::string>::const_iterator end = targets.end();
        for ( ; it != end; ++it)
        {
            const std::string & single_target = *it;

            try
            {
                out_msg =
                    do_send_to_single_target(single_target,
                        header, content, priority, message_id,
                        auto_connect, one_way,
                        wait_for_transmission, wait_for_reply_or_reject,
                        outgoing_message_callback, message, message_create);

                // in the absence of exceptions
                // consider this message to be successfully sent

                last_exception.clear();
                break;
            }
            catch (const std::exception & e)
            {
                last_exception = e.what();
            }
            catch (...)
            {
                last_exception =
                    "Unknown exception while trying to send message.";
            }
        }
    }
    else
    {
        // this is a single-target message
        // do not force any waiting

        out_msg =
            do_send_to_single_target(target,
                header, content, priority, message_id,
                auto_connect, one_way,
                wait_for_transmission, wait_for_reply_or_reject,
                outgoing_message_callback, message, message_create);
    }

    if (last_exception.empty() == false)
    {
        throw yami_runtime_error(last_exception);
    }
    else
    {
        if (out_message_id != NULL)
        {
            *out_message_id = message_id;
        }
        
        return out_msg;
    }
}

std::auto_ptr<outgoing_message> agent_impl::do_send_to_single_target(
    const std::string & target,
    const core::parameters & header,
    const serializable & content,
    std::size_t priority,
    long long message_id,
    bool auto_connect,
    bool one_way,
    bool wait_for_transmission, bool wait_for_reply_or_reject,
    std::auto_ptr<outgoing_message_dispatcher_base>
        outgoing_message_callback,
    outgoing_message * message,
    message_creation_policy message_create)
{
    // flow control
    outgoing_flow_allowed_.wait();

    core::channel_descriptor cd =
        make_sure_channel_exists(target, auto_connect);

    core::message_progress_function progress_feedback;
    void * feedback_hint;
    outgoing_message_info * raw_info = NULL;

    if (one_way == false || wait_for_transmission || wait_for_reply_or_reject)
    {
        // this is either the two-way message or a failover message
        // with forced wait on transmission or reply or rejection

        // in order to avoid hazards, the internally managed objects need to
        // be prepared for the reception of replies/rejections/etc. before
        // the message is physically sent
        // - conditionally create the internal outgoing_message_info object
        //   before actually posting the message

        std::auto_ptr<outgoing_message_info> info(
            new outgoing_message_info());

        info->state = posted;
        info->message_id = message_id;
        info->sent_bytes = 0;
        info->total_byte_count = 0;
        info->reply_body = NULL;
        info->reply_raw_buffer = NULL;
        info->mtx.init();
        info->agent = this;
        info->ref_count = 0;
        info->out_msg = NULL;
        info->message_callback = outgoing_message_callback.get();

        raw_info = info.get();

        outgoing_manager_.put(message_id, raw_info);

        // at this point the manager references the info object
        // and takes responsibility for it, so the auto_ptr can be released

        if (info->ref_count != 1)
        {
            fatal_failure(__FILE__, __LINE__);
        }

        info.release();
        outgoing_message_callback.release();

        // the progress reporting mechanism is a second, logical referee
        // for the info object, so the refcounter can be artificially
        // increased to account for this

        ++(raw_info->ref_count);

        progress_feedback = &outgoing_message_progress_callback;
        feedback_hint = raw_info;
    }
    else
    {
        // purely one-way messages have a separate progress callback

        progress_feedback = &one_way_message_progress_callback;
        feedback_hint = this;
    }

    try
    {
        // flow control
        // to avoid hazards increase before posting
        increase_outgoing_flow();

        std::auto_ptr<outgoing_message> out_msg;

        if (raw_info != NULL)
        {
            // this is either two-way message or a one-way message
            // with forced feedback

            if (message_create == create_new || message_create == helper ||
                message_create == keep_for_callback)
            {
                // create the outgoing_message object for this message

                out_msg.reset(new outgoing_message());
                out_msg->reset(outgoing_manager_, *raw_info);

                if (message_create == keep_for_callback)
                {
                    // in the case of callback, the info object
                    // takes ownership of this outgoing message object

                    raw_info->out_msg = out_msg.release();
                    
                    // note, however, that the circular ownership
                    // dependency (info has message, message refers to info),
                    // has to be avoided

                    raw_info->out_msg->disown_info_object();
                }
            }
            else if (message_create == replace)
            {
                if (message == NULL)
                {
                    fatal_failure(__FILE__, __LINE__);
                }

                message->reset(outgoing_manager_, *raw_info);
            }
        }

        translate_result_to_exception(
            agent_.post(cd, header, content.get_core_object(), priority,
                progress_feedback, feedback_hint));

        if (wait_for_transmission)
        {
            out_msg->wait_for_transmission();
        }
        if (wait_for_reply_or_reject)
        {
            out_msg->wait_for_completion();
            if (out_msg->get_state() == abandoned)
            {
                throw yami_runtime_error("The message has been abandoned.");
            }
        }

        if (one_way)
        {
            if (out_msg.get() != NULL)
            {
                // the outgoing_message object has been artificially created
                // it is of no interest to the user,
                // so needs to be cleaned up

                out_msg.reset();
            }
        }

        return out_msg;
    }
    catch (...)
    {
        // the outgoing_message object released its reference
        // in the destructor (if it existed at all)

        // no progress will be ever reported for this message,
        // so the artificial reference from the progress reporting
        // mechanism can be released
        if (raw_info != NULL)
        {
            raw_info->dec_ref_count();

            // and there is no reason to keep the info in the manager
            // (the manager decrements the refcount on its behalf)
            outgoing_manager_.remove(message_id);

            // at this point the info object does not exist any longer
        }

        // flow control
        decrease_outgoing_flow();

        throw;
    }
}

void agent_impl::send_reply(const std::string & source,
    long long message_id, const serializable & body, std::size_t priority)
{
    core::parameters header;

    translate_result_to_exception(
        header.set_string("type", "reply"));
    translate_result_to_exception(
        header.set_long_long("message_id", message_id));

    core::channel_descriptor cd;
    bool created_new_channel;
    translate_result_to_exception(
        agent_.open(source.c_str(), cd, created_new_channel));

    if (created_new_channel)
    {
        report_connection_event(source.c_str(), new_outgoing_connection);
    }

    translate_result_to_exception(
        agent_.post(cd, header, body.get_core_object(), priority));
}

void agent_impl::send_rejection(const std::string & source,
    long long message_id, const std::string & reason, std::size_t priority)
{
    core::parameters header;

    translate_result_to_exception(
        header.set_string("type", "exception"));
    translate_result_to_exception(
        header.set_long_long("message_id", message_id));
    translate_result_to_exception(
        header.set_string("reason", reason.c_str()));

    core::parameters empty_body;

    core::channel_descriptor cd;
    bool created_new_channel;
    translate_result_to_exception(
        agent_.open(source.c_str(), cd, created_new_channel));

    if (created_new_channel)
    {
        report_connection_event(source.c_str(), new_outgoing_connection);
    }

    translate_result_to_exception(
        agent_.post(cd, header, empty_body, priority));
}

core::channel_descriptor agent_impl::make_sure_channel_exists(
    const std::string & target, bool auto_connect,
    const parameters * overriding_options)
{
    core::channel_descriptor cd;
    core::result res = core::io_error;

    const core::parameters * core_overriding_options;

    if (overriding_options != NULL)
    {
        core_overriding_options = &(overriding_options->get_core_object());
    }
    else
    {
        core_overriding_options = NULL;
    }

    if (auto_connect)
    {
        // smart retry when connection does not exist or failed is implemented
        // by bounded retry with randomized sleep between retries to
        // spread out initialization bursts in bigger systems

        const std::size_t connection_retries = options_.connection_retries;
        const std::size_t connection_retry_delay_spread =
            options_.connection_retry_delay_spread;

        for (std::size_t i = 0;
             i != connection_retries && res != core::ok; ++i)
        {
            bool created_new_channel;
            res = agent_.open(target.c_str(), cd, created_new_channel,
                core_overriding_options);

            if (res == core::ok)
            {
                if (created_new_channel)
                {
                    report_connection_event(target.c_str(),
                        new_outgoing_connection);
                }
            }
            else
            {
                // the connection was not successful - try again

                if (i < connection_retries - 1 &&
                    connection_retry_delay_spread != 0)
                {
                    // the distribution does not matter
                    const unsigned long long pause =
                        std::rand() % connection_retry_delay_spread;

                    delay(pause);
                }
            }
        }
    }
    else
    {
        // do not attempt any reconnection
        res = agent_.is_open(target.c_str(), cd);
    }

    if (res != core::ok)
    {
        if (res == core::no_such_name)
        {
            // the given channel does not exist, treat it as a run-time error
            translate_result_to_exception(core::io_error);
        }
        else
        {
            translate_result_to_exception(res);
        }
    }

    return cd;
}

void agent_impl::close_connection(
    const std::string & target, std::size_t priority)
{
    translate_result_to_exception(agent_.close(target.c_str(), priority));
}

void agent_impl::register_object(
    const std::string & object_name,
    std::auto_ptr<incoming_message_dispatcher_base> object)
{
    resolver_.register_object(object_name, object);

    if (event_listener_ != NULL)
    {
        try
        {
            event_listener_->object_registered(object_name.c_str());
        }
        catch (...)
        {
            // ignore errors from user callback
        }
    }
}

void agent_impl::unregister_object(const std::string & object_name)
{
    resolver_.unregister_object(object_name);

    if (event_listener_ != NULL)
    {
        try
        {
            event_listener_->object_unregistered(object_name.c_str());
        }
        catch (...)
        {
            // ignore errors from user callback
        }
    }
}

void agent_impl::register_connection_event_monitor(
    std::auto_ptr<connection_event_dispatcher_base> monitor)
{
    connection_event_monitor_ = monitor;
}

void agent_impl::register_io_error_logger(
    std::auto_ptr<io_error_dispatcher_base> logger)
{
    io_error_logger_ = logger;

    agent_.install_io_error_logger(
        io_error_callback, io_error_logger_.get());
}

void agent_impl::report_connection_event(
    const char * name, connection_event event)
{
    if (connection_event_monitor_.get() != NULL)
    {
        connection_event_monitor_->dispatch(name, event);
    }
}

void agent_impl::queue_incoming(std::auto_ptr<incoming_message_info> incoming)
{
    // flow control
    // increase before posting to avoid hazards
    increase_incoming_flow();

    try
    {
        incoming_queue_.push(incoming);
    }
    catch (...)
    {
        decrease_incoming_flow();
        throw;
    }
}

void agent_impl::report_replied(
    long long message_id, std::auto_ptr<parameters> body)
{
    outgoing_manager_.report_replied(message_id, body);
}

void agent_impl::report_replied(
    long long message_id, std::auto_ptr<std::vector<char> > raw_buffer)
{
    outgoing_manager_.report_replied(message_id, raw_buffer);
}

void agent_impl::report_rejected(
    long long message_id, const std::string & reason)
{
    outgoing_manager_.report_rejected(message_id, reason);
}

void agent_impl::do_message_dispatching(std::size_t dispatcher_index)
{
    bool finished = false;
    while (finished == false)
    {
        std::auto_ptr<incoming_message_info> im_info(incoming_queue_.pop());

        if (im_info.get() == NULL)
        {
            finished = true;
        }
        else
        {
            incoming_message im(*this, *im_info);

            // find appropriate dispatcher object and call it

            const std::string source_name = im.get_source();
            const std::string & object_name = im.get_object_name();
            const long long message_id = im_info->message_id;

            incoming_message_dispatcher_base * dispatcher =
                resolver_.resolve(object_name);
            if (dispatcher != NULL)
            {
                try
                {
                    // if the user-provided callback throws any exceptions:
                    // if the message_id is known
                    // (which means that it was not a one-way message),
                    // send back the rejection message,
                    // otherwise ignore all errors

                    dispatcher->dispatch(im);
                }
                catch (const std::exception & e)
                {
                    if (message_id != 0)
                    {
                        try
                        {
                            send_rejection(source_name, message_id,
                                std::string(
                                    "Exception at the receiver side: ") +
                                e.what(), 0);
                        }
                        catch (...)
                        {
                            // ignore all errors here
                        }
                    }
                }
                catch (...)
                {
                    if (message_id != 0)
                    {
                        try
                        {
                            send_rejection(source_name, message_id,
                                "Unknown exception at the receiver side.", 0);
                        }
                        catch (...)
                        {
                            // ignore all errors here
                        }
                    }
                }
            }
            else
            {
                // no appropriate dispatcher was found

                if (message_id != 0)
                {
                    try
                    {
                        send_rejection(source_name, message_id,
                            "Unknown destination object.", 0);
                    }
                    catch (...)
                    {
                        // ignore all errors here
                    }
                }
            }

            // flow control
            decrease_incoming_flow();
        }
    }

    dispatcher_stopped_flags_[dispatcher_index].notify();
}

void agent_impl::do_event_loop()
{
    // the timeout is arbitrary here, but needs to be within limits
    const std::size_t timeout = 10000;

    bool finished = false;
    while (finished == false)
    {
        // flow control:
        // the outgoing traffic is controlled at the level of
        // send/send_one_way functions - that is, even before the message
        // reaches the transport layer and so the output at the transport
        // layer is always enabled
        // the incoming traffic is controlled at the transport level,
        // depending on the length of the incoming message queue

        const bool allow_outgoing_traffic = true;
        bool allow_incoming_traffic;
        {
            mutex_lock lock(allow_incoming_traffic_mtx_);

            allow_incoming_traffic = allow_incoming_traffic_;
        }

        (void) agent_.do_some_work(timeout,
            allow_outgoing_traffic, allow_incoming_traffic);

        {
            mutex_lock lock(worker_stop_mtx_);

            if (worker_stop_request_)
            {
                finished = true;
            }
        }
    }

    worker_stopped_.notify();
}

void agent_impl::increase_outgoing_flow()
{
    const water_flow_control what_to_do =
        outgoing_flow_manager_.increase();
    if (what_to_do == suppress)
    {
        // the outgoing queue is full, do not allow any more messages
        outgoing_flow_allowed_.suppress();
    }
    else
    {
        if (what_to_do != no_change)
        {
            fatal_failure(__FILE__, __LINE__);
        }
    }
}

void agent_impl::decrease_outgoing_flow()
{
    const water_flow_control what_to_do =
        outgoing_flow_manager_.decrease();
    if (what_to_do == allow)
    {
        // the outgoing queue is below low-water mark,
        // allow more messages
        outgoing_flow_allowed_.notify();
    }
    else
    {
        if (what_to_do != no_change)
        {
            fatal_failure(__FILE__, __LINE__);
        }
    }
}

void agent_impl::get_channel_usage(int & max_allowed, int & used)
{
    agent_.get_channel_usage(max_allowed, used);
}

void agent_impl::increase_incoming_flow()
{
    const water_flow_control what_to_do =
        incoming_flow_manager_.increase();
    if (what_to_do == suppress)
    {
        // the incoming queue is full, do not allow any more messages

        mutex_lock lock(allow_incoming_traffic_mtx_);

        allow_incoming_traffic_ = false;
    }
    else
    {
        if (what_to_do != no_change)
        {
            fatal_failure(__FILE__, __LINE__);
        }
    }
}

void agent_impl::decrease_incoming_flow()
{
    const water_flow_control what_to_do =
        incoming_flow_manager_.decrease();
    if (what_to_do == allow)
    {
        // the incoming queue is below low-water mark,
        // allow more messages

        {
            mutex_lock lock(allow_incoming_traffic_mtx_);

            allow_incoming_traffic_ = true;
        }

        (void) agent_.interrupt_work_waiter();
    }
    else
    {
        if (what_to_do != no_change)
        {
            fatal_failure(__FILE__, __LINE__);
        }
    }
}

void agent_impl::get_outgoing_flow_state(std::size_t & current_level,
    std::size_t & high_water_mark, std::size_t & low_water_mark) const
{
    outgoing_flow_manager_.get_state(
        current_level, high_water_mark, low_water_mark);
}
