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

#include "selector.h"
#include "../channel.h"
#include "../io_error_handler.h"
#include "../listener.h"
#include <cstring>

#include <Winsock2.h>

using namespace yami;
using namespace details;

core::result selector::init()
{
    core::result res = core::io_error;

    WORD versionRequested = MAKEWORD(2, 0);
    WSADATA wsadata;
    (void)WSAStartup(versionRequested, &wsadata);

    // create artificial pipe on a pair of sockets

    io_descriptor_type ld = ::socket(AF_INET, SOCK_STREAM, 0);
    if (ld != INVALID_SOCKET)
    {
        int localhost = htonl(0x7f000001); // 127.0.0.1

        sockaddr_in local;

        std::memset(&local, 0, sizeof(local));

        local.sin_family = AF_INET;
        local.sin_port = 0; // system-assigned port
        local.sin_addr.s_addr = localhost;

        int cc = ::bind(ld,
            reinterpret_cast<sockaddr *>(&local), sizeof(local));
        if (cc != SOCKET_ERROR)
        {
            cc = ::listen(ld, 1);
            if (cc != SOCKET_ERROR)
            {
                // figure out the system-assigned port
                int dummy = sizeof(local);
                cc = ::getsockname(ld,
                    reinterpret_cast<sockaddr*>(&local), &dummy);
                if (cc != SOCKET_ERROR)
                {
                    const u_short assigned_listener_port = local.sin_port;

                    io_descriptor_type sock1 =
                        ::socket(AF_INET, SOCK_STREAM, 0);
                    if (sock1 != INVALID_SOCKET)
                    {
                        struct sockaddr_in servaddr;

                        std::memset(&servaddr, 0, sizeof(servaddr));
                        servaddr.sin_family = AF_INET;
                        servaddr.sin_port = assigned_listener_port;
                        memcpy(&(servaddr.sin_addr),
                            &localhost, sizeof(int));

                        cc = ::connect(sock1,
                            (struct sockaddr*)&servaddr, sizeof(servaddr));
                        if (cc != SOCKET_ERROR)
                        {
                            // connection created, accept the other end
                            // and close the temporary listening socket

                            sockaddr_in client_addr;
                            int client_addr_size = sizeof(client_addr);

                            io_descriptor_type sock2 = ::accept(ld,
                                reinterpret_cast<sockaddr *>(&client_addr),
                                &client_addr_size);
                            if (sock2 != INVALID_SOCKET)
                            {
                                ::closesocket(ld);

                                interrupt_pipe_write_ = sock1;
                                interrupt_pipe_read_ = sock2;

                                res = core::ok;
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}

void selector::install_io_error_logger(
    core::io_error_function io_error_callback,
    void * io_error_callback_hint)
{
    io_error_callback_ = io_error_callback;
    io_error_callback_hint_ = io_error_callback_hint;
}

void selector::clean()
{
    ::closesocket(interrupt_pipe_write_);
    ::closesocket(interrupt_pipe_read_);

    (void)WSACleanup();
}

void selector::reset()
{
    FD_ZERO(&read_set_);
    FD_ZERO(&write_set_);

    // always add the internal interrupt pipe

    FD_SET(interrupt_pipe_read_, &read_set_);
    
    num_of_channels_used_ = 1; // internal pipe
}

void selector::add_channel(const channel & ch,
    bool allow_outgoing_traffic, bool allow_incoming_traffic)
{
    io_descriptor_type fd;
    io_direction direction;

    ch.get_io_descriptor(fd, direction);

    if ((direction == input || direction == inout) && allow_incoming_traffic)
    {
        FD_SET(fd, &read_set_);
    }

    if ((direction == output || direction == inout) && allow_outgoing_traffic)
    {
        FD_SET(fd, &write_set_);
    }
    
    ++num_of_channels_used_;
}

void selector::add_listener(const listener & lst)
{
    io_descriptor_type fd = lst.get_io_descriptor();

    FD_SET(fd, &read_set_);
    
    ++num_of_channels_used_;
}

void selector::get_channel_usage(int & max_allowed, int & used)
{
    max_allowed = FD_SETSIZE;
    used = num_of_channels_used_;
}

core::result selector::wait(std::size_t timeout)
{
    core::result res;

    struct timeval tv;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = 1000 * (timeout % 1000);

    const int cc = ::select(0, &read_set_, &write_set_, NULL, &tv);
    if (cc > 0)
    {
        // the wait might have been interrupted with the internal pipe
        // if that is the case, consume the dummy info

        if (FD_ISSET(interrupt_pipe_read_, &read_set_) != 0)
        {
            char dummy;
            int rn =
                ::recv(interrupt_pipe_read_, &dummy, sizeof(dummy), 0);
            if (rn == SOCKET_ERROR)
            {
                handle_io_error("selector pipe read",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }
            else
            {
                res = core::ok;
            }
        }
        else
        {
            // the wait was interrupted because there is
            // a regular event to process

            res = core::ok;
        }
    }
    else if (cc == 0)
    {
        res = core::timed_out;
    }
    else
    {
        handle_io_error("selector wait",
            io_error_callback_, io_error_callback_hint_);

        res = core::io_error;
    }

    return res;
}

bool selector::is_channel_ready(
    const channel & ch, io_direction & direction) const
{
    io_descriptor_type fd;
    io_direction dir;

    ch.get_io_descriptor(fd, dir);

    bool ready_for_reading = false;
    bool ready_for_writing = false;

    if (dir == input || dir == inout)
    {
        if (FD_ISSET(fd, &read_set_) != 0)
        {
            ready_for_reading = true;
        }
    }

    if (dir == output || dir == inout)
    {
        if (FD_ISSET(fd, &write_set_) != 0)
        {
            ready_for_writing = true;
        }
    }

    if (ready_for_reading && ready_for_writing)
    {
        direction = inout;
    }
    else if (ready_for_reading)
    {
        direction = input;
    }
    else if (ready_for_writing)
    {
        direction = output;
    }

    return ready_for_reading || ready_for_writing;
}

bool selector::is_listener_ready(const listener & lst) const
{
    io_descriptor_type fd = lst.get_io_descriptor();
    
    return FD_ISSET(fd, &read_set_) != 0;
}

core::result selector::interrupt()
{
    // wake the selector up from its current wait state

    core::result res = core::ok;

    char dummy = '\0';
    int written =
        ::send(interrupt_pipe_write_, &dummy, sizeof(dummy), 0);
    if (written == SOCKET_ERROR)
    {
        handle_io_error("selector pipe write",
            io_error_callback_, io_error_callback_hint_);

        res = core::io_error;
    }
    else
    {
        res = core::ok;
    }

    return res;
}
