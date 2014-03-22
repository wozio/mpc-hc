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

#include "channel_group.h"
#include "allocator.h"
#include "channel.h"
#include "channel_holder.h"
#include "listener.h"
#include "options.h"
#include "outgoing_frame.h"
#include "parameters.h"
#include "serialization.h"
#include <cstring>

using namespace yami;
using namespace details;

namespace // unnamed namespace
{

const std::size_t frame_header_size = 16; // four words

// helper for serialization, deallocates (deeply) the array of frames
void deallocate_array_of_frames(allocator & alloc,
    char * * buffers, std::size_t * buffer_sizes,
    std::size_t number_of_frames)
{
    if (buffers != NULL)
    {
        for (std::size_t i = 0; i != number_of_frames; ++i)
        {
            if (buffers[i] != NULL)
            {
                alloc.deallocate(buffers[i]);
            }
        }

        alloc.deallocate(buffers);
    }

    if (buffer_sizes != NULL)
    {
        alloc.deallocate(buffer_sizes);
    }
}

// helper for serialization, prepares the array of outgoing frames
// with properly allocated data segments
core::result prepare_array_of_frames(allocator & alloc,
    std::size_t message_id,
    std::size_t header_total_buffer_size,
    std::size_t body_total_buffer_size,
    std::size_t preferred_frame_size,
    std::size_t & index_of_border_frame,
    std::size_t & body_offset_in_border_frame,
    std::size_t & size_of_last_frame,
    std::size_t & number_of_frames,
    char * * & buffers, std::size_t * & buffer_sizes)
{
    core::result res;

    const std::size_t frame_payload_size =
        preferred_frame_size - frame_header_size;

    const std::size_t total_buffer_size =
        header_total_buffer_size + body_total_buffer_size;

    // this is the index of frame where the first byte of body data is located
    index_of_border_frame = header_total_buffer_size / frame_payload_size;
    
    body_offset_in_border_frame =
        header_total_buffer_size % frame_payload_size + frame_header_size;

    number_of_frames =
        (total_buffer_size + frame_payload_size - 1) / frame_payload_size;

    size_of_last_frame =
        (total_buffer_size - 1) % frame_payload_size + 1 + frame_header_size;

    // allocate the array of buffers

    buffers = static_cast<char * *>(
        alloc.allocate(number_of_frames * sizeof(char *)));
    if (buffers != NULL)
    {
        // clear the pointers
        for (std::size_t i = 0; i != number_of_frames; ++i)
        {
            buffers[i] = NULL;
        }
    }

    buffer_sizes = static_cast<std::size_t *>(
        alloc.allocate(number_of_frames * sizeof(std::size_t)));
    if (buffers != NULL && buffer_sizes != NULL)
    {

        // allocate the data segment for each outgoing frame

        res = core::ok;
        for (std::size_t i = 0; res == core::ok && i != number_of_frames; ++i)
        {
            std::size_t size_of_this_frame;

            if (i != number_of_frames - 1)
            {
                // not the last frame, ask for full preferred size

                size_of_this_frame = preferred_frame_size;
            }
            else
            {
                // last frame, possibly shorter

                size_of_this_frame = size_of_last_frame;
            }

            buffer_sizes[i] = size_of_this_frame;
            buffers[i] = static_cast<char *>(
                alloc.allocate(size_of_this_frame));
            if (buffers[i] != NULL)
            {
                // fill the frame header with proper information

                // frame number start with 1
                int frame_number = i + 1;

                if (i == number_of_frames - 1)
                {
                    // this is the last frame in the whole message
                    // use negative frame number to mark this

                    frame_number *= -1;
                }

                fill_outgoing_frame_header(buffers[i],
                    message_id,
                    frame_number,
                    header_total_buffer_size,
                    size_of_this_frame - frame_header_size); // frame payload
            }
            else
            {
                // cannot allocate data segment

                res = core::no_memory;
            }
        }
    }
    else
    {
        res = core::no_memory;
    }

    return res;
}

// helper for preparing array of buffer pointers and buffer sizes
// that will be used for serialization
// they are offset to take into account space for frame headers
core::result prepare_array_of_serialize_buffer_pointers(
    allocator & alloc,
    char * * buffers, std::size_t * buffer_sizes,
    std::size_t number_of_frames,
    char * * & serialize_buffers, std::size_t * & serialize_buffer_sizes)
{
    core::result res = core::ok;

    serialize_buffers = static_cast<char * *>(
        alloc.allocate(number_of_frames * sizeof(char *)));
    if (serialize_buffers != NULL)
    {
        for (std::size_t i = 0; i != number_of_frames; ++i)
        {
            // the serialization buffer starts right after the frame header
            serialize_buffers[i] = buffers[i] + frame_header_size;
        }
    }
    else
    {
        res = core::no_memory;
    }

    if (res == core::ok)
    {
        serialize_buffer_sizes = static_cast<std::size_t *>(
            alloc.allocate(number_of_frames * sizeof(std::size_t)));
        if (serialize_buffer_sizes != NULL)
        {
            for (std::size_t i = 0; i != number_of_frames; ++i)
            {
                serialize_buffer_sizes[i] =
                    buffer_sizes[i] - frame_header_size;
            }
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

// helper function for serializing the new message and injecting the
// output frames into the output queue of the given channel
core::result serialize_and_post(allocator & alloc,
    channel & ch, std::size_t message_id, std::size_t priority,
    const core::serializable & message_header,
    const core::serializable & message_body,
    bool & first_frame,
    core::message_progress_function progress_callback,
    void * progress_hint)
{
    // note:
    // There are two arrays in use:
    // 1. the array of pointers to buffers that will be used as complete
    //    frames and will be injected to the outgoing queue
    //    of the given channel - this array owns the buffers
    // 2. the array of pointers to buffers that are used for serialization
    //    these pointers are offset with regard to those in array 1. to
    //    take into account the necessary space for frame headers
    //    this array does not own the actual buffers

    core::result res;

    const std::size_t preferred_frame_size = ch.get_frame_size();

    std::size_t header_total_buffer_size;
    std::size_t body_total_buffer_size;

    res = message_header.get_serialize_buffer_size(header_total_buffer_size);
    if (res == core::ok)
    {
        res = message_body.get_serialize_buffer_size(body_total_buffer_size);
    }

    // prepare the array of buffers for all outgoing frames

    std::size_t number_of_frames;
    std::size_t index_of_border_frame;
    std::size_t body_offset_in_border_frame;
    std::size_t size_of_last_frame;
    char * * buffers = NULL;
    std::size_t * buffer_sizes = NULL;
    if (res == core::ok)
    {
        res = prepare_array_of_frames(alloc, message_id,
            header_total_buffer_size, body_total_buffer_size,
            preferred_frame_size,
            index_of_border_frame, body_offset_in_border_frame,
            size_of_last_frame, number_of_frames,
            buffers, buffer_sizes);
    }

    // prepare the array of pointers for serialization

    char * * serialize_buffers = NULL;
    std::size_t * serialize_buffer_sizes = NULL;
    if (res == core::ok)
    {
        res = prepare_array_of_serialize_buffer_pointers(alloc,
            buffers, buffer_sizes, number_of_frames,
            serialize_buffers, serialize_buffer_sizes);
    }

    // serialize the message header

    if (res == core::ok)
    {
        // note: it is allowed to provide more buffers than are actually
        // needed for serialization

        res = message_header.serialize(
            serialize_buffers, serialize_buffer_sizes, number_of_frames);
    }

    // serialize the message body

    if (res == core::ok)
    {
        // the buffer pointer that will be used as the first one
        // for serializing the message body needs to be ammended
        // in order to take into account the tail of message header data

        serialize_buffers[index_of_border_frame] =
            buffers[index_of_border_frame] + body_offset_in_border_frame;
        serialize_buffer_sizes[index_of_border_frame] =
            buffer_sizes[index_of_border_frame] - body_offset_in_border_frame;

        res = message_body.serialize(
            serialize_buffers + index_of_border_frame,
            serialize_buffer_sizes + index_of_border_frame,
            number_of_frames - index_of_border_frame);
    }
    
    // post new outgoing frames to the given channel

    if (res == core::ok)
    {
        res = ch.post(priority,
            buffers, buffer_sizes, number_of_frames,
            first_frame,
            progress_callback, progress_hint);
    }

    // clean all helper buffers

    if (serialize_buffers != NULL)
    {
        alloc.deallocate(serialize_buffers);
    }

    if (serialize_buffer_sizes != NULL)
    {
        alloc.deallocate(serialize_buffer_sizes);
    }

    if (res == core::ok)
    {
        // actual buffers were transferred to the channel,
        // do not deallocate them
        alloc.deallocate(buffers);
        alloc.deallocate(buffer_sizes);
    }
    else
    {
        // buffers were not transferred or there was some other error
        // -> deallocate deeply the whole structure
        deallocate_array_of_frames(alloc,
            buffers, buffer_sizes, number_of_frames);
    }

    return res;
}

} // unnamed namespace

core::result channel_group::init(allocator & alloc,
    const core::parameters * configuration_options,
    core::incoming_message_dispatch_function dispatch_callback,
    void * dispatch_hint,
    core::closed_connection_function disconnection_hook,
    void * disconnection_hook_hint)
{
    channel_holders_ = NULL;
    channels_num_ = 0;
    shadow_channels_ = NULL;
    shadow_channels_num_ = 0;
    first_listener_ = NULL;
    alloc_ = &alloc;
    closing_ = false;
    last_message_id_ = 0;
    incoming_message_callback_ = dispatch_callback;
    incoming_message_hint_ = dispatch_hint;
    disconnection_hook_ = disconnection_hook;
    disconnection_hook_hint_ = disconnection_hook_hint;
    event_notification_callback_ = NULL;
    io_error_callback_ = NULL;

    configuration_options_.init(configuration_options);
    selector_mtx_.init();
    mtx_.init();

    const core::result res = selector_.init();

    return res;
}

void channel_group::install_event_notifications(
    core::event_notification_function event_notification_callback,
    void * event_notification_hint)
{
    event_notification_callback_ = event_notification_callback;
    event_notification_hint_ = event_notification_hint;
}

void channel_group::install_io_error_logger(
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    io_error_callback_ = io_error_callback;
    io_error_callback_hint_ = io_error_callback_hint;

    selector_.install_io_error_logger(
        io_error_callback, io_error_callback_hint);
}

void channel_group::clean(bool uses_private_area)
{
    // note: if uses_private_area is false, then the memory needs to be
    // deallocated, because it was acquired from the global free store
    // otherwise (uses_private_area == true) all structures were
    // created in the private storage and need not be deallocated

    // in any case, all files and sockets need to be physically closed

    closing_ = true;

    mtx_.lock();

    // clean the channels
    for (std::size_t i = 0; i != channels_num_; ++i)
    {
        channel_holder & ch_holder = channel_holders_[i];

        channel * ch = ch_holder.get_channel();
        if (ch != NULL)
        {
            const char * target = ch->move_target();

            if (uses_private_area)
            {
                // it is enough to close the physical connection
                ch->close_connection();
            }
            else
            {
                // force full cleanup of the channel
                ch->clean();
                alloc_->deallocate(ch);
            }

            if (event_notification_callback_ != NULL)
            {
                try
                {
                    event_notification_callback_(
                        event_notification_hint_,
                        core::connection_closed,
                        target, 0);
                }
                catch (...)
                {
                    // ignore errors from user callback
                }
            }

            if (disconnection_hook_ != NULL)
            {
                mtx_.unlock();

                try
                {
                    disconnection_hook_(disconnection_hook_hint_,
                        target, core::channel_closed);
                }
                catch (...)
                {
                    // ignore errors from user callback
                }

                mtx_.lock();
            }

            if (uses_private_area == false)
            {
                alloc_->deallocate(target);
            }
        }
    }

    if (uses_private_area == false)
    {
        alloc_->deallocate(channel_holders_);
        alloc_->deallocate(shadow_channels_);
    }

    // clean the list of listeners
    while (first_listener_ != NULL)
    {
        listener * next = first_listener_->next;

        if (event_notification_callback_ != NULL)
        {
            try
            {
                event_notification_callback_(
                    event_notification_hint_,
                    core::listener_removed,
                    first_listener_->get_target(), 0);
            }
            catch (...)
            {
                // ignore errors from user callback
            }
        }

        if (uses_private_area)
        {
            // it is enough to close the listening socket
            first_listener_->close_socket();
        }
        else
        {
            // force full cleanup
            first_listener_->clean();
            alloc_->deallocate(first_listener_);
        }

        first_listener_ = next;
    }

    mtx_.unlock();

    // clean the selector (it has internal pipe that needs to be closed)

    selector_.clean();
    mtx_.clean();
    selector_mtx_.clean();

    if (event_notification_callback_ != NULL)
    {
        try
        {
            event_notification_callback_(
                event_notification_hint_,
                core::agent_closed,
                NULL, 0);
        }
        catch (...)
        {
            // ignore errors from user callback
        }
    }
}

core::result channel_group::open(const char * target,
    core::channel_descriptor & new_channel,
    bool & created_new_channel, bool do_lock,
    const core::parameters * overriding_options)
{
    // note:
    // This function can be called by multiple threads.
    // In order to allow for higher concurrency it can lead to
    // multiple connections being created for the same target,
    // although only one such connection is ultimately kept open.

    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        std::size_t index;
        std::size_t sequence_number;
        channel * existing_channel;

        if (do_lock)
        {
            mtx_.lock();
        }

        // try to find the existing channel with the same target
        res = find_existing_channel(
            target, index, sequence_number, existing_channel);

        if (do_lock)
        {
            mtx_.unlock();
        }

        if (res == core::ok)
        {
            new_channel = core::channel_descriptor(index, sequence_number);
            created_new_channel = false;
        }
        else
        {
            // no channel with this target exists,
            // initialize a new channel in some empty slot
            
            channel * ch = static_cast<channel *>(
                alloc_->allocate(sizeof(channel)));

            if (ch != NULL)
            {
                // initialize channel outside of the critical section
                // - this can lead to multiple channels being open

                res = ch->init(*alloc_, mtx_,
                    configuration_options_, overriding_options,
                    target,
                    incoming_message_callback_, incoming_message_hint_,
                    event_notification_callback_, event_notification_hint_,
                    io_error_callback_, io_error_callback_hint_);


                if (res == core::ok)
                {
                    bool channel_added = false;

                    if (do_lock)
                    {
                        mtx_.lock();
                    }

                    // re-check to verify that the channel
                    // is not yet in the set
                    res = find_existing_channel(
                        target, index, sequence_number, existing_channel);

                    if (res == core::ok)
                    {
                        // the channel with this name is in the set
                        // (probably just added by another thread)

                        // return the descriptor for that existing channel

                        new_channel = core::channel_descriptor(
                            index, sequence_number);
                        created_new_channel = false;
                    }
                    else
                    {
                        res = find_unused_channel(index, true);
                        if (res == core::ok)
                        {
                            channel_holders_[index].set(ch, sequence_number);
                            new_channel = core::channel_descriptor(
                                index, sequence_number);

                            created_new_channel = true;
                            channel_added = true;

                            if (event_notification_callback_ != NULL)
                            {
                                try
                                {
                                    event_notification_callback_(
                                        event_notification_hint_,
                                        core::outgoing_connection_open,
                                        target, 0);
                                }
                                catch (...)
                                {
                                    // ignore errors from user callback
                                }
                            }
                        }
                    }

                    if (do_lock)
                    {
                        mtx_.unlock();
                    }

                    if (channel_added)
                    {
                        // notify the selector so that it can
                        // take the new channel into account
                        
                        res = selector_.interrupt();
                    }
                    else
                    {
                        // channel was initialized,
                        // but not included in the set

                        ch->clean();
                        alloc_->deallocate(ch);
                    }
                }
                else
                {
                    // channel was allocated but not initialized properly

                    alloc_->deallocate(ch);
                }
            }
            else
            {
                res = core::no_memory;
            }
        }
    }

    return res;
}

core::result channel_group::is_open(const char * target,
    core::channel_descriptor & existing_channel) const
{
    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        std::size_t index;
        std::size_t sequence_number;
        channel * dummy_ch;

        mtx_.lock();

        res = find_existing_channel(
            target, index, sequence_number, dummy_ch);

        if (res == core::ok)
        {
            existing_channel =
                core::channel_descriptor(index, sequence_number);
        }

        mtx_.unlock();
    }

    return res;
}

// synchronized by caller
core::result channel_group::add_existing(
    char * target, io_descriptor_type fd, protocol prot,
    std::size_t preferred_frame_size,
    core::channel_descriptor & new_descriptor)
{
    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        // make sure that no channel for this target already exists

        std::size_t index;
        std::size_t sequence_number;
        channel * dummy_ch;

        // try to find the existing channel with the same target
        // (it should not exist)
        res = find_existing_channel(target,
            index, sequence_number, dummy_ch);

        if (res == core::ok)
        {
            // reject the new channel, there is some I/O hazard
            // (it is not normal for the listener to accept a channel
            // for the target that already exists)
            res = core::io_error;
        }
        else
        {
            // initialize a new channel in some empty slot

            res = find_unused_channel(index, false);
            if (res == core::ok)
            {
                channel * ch = static_cast<channel *>(
                    alloc_->allocate(sizeof(channel)));
                if (ch != NULL)
                {
                    ch->init(*alloc_, mtx_, configuration_options_,
                        target, fd, prot, preferred_frame_size,
                        incoming_message_callback_, incoming_message_hint_,
                        event_notification_callback_,
                        event_notification_hint_,
                        io_error_callback_, io_error_callback_hint_);

                    channel_holders_[index].set(ch, sequence_number);

                    new_descriptor =
                        core::channel_descriptor(index, sequence_number);

                    if (event_notification_callback_ != NULL)
                    {
                        try
                        {
                            event_notification_callback_(
                                event_notification_hint_,
                                core::incoming_connection_open,
                                target, 0);
                        }
                        catch (...)
                        {
                            // ignore errors from user callback
                        }
                    }
                }
                else
                {
                    res = core::no_memory;
                }
            }
        }
    }

