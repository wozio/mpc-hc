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

#ifndef YAMICORE_LISTENER_H_INCLUDED
#define YAMICORE_LISTENER_H_INCLUDED

#include "core.h"
#include "details-fwd.h"
#include <cstddef>

// selected per platform
#include <details-types.h>

namespace yami
{

namespace details
{

class listener
{
public:

    void init(allocator & alloc,
        channel_group & group, mutex & mtx,
        core::new_incoming_connection_function connection_hook,
        void * connection_hook_hint,
        core::io_error_function io_error_callback,
        void * io_error_callback_hint);

    core::result prepare(const char * target);

    core::result do_some_work();

    void close_socket();
    void clean();

    const char * get_target() const { return target_; }

    io_descriptor_type get_io_descriptor() const { return fd_; }

    void inc_ref() { ++ref_count_; }
    void dec_ref() { --ref_count_; }
    bool can_be_removed() const { return ref_count_ == 0; }

    listener * next;

private:

    protocol protocol_;

    core::result prepare_tcp(const char * address);
    core::result prepare_udp(const char * address);
    core::result prepare_unix(const char * path);

    core::result accept_tcp();
    core::result accept_udp();
    core::result accept_unix();

    allocator * alloc_;
    mutex * mtx_;
    std::size_t ref_count_;

    const char * target_;

    int tcp_port_; // network port (in local order)

    io_descriptor_type fd_;

    channel_group * group_;

    // used only for UDP
    char * frame_buffer_;
    std::size_t frame_buffer_size_;

    core::new_incoming_connection_function connection_hook_;
    void * connection_hook_hint_;

    core::io_error_function io_error_callback_;
    void * io_error_callback_hint_;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_LISTENER_H_INCLUDED
