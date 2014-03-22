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

#include "channel.h"
#include "allocator.h"
#include "fatal_errors.h"
#include "incoming_frame.h"
#include "incoming_message_frame_list.h"
#include "outgoing_frame.h"
#include "serialization.h"
#include <cstring>

// selected per platform
#include <mutex.h>

using namespace yami;
using namespace details;

void channel::common_init(allocator & alloc, mutex & mtx,
    const options & configuration_options,
    const core::parameters * overriding_options,
    core::incoming_message_dispatch_function incoming_message_callback,
    void * incoming_message_hint,
    core::event_notification_function event_notification_callback,
    void * event_notification_hint,
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    configuration_options_ = configuration_options;
    configuration_options_.override(overriding_options);

    alloc_ = &alloc;
    mtx_ = &mtx;
    target_ = NULL;
    ref_count_ = 1;
    mode_ = operational;
    direction_ = input;
    fd_ = empty_io_descriptor;
    target_address_ = NULL;
    datagram_buffer_ = NULL;
    output_frame_offset_ = 0;
    first_outgoing_frame_ = NULL;
    last_outgoing_frame_ = NULL;
    incoming_state_ = read_frame_header;
    read_offset_ = 0;
    current_message_id_ = 0;
    current_frame_number_ = 0;
    current_message_header_size_ = 0;
    current_frame_payload_size_ = 0;
    current_frame_payload_ = NULL;
    last_frame_ = false;
    first_incoming_frame_list_ = NULL;
    last_incoming_frame_list_ = NULL;
    incoming_message_callback_ = incoming_message_callback;
    incoming_message_hint_ = incoming_message_hint;
    event_notification_callback_ = event_notification_callback;
    event_notification_hint_ = event_notification_hint;
    io_error_callback_ = io_error_callback;
    io_error_callback_hint_ = io_error_callback_hint;
}

// to be synchronized by caller
void channel::clean()
{
    close_connection();

    if (target_ != NULL)
    {
        alloc_->deallocate(target_);
    }

    if (protocol_ == proto_udp)
    {
        if (target_address_ != NULL)
        {
            alloc_->deallocate(target_address_);
        }

        if (datagram_buffer_ != NULL)
        {
            alloc_->deallocate(datagram_buffer_);
        }
    }

    clean_outgoing_frames(first_outgoing_frame_);
    clean_incoming_messages();
}

// should be called outside of critical section
core::result channel::init(allocator & alloc, mutex & mtx,
    const options & configuration_options,
    const core::parameters * overriding_options,
    const char * target,
    core::incoming_message_dispatch_function incoming_message_callback,
    void * incoming_message_hint,
    core::event_notification_function event_notification_callback,
    void * event_notification_hint,
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    common_init(alloc, mtx, configuration_options, overriding_options,
        incoming_message_callback, incoming_message_hint,
        event_notification_callback, event_notification_hint,
        io_error_callback, io_error_callback_hint);

    const std::size_t target_length = std::strlen(target);

    target_ = static_cast<char *>(alloc_->allocate(target_length + 1));

    core::result res;
    if (target_ != NULL)
    {
        std::strcpy(target_, target);

        res = connect();

        if (res != core::ok)
        {
            alloc_->deallocate(target_);
            target_ = NULL;
        }
    }
    else
    {
        res = core::no_memory;
    }

    return res;
}

void channel::init(allocator & alloc, mutex & mtx,
    const options & configuration_options,
    char * target,
    io_descriptor_type fd, protocol prot, std::size_t preferred_frame_size,
    core::incoming_message_dispatch_function incoming_message_callback,
    void * incoming_message_hint,
    core::event_notification_function event_notification_callback,
    void * event_notification_hint,
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    common_init(alloc, mtx, configuration_options, NULL,
        incoming_message_callback, incoming_message_hint,
        event_notification_callback, event_notification_hint,
        io_error_callback, io_error_callback_hint);

    // ownership transfer
    target_ = target;

    alloc_ = &alloc;
    fd_ = fd;
    protocol_ = prot;
    direction_ = inout;
    preferred_frame_size_ = preferred_frame_size;
}

