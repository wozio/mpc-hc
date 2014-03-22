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

#ifndef YAMICORE_OPTIONS_H_INCLUDED
#define YAMICORE_OPTIONS_H_INCLUDED

#include "core.h"

namespace yami
{

namespace core
{
class parameters;
} // namespace core

namespace details
{

struct options
{
    void init(const core::parameters * params);

    void override(const core::parameters * params);

    int tcp_listen_backlog;
    bool tcp_reuseaddr;
    bool tcp_nonblocking;
    std::size_t tcp_connect_timeout;
    bool tcp_nodelay;
    bool tcp_keepalive;
    std::size_t tcp_frame_size;

    std::size_t udp_frame_size;

    int unix_listen_backlog;
    bool unix_nonblocking;
    std::size_t unix_frame_size;

    bool file_nonblocking;
    std::size_t file_frame_size;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_OPTIONS_H_INCLUDED
