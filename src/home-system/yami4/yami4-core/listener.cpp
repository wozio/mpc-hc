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

#include "listener.h"
#include "allocator.h"
#include "fatal_errors.h"
#include <cstring>

using namespace yami;
using namespace details;

namespace // unnamed
{
const char tcp_prefix[] = "tcp://";
const char udp_prefix[] = "udp://";
const char unix_prefix[] = "unix://";
} // unnamed namespace

void listener::init(allocator & alloc,
    channel_group & group, mutex & mtx,
    core::new_incoming_connection_function connection_hook,
    void * connection_hook_hint,
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    next = NULL;
    alloc_ = &alloc;
    target_ = NULL;
    ref_count_ = 1;
    tcp_port_ = -1;
    fd_ = empty_io_descriptor;
    group_ = &group;
    frame_buffer_ = NULL;
    mtx_ = &mtx;
    connection_hook_ = connection_hook;
    connection_hook_hint_ = connection_hook_hint;
    io_error_callback_ = io_error_callback;
    io_error_callback_hint_ = io_error_callback_hint;
}

core::result listener::prepare(const char * target)
{
    // this function can be called only in the uninitialized state
    if (target_ != NULL)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    core::result res;

    // recognize the protocol

    const std::size_t tcp_prefix_size = sizeof(tcp_prefix) - 1;
    const std::size_t udp_prefix_size = sizeof(udp_prefix) - 1;
    const std::size_t unix_prefix_size = sizeof(unix_prefix) - 1;

    if (std::strncmp(target, tcp_prefix, tcp_prefix_size) == 0)
    {
        res = prepare_tcp(target + tcp_prefix_size);
    }
    else if (std::strncmp(target, udp_prefix, udp_prefix_size) == 0)
    {
        res = prepare_udp(target + udp_prefix_size);
    }
    else if (std::strncmp(target, unix_prefix, unix_prefix_size) == 0)
    {
        res = prepare_unix(target + unix_prefix_size);
    }
    else
    {
        // unknown protocol

        res = core::bad_protocol;
    }

    return res;
}

// synchronized by caller
void listener::clean()
{
    close_socket();
    alloc_->deallocate(target_);

    if (frame_buffer_ != NULL)
    {
        alloc_->deallocate(frame_buffer_);
    }
}
