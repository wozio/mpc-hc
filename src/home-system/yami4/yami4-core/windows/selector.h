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

#ifndef YAMICORE_SELECTOR_H_INCLUDED
#define YAMICORE_SELECTOR_H_INCLUDED

#include "../core.h"
#include "../details-fwd.h"
#include "details-types.h"
#include <Winsock.h>

namespace yami
{

namespace details
{

class channel;
class listener;

class selector
{
public:

    // shuld be called once
    core::result init();

    void install_io_error_logger(
        core::io_error_function io_error_callback,
        void * io_error_callback_hint);

    void clean();

    // can be called many times
    void reset();

    void add_channel(const channel & ch,
        bool allow_outgoing_traffic, bool allow_incoming_traffic);

    void add_listener(const listener & lst);

    void get_channel_usage(int & max_allowed, int & used);

    // timeout is in ms
    core::result wait(std::size_t timeout);

    bool is_channel_ready(const channel & ch, io_direction & direction) const;
    bool is_listener_ready(const listener & lst) const;

    // forces the selector to wake up from the current wait operation
    // without executing any action
    core::result interrupt();

private:

    mutable fd_set read_set_;
    mutable fd_set write_set_;

    int num_of_channels_used_;

    io_descriptor_type interrupt_pipe_write_;
    io_descriptor_type interrupt_pipe_read_;

    core::io_error_function io_error_callback_;
    void * io_error_callback_hint_;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_SELECTOR_H_INCLUDED