    return res;
}

core::result channel_group::post(
    core::channel_descriptor cd,
    const core::serializable & message_header,
    const core::serializable & message_body,
    std::size_t priority,
    core::message_progress_function progress_callback,
    void * progress_hint)
{
    core::result res;
    bool first_frame;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        res = core::no_such_index;

        std::size_t index;
        std::size_t sequence_number;

        cd.get_details(index, sequence_number);

        mtx_.lock();

        if (index < channels_num_)
        {
            channel_holder & ch_holder = channel_holders_[index];
            channel * ch = ch_holder.get_channel();

            if (ch != NULL)
            {
                if (ch_holder.get_sequence_number() == sequence_number)
                {
                    const std::size_t message_id = generate_message_id();

                    res = serialize_and_post(*alloc_,
                        *ch, message_id, priority,
                        message_header, message_body,
                        first_frame,
                        progress_callback, progress_hint);
                }
            }
        }

        mtx_.unlock();
    }

    if (res == core::ok && first_frame)
    {
        // interrupt selector so that it can perform an output operation
        res = selector_.interrupt();
    }

    return res;
}

core::result channel_group::post(
    const char * target,
    const core::serializable & message_header,
    const core::serializable & message_body,
    std::size_t priority,
    core::message_progress_function progress_callback,
    void * progress_hint)
{
    core::result res;
    bool first_frame;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        std::size_t index;
        std::size_t dummy_sequence_number;
        channel * ch;

        mtx_.lock();

        // try to find the existing channel with the same target
        res = find_existing_channel(
            target, index, dummy_sequence_number, ch);

        if (res == core::ok)
        {
            const std::size_t message_id = generate_message_id();

            res = serialize_and_post(*alloc_,
                *ch, message_id, priority, message_header, message_body,
                first_frame,
                progress_callback, progress_hint);
        }

        mtx_.unlock();
    }

    if (res == core::ok && first_frame)
    {
        // interrupt selector so that it can perform an output operation
        res = selector_.interrupt();
    }

    return res;
}

