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

#include "network_utils.h"
#include "allocator.h"
#include "mutex.h"
#include <cstdlib>
#include <cstring>

using namespace yami;
using namespace details;

core::result details::parse_address(
    allocator & alloc, const char * address, ip_address & ipa,
    core::io_error_function io_error_callback, void * io_error_callback_hint)
{
    core::result res = core::bad_protocol;

    if (*address != '\0')
    {
        const char * port_position;
        const char * colon = std::strchr(address, ':');
        if (colon != NULL)
        {
            port_position = colon + 1;

            const std::size_t hostname_length = colon - address;
            if (hostname_length > 0)
            {
                if (*address == '*')
                {
                    // the hostname part of the address is ANY
                    ipa.host = 0;

                    res = core::ok;
                }
                else
                {
                    // look up the hostname part in the name server

                    char * hostname = static_cast<char *>(
                        alloc.allocate(hostname_length + 1));
                    if (hostname != NULL)
                    {
                        std::strncpy(hostname, address, hostname_length);
                        hostname[hostname_length] = '\0';

                        res = get_host_by_name(hostname, ipa.host,
                            io_error_callback, io_error_callback_hint);

                        alloc.deallocate(hostname);
                    }
                    else
                    {
                        res = core::no_memory;
                    }
                }
            }
        }
        else
        {
            // no colon in the address - consider the hostname part as ANY
            ipa.host = 0;
            port_position = address;

            res = core::ok;
        }

        if (res == core::ok)
        {
            if (*port_position == '*')
            {
                ipa.port = 0;
            }
            else
            {
                char * endptr;
                const long port = std::strtol(port_position, &endptr, 10);
                if (*endptr == '\0')
                {
                    ipa.port = get_network_port(port);
                }
                else
                {
                    res = core::bad_protocol;
                }
            }
        }
    }

    return res;
}
