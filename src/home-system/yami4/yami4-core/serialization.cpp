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

// Note: this file implements serialization routines
// for both little-endian and big-endian platforms

#include "serialization.h"
#include "fatal_errors.h"
#include <cstring>

using namespace yami;
using namespace yami::details;

namespace // unnamed
{

// implementation machine-level assumptions
// do not mess with them, these constants are declared only to avoid
// "magic numbers" in the code

const std::size_t bytes_in_word = 4;
const std::size_t round_down_word_mask = ~0x3u;
const std::size_t bits_in_byte = 8;
const std::size_t bits_in_word = bytes_in_word * bits_in_byte;

const unsigned int byte_1_mask = 0xFF;
const unsigned int byte_2_mask = 0xFF00;
const unsigned int byte_3_mask = 0xFF0000;
const unsigned int byte_4_mask = 0xFF000000ul;
const unsigned int low_word_mask = 0xFFFFFFFFul;
const unsigned long long high_word_mask = 0xFFFFFFFF00000000ull;

const std::size_t byte_2_shift = bits_in_byte;
const std::size_t byte_3_shift = 2 * bits_in_byte;
const std::size_t byte_4_shift = 3 * bits_in_byte;
const std::size_t high_word_shift = bits_in_word;


core::result put_word_preserve_order(
    char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const char * v)
{
    const std::size_t size_of_current_buffer = sizes[current_buffer];
    const std::size_t used_in_current_buffer =
        static_cast<std::size_t>(buffer_position - buffers[current_buffer]);
    const std::size_t space_left =
        size_of_current_buffer - used_in_current_buffer;

    core::result res = core::ok;
    if (space_left < bytes_in_word)
    {
        // move to next buffer
        if (current_buffer != num_of_buffers - 1)
        {
            ++current_buffer;
            buffer_position = buffers[current_buffer];
        }
        else
        {
            res = core::not_enough_space;
        }
    }

    if (res == core::ok)
    {
        *reinterpret_cast<unsigned int *>(buffer_position) =
            *reinterpret_cast<const unsigned int *>(v);
        buffer_position += bytes_in_word;
    }
    
    return res;
}

core::result put_word(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const char * v)
{
    const std::size_t size_of_current_buffer = sizes[current_buffer];
    const std::size_t used_in_current_buffer =
        static_cast<std::size_t>(buffer_position - buffers[current_buffer]);
    const std::size_t space_left =
        size_of_current_buffer - used_in_current_buffer;

    core::result res = core::ok;
    if (space_left < bytes_in_word)
    {
        // move to next buffer
        if (current_buffer != num_of_buffers - 1)
        {
            ++current_buffer;
            buffer_position = buffers[current_buffer];
        }
        else
        {
            res = core::not_enough_space;
        }
    }

    if (res == core::ok)
    {
        // place the given word in the target buffer

        const unsigned int word =
            *reinterpret_cast<const unsigned int *>(v);
        *buffer_position++ =
            static_cast<char>(word & byte_1_mask);
        *buffer_position++ =
            static_cast<char>((word & byte_2_mask) >> byte_2_shift);
        *buffer_position++ =
            static_cast<char>((word & byte_3_mask) >> byte_3_shift);
        *buffer_position++ =
            static_cast<char>((word & byte_4_mask) >> byte_4_shift);
    }
    
    return res;
}

core::result get_word_preserve_order(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    char * v)
{
    const std::size_t size_of_current_buffer = sizes[current_buffer];
    const std::size_t used_in_current_buffer =
        buffer_position - buffers[current_buffer];
    const std::size_t space_left =
        size_of_current_buffer - used_in_current_buffer;

    core::result res = core::ok;
    if (space_left < bytes_in_word)
    {
        // move to next buffer
        if (current_buffer != num_of_buffers - 1)
        {
            ++current_buffer;
            buffer_position = buffers[current_buffer];
        }
        else
        {
            res = core::not_enough_space;
        }
    }

    if (res == core::ok)
    {
        // read the word from the source buffer

        *reinterpret_cast<unsigned int *>(v) =
            *reinterpret_cast<const unsigned int *>(buffer_position);
        buffer_position += bytes_in_word;
    }
    
    return res;
}

core::result get_word(const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    char * v)
{
    const std::size_t size_of_current_buffer = sizes[current_buffer];
    const std::size_t used_in_current_buffer =
        buffer_position - buffers[current_buffer];
    const std::size_t space_left =
        size_of_current_buffer - used_in_current_buffer;

    core::result res = core::ok;
    if (space_left < bytes_in_word)
    {
        // move to next buffer
        if (current_buffer != num_of_buffers - 1)
        {
            ++current_buffer;
            buffer_position = buffers[current_buffer];
        }
        else
        {
            res = core::not_enough_space;
        }
    }

    if (res == core::ok)
    {
        // read the word from the source buffer

        const unsigned char * byte_1 =
            reinterpret_cast<const unsigned char *>(buffer_position);
        const unsigned char * byte_2 = byte_1 + 1;
        const unsigned char * byte_3 = byte_2 + 1;
        const unsigned char * byte_4 = byte_3 + 1;

        *reinterpret_cast<unsigned int *>(v) =
            static_cast<unsigned int>(*byte_1) |
            (static_cast<unsigned int>(*byte_2) << byte_2_shift) |
            (static_cast<unsigned int>(*byte_3) << byte_3_shift) |
            (static_cast<unsigned int>(*byte_4) << byte_4_shift);

        buffer_position += bytes_in_word;
    }
    
    return res;
}

core::result put_double_word(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const char * v)
{
    const unsigned long long dw =
        *reinterpret_cast<const unsigned long long *>(v);

    const unsigned int low = dw & low_word_mask;
    const unsigned int high = (dw & high_word_mask) >> high_word_shift;

    core::result res = put_word(
        buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&low));
    if (res == core::ok)
    {
        res = put_word(
            buffers, sizes, num_of_buffers,
            current_buffer, buffer_position,
            reinterpret_cast<const char *>(&high));
    }

