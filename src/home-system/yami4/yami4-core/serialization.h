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

#ifndef YAMICORE_SERIALIZATION_H_INCLUDED
#define YAMICORE_SERIALIZATION_H_INCLUDED

#include "allocator.h"
#include "core.h"
#include <cstddef>

namespace yami
{

namespace details
{

inline std::size_t round_up_4(std::size_t v)
{
    return (v + 3) & ~0x03;
}

core::result put_integer(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    int value);

core::result get_integer(const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    int & value);

core::result put_long_long(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    long long value);

core::result get_long_long(const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    long long & value);

core::result put_double_float(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    double value);

core::result get_double_float(const char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    double & value);

core::result put_string(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const char * value, std::size_t length);

core::result get_string(const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    const char * & value, std::size_t & length,
    allocator & alloc);

core::result put_boolean_array(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const bool * values, std::size_t length);

core::result get_boolean_array(const char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    bool * & values, std::size_t & length,
    allocator & alloc);

core::result put_integer_array(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const int * values, std::size_t length);

core::result get_integer_array(const char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    int * & values, std::size_t & length,
    allocator & alloc);

core::result put_long_long_array(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const long long * values, std::size_t length);

core::result get_long_long_array(const char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    long long * & values, std::size_t & length,
    allocator & alloc);

core::result put_double_float_array(char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const double * values, std::size_t length);

core::result get_double_float_array(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    double * & values, std::size_t & length,
    allocator & alloc);

void fill_outgoing_frame_header(
    char * buffer,
    std::size_t message_id, int frame_number,
    std::size_t message_header_size, std::size_t frame_payload_size);

} // namespace details

} // namespace yami

#endif // YAMICORE_SERIALIZATION_H_INCLUDED
