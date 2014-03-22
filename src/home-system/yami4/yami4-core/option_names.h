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

#ifndef YAMICORE_OPTION_NAMES_H_INCLUDED
#define YAMICORE_OPTION_NAMES_H_INCLUDED

#include "dll.h"

namespace yami
{

namespace core
{

namespace option_names
{

DLL const char tcp_listen_backlog[] =  "tcp_listen_backlog";
DLL const char tcp_reuseaddr[] =       "tcp_reuseaddr";
DLL const char tcp_nonblocking[] =     "tcp_nonblocking";
DLL const char tcp_connect_timeout[] = "tcp_connect_timeout";
DLL const char tcp_nodelay[] =         "tcp_nodelay";
DLL const char tcp_keepalive[] =       "tcp_keepalive";
DLL const char tcp_frame_size[] =      "tcp_frame_size";
DLL const char udp_frame_size[] =      "udp_frame_size";
DLL const char unix_listen_backlog[] = "unix_listen_backlog";
DLL const char unix_nonblocking[] =    "unix_nonblocking";
DLL const char unix_frame_size[] =     "unix_frame_size";
DLL const char file_nonblocking[] =    "file_nonblocking";
DLL const char file_frame_size[] =     "file_frame_size";

} // namespace option_names

} // namespace core

} // namespace yami

#endif // YAMICORE_OPTION_NAMES_H_INCLUDED