core::result channel::connect()
{
    core::result res;

    const char tcp_prefix[] = "tcp://";
    const std::size_t tcp_prefix_size = sizeof(tcp_prefix) - 1;

    const char udp_prefix[] = "udp://";
    const std::size_t udp_prefix_size = sizeof(udp_prefix) - 1;

    const char unix_prefix[] = "unix://";
    const std::size_t unix_prefix_size = sizeof(unix_prefix) - 1;

    const char file_prefix[] = "file://";
    const std::size_t file_prefix_size = sizeof(file_prefix) - 1;

    if (std::strncmp(tcp_prefix, target_, tcp_prefix_size) == 0)
    {
        const char * tcp_address = target_ + tcp_prefix_size;
        res = connect_to_tcp(tcp_address);
    }
    else if (std::strncmp(udp_prefix, target_, udp_prefix_size) == 0)
    {
        const char * udp_address = target_ + udp_prefix_size;
        res = connect_to_udp(udp_address);
    }
    else if (std::strncmp(unix_prefix, target_, unix_prefix_size) == 0)
    {
        const char * path = target_ + unix_prefix_size;
        res = connect_to_unix(path);
    }
    else if (std::strncmp(file_prefix, target_, file_prefix_size) == 0)
    {
        const char * file_name = target_ + file_prefix_size;
        res = connect_to_file(file_name);
    }
    else
    {
        res = core::bad_protocol;
    }

    return res;
}

std::size_t channel::get_frame_size() const
{
    return preferred_frame_size_;
}

core::result channel::post(std::size_t priority,
    char * * buffers, const std::size_t * buffer_sizes,
    std::size_t number_of_frames,
    bool & first_frame,
    core::message_progress_function progress_callback,
    void * progress_hint)
{
    core::result res;

    if (mode_ != operational)
    {
        res = core::bad_state;
    }
    else
    {
        // this function inserts new set of nodes to the outgoing queue
        // based on the frame descriptions given in parameters

        first_frame = first_outgoing_frame_ == NULL;

        std::size_t total_byte_count = 0;
        for (std::size_t i = 0; i != number_of_frames; ++i)
        {
            total_byte_count += buffer_sizes[i];
        }

        // prepare the sublist of new nodes

        res = core::ok;
        std::size_t byte_count = 0;
        outgoing_frame * first_new_frame = NULL;
        outgoing_frame * last_new_frame = NULL;
        for (std::size_t i = 0; i != number_of_frames; ++i)
        {
            outgoing_frame * frame = static_cast<outgoing_frame *>(
                alloc_->allocate(sizeof(outgoing_frame)));

            if (frame != NULL)
            {
                if (first_new_frame == NULL)
                {
                    first_new_frame = frame;
                }

                byte_count += buffer_sizes[i];

                frame->data = buffers[i];
                frame->size = buffer_sizes[i];
                frame->progress_callback = progress_callback;
                frame->progress_hint = progress_hint;
                frame->byte_count = byte_count;
                frame->total_byte_count = total_byte_count;
                frame->priority = priority;
                frame->close_flag = false;
                frame->next = NULL;

                if (last_new_frame != NULL)
                {
                    last_new_frame->next = frame;
                }

                last_new_frame = frame;
            }
            else
            {
                // rollback what was already allocated
                while (first_new_frame != NULL)
                {
                    outgoing_frame * next = first_new_frame->next;
                    alloc_->deallocate(first_new_frame);
                    first_new_frame = next;
                }

                res = core::no_memory;
                break;
            }
        }

        if (res == core::ok)
        {
            outgoing_frame * * insertion_point =
                find_outgoing_insertion_point(priority);

            // continuation of the list after newly added nodes
            outgoing_frame * following_frame = *insertion_point;

            // insert new nodes

            last_new_frame->next = following_frame;
            *insertion_point = first_new_frame;

            if (following_frame == NULL)
            {
                // the nodes were added at the end of the list
                last_outgoing_frame_ = last_new_frame;
            }
        }
    }

    return res;
}

core::result channel::do_some_work(io_direction direction, bool & close_me)
{
    core::result res = core::ok;

    if (mode_ == operational && (direction == input || direction == inout))
    {
        res = do_some_input();

        if (res == core::io_error ||
            res == core::channel_closed ||
            res == core::unexpected_value)
        {
            mode_ = hard_close;
            close_me = true;
        }
    }

    // output is allowed even in the soft closing state
    // (pending messages need to be pushed out)
    if (res == core::ok &&
        (mode_ == operational || mode_ == soft_close) &&
        (direction == output || direction == inout))
    {
        res = do_some_output(close_me);
    }

    return res;
}

