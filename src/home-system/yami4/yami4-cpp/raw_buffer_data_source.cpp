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

#include "raw_buffer_data_source.h"
#include "details.h"

using namespace yami;

raw_buffer_data_source::raw_buffer_data_source(
    const char * * buffers,
    const std::size_t * buffer_sizes,
    std::size_t num_of_buffers)
    : buffer_wrapper_(buffers, buffer_sizes, num_of_buffers)
{
}

raw_buffer_data_source::raw_buffer_data_source(
    const char * buffer, std::size_t buffer_size)
    : buffer_wrapper_(buffer, buffer_size)
{
}

raw_buffer_data_source::raw_buffer_data_source(
    const std::vector<char> & buffer)
    : buffer_wrapper_(&buffer[0], buffer.size())
{
}

std::size_t raw_buffer_data_source::serialize_buffer_size() const
{
    std::size_t size;
    details::translate_result_to_exception(
        buffer_wrapper_.get_serialize_buffer_size(size));

    return size;
}

void raw_buffer_data_source::serialize(
    char * * buffers,
    std::size_t * buffer_sizes,
    std::size_t num_of_buffers) const
{
    details::translate_result_to_exception(
        buffer_wrapper_.serialize(buffers, buffer_sizes, num_of_buffers));
}

const core::serializable & raw_buffer_data_source::get_core_object() const
{
    return buffer_wrapper_;
}
