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

using namespace yami;
using namespace yami::core;

namespace // unnamed
{

// implementation machine-level assumptions
// do not mess with them, these constants are declared only to avoid
// "magic numbers" in the code

const std::size_t bytes_in_word = 4;

result check_buffer_sizes(
    const std::size_t * buffer_sizes,
    std::size_t num_of_buffers)
{
    result res = ok;
    for (std::size_t i = 0; i != num_of_buffers; ++i)
    {
        if (buffer_sizes[i] % bytes_in_word != 0)
        {
            res = unexpected_value;
            break;
        }
    }

    return res;
}

} // unnamed namespace

raw_buffer_data_source::raw_buffer_data_source(
    const char * * buffers,
    const std::size_t * buffer_sizes,
    std::size_t num_of_buffers)
    : buffers_(buffers), buffer_sizes_(buffer_sizes),
      num_of_buffers_(num_of_buffers)
{
}

raw_buffer_data_source::raw_buffer_data_source(
    const char * buffer, std::size_t buffer_size)
    : buffers_(single_buffer), buffer_sizes_(single_buffer_size),
      num_of_buffers_(1)
{
    single_buffer[0] = buffer;
    single_buffer_size[0] = buffer_size;
}

result raw_buffer_data_source::get_serialize_buffer_size(
    std::size_t & size) const
{
    size = 0;
    for (std::size_t i = 0; i != num_of_buffers_; ++i)
    {
        size += buffer_sizes_[i];
    }

    return ok;
}

result raw_buffer_data_source::serialize(
    char * * target_buffers,
    const std::size_t * target_buffer_sizes,
    std::size_t num_of_target_buffers) const
{
    std::size_t source_buffer_index = 0;     // which buffer
    std::size_t source_buffer_position = 0;  // where in the buffer

    std::size_t target_buffer_index = 0;     // which buffer
    std::size_t target_buffer_position = 0;  // where in the buffer

    result res = check_buffer_sizes(buffer_sizes_, num_of_buffers_);
    if (res == ok)
    {
        res = check_buffer_sizes(target_buffer_sizes, num_of_target_buffers);
    }

    if (res == ok)
    {
        while (true)
        {
            const unsigned int * source =
                reinterpret_cast<const unsigned int *>(
                    &buffers_[source_buffer_index][source_buffer_position]);

            unsigned int * target =
                reinterpret_cast<unsigned int *>(
                    &target_buffers[
                        target_buffer_index][target_buffer_position]);

            *target = *source;

            source_buffer_position += bytes_in_word;
            if (source_buffer_position == buffer_sizes_[source_buffer_index])
            {
                ++source_buffer_index;
                if (source_buffer_index == num_of_buffers_)
                {
                    // consumed source
                    break;
                }

                source_buffer_position = 0;
            }

            target_buffer_position += bytes_in_word;
            if (target_buffer_position ==
                target_buffer_sizes[target_buffer_index])
            {
                ++target_buffer_index;
                if (target_buffer_index == num_of_target_buffers)
                {
                    // filled the target, but source still not consumed
                    res = not_enough_space;
                    break;
                }

                target_buffer_position = 0;
            }
        }
    }

    return res;
}
