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

#ifndef YAMICPP_OPTIONS_H_INCLUDED
#define YAMICPP_OPTIONS_H_INCLUDED

#include <cstddef>

namespace yami
{

class parameters;

namespace details
{

struct cpp_options
{
    void init(const parameters & params);

    std::size_t dispatcher_threads;
    std::size_t connection_retries;
    std::size_t connection_retry_delay_spread;
    std::size_t outgoing_high_water_mark;
    std::size_t outgoing_low_water_mark;
    std::size_t incoming_high_water_mark;
    std::size_t incoming_low_water_mark;

    bool deliver_as_raw_binary;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_OPTIONS_H_INCLUDED
