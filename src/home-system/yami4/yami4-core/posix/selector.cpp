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

#include <errno.h>

using namespace yami;
using namespace details;

core::result selector::init()
{
    io_error_callback_ = NULL;

    core::result res;

    io_descriptor_type pipe_descriptors[2];

    int cc = ::pipe(pipe_descriptors);
    if (cc == 0)
    {
        interrupt_pipe_write_ = pipe_descriptors[1];
        interrupt_pipe_read_ = pipe_descriptors[0];

        res = core::ok;
    }
    else
    {
        res = core::io_error;
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
    ::close(interrupt_pipe_write_);
    ::close(interrupt_pipe_read_);
}

void selector::reset()
{
    FD_ZERO(&read_set_);
    FD_ZERO(&write_set_);

    // always add the internal interrupt pipe

    FD_SET(interrupt_pipe_read_, &read_set_);
    num_of_descriptors_to_test_ = interrupt_pipe_read_ + 1;
    
    num_of_channels_used_ = 1; // internal pipe
}

void selector::add_channel(const channel & ch,
    bool allow_outgoing_traffic, bool allow_incoming_traffic)
{
    io_descriptor_type fd;
    io_direction direction;

    ch.get_io_descriptor(fd, direction);

    if (fd > num_of_descriptors_to_test_ - 1)
    {
        num_of_descriptors_to_test_ = fd + 1;
    }

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

    if (fd > num_of_descriptors_to_test_ - 1)
    {
        num_of_descriptors_to_test_ = fd + 1;
    }

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

    const int cc = ::select(num_of_descriptors_to_test_,
        &read_set_, &write_set_, NULL, &tv);
    if (cc > 0)
    {
        // the wait might have been interrupted with the internal pipe
        // if that is the case, consume the dummy info

        if (FD_ISSET(interrupt_pipe_read_, &read_set_) != 0)
        {
            bool keep_trying = true;
            while (keep_trying)
            {
                char dummy;
                ssize_t rn =
                    ::read(interrupt_pipe_read_, &dummy, sizeof(dummy));

                if (rn == -1)
                {
                    // if the attempt to read is abandoned due to signal,
                    // it should be repeated
                    // otherwise it was an error

                    if (errno != EINTR)
                    {
                        handle_io_error("selector pipe read",
                            io_error_callback_, io_error_callback_hint_);

                        res = core::io_error;
                        keep_trying = false;
                    }
                }
                else
                {
                    res = core::ok;
                    keep_trying = false;
                }
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
        if (cc == EINTR)
        {
            // the wait operation will be repeated in the outer loop
            
            res = core::ok;
        }
        else
        {
            handle_io_error("selector wait",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }
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

    if (fd < num_of_descriptors_to_test_)
    {
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
    
    return fd < num_of_descriptors_to_test_ &&
        FD_ISSET(fd, &read_set_) != 0;
}

core::result selector::interrupt()
{
    // wake the selector up from its current wait state

    core::result res = core::ok;

    bool keep_trying = true;
    while (keep_trying)
    {
        char dummy = '\0';
        ssize_t written =
            ::write(interrupt_pipe_write_, &dummy, sizeof(dummy));

        if (written == -1)
        {
            // if the attempt to write is abandoned due to signal,
            // it should be repeated
            // otherwise it was an error

            if (errno != EINTR)
            {
                handle_io_error("selector pipe write",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
                keep_trying = false;
            }
        }
        else
        {
            res = core::ok;
            keep_trying = false;
        }
    }

    return res;
}
