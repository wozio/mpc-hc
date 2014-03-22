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

#include "agent.h"
#include "channel_group.h"

using namespace yami;
using namespace core;

agent::agent()
    : initialized_(false), alloc_(), uses_private_area_(false),
      ch_group_(NULL)
{
}

result agent::init(
    incoming_message_dispatch_function dispatch_callback,
    void * dispatch_hint,
    closed_connection_function disconnection_hook,
    void * disconnection_hook_hint,
    void * working_area, std::size_t area_size)
{
    result res;
    if (initialized_)
    {
        res = bad_state;
    }
    else
    {
        res = do_init(NULL,
            dispatch_callback, dispatch_hint,
            disconnection_hook, disconnection_hook_hint,
            working_area, area_size);
    }

    return res;
}

result agent::init(const parameters & configuration_options,
    incoming_message_dispatch_function dispatch_callback,
    void * dispatch_hint,
    closed_connection_function disconnection_hook,
    void * disconnection_hook_hint,
    void * working_area, std::size_t area_size)
{
    result res;
    if (initialized_)
    {
        res = bad_state;
    }
    else
    {
        res = do_init(&configuration_options,
            dispatch_callback, dispatch_hint,
            disconnection_hook, disconnection_hook_hint,
            working_area, area_size);
    }

    return res;
}

result agent::do_init(const parameters * configuration_options,
    incoming_message_dispatch_function dispatch_callback,
    void * dispatch_hint,
    closed_connection_function disconnection_hook,
    void * disconnection_hook_hint,
    void * working_area, std::size_t area_size)
{
    alloc_.set_working_area(working_area, area_size);
    uses_private_area_ = area_size > 0;

    // allocate the implementation object
    ch_group_ = static_cast<details::channel_group *>(
        alloc_.allocate(sizeof(details::channel_group)));

    core::result res;
    if (ch_group_ != NULL)
    {
        res = ch_group_->init(alloc_, configuration_options,
            dispatch_callback, dispatch_hint,
            disconnection_hook, disconnection_hook_hint);

        if (res == core::ok)
        {
            initialized_ = true;
        }
        else
        {
            alloc_.deallocate(ch_group_);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

void agent::install_event_notifications(
    event_notification_function event_notification_callback,
    void * event_notification_hint)
{
    ch_group_->install_event_notifications(
        event_notification_callback, event_notification_hint);
}

void agent::install_io_error_logger(
    io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    ch_group_->install_io_error_logger(
        io_error_callback, io_error_callback_hint);
}

void agent::clean()
{
    if (initialized_)
    {
        ch_group_->clean(uses_private_area_);

        if (uses_private_area_ == false)
        {
            // this object uses global dynamic memory

            alloc_.deallocate(ch_group_);
        }

        initialized_ = false;
    }
}

agent::~agent()
{
    clean();
}

result agent::open(const char * target)
{
    channel_descriptor dummy_descriptor;
    bool dummy_created_new_channel;
    return open(target, dummy_descriptor, dummy_created_new_channel);
}

result agent::open(const char * target, channel_descriptor & new_channel,
    bool & created_new_channel)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->open(target, new_channel, created_new_channel);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::open(const char * target, channel_descriptor & new_channel,
    bool & created_new_channel, const parameters * overriding_options)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->open(target, new_channel, created_new_channel,
            true, overriding_options);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::is_open(const char * target,
    channel_descriptor & existing_channel) const
{
    result res;
    if (initialized_)
    {
        res = ch_group_->is_open(target, existing_channel);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::close(channel_descriptor cd, std::size_t priority)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->close(cd, priority);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::close(const char * target, std::size_t priority)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->close(target, priority);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::post(channel_descriptor cd,
    const serializable & message_header,
    const serializable & message_body,
    std::size_t priority,
    message_progress_function progress_callback,
    void * progress_hint)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->post(cd, message_header, message_body, priority,
            progress_callback, progress_hint);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::post(const char * target,
    const serializable & message_header,
    const serializable & message_body,
    std::size_t priority,
    message_progress_function progress_callback,
    void * progress_hint)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->post(target, message_header, message_body, priority,
            progress_callback, progress_hint);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::add_listener(const char * target,
    new_incoming_connection_function connection_hook,
    void * connection_hook_hint,
    const char * * resolved_target)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->add_listener(target,
            connection_hook, connection_hook_hint,
            resolved_target);
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::remove_listener(const char * target)
{
    result res;
    if (initialized_)
    {
        ch_group_->remove_listener(target);
        res = ok;
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::do_some_work(std::size_t timeout,
    bool allow_outgoing_traffic, bool allow_incoming_traffic)
{
    result res;
    if (initialized_)
    {
        res = ch_group_->do_some_work(timeout,
            allow_outgoing_traffic, allow_incoming_traffic);

        if (res == ok)
        {
            // only now the selector has up-to-date usage values
            // -> store them aside for future requests
            
            ch_group_->get_channel_usage(
                max_channels_allowed_, channels_used_);
        }
    }
    else
    {
        res = bad_state;
    }

    return res;
}

result agent::interrupt_work_waiter()
{
    result res;
    if (initialized_)
    {
        res = ch_group_->interrupt_work_waiter();
    }
    else
    {
        res = bad_state;
    }

    return res;
}

void agent::get_channel_usage(int & max_allowed, int & used)
{
    max_allowed = max_channels_allowed_;
    used = channels_used_;
}