core::result channel::post_close(std::size_t priority, bool & close_me)
{
    core::result res;

    if (mode_ != operational)
    {
        // second close on the same channel is not allowed
        res = core::bad_state;
    }
    else
    {
        if (first_outgoing_frame_ == NULL)
        {
            // the list is empty, immediately close the connection

            mode_ = hard_close;
            close_me = true;
            res = core::ok;
        }
        else
        {
            outgoing_frame * poison_pill = static_cast<outgoing_frame *>(
                alloc_->allocate(sizeof(outgoing_frame)));

            if (poison_pill != NULL)
            {
                poison_pill->data = NULL;
                poison_pill->size = 0;
                poison_pill->close_flag = true;
                poison_pill->next = NULL;

                outgoing_frame * * insertion_point =
                    find_outgoing_insertion_point(priority);

                // continuation of the list after poison pill
                outgoing_frame * following_frame = *insertion_point;

                // insert the poison pill
                // and clean the remaining part of the list

                *insertion_point = poison_pill;
                last_outgoing_frame_ = poison_pill;

                mode_ = soft_close;

                // at this point the list is terminated
                // just after the poison pill,
                // which means that the following part of the list
                // is not visible to the outside observers
                // - it is safe to execute user callbacks in this state

                clean_outgoing_frames(following_frame);

                res = core::ok;
            }
            else
            {
                res = core::no_memory;
            }
        }

        clean_incoming_messages();
    }

    return res;
}

const char * channel::move_target()
{
    // destructive move to the caller,
    // intended to call just before clean if the target information
    // is still needed

    const char * res = target_;
    target_ = NULL;
    return res;
}

void channel::get_io_descriptor(
    io_descriptor_type & fd, io_direction & direction) const
{
    fd = fd_;

    // if there are no outgoing frames, then the channel should not
    // advertise itself as being interested in doing output operation
    // it can still be interested in doing input

    switch (direction_)
    {
    case none:
        // the channel should never be in this state
        fatal_failure(__FILE__, __LINE__);
        break;
    case input:
        direction = input;
        break;
    case output:
        if (first_outgoing_frame_ != NULL)
        {
            direction = output;
        }
        else
        {
            direction = none;
        }
        break;
    case inout:
        if (first_outgoing_frame_ != NULL)
        {
            direction = inout;
        }
        else
        {
            direction = input;
        }
        break;
    }
}

outgoing_frame * * channel::find_outgoing_insertion_point(
    std::size_t priority)
{
    // find the proper place for new node(s), according to priorities
    // the first frame is left in place independent of priorities

    outgoing_frame * * insertion_point;
    if (first_outgoing_frame_ == NULL)
    {
        // list is empty
        insertion_point = &first_outgoing_frame_;
    }
    else
    {
        // the list contains at least one frame

        // check for the most common scenario,
        // when the priority of the new node(s) is not higher
        // than the priorities of existing nodes

        if (priority <= last_outgoing_frame_->priority)
        {
            // append at the end
            insertion_point = &(last_outgoing_frame_->next);
        }
        else
        {
            // find the insertion point according to priorities
            // but leave the first frame intact

            insertion_point = &(first_outgoing_frame_->next);

            while (*insertion_point != NULL &&
                (*insertion_point)->priority >= priority)
            {
                insertion_point = &((*insertion_point)->next);
            }
        }
    }

    return insertion_point;
}

void channel::clean_outgoing_frames(outgoing_frame * frame)
{
    // this function is also responsible for notifying all
    // abandoned messages about their cancellation
    // the notification (callback to the user code) is always safe,
    // becuase this function is called in a state when the outgoing
    // frames starting from frame are not visible to outside observers

    while (frame != NULL)
    {
        if (frame->close_flag == false &&
            (frame->byte_count == frame->total_byte_count))
        {
            // this is the last outgoing frame for the given message

            core::message_progress_function progress_callback =
                frame->progress_callback;
            void * progress_hint = frame->progress_hint;

            // notify the message that it was cancelled

            if (progress_callback != NULL)
            {
                notify_cancellation(progress_callback, progress_hint);
            }
        }

        const char * data = frame->data;
        if (data != NULL)
        {
            alloc_->deallocate(data);
        }

        outgoing_frame * next = frame->next;
        alloc_->deallocate(frame);
        frame = next;
    }
}

