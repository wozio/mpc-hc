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

#include "../network_utils.h"
#include "../allocator.h"
#include "../core.h"
#include "../io_error_handler.h"
#include "mutex.h"

#include <cstring>

#include <fcntl.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>

using namespace yami;
using namespace details;

core::result details::get_host_by_name(const char * hostname, int & host,
    core::io_error_function io_error_callback, void * io_error_callback_hint)
{
    core::result res;

    struct addrinfo hints;
    struct addrinfo * info = NULL;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int cc = ::getaddrinfo(hostname, NULL, &hints, &info);
    if (cc == 0)
    {
        struct sockaddr_in * sin =
            reinterpret_cast<struct sockaddr_in *>(info->ai_addr);
        host = reinterpret_cast<int &>(sin->sin_addr.s_addr);

        ::freeaddrinfo(info);

        res = core::ok;
    }
    else
    {
        handle_io_error("get host by name",
            io_error_callback, io_error_callback_hint);

        res = core::io_error;
    }

    return res;
}

int details::get_network_port(long port)
{
    return htons(static_cast<u_short>(port));
}

core::result details::set_nonblocking(io_descriptor_type fd)
{
    core::result res;

    u_long value = 1;
    const int cc = ::ioctlsocket(fd, FIONBIO, &value);
    if (cc != SOCKET_ERROR)
    {
        res = core::ok;
    }
    else
    {
        res = core::io_error;
    }

    return res;
}

core::result details::set_reuseaddr(io_descriptor_type fd)
{
    core::result res;

    const char value = 1;
    const int cc = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
        &value, sizeof(int));
    if (cc == 0)
    {
        res = core::ok;
    }
    else
    {
        res = core::io_error;
    }

    return res;
}

core::result details::set_nodelay(io_descriptor_type fd)
{
    core::result res;

    const char value = 1;
    const int cc = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
        &value, sizeof(int));
    if (cc == 0)
    {
        res = core::ok;
    }
    else
    {
        res = core::io_error;
    }

    return res;
}

core::result details::set_keepalive(io_descriptor_type fd)
{
    core::result res;

    const char value = 1;
    const int cc = ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
        &value, sizeof(int));
    if (cc == 0)
    {
        res = core::ok;
    }
    else
    {
        res = core::io_error;
    }

    return res;
}
