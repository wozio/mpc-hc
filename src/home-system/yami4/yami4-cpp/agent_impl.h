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

#ifndef YAMICPP_AGENT_IMPL_H_INCLUDED
#define YAMICPP_AGENT_IMPL_H_INCLUDED

#include "agent_impl_base.h"
#include "id_generator.h"
#include "incoming_message_info.h"
#include "incoming_message_queue.h"
#include "name_resolver.h"
#include "options.h"
#include "outgoing_message.h"
#include "outgoing_message_manager.h"
#include "water_flow_manager.h"
#include <yami4-core/agent.h>
#include <map>
#include <string>
#include <vector>

// selected per platform
#include <mutex.h>
#include <flag.h>

namespace yami
{

class event_callback;
class parameters;
class serializable;

namespace details
{

class incoming_message_dispatcher_base;

class agent_impl : public agent_impl_base
{
public:

    agent_impl(const parameters & options);

    agent_impl(const parameters & options, event_callback & event_listener);

    ~agent_impl();

    std::string add_listener(const std::string & listener);
    void remove_listener(const std::string & listener);

    void open_connection(const std::string & target);
    void open_connection(const std::string & target,
        const parameters & options);

    std::auto_ptr<outgoing_message> send(
        const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content,
        std::size_t priority,
        bool auto_connect);

    void send(
        outgoing_message & message,
        const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content,
        std::size_t priority,
        bool auto_connect);

    void clean_outgoing_message_callback(long long id);
    
    void send_one_way(const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content,
        std::size_t priority,
        bool auto_connect);

    void send_reply(const std::string & source,
        long long message_id, const serializable & body,
        std::size_t priority);

    void send_rejection(const std::string & source,
        long long message_id, const std::string & reason,
        std::size_t priority);

    void close_connection(const std::string & target, std::size_t priority);

    virtual void register_object(
        const std::string & object_name,
        std::auto_ptr<incoming_message_dispatcher_base> object);

    void unregister_object(const std::string & object_name);

    virtual long long send(
        std::auto_ptr<outgoing_message_dispatcher_base>
            outgoing_message_callback,
        const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content,
        std::size_t priority,
        bool auto_connect);
        
    virtual void register_connection_event_monitor(
        std::auto_ptr<connection_event_dispatcher_base> monitor);

    virtual void register_io_error_logger(
        std::auto_ptr<io_error_dispatcher_base> logger);

    void report_connection_event(
        const char * name, connection_event event);

    // executed as a callback when the incoming message arrives
    void queue_incoming(std::auto_ptr<incoming_message_info> incoming);

    // executed as a callback when the reply arrives
    void report_replied(
        long long message_id, std::auto_ptr<parameters> body);
    void report_replied(
        long long message_id, std::auto_ptr<std::vector<char> > raw_buffer);

    // executed as a callback when the rejection/exception arrives
    void report_rejected(long long message_id, const std::string & reason);

    // executed from separate thread, dispatches queued incoming messages
    void do_message_dispatching(std::size_t dispatcher_index);

    // executed from dedicated worker thread, runs the I/O event loop
    void do_event_loop();

    void get_outgoing_flow_state(std::size_t & current_level,
        std::size_t & high_water_mark, std::size_t & low_water_mark) const;

    void decrease_outgoing_flow();

    void get_channel_usage(int & max_allowed, int & used);
    
    cpp_options options_;

private:
    agent_impl(const agent_impl &);
    void operator=(const agent_impl &);

    void init(const parameters & options);
    void clean();

    enum message_creation_policy
    {
        none,
        create_new,       // new out_msg object is created dynamically

        replace,          // user-provided object is reinitialized in-place

        helper,           // a helper outgoing_message is created to handle
                          // one-way interaction with forced wait

        keep_for_callback // out_msg object is created, stored for callback
                          // and destroyed after the callback is delivered
    };

    std::auto_ptr<outgoing_message> do_send(
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
        long long * out_message_id = NULL);

    std::auto_ptr<outgoing_message> do_send_to_single_target(
        const std::string & target,
        const core::parameters & header,
        const serializable & content,
        std::size_t priority,
        long long message_id,
        bool auto_connect,
        bool one_way, bool wait_for_transmission, bool wait_for_completion,
        std::auto_ptr<outgoing_message_dispatcher_base>
            outgoing_message_callback,
        outgoing_message * message,
        message_creation_policy message_create);

    core::channel_descriptor make_sure_channel_exists(
        const std::string & target, bool auto_connect,
        const parameters * overriding_options = NULL);

    void increase_outgoing_flow();
    void increase_incoming_flow();
    void decrease_incoming_flow();

    void stop_worker_thread();
    void stop_dispatcher_threads(std::size_t num_of_threads);

    core::agent agent_;

    outgoing_message_manager outgoing_manager_;

    water_flow_manager outgoing_flow_manager_;
    flag outgoing_flow_allowed_;

    incoming_message_queue incoming_queue_;
    flag * dispatcher_stopped_flags_; // array

    water_flow_manager incoming_flow_manager_;
    bool allow_incoming_traffic_;
    mutex allow_incoming_traffic_mtx_;

    std::auto_ptr<connection_event_dispatcher_base> connection_event_monitor_;
    std::auto_ptr<io_error_dispatcher_base> io_error_logger_;
    event_callback * event_listener_;

    name_resolver resolver_;

    id_generator id_gen_;

    bool worker_stop_request_;
    mutex worker_stop_mtx_;
    flag worker_stopped_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_AGENT_IMPL_H_INCLUDED