void channel::notify_cancellation(
    core::message_progress_function progress_callback,
    void * progress_hint)
{
    mtx_->unlock();

    try
    {
        // zero sizes in callback indicate cancellation
        progress_callback(progress_hint, 0, 0);
    }
    catch (...)
    {
        // ignore exceptions from user callbacks
    }

    // revert the locked state
    mtx_->lock();
}

void channel::parse_incoming_frame_header()
{
    // note: the buffer has exactly 4 words,
    // there is no possibility for error here

    const std::size_t num_of_buffers = 1;
    const char * buffers[num_of_buffers];
    std::size_t sizes[num_of_buffers];

    buffers[0] = frame_head_buffer_;
    sizes[0] = frame_head_size;

    std::size_t current_buffer = 0;
    const char * buffer_position = buffers[0];

    int value;
    core::result res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, value);
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    current_message_id_ = value;

    res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, value);
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    if (value >= 0)
    {
        last_frame_ = false;
        current_frame_number_ = value;
    }
    else
    {
        last_frame_ = true;
        current_frame_number_ = -value;
    }

    res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, value);
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    current_message_header_size_ = static_cast<std::size_t>(value);

    res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, value);
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    current_frame_payload_size_ = static_cast<std::size_t>(value);
}

core::result channel::store_incoming_frame()
{
    core::result res;

    incoming_message_frame_list * list;

    incoming_frame * new_frame = static_cast<incoming_frame *>(
        alloc_->allocate(sizeof(incoming_frame)));
    if (new_frame != NULL)
    {
        new_frame->frame_number = current_frame_number_;
        new_frame->data = current_frame_payload_;
        new_frame->size = current_frame_payload_size_;

        current_frame_payload_ = NULL;

        if (first_incoming_frame_list_ == NULL)
        {
            // no incoming frame list yet,
            // create the new list with single frame

            list = static_cast<incoming_message_frame_list *>(
                alloc_->allocate(sizeof(incoming_message_frame_list)));
            if (list != NULL)
            {
                list->message_id = current_message_id_;
                list->header_size = current_message_header_size_;
                list->check_completeness = last_frame_;
                list->next_list = NULL;

                new_frame->next = NULL;
                list->first_frame = new_frame;
                list->last_frame = new_frame;

                first_incoming_frame_list_ = list;
                last_incoming_frame_list_ = list;

                res = core::ok;
            }
            else
            {
                alloc_->deallocate(new_frame->data);
                alloc_->deallocate(new_frame);

                res = core::no_memory;
            }
        }
        else
        {
            // some lists already exist - find the list for the same message

            list = first_incoming_frame_list_;
            while (list != NULL && list->message_id != current_message_id_)
            {
                list = list->next_list;
            }

            if (list != NULL)
            {
                // found appropriate list,
                // insert the new (current) frame in the
                // correct location, according to its frame number
                // note: each list has at least one frame already in it,
                // there are no empty lists

                // the optimistic and most frequent case is
                // when the current frame
                // should be appended to the end of the list

                if (current_frame_number_ >= list->last_frame->frame_number)
                {
                    new_frame->next = NULL;
                    list->last_frame->next = new_frame;
                    list->last_frame = new_frame;

                    if (last_frame_)
                    {
                        list->check_completeness = true;
                    }
                }
                else
                {
                    // insert new frame in between existing frames

                    incoming_frame * * insertion_point = &(list->first_frame);

                    while ((*insertion_point)->frame_number <=
                        current_frame_number_)
                    {
                        insertion_point = &((*insertion_point)->next);
                    }

                    new_frame->next = *insertion_point;
                    *insertion_point = new_frame;

                    // use the message header size from first frame
                    // (values from other frames are ignored)
                    if (current_frame_number_ == 1)
                    {
                        list->header_size = current_message_header_size_;
                    }
                }

                res = core::ok;
            }
            else
            {
                // no appropriate list found - this is the first frame
                // of the message that was not yet seen

                // create new list with this single frame
                // and append it after all existing lists

                list = static_cast<incoming_message_frame_list *>(
                    alloc_->allocate(sizeof(incoming_message_frame_list)));
                if (list != NULL)
                {
                    list->message_id = current_message_id_;
                    list->header_size = current_message_header_size_;
                    list->check_completeness = last_frame_;
                    list->next_list = NULL;

                    new_frame->next = NULL;
                    list->first_frame = new_frame;
                    list->last_frame = new_frame;

                    last_incoming_frame_list_->next_list = list;
                    last_incoming_frame_list_ = list;

                    res = core::ok;
                }
                else
                {
                    alloc_->deallocate(new_frame->data);
                    alloc_->deallocate(new_frame);

                    res = core::no_memory;
                }
            }
        }
    }
    else
    {
        alloc_->deallocate(current_frame_payload_);
        current_frame_payload_ = NULL;

        res = core::no_memory;
    }

    if (res == core::ok && list->check_completeness)
    {
        // this message already contains its closing frame
        // -> check its completeness and dispatch

        res = dispatch_incoming_message(list);
    }

    return res;
}

