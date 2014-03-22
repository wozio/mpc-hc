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

#ifndef YAMICORE_NETWORK_UTILS_H_INCLUDED
#define YAMICORE_NETWORK_UTILS_H_INCLUDED

#include "core.h"
#include "details-fwd.h"

// selected per platform
#include <details-types.h>

namespace yami
{

namespace details
{

struct ip_address
{
    int host; // network byte order
    int port; // network byte order
};

// parses addresses of any of the form:
// - "port"
// - "*:port"
// - "xxx.xxx.xxx.xxx:port"
// - "hostname:port"
core::result parse_address(
    allocator & alloc, const char * address, ip_address & ipa,
    core::io_error_function io_error_callback, void * io_error_callback_hint);

core::result get_host_by_name(const char * hostname, int & host,
    core::io_error_function io_error_callback, void * io_error_callback_hint);
int get_network_port(long port);
core::result set_nonblocking(io_descriptor_type fd);
core::result set_reuseaddr(io_descriptor_type fd);
core::result set_nodelay(io_descriptor_type fd);
core::result set_keepalive(io_descriptor_type fd);

} // namespace details

} // namespace yami

#endif // YAMICORE_NETWORK_UTILS_H_INCLUDED
