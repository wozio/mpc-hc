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

#ifndef YAMICORE_INCOMING_MESSAGE_FRAME_LIST_H_INCLUDED
#define YAMICORE_INCOMING_MESSAGE_FRAME_LIST_H_INCLUDED

#include <cstddef>

namespace yami
{

namespace details
{

struct incoming_frame;
struct incoming_message_frame_list
{
    int message_id;
    std::size_t header_size;

    bool check_completeness;

    incoming_frame * first_frame;
    incoming_frame * last_frame;

    incoming_message_frame_list * next_list;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_INCOMING_MESSAGE_FRAME_LIST_H_INCLUDED