core::result channel_group::close(
    core::channel_descriptor cd, std::size_t priority)
{
    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        std::size_t index;
        std::size_t sequence_number;

        cd.get_details(index, sequence_number);

        // do not report errors if channel does not exist

        mtx_.lock();

        res = core::ok;
        if (index < channels_num_)
        {
            channel_holder & ch_holder = channel_holders_[index];
            channel * ch = ch_holder.get_channel();

            if (ch != NULL)
            {
                if (ch_holder.get_sequence_number() == sequence_number)
                {
                    res = do_close(ch, priority, index);
                }
            }
        }

        mtx_.unlock();
    }

    return res;
}

core::result channel_group::close(const char * target, std::size_t priority)
{
    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        std::size_t index;
        std::size_t dummy_sequence_number;
        channel * ch;

        mtx_.lock();

        // try to find the existing channel with the same target
        res = find_existing_channel(target,
            index, dummy_sequence_number, ch);

        if (res == core::ok)
        {
            res = do_close(ch, priority, index);
        }
        else if (res == core::no_such_name)
        {
            // do not report errors if channel does not exist
            res = core::ok;
        }

        mtx_.unlock();
    }

    return res;
}

// synchronized by caller
core::result channel_group::do_close(
    channel * ch, std::size_t priority, std::size_t index)
{
    bool close_me = false;
    core::result res = ch->post_close(priority, close_me);

    if (res == core::ok && close_me)
    {
        // immediate close

        bool destroyed = channel_dec_ref(index, ch);
        if (destroyed == false)
        {
            // channel was not destroyed,
            // because there are more references to it
            // -> wake up the worker, so it can finalize the process

            res = interrupt_work_waiter();
        }
    }

    return res;
}

