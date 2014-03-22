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

#include "../io_error_handler.h"

#include <cstdio>
#include <cstring>
#include <Winsock2.h>

void yami::details::handle_io_error(
    const char * prefix,
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    handle_io_error_with_code(prefix, WSAGetLastError(),
        io_error_callback, io_error_callback_hint);
}

void yami::details::handle_io_error_with_code(
    const char * prefix,
    int error_code,
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    if (io_error_callback != NULL)
    {
        const std::size_t max_msg_size = 500;
        char msg[max_msg_size];

        _snprintf(msg, max_msg_size, "%s: WSA error code: %d",
            prefix, error_code);

        try
        {
            io_error_callback(io_error_callback_hint, error_code, msg);
        }
        catch (...)
        {
            // ignore errors from user callbacks
        }
    }
}
