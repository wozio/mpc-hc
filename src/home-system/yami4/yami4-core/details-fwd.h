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

#ifndef YAMICORE_DETAILS_FWD_H_INCLUDED
#define YAMICORE_DETAILS_FWD_H_INCLUDED

namespace yami
{

namespace details
{

class allocator;
struct entry;

class fsm_common;
class get_serialize_buffer_size_fsm;
class serialize_fsm;
class deserialize_fsm;

struct outgoing_frame;
struct incoming_frame;
struct incoming_message_frame_list;

class channel;
class channel_holder;
class listener;
class channel_group;
class mutex;
struct options;

enum protocol { proto_tcp, proto_udp, proto_unix, proto_file };
enum io_direction { none, input, output, inout };

class dump_sink;

} // namespace details

} // namespace yami

#endif // YAMICORE_DETAILS_FWD_H_INCLUDED