core::result channel_group::add_listener(const char * target,
    core::new_incoming_connection_function connection_hook,
    void * connection_hook_hint,
    const char * * resolved_target)
{
    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        listener * new_listener = static_cast<listener *>(
            alloc_->allocate(sizeof(listener)));

        // preparation of the listener is potentially blocking
        // and is performed with mutex unlocked

        if (new_listener != NULL)
        {
            new_listener->init(*alloc_, *this, mtx_,
                connection_hook, connection_hook_hint,
                io_error_callback_, io_error_callback_hint_);

            res = new_listener->prepare(target);
            if (res == core::ok)
            {
                if (resolved_target != NULL)
                {
                    *resolved_target = new_listener->get_target();
                }

                // add new listener to the list

                mtx_.lock();

                new_listener->next = first_listener_;
                first_listener_ = new_listener;

                mtx_.unlock();

                if (event_notification_callback_ != NULL)
                {
                    try
                    {
                        event_notification_callback_(
                            event_notification_hint_,
                            core::listener_added,
                            *resolved_target, 0);
                    }
                    catch (...)
                    {
                        // ignore errors from user callback
                    }
                }

                // notify the selector so that it can
                // take the new listener into account

                res = selector_.interrupt();
            }
            else
            {
                alloc_->deallocate(new_listener);
            }
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

void channel_group::remove_listener(const char * target)
{
    if (closing_ == false)
    {
        mtx_.lock();

        listener * lst = first_listener_;
        bool found = false;
        while (lst != NULL && found == false)
        {
            if (std::strcmp(lst->get_target(), target) == 0)
            {
                found = true;
            }
            else
            {
                lst = lst->next;
            }
        }

        if (found)
        {
            lst->dec_ref();
            prune_listeners();
        }

        mtx_.unlock();
    }
}

core::result channel_group::do_some_work(std::size_t timeout,
    bool allow_outgoing_traffic, bool allow_incoming_traffic)
{
    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        // note: listeners can change the set of channels
        // (they can add new channels to the collection),
        // and the work done by channels themselves can change the collection
        // as well (due to user activity in callbacks)
        // to avoid conflicts the list of channel pointers is first
        // copied to the shadow array and used for checking with the selector
        // so that the primary collection changes do not affect this process

        // it is possible that any given channel or listener will have
        // its ref counter decremented in the meantime - this is checked
        // after the whole selector operation is finished

        selector_mtx_.lock();
        mtx_.lock();

        // make sure the shadow channel array has proper size
        res = core::ok;
        if (shadow_channels_num_ != channels_num_)
        {
            if (shadow_channels_ != NULL)
            {
                alloc_->deallocate(shadow_channels_);
            }

            shadow_channels_ = static_cast<channel * *>(
                alloc_->allocate(sizeof(channel *) * channels_num_));
            if (shadow_channels_ != NULL)
            {
                shadow_channels_num_ = channels_num_;
            }
            else
            {
                res = core::no_memory;
            }
        }

        if (res == core::ok)
        {
            selector_.reset();

            // freeze the shadow channel array and add channels to selector
            for (std::size_t i = 0; i != channels_num_; ++i)
            {
                channel * ch = channel_holders_[i].get_channel();
                if (ch != NULL)
                {
                    ch->inc_ref();
                    selector_.add_channel(*ch,
                        allow_outgoing_traffic, allow_incoming_traffic);
                }

                shadow_channels_[i] = ch;
            }

            // add listeners to selector

            listener * lst = first_listener_;
            while (lst != NULL)
            {
                selector_.add_listener(*lst);

                lst->inc_ref();

                lst = lst->next;
            }

            // at this point all channels and listeners are "pinned"
            // with increased ref counters

            mtx_.unlock();

            // wait for the work
            // (the main mutex is unlocked during the waiting phase)

            res = selector_.wait(timeout);

            mtx_.lock();

            for (std::size_t i = 0;
                 res == core::ok && i != shadow_channels_num_; ++i)
            {
                channel * ch = shadow_channels_[i];

                if (ch != NULL)
                {
                    io_direction direction;
                    if (selector_.is_channel_ready(*ch, direction))
                    {
                        bool close_me = false;
                        res = ch->do_some_work(direction, close_me);

                        if (close_me)
                        {
                            // the channel should be closed due to error, EOF
                            // or regular close request in the outgoing queue

                            if (res == core::channel_closed)
                            {
                                // the EOF condition is not an error
                                // from the point of view of higher layers
                                // (ie. from the point of view of what
                                // do_some_work returns)

                                // note: the channel_closed value was
                                // used for the callback above to propagate
                                // proper reason to the user code

                                res = core::ok;
                            }
                            else
                            {
                                // error condition

                                if (event_notification_callback_ != NULL)
                                {
                                    try
                                    {
                                        event_notification_callback_(
                                            event_notification_hint_,
                                            core::connection_error,
                                            ch->get_target(), 0);
                                    }
                                    catch (...)
                                    {
                                        // ignore errors from user callback
                                    }
                                }
                            }

                            channel_dec_ref(i, ch);
                        }
                    }
                }
            }

            lst = first_listener_;
            while (res == core::ok && lst != NULL)
            {
                if (selector_.is_listener_ready(*lst))
                {
                    lst->inc_ref();

                    res = lst->do_some_work();

                    lst->dec_ref();
                }

                lst = lst->next;
            }

            // scan the list of listeners and remove those that are
            // no longer referenced by any thread

            prune_listeners();

            // unref the channels from the shadow array
            for (std::size_t i = 0; i != shadow_channels_num_; ++i)
            {
                channel * ch = shadow_channels_[i];
                if (ch != NULL)
                {
                    channel_dec_ref(i, ch);
                }
            }

            mtx_.unlock();
        }
        else
        {
            mtx_.unlock();
        }

        selector_mtx_.unlock();
    }

    return res;
}

// synchronized by caller
core::result channel_group::process_complete_incoming_frame(
    core::channel_descriptor cd,
    const char * buffer, const std::size_t buffer_size)
{
    core::result res;
    if (closing_)
    {
        res = core::bad_state;
    }
    else
    {
        res = core::no_such_index;

        std::size_t index;
        std::size_t sequence_number;

        cd.get_details(index, sequence_number);

        if (index < channels_num_)
        {
            channel_holder & ch_holder = channel_holders_[index];
            channel * ch = ch_holder.get_channel();

            if (ch != NULL)
            {
                if (ch_holder.get_sequence_number() == sequence_number)
                {
                    res = ch->process_complete_incoming_frame(
                        buffer, buffer_size);
                }
            }
        }
    }

    return res;
}

core::result channel_group::interrupt_work_waiter()
{
    const core::result res = selector_.interrupt();
    return res;
}

void channel_group::get_channel_usage(int & max_allowed, int & used)
{
    selector_.get_channel_usage(max_allowed, used);
}

// synchronized by caller
core::result channel_group::find_existing_channel(const char * target,
    std::size_t & index, std::size_t & sequence_number, channel * & ch) const
{
    core::result res = core::no_such_name;

    for (std::size_t i = 0; i != channels_num_; ++i)
    {
        const channel_holder & ch_holder = channel_holders_[i];
        channel * chi = ch_holder.get_channel();

        if (chi != NULL)
        {
            if (std::strcmp(chi->get_target(), target) == 0)
            {
                index = i;
                sequence_number = ch_holder.get_sequence_number();
                ch = chi;
                res = core::ok;
                break;
            }
        }
    }

    return res;
}

// synchronized by caller
core::result channel_group::find_unused_channel(
    std::size_t & index, bool reserve)
{
    bool found = false;
    for (std::size_t i = 0; found == false && i != channels_num_; ++i)
    {
        channel_holder & ch_holder = channel_holders_[i];
        const channel * ch = ch_holder.get_channel();

        if (ch_holder.is_reserved() == false && ch == NULL)
        {
            index = i;
            found = true;

            if (reserve)
            {
                ch_holder.reserve();
            }
        }
    }

    core::result res;
    if (found)
    {
        res = core::ok;
    }
    else
    {
        const std::size_t initial_block_size = 10;

        std::size_t new_channels_num;
        if (channels_num_ < initial_block_size)
        {
            new_channels_num = initial_block_size;
        }
        else
        {
            new_channels_num = 2 * channels_num_;
        }

        channel_holder * new_channel_holders = static_cast<channel_holder *>(
            alloc_->allocate(sizeof(channel_holder) * new_channels_num));
        if (new_channel_holders != NULL)
        {
            std::memcpy(new_channel_holders, channel_holders_,
                sizeof(channel_holder) * channels_num_);

            for (std::size_t i = channels_num_; i != new_channels_num; ++i)
            {
                channel_holder * ch_holder = new_channel_holders + i;
                ch_holder->init();
            }

            // pick the first slot after those that already existed
            index = channels_num_;
            if (reserve)
            {
                new_channel_holders[index].reserve();
            }

            if (channel_holders_ != NULL)
            {
                alloc_->deallocate(channel_holders_);
            }

            channel_holders_ = new_channel_holders;
            channels_num_ = new_channels_num;

            res = core::ok;
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

// synchronized by caller
bool channel_group::channel_dec_ref(std::size_t index, channel * ch)
{
    bool destroyed = false;

    ch->dec_ref();
    if (ch->can_be_removed())
    {
        channel_holders_[index].clean();
        const char * target = ch->move_target();

        // the user callback can be invoked from this,
        // but the state of all structures is safe at this point

        ch->clean();
        alloc_->deallocate(ch);

        if (event_notification_callback_ != NULL)
        {
            try
            {
                event_notification_callback_(
                    event_notification_hint_,
                    core::connection_closed,
                    target, 0);
            }
            catch (...)
            {
                // ignore errors from user callback
            }
        }

        if (disconnection_hook_ != NULL)
        {
            // user callback is performed without the lock
            mtx_.unlock();

            try
            {
                disconnection_hook_(disconnection_hook_hint_,
                    target, core::channel_closed);
            }
            catch (...)
            {
                // ignore errors from user callback
            }

            mtx_.lock();
        }

        alloc_->deallocate(target);

        destroyed = true;
    }

    return destroyed;
}

// synchronized by caller
void channel_group::prune_listeners()
{
    listener * * removal_point = &first_listener_;
    while (*removal_point != NULL)
    {
        listener * lst = *removal_point;
        if (lst->can_be_removed())
        {
            if (event_notification_callback_ != NULL)
            {
                try
                {
                    event_notification_callback_(
                        event_notification_hint_,
                        core::listener_removed,
                        lst->get_target(), 0);
                }
                catch (...)
                {
                    // ignore errors from user callback
                }
            }
            
            *removal_point = lst->next;
            lst->clean();
            alloc_->deallocate(lst);
        }
        else
        {
            removal_point = &(lst->next);
        }
    }
}

// synchronized by caller
std::size_t channel_group::generate_message_id()
{
    return ++last_message_id_;
}