core::result channel::dispatch_short_incoming_message()
{
    // the whole message (header and body) is contained in the single frame

    core::result res;

    if ((current_message_header_size_ >= current_frame_payload_size_) ||
        (current_message_header_size_ % word_size != 0) ||
        (current_frame_payload_size_ % word_size != 0))
    {
        alloc_->deallocate(current_frame_payload_);
        current_frame_payload_ = NULL;

        res = core::unexpected_value;
    }
    else
    {
        if (event_notification_callback_ != NULL)
        {
            try
            {
                event_notification_callback_(
                    event_notification_hint_,
                    core::message_received,
                    target_,
                    current_frame_payload_size_ + frame_head_size);
            }
            catch (...)
            {
                // ignore errors from user callback
            }
        }

        if (incoming_message_callback_ != NULL)
        {
            const char * header_buffers[1];
            header_buffers[0] = current_frame_payload_;

            std::size_t header_buffer_sizes[1];
            header_buffer_sizes[0] = current_message_header_size_;

            std::size_t num_of_header_buffers = 1;

            const char * body_buffers[1];
            body_buffers[0] =
                current_frame_payload_ + current_message_header_size_;

            std::size_t body_buffer_sizes[1];
            body_buffer_sizes[0] =
                current_frame_payload_size_ - current_message_header_size_;

            std::size_t num_of_body_buffers = 1;

            mtx_->unlock();

            try
            {
                incoming_message_callback_(
                    incoming_message_hint_,
                    target_,
                    header_buffers,
                    header_buffer_sizes,
                    num_of_header_buffers,
                    body_buffers,
                    body_buffer_sizes,
                    num_of_body_buffers);
            }
            catch (...)
            {
                // ignore exceptions from user callbacks
            }

            mtx_->lock();
        }

        // the incoming buffer is already used at this point

        alloc_->deallocate(current_frame_payload_);
        current_frame_payload_ = NULL;

        res = core::ok;
    }

    return res;
}

