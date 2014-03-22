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

#ifndef YAMICORE_OUTGOING_FRAME_H_INCLUDED
#define YAMICORE_OUTGOING_FRAME_H_INCLUDED

#include "core.h"
#include <cstddef>

namespace yami
{

namespace details
{

struct outgoing_frame
{
    // primary information

    const char * data;
    std::size_t size;

    // all following fields are internally managed by channel

    core::message_progress_function progress_callback;
    void * progress_hint;

    std::size_t byte_count; // number of bytes from the beginning of message
    std::size_t total_byte_count; // size of the whole message

    // used for proper queue management
    std::size_t priority;
    bool close_flag;
    outgoing_frame * next;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_OUTGOING_FRAME_H_INCLUDED