    return res;
}

core::result get_double_word(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    char * v)
{
    unsigned int low;
    core::result res = get_word(
        buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<char *>(&low));
    if (res == core::ok)
    {
        unsigned int high;
        res = get_word(
            buffers, sizes, num_of_buffers,
            current_buffer, buffer_position,
            reinterpret_cast<char *>(&high));

        if (res == core::ok)
        {
            *reinterpret_cast<unsigned long long *>(v) =
                (static_cast<unsigned long long>(high) << high_word_shift) |
                low;
        }
    }

    return res;
}

// puts the string without its length
core::result put_raw_string(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const char * value, std::size_t length)
{
    core::result res = core::ok;

    const std::size_t length_full_words = length & round_down_word_mask;
    for (std::size_t i = 0; i != length_full_words; i += bytes_in_word)
    {
        res = put_word_preserve_order(buffers, sizes, num_of_buffers,
            current_buffer, buffer_position, value + i);
        if (res != core::ok)
        {
            break;
        }
    }

    // tail (what could not be stored as full words)
    if (res == core::ok && length != length_full_words)
    {
        char tail[bytes_in_word] = {0};
        for (std::size_t i = length_full_words; i != length; ++i)
        {
            tail[i - length_full_words] = value[i];
        }

        res = put_word_preserve_order(buffers, sizes, num_of_buffers,
            current_buffer, buffer_position, tail);
    }

    return res;
}

// gets the string without its length to already allocated buffer
core::result get_raw_string(const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    char * value, std::size_t length)
{
    core::result res = core::ok;

    const std::size_t length_full_words = length & round_down_word_mask;
    for (std::size_t i = 0; i != length_full_words; i += bytes_in_word)
    {
        res = get_word_preserve_order(buffers, sizes, num_of_buffers,
            current_buffer, buffer_position, value + i);
        if (res != core::ok)
        {
            break;
        }
    }

    // tail (what could not be stored as full words)
    if (res == core::ok && length != length_full_words)
    {
        char tail[bytes_in_word] = {0};
        res = get_word_preserve_order(buffers, sizes, num_of_buffers,
            current_buffer, buffer_position, tail);

        if (res == core::ok)
        {
            for (std::size_t i = length_full_words; i != length; ++i)
            {
                value[i] = tail[i - length_full_words];
            }
        }
    }

    return res;
}

} // unnamed namespace

core::result details::put_integer(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    int value)
{
    return put_word(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&value));
}

core::result details::get_integer(const char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    int & value)
{
    return get_word(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<char *>(&value));
}

core::result details::put_long_long(char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    long long value)
{
    return put_double_word(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&value));
}

core::result details::get_long_long(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    long long & value)
{
    return get_double_word(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<char *>(&value));
}

core::result details::put_double_float(char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    double value)
{
    return put_double_word(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&value));
}

core::result details::get_double_float(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    double & value)
{
    return get_double_word(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position,
        reinterpret_cast<char *>(&value));
}