core::result channel::dispatch_incoming_message(
    incoming_message_frame_list * list)
{
    core::result res;

    // check sizes, completeness of the message and count frames

    std::size_t previous_frame_number = 0;

    std::size_t num_of_header_buffers = 0;
    std::size_t num_of_body_buffers = 0;
    incoming_frame * last_header_frame = NULL; // dummy initialization
    std::size_t header_part_of_last_header_frame = 0; // dummy initialization
    incoming_frame * first_body_frame = NULL;
    std::size_t body_offset_of_first_body_frame = 0; // dummy initialization

    std::size_t header_size_so_far = 0;
    enum { header, body } state = header;

    bool message_complete = true;

    incoming_frame * frame = list->first_frame;
    while (frame != NULL)
    {
        if (frame->frame_number != previous_frame_number + 1)
        {
            // message is not yet complete (missing frames)
            // continue without error, wait for next opportunity

            message_complete = false;
            frame = NULL;
        }
        else
        {
            if (state == header)
            {
                ++num_of_header_buffers;

                if (header_size_so_far + frame->size >= list->header_size)
                {
                    // this frame completes the message header

                    last_header_frame = frame;
                    header_part_of_last_header_frame =
                        list->header_size - header_size_so_far;

                    if (header_size_so_far + frame->size > list->header_size)
                    {
                        // this frame also contains some initial data
                        // for the body

                        first_body_frame = frame;
                        body_offset_of_first_body_frame =
                            list->header_size - header_size_so_far;

                        ++num_of_body_buffers;
                    }

                    state = body;
                }
                else
                {
                    header_size_so_far += frame->size;
                }
            }
            else // state == body
            {
                if (first_body_frame == NULL)
                {
                    first_body_frame = frame;
                    body_offset_of_first_body_frame = 0;
                }

                ++num_of_body_buffers;
            }

            previous_frame_number = frame->frame_number;
            frame = frame->next;
        }
    }

    if (message_complete)
    {
        const char * * header_buffers = static_cast<const char * *>(
            alloc_->allocate(num_of_header_buffers * sizeof(const char *)));
        std::size_t * header_buffer_sizes = static_cast<std::size_t *>(
            alloc_->allocate(num_of_header_buffers * sizeof(std::size_t)));
        const char * * body_buffers = static_cast<const char * *>(
            alloc_->allocate(num_of_body_buffers * sizeof(const char *)));
        std::size_t * body_buffer_sizes = static_cast<std::size_t *>(
            alloc_->allocate(num_of_body_buffers * sizeof(std::size_t)));

        if (header_buffers != NULL &&
            header_buffer_sizes != NULL &&
            body_buffers != NULL &&
            body_buffer_sizes != NULL)
        {
            std::size_t total_bytes = 0;
            std::size_t i = 0;
            frame = list->first_frame;
            state = header;
            while (frame != NULL)
            {
                if (state == header)
                {
                    header_buffers[i] = frame->data;
                    if (frame != last_header_frame)
                    {
                        header_buffer_sizes[i] = frame->size;

                        ++i;
                    }
                    else
                    {
                        header_buffer_sizes[i] =
                            header_part_of_last_header_frame;

                        state = body;
                        i = 0;

                        if (frame == first_body_frame)
                        {
                            body_buffers[i] =
                                frame->data + body_offset_of_first_body_frame;
                            body_buffer_sizes[i] =
                                frame->size - body_offset_of_first_body_frame;

                            ++i;
                        }
                    }
                }
                else // state == body
                {
                    body_buffers[i] = frame->data;
                    body_buffer_sizes[i] = frame->size;

                    ++i;
                }

                total_bytes += frame->size + frame_head_size;
                
                frame = frame->next;
            }

            if (event_notification_callback_ != NULL)
            {
                try
                {
                    event_notification_callback_(
                        event_notification_hint_,
                        core::message_received,
                        target_, total_bytes);
                }
                catch (...)
                {
                    // ignore errors from user callback
                }
            }

            if (incoming_message_callback_ != NULL)
            {
                mtx_->unlock();

                try
                {
                    incoming_message_callback_(
                        incoming_message_hint_,
                        target_,
                        header_buffers,
                        header_buffer_sizes,
                        num_of_header_buffers,
                        body_buffers,
                        body_buffer_sizes,
                        num_of_body_buffers);
                }
                catch (...)
                {
                    // ignore exceptions from user callbacks
                }

                mtx_->lock();
            }

            // the incoming frames are no longer needed

            // detach this list from the rest
            
            incoming_message_frame_list * previous_list = NULL;
            incoming_message_frame_list * * removal_point =
                &first_incoming_frame_list_;
            while (*removal_point != list)
            {
                previous_list = *removal_point;
                removal_point = &((*removal_point)->next_list);
            }

            if (*removal_point == last_incoming_frame_list_)
            {
                // it is the last list of frames which is removed,
                // update the pointer to the rightmost list

                last_incoming_frame_list_ = previous_list;
            }

            *removal_point = (*removal_point)->next_list;

            clean_incoming_frames(list);
            alloc_->deallocate(list);

            res = core::ok;
        }
        else
        {
            res = core::no_memory;
        }

        if (header_buffers != NULL)
        {
            alloc_->deallocate(header_buffers);
        }
        if (header_buffer_sizes != NULL)
        {
            alloc_->deallocate(header_buffer_sizes);
        }
        if (body_buffers != NULL)
        {
            alloc_->deallocate(body_buffers);
        }
        if (body_buffer_sizes != NULL)
        {
            alloc_->deallocate(body_buffer_sizes);
        }
    }
    else
    {
        // do not report errors on incomplete messages
        // wait for the next opportunity to test for completeness

        res = core::ok;
    }

    return res;
}

