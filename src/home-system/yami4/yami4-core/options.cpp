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

#include "options.h"
#include "option_names.h"
#include "parameters.h"

using namespace yami;
using namespace details;

namespace // unnamed
{

void override_if_defined(const core::parameters * params, const char * name,
    bool & value)
{
    core::parameter_type type;
    core::result res = params->get_type(name, type);
    if (res == core::ok && type == core::boolean)
    {
        params->get_boolean(name, value);
    }
}

void override_if_defined(const core::parameters * params, const char * name,
    int & value)
{
    core::parameter_type type;
    core::result res = params->get_type(name, type);
    if (res == core::ok && type == core::integer)
    {
        params->get_integer(name, value);
    }
}

void override_if_defined(const core::parameters * params, const char * name,
    std::size_t & value)
{
    core::parameter_type type;
    core::result res = params->get_type(name, type);
    if (res == core::ok && type == core::integer)
    {
        int tmp;
        params->get_integer(name, tmp);

        if (tmp >= 0)
        {
            value = static_cast<std::size_t>(tmp);
        }
    }
}

} // unnamed namespace

void options::init(const core::parameters * params)
{
    // set default values

    // listen backlog for TCP is 10
    tcp_listen_backlog = 10;

    // TCP listeners can reuse already bound addresses
    tcp_reuseaddr = true;

    // TCP connections are non-blocking
    tcp_nonblocking = true;

    // TCP connect is blocking by default (even with non-blocking setup)
    tcp_connect_timeout = 0;

    // do not use the Nagle algorithm (optimize for LANs)
    tcp_nodelay = true;

    // do not use keepalive
    tcp_keepalive = false;

    // frame size for TCP transfer is 4kB
    tcp_frame_size = 4096;

    // frame size for UDP transfer is 512 bytes
    udp_frame_size = 512;

    // listen backlog for UNIX is 10
    unix_listen_backlog = 10;

    // UNIX connections are non-blocking
    unix_nonblocking = true;

    // frame size for UNIX transfer is 4kB
    unix_frame_size = 4096;

    // file connections are non-blocking
    file_nonblocking = true;

    // frame size for file transfer is 4kB
    file_frame_size = 4096;

    // extract user-provided options and override default settings

    override (params);
}

void options::override(const core::parameters * params)
{
    if (params != NULL)
    {
        override_if_defined(params,
            core::option_names::tcp_listen_backlog, tcp_listen_backlog);

        override_if_defined(params,
            core::option_names::tcp_reuseaddr, tcp_reuseaddr);

        override_if_defined(params,
            core::option_names::tcp_nonblocking, tcp_nonblocking);

        override_if_defined(params,
            core::option_names::tcp_connect_timeout, tcp_connect_timeout);

        override_if_defined(params,
            core::option_names::tcp_nodelay, tcp_nodelay);

        override_if_defined(params,
            core::option_names::tcp_keepalive, tcp_keepalive);

        override_if_defined(params,
            core::option_names::tcp_frame_size, tcp_frame_size);

        override_if_defined(params,
            core::option_names::unix_listen_backlog, unix_listen_backlog);

        override_if_defined(params,
            core::option_names::unix_nonblocking, unix_nonblocking);

        override_if_defined(params,
            core::option_names::unix_frame_size, unix_frame_size);

        override_if_defined(params,
            core::option_names::file_nonblocking, file_nonblocking);

        override_if_defined(params,
            core::option_names::file_frame_size, file_frame_size);
    }
}