core::result details::put_string(char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const char * value, std::size_t length)
{
    core::result res = put_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, static_cast<int>(length));

    if (res == core::ok)
    {
        res = put_raw_string(buffers, sizes, num_of_buffers,
            current_buffer, buffer_position,
            value, length);
    }

    return res;
}

core::result details::get_string(const char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    const char * & value, std::size_t & length,
    allocator & alloc)
{
    int raw_length;
    core::result res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, raw_length);

    if (res == core::ok)
    {
        length = static_cast<std::size_t>(raw_length);

        char * new_buffer =
            static_cast<char *>(alloc.allocate(length));
        if (new_buffer != NULL)
        {
            value = new_buffer;

            res = get_raw_string(buffers, sizes, num_of_buffers,
                current_buffer, buffer_position,
                new_buffer, length);

            if (res != core::ok)
            {
                alloc.deallocate(new_buffer);
            }
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

core::result details::put_boolean_array(char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const bool * values, std::size_t length)
{
    core::result res = put_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, static_cast<int>(length));

    if (res == core::ok)
    {
        const std::size_t full_words = length / bits_in_word;

        // first process full words
        for (std::size_t i = 0; i != full_words; ++i)
        {
            char word[bytes_in_word] = {0};

            for (std::size_t j = 0; j != bits_in_word; ++j)
            {
                const std::size_t byte_position = j / bits_in_byte;
                const std::size_t bit_position = j % bits_in_byte;

                if (values[i * bits_in_word + j])
                {
                    word[byte_position] |= 1 << bit_position;
                }
            }

            res = put_word_preserve_order(buffers, sizes, num_of_buffers,
                current_buffer, buffer_position, word);
            if (res != core::ok)
            {
                break;
            }
        }

        // tail (what could not be stored as a full word)
        if (res == core::ok)
        {
            char word[bytes_in_word] = {0};

            const std::size_t already_stored_bits =
                full_words * bits_in_word;

            for (std::size_t j = already_stored_bits; j != length; ++j)
            {
                const std::size_t byte_position =
                    (j - already_stored_bits) / bits_in_byte;
                const std::size_t bit_position = j % bits_in_byte;

                if (values[j])
                {
                    word[byte_position] |= 1 << bit_position;
                }
            }

            res = put_word_preserve_order(buffers, sizes, num_of_buffers,
                current_buffer, buffer_position, word);
        }
    }

    return res;
}

core::result details::get_boolean_array(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    bool * & values, std::size_t & length,
    allocator & alloc)
{
    int raw_length;
    core::result res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, raw_length);

    if (res == core::ok)
    {
        length = static_cast<std::size_t>(raw_length);

        const std::size_t byte_length = length * sizeof(bool);
        bool * new_array = static_cast<bool *>(
            alloc.allocate(byte_length));
        if (new_array != NULL)
        {
            std::memset(new_array, 0, byte_length);

            values = new_array;

            const std::size_t full_words = length / bits_in_word;
            char word[bytes_in_word];

            // first process full words
            for (std::size_t i = 0; i != full_words; ++i)
            {
                res = get_word_preserve_order(
                    buffers, sizes, num_of_buffers,
                    current_buffer, buffer_position, word);
                if (res != core::ok)
                {
                    break;
                }

                for (std::size_t j = 0; j != bits_in_word; ++j)
                {
                    const std::size_t byte_position = j / bits_in_byte;
                    const std::size_t bit_position = j % bits_in_byte;

                    if (word[byte_position] & (1 << bit_position))
                    {
                        values[i * bits_in_word + j] = true;
                    }
                }
            }

            // tail (what could not be read as a full word)
            if (res == core::ok)
            {
                res = get_word_preserve_order(
                    buffers, sizes, num_of_buffers,
                    current_buffer, buffer_position, word);

                if (res == core::ok)
                {
                    const std::size_t already_read_bits =
                        full_words * bits_in_word;

                    for (std::size_t j = already_read_bits;
                         j != length; ++j)
                    {
                        const std::size_t byte_position =
                            (j - already_read_bits) / bits_in_byte;
                        const std::size_t bit_position = j % bits_in_byte;

                        if (word[byte_position] & (1 << bit_position))
                        {
                            values[j] = true;
                        }
                    }
                }
            }

            if (res != core::ok)
            {
                alloc.deallocate(new_array);
            }
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

core::result details::put_integer_array(char * * buffers,
    const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const int * values, std::size_t length)
{
    core::result res = put_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, static_cast<int>(length));

    if (res == core::ok)
    {
        for (std::size_t i = 0; i != length; ++i)
        {
            res = put_integer(buffers, sizes, num_of_buffers,
                current_buffer, buffer_position,
                values[i]);
            if (res != core::ok)
            {
                break;
            }
        }
    }

    return res;
}

core::result details::get_integer_array(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    int * & values, std::size_t & length,
    allocator & alloc)
{
    int raw_length;
    core::result res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, raw_length);

    if (res == core::ok)
    {
        length = static_cast<std::size_t>(raw_length);

        int * new_array = static_cast<int *>(
            alloc.allocate(length * sizeof(int)));
        if (new_array != NULL)
        {
            values = new_array;

            for (std::size_t i = 0; res == core::ok && i != length; ++i)
            {
                res = get_integer(buffers, sizes, num_of_buffers,
                    current_buffer, buffer_position, values[i]);
            }

            if (res != core::ok)
            {
                alloc.deallocate(new_array);
            }
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

core::result details::put_long_long_array(
    char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const long long * values, std::size_t length)
{
    core::result res = put_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, static_cast<int>(length));

    if (res == core::ok)
    {
        for (std::size_t i = 0; i != length; ++i)
        {
            res = put_long_long(buffers, sizes, num_of_buffers,
                current_buffer, buffer_position,
                values[i]);
            if (res != core::ok)
            {
                break;
            }
        }
    }

    return res;
}

core::result details::get_long_long_array(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    long long * & values, std::size_t & length,
    allocator & alloc)
{
    int raw_length;
    core::result res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, raw_length);

    if (res == core::ok)
    {
        length = static_cast<std::size_t>(raw_length);

        long long * new_array = static_cast<long long *>(
            alloc.allocate(length * sizeof(long long)));
        if (new_array != NULL)
        {
            values = new_array;

            for (std::size_t i = 0; res == core::ok && i != length; ++i)
            {
                res = get_long_long(buffers, sizes, num_of_buffers,
                    current_buffer, buffer_position, values[i]);
            }

            if (res != core::ok)
            {
                alloc.deallocate(new_array);
            }
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

core::result details::put_double_float_array(
    char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, char * & buffer_position,
    const double * values, std::size_t length)
{
    core::result res = put_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, static_cast<int>(length));

    if (res == core::ok)
    {
        for (std::size_t i = 0; i != length; ++i)
        {
            res = put_double_float(buffers, sizes, num_of_buffers,
                current_buffer, buffer_position,
                values[i]);
            if (res != core::ok)
            {
                break;
            }
        }
    }

    return res;
}

core::result details::get_double_float_array(
    const char * * buffers, const std::size_t * sizes,
    std::size_t num_of_buffers,
    std::size_t & current_buffer, const char * & buffer_position,
    double * & values, std::size_t & length,
    allocator & alloc)
{
    int raw_length;
    core::result res = get_integer(buffers, sizes, num_of_buffers,
        current_buffer, buffer_position, raw_length);

    if (res == core::ok)
    {
        length = static_cast<std::size_t>(raw_length);

        double * new_array = static_cast<double *>(
            alloc.allocate(length * sizeof(double)));
        if (new_array != NULL)
        {
            values = new_array;

            for (std::size_t i = 0; res == core::ok && i != length; ++i)
            {
                res = get_double_float(buffers, sizes, num_of_buffers,
                    current_buffer, buffer_position, values[i]);
            }

            if (res != core::ok)
            {
                alloc.deallocate(new_array);
            }
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

void details::fill_outgoing_frame_header(
    char * buffer,
    std::size_t message_id, int frame_number,
    std::size_t message_header_size, std::size_t frame_payload_size)
{
    // there is no reason for this function to fail,
    // with the assumption that the provided buffer is large enough
    // for the whole header information (4 words)

    const std::size_t words_in_frame_header = 4;

    char * buffers[1] = { buffer };
    std::size_t buffer_sizes[1] = { words_in_frame_header * bytes_in_word };
    std::size_t current_buffer = 0;
    char * buffer_position = buffer;

    core::result res = put_word(buffers, buffer_sizes, 1,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&message_id));
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    res = put_word(buffers, buffer_sizes, 1,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&frame_number));
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    res = put_word(buffers, buffer_sizes, 1,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&message_header_size));
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    res = put_word(buffers, buffer_sizes, 1,
        current_buffer, buffer_position,
        reinterpret_cast<const char *>(&frame_payload_size));
    if (res != core::ok)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}