void channel::clean_incoming_messages()
{
    if (current_frame_payload_ != NULL)
    {
        alloc_->deallocate(current_frame_payload_);
        current_frame_payload_ = NULL;
    }

    // clean all incoming lists
    while (first_incoming_frame_list_ != NULL)
    {
        clean_incoming_frames(first_incoming_frame_list_);

        incoming_message_frame_list * next_list =
            first_incoming_frame_list_->next_list;
        alloc_->deallocate(first_incoming_frame_list_);
        first_incoming_frame_list_ = next_list;
    }

    last_incoming_frame_list_ = NULL;
}

void channel::clean_incoming_frames(incoming_message_frame_list * list)
{
    incoming_frame * frame = list->first_frame;
    while (frame != NULL)
    {
        alloc_->deallocate(frame->data);

        incoming_frame * next = frame->next;
        alloc_->deallocate(frame);
        frame = next;
    }
}

outgoing_frame * channel::get_first_outgoing_frame() const
{
    return first_outgoing_frame_;
}

outgoing_frame * channel::get_last_outgoing_frame() const
{
    return last_outgoing_frame_;
}

incoming_message_frame_list * channel::get_first_incoming_frame_list() const
{
    return first_incoming_frame_list_;
}

incoming_message_frame_list * channel::get_last_incoming_frame_list() const
{
    return last_incoming_frame_list_;
}

incoming_fsm channel::get_incoming_state() const
{
    return incoming_state_;
}

core::result channel::process_complete_incoming_frame(
    const char * buffer, const std::size_t buffer_size)
{
    core::result res;

    // parse the header

    if (buffer_size >= frame_head_size)
    {
        std::memcpy(frame_head_buffer_, buffer, frame_head_size);
        parse_incoming_frame_header();

        if (current_frame_payload_size_ == buffer_size - frame_head_size)
        {
            // behave as if the whole payload has been already read

            current_frame_payload_ = static_cast<char *>(
                alloc_->allocate(current_frame_payload_size_));
            if (current_frame_payload_ != NULL)
            {
                std::memcpy(current_frame_payload_,
                    buffer + frame_head_size, current_frame_payload_size_);

                // short messages fit entirely in a single frame,
                // so can be dispatched without creating
                // any additional data structures
                
                if (current_frame_number_ == 1 && last_frame_)
                {
                    res = dispatch_short_incoming_message();
                }
                else
                {
                    res = store_incoming_frame();
                }

                current_message_header_size_ = 0;
            }
            else
            {
                res = core::no_memory;
            }
        }
        else
        {
            // corrupted buffer

            res = core::unexpected_value;
        }
    }
    else
    {
        // corrupted buffer

        res = core::unexpected_value;
    }

    return res;
}

core::result channel::do_some_input()
{
    core::result res;
    std::size_t readn;

    if (protocol_ == proto_udp)
    {
        // This is a datagram protocol,
        // read the whole frame in a single operation.

        res = read_bytes(datagram_buffer_, preferred_frame_size_, readn);
        if (res == core::ok)
        {
            process_complete_incoming_frame(datagram_buffer_, readn);
        }
    }
    else
    {
        // This is a stream protocol,
        // read the frame header or frame payload according
        // to the state machine.

        if (incoming_state_ == read_frame_header)
        {
            res = read_bytes(frame_head_buffer_ + read_offset_,
                frame_head_size - read_offset_, readn);
            if (res == core::ok)
            {
                read_offset_ += readn;

                if (read_offset_ == frame_head_size)
                {
                    // the whole frame header has been read

                    parse_incoming_frame_header();

                    if (current_frame_payload_size_ != 0 &&
                        current_frame_payload_size_ % 4 == 0)
                    {
                        current_frame_payload_ = static_cast<char *>(
                            alloc_->allocate(current_frame_payload_size_));
                        if (current_frame_payload_ != NULL)
                        {
                            read_offset_ = 0;
                            incoming_state_ = read_frame_payload;
                        }
                        else
                        {
                            res = core::no_memory;
                        }
                    }
                    else
                    {
                        res = core::unexpected_value;
                    }
                }
            }
        }
        else // incoming_state_ == read_frame_payload
        {
            res = read_bytes(current_frame_payload_ + read_offset_,
                current_frame_payload_size_ - read_offset_, readn);
            if (res == core::ok)
            {
                read_offset_ += readn;

                if (read_offset_ == current_frame_payload_size_)
                {
                    // the whole payload has been read

                    // short messages fit entirely in a single frame,
                    // so can be dispatched without creating
                    // any additional data structures

                    if (current_frame_number_ == 1 && last_frame_)
                    {
                        res = dispatch_short_incoming_message();
                    }
                    else
                    {
                        res = store_incoming_frame();
                    }

                    current_message_header_size_ = 0;
                    read_offset_ = 0;
                    incoming_state_ = read_frame_header;
                }
            }
        }

        if (res == core::io_error)
        {
            // general I/O error
            // abandon all incoming frames, because they all
            // belong to messages that are still being read
            // and there is no hope of resynchronizing the stream
            // to get them right again

            clean_incoming_messages();
        }
    }

    return res;
}

