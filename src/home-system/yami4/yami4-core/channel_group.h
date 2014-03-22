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

#ifndef YAMICORE_CHANNEL_GROUP_H_INCLUDED
#define YAMICORE_CHANNEL_GROUP_H_INCLUDED

#include "channel_descriptor.h"
#include "core.h"
#include "details-fwd.h"
#include "options.h"

// selected per platform
#include <details-types.h>
#include <mutex.h>
#include <selector.h>

namespace yami
{

namespace core
{

class parameters;
class serializable;

} //  namespace core

namespace details
{

class channel_group
{
public:

    core::result init(allocator & alloc,
        const core::parameters * configuration_options,
        core::incoming_message_dispatch_function dispatch_callback,
        void * dispatch_hint,
        core::closed_connection_function disconnection_hook,
        void * disconnection_hook_hint);

    void install_event_notifications(
        core::event_notification_function event_notification_callback,
        void * event_notification_hint);

    void install_io_error_logger(
        core::io_error_function io_error_callback,
        void * io_error_callback_hint);

    void clean(bool uses_private_area);

    // opens new channel for the given target
    // or returns the descriptor of the existing channel
    core::result open(const char * target,
        core::channel_descriptor & new_channel,
        bool & created_new_channel, bool do_lock = true,
        const core::parameters * overriding_options = NULL);

    core::result is_open(const char * target,
        core::channel_descriptor & existing_channel) const;

    // adds an existing channel to the list, used to add new channels
    // created by listeners; target's ownership is transfered
    core::result add_existing(
        char * target, io_descriptor_type fd, protocol prot,
        std::size_t preferred_frame_size,
        core::channel_descriptor & new_descriptor);

    // insert a poison pill to the given channel
    // do nothing if no such channel exists
    core::result close(core::channel_descriptor cd, std::size_t priority);
    core::result close(const char * target, std::size_t priority);

    // serializes the new message into frames and injects them
    // into the output queue of the given channel
    core::result post(core::channel_descriptor cd,
        const core::serializable & message_header,
        const core::serializable & message_body,
        std::size_t priority,
        core::message_progress_function progress_callback,
        void * progress_hint);
    core::result post(const char * target,
        const core::serializable & message_header,
        const core::serializable & message_body,
        std::size_t priority,
        core::message_progress_function progress_callback,
        void * progress_hint);

    core::result add_listener(const char * target,
        core::new_incoming_connection_function connection_hook,
        void * connection_hook_hint,
        const char * * resolved_target);
    void remove_listener(const char * target);

    // waits for the readiness of any dependent channel
    // to perform I/O work and executing a unit of work
    // (the unit of work can involve many channels, if more than one
    // is ready for transmission; it can also return without
    // physically doing any I/O-related work)
    // the timeout is in milliseconds,
    // 0 means immediate check and action (non-blocking)
    core::result do_some_work(std::size_t timeout,
        bool allow_outgoing_traffic,
        bool allow_incoming_traffic);

    // used to inject already received frames (UDP),
    // should be called from synchronized context
    core::result process_complete_incoming_frame(core::channel_descriptor cd,
        const char * buffer, const std::size_t buffer_size);

    core::result interrupt_work_waiter();

    const options & get_options() const { return configuration_options_; }

    void get_channel_usage(int & max_allowed, int & used);
    
private:

    channel_group(const channel_group &);
    void operator=(const channel_group &);

    core::result do_close(channel * ch, std::size_t priority,
        std::size_t index);

    // finds existing channel with the given target name
    // returns ok or no_such_name
    core::result find_existing_channel(const char * target,
        std::size_t & index, std::size_t & sequence_number,
        channel * & ch) const;

    // finds an empty slot or makes room for new empty slot
    // reservation means that the empty slot will not be found
    // the second time until it is filled and cleared for reuse
    // returns ok or no_memory
    core::result find_unused_channel(std::size_t & index, bool reserve);

    bool channel_dec_ref(std::size_t index, channel * ch);

    void prune_listeners();

    std::size_t generate_message_id();

    mutable mutex mtx_;
    mutex selector_mtx_; // protects the selector and the shadow channel array

    options configuration_options_;

    channel_holder * channel_holders_;
    std::size_t channels_num_;
    channel * * shadow_channels_;
    std::size_t shadow_channels_num_;

    listener * first_listener_;

    selector selector_;

    allocator * alloc_;

    bool closing_;

    std::size_t last_message_id_;

    core::incoming_message_dispatch_function incoming_message_callback_;
    void * incoming_message_hint_;
    core::closed_connection_function disconnection_hook_;
    void * disconnection_hook_hint_;

    core::event_notification_function event_notification_callback_;
    void * event_notification_hint_;

    core::io_error_function io_error_callback_;
    void * io_error_callback_hint_;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_CHANNEL_GROUP_H_INCLUDED
