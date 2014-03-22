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

#ifndef YAMICORE_CHANNEL_H_INCLUDED
#define YAMICORE_CHANNEL_H_INCLUDED

#include "core.h"
#include "details-fwd.h"
#include "options.h"
#include <cstddef>

// selected per platform
#include <details-types.h>

namespace yami
{

namespace details
{

enum incoming_fsm { read_frame_header, read_frame_payload };

const std::size_t word_size = 4;
const std::size_t frame_head_size = 4 * word_size;

class channel
{
public:

    // for initializing newly created channels
    // this function attempts to create a physical connection
    // according to the given target
    // should be called outside of critical section
    core::result init(allocator & alloc, mutex & mtx,
        const options & configuration_options,
        const core::parameters * overriding_options,
        const char * target,
        core::incoming_message_dispatch_function incoming_message_callback,
        void * incoming_message_hint,
        core::event_notification_function event_notification_callback,
        void * event_notification_hint,
        core::io_error_function io_error_callback,
        void * io_error_callback_hint);

    // for initializing channels that are already physically created
    // by listeners
    // target is transfered from listener to this channel
    void init(allocator & alloc, mutex & mtx,
        const options & configuration_options,
        char * target, io_descriptor_type fd, protocol prot,
        std::size_t preferred_frame_size,
        core::incoming_message_dispatch_function incoming_message_callback,
        void * incoming_message_hint,
        core::event_notification_function event_notification_callback,
        void * event_notification_hint,
        core::io_error_function io_error_callback,
        void * io_error_callback_hint);

    void close_connection();
    void clean();

    std::size_t get_frame_size() const;

    // buffers are transferred into the internally managed outgoing queue
    core::result post(std::size_t priority,
        char * * buffers, const std::size_t * buffer_sizes,
        std::size_t number_of_frames,
        bool & first_frame,
        core::message_progress_function progress_callback,
        void * progress_hint);

    // intended for cases when the incoming frame is physically read
    // outside of the channel, but also used for datagrams (UDP)
    core::result process_complete_incoming_frame(
        const char * buffer, const std::size_t buffer_size);

    core::result do_some_work(io_direction direction, bool & close_me);

    core::result post_close(std::size_t priority, bool & close_me);

    const char * get_target() const { return target_; }
    const char * move_target();

    void get_io_descriptor(
        io_descriptor_type & fd, io_direction & direction) const;

    void inc_ref() { ++ref_count_; }
    void dec_ref() { --ref_count_; }
    bool can_be_removed() const { return ref_count_ == 0; }

    // for unit tests
    outgoing_frame * get_first_outgoing_frame() const;
    outgoing_frame * get_last_outgoing_frame() const;
    incoming_message_frame_list * get_first_incoming_frame_list() const;
    incoming_message_frame_list * get_last_incoming_frame_list() const;
    incoming_fsm get_incoming_state() const;

private:

    void common_init(allocator & alloc, mutex & mtx,
        const options & configuration_options,
        const core::parameters * overriding_options,
        core::incoming_message_dispatch_function incoming_message_callback,
        void * incoming_message_hint,
        core::event_notification_function event_notification_callback,
        void * event_notification_hint,
        core::io_error_function io_error_callback,
        void * io_error_callback_hint);

    outgoing_frame * * find_outgoing_insertion_point(std::size_t priority);

    // cleans all frames starting with the given one
    void clean_outgoing_frames(outgoing_frame * frame);

    void notify_cancellation(
        core::message_progress_function progress_callback,
        void * progress_hint);

    void parse_incoming_frame_header();

    core::result store_incoming_frame();

    core::result dispatch_short_incoming_message();
    core::result dispatch_incoming_message(
        incoming_message_frame_list * list);

    // cleans all incoming frame lists and their frames
    void clean_incoming_messages();

    // cleans the given list
    void clean_incoming_frames(incoming_message_frame_list * list);

    void notify_progress_and_consume_frame_list();

    core::result do_some_input();
    core::result do_some_output(bool & close_me);

    core::result connect();
    core::result connect_to_file(const char * file_name);
    core::result connect_to_tcp(const char * tcp_address);
    core::result connect_to_udp(const char * udp_address);
    core::result connect_to_unix(const char * path);

    core::result read_bytes(char * buf, std::size_t size,
        std::size_t & readn);
    core::result write_bytes(const char * buf, std::size_t size,
        std::size_t & written);

    options configuration_options_;
    allocator * alloc_;

    mutex * mtx_; // provided by channel group

    char * target_;
    protocol protocol_;
    io_direction direction_;
    io_descriptor_type fd_;
    std::size_t preferred_frame_size_;

    // used only with UDP, as it is needed with every send operation
    void * target_address_;
    int target_address_size_;
    char * datagram_buffer_;

    std::size_t ref_count_;

    // soft close allows output of remaining existing frames,
    // hard close means dropping everything
    enum mode { operational, soft_close, hard_close };
    mode mode_;

    // state of outgoing queue

    std::size_t output_frame_offset_;
    outgoing_frame * first_outgoing_frame_;
    outgoing_frame * last_outgoing_frame_;

    // state of incoming queue

    incoming_fsm incoming_state_;
    std::size_t read_offset_; // within a given state

    char frame_head_buffer_[frame_head_size];
    int current_message_id_;
    std::size_t current_frame_number_;
    std::size_t current_message_header_size_;
    std::size_t current_frame_payload_size_;
    char * current_frame_payload_;
    bool last_frame_;

    incoming_message_frame_list * first_incoming_frame_list_;
    incoming_message_frame_list * last_incoming_frame_list_;

    core::incoming_message_dispatch_function incoming_message_callback_;
    void * incoming_message_hint_;

    core::event_notification_function event_notification_callback_;
    void * event_notification_hint_;

    core::io_error_function io_error_callback_;
    void * io_error_callback_hint_;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_CHANNEL_H_INCLUDED