// common helper for stream and datagram channels,
// called when the complete frame was pushed out
void channel::notify_progress_and_consume_frame_list()
{
    const std::size_t sent_bytes =
        first_outgoing_frame_->byte_count;
    const std::size_t total_message_size =
        first_outgoing_frame_->total_byte_count;

    if (event_notification_callback_ != NULL)
    {
        if (sent_bytes == total_message_size)
        {
            try
            {
                event_notification_callback_(
                    event_notification_hint_,
                    core::message_sent,
                    target_, total_message_size);
            }
            catch (...)
            {
                // ignore errors from user callback
            }
        }
    }

    core::message_progress_function progress_callback =
        first_outgoing_frame_->progress_callback;
    void * progress_hint =
        first_outgoing_frame_->progress_hint;

    if (progress_callback != NULL)
    {
        mtx_->unlock();

        try
        {
            progress_callback(progress_hint,
                sent_bytes, total_message_size);
        }
        catch (...)
        {
            // ignore exceptions from user callbacks
        }

        mtx_->lock();
    }

    outgoing_frame * next = first_outgoing_frame_->next;

    alloc_->deallocate(first_outgoing_frame_->data);
    alloc_->deallocate(first_outgoing_frame_);
    first_outgoing_frame_ = next;
    output_frame_offset_ = 0;
    if (next == NULL)
    {
        // the list was exhausted
        last_outgoing_frame_ = NULL;
    }
}

core::result channel::do_some_output(bool & close_me)
{
    if (first_outgoing_frame_ == NULL)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    core::result res;

    // close the connection if the poison pill happens to be
    // at the beginning of the queue

    if (first_outgoing_frame_->close_flag)
    {
        // there are no more frames in the queue
        if (first_outgoing_frame_->next != NULL)
        {
            fatal_failure(__FILE__, __LINE__);
        }

        mode_ = hard_close;
        close_me = true;
        res = core::ok;
    }
    else
    {
        if (protocol_ == proto_udp)
        {
            // This is a datagram channel,
            // write the whole frame in a single operation.

            std::size_t written;
            res = write_bytes(
                first_outgoing_frame_->data + output_frame_offset_,
                first_outgoing_frame_->size - output_frame_offset_,
                written);
            if (res == core::ok)
            {
                // the whole frame has been pushed

                notify_progress_and_consume_frame_list();
            }
        }
        else
        {
            // This is a stream channel.

            std::size_t written;
            res = write_bytes(
                first_outgoing_frame_->data + output_frame_offset_,
                first_outgoing_frame_->size - output_frame_offset_,
                written);
            if (res == core::ok)
            {
                output_frame_offset_ += written;

                if (output_frame_offset_ == first_outgoing_frame_->size)
                {
                    // the whole frame has been already pushed

                    notify_progress_and_consume_frame_list();
                }
            }
        }

        if (res != core::ok)
        {
            // general I/O error
            // request close
            // (all frames in the output queue will be cleaned up
            // and progress will be notified as part of the channel closing)

            mode_ = hard_close;
            close_me = true;
        }
    }

    return res;
}
