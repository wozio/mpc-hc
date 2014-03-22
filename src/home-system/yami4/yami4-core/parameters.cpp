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

#include "parameters.h"
#include "fatal_errors.h"
#include "parameter_entry.h"
#include "parameter_iterator.h"
#include "parameters-details.h"
#include <cstring>
#include <new>

using namespace yami;
using namespace yami::core;

parameters::parameters(void * working_area, std::size_t area_size)
    : own_allocator_(), allocator_(own_allocator_),
      uses_private_area_(area_size > 0),
      data_(NULL), num_of_entries_(0)
{
    own_allocator_.set_working_area(working_area, area_size);
}

parameters::parameters(details::allocator & alloc, bool private_area)
    : own_allocator_(), allocator_(alloc),
      uses_private_area_(private_area),
      data_(NULL), num_of_entries_(0)
{
}

parameters::~parameters()
{
    if (uses_private_area_ == false)
    {
        // this object uses global dynamic memory

        clear();
    }
}

void parameters::clear()
{
    details::clear_fsm fsm(*this, allocator_);
    (void) fsm.execute();

    if (data_ != NULL)
    {
        allocator_.deallocate(data_);
        data_ = NULL;

        num_of_entries_ = 0;
    }
}

result parameters::set_boolean(const char * name, std::size_t name_length,
    bool value)
{
    std::size_t index;
    const result res = details::prepare_for_set(data_, num_of_entries_,
        name, name_length, index, allocator_);

    if (res == ok)
    {
        details::entry & e = data_[index];

        e.type = boolean;
        e.item.b = value;
    }

    return res;
}

result parameters::set_boolean(const char * name, bool value)
{
    return set_boolean(name, std::strlen(name), value);
}

result parameters::get_boolean(const char * name, std::size_t name_length,
    bool & value) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == boolean)
        {
            value = e.item.b;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_boolean(const char * name, bool & value) const
{
    return get_boolean(name, std::strlen(name), value);
}

result parameters::set_integer(const char * name, std::size_t name_length,
    int value)
{
    std::size_t index;
    const result res = details::prepare_for_set(data_, num_of_entries_,
        name, name_length, index, allocator_);

    if (res == ok)
    {
        details::entry & e = data_[index];

        e.type = integer;
        e.item.i = value;
    }

    return res;
}

result parameters::set_integer(const char * name, int value)
{
    return set_integer(name, std::strlen(name), value);
}

result parameters::get_integer(const char * name, std::size_t name_length,
    int & value) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == integer)
        {
            value = e.item.i;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_integer(const char * name, int & value) const
{
    return get_integer(name, std::strlen(name), value);
}

result parameters::set_long_long(const char * name, std::size_t name_length,
    long long value)
{
    std::size_t index;
    const result res = details::prepare_for_set(data_, num_of_entries_,
        name, name_length, index, allocator_);

    if (res == ok)
    {
        details::entry & e = data_[index];

        e.type = long_long;
        e.item.L = value;
    }

    return res;
}

result parameters::set_long_long(const char * name, long long value)
{
    return set_long_long(name, std::strlen(name), value);
}

result parameters::get_long_long(const char * name, std::size_t name_length,
    long long & value) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == long_long)
        {
            value = e.item.L;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_long_long(const char * name, long long & value) const
{
    return get_long_long(name, std::strlen(name), value);
}

result parameters::set_double_float(
    const char * name, std::size_t name_length, double value)
{
    std::size_t index;
    const result res = details::prepare_for_set(data_, num_of_entries_,
        name, name_length, index, allocator_);

    if (res == ok)
    {
        details::entry & e = data_[index];

        e.type = double_float;
        e.item.d = value;
    }

    return res;
}

result parameters::set_double_float(const char * name, double value)
{
    return set_double_float(name, std::strlen(name), value);
}

result parameters::get_double_float(
    const char * name, std::size_t name_length, double & value) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == double_float)
        {
            value = e.item.d;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_double_float(const char * name, double & value) const
{
    return get_double_float(name, std::strlen(name), value);
}

result parameters::set_string(const char * name, std::size_t name_length,
    const char * value, std::size_t value_length)
{
    result res;

    char * new_buffer =
        static_cast<char *>(allocator_.allocate(value_length));
    if (new_buffer != NULL)
    {
        std::memcpy(new_buffer, value, value_length);

        res = details::do_set_string(name, name_length,
            new_buffer, value_length,
            data_, num_of_entries_, allocator_, true);

        if (res != ok)
        {
            allocator_.deallocate(new_buffer);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::set_string(const char * name, const char * value)
{
    return set_string(name, std::strlen(name), value, std::strlen(value));
}

result parameters::set_string_shallow(
    const char * name, std::size_t name_length,
    const char * value, std::size_t value_length)
{
    const result res = details::do_set_string(name, name_length,
        value, value_length,
        data_, num_of_entries_, allocator_, false);

    return res;
}

result parameters::set_string_shallow(const char * name, const char * value)
{
    return set_string_shallow(name, std::strlen(name),
        value, std::strlen(value));
}

result parameters::get_string(const char * name, std::size_t name_length,
    const char * & value, std::size_t & value_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == string)
        {
            value = e.item.str.value;
            value_length = e.item.str.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_string(const char * name,
    const char * & value, std::size_t & value_length) const
{
    return get_string(name, std::strlen(name), value, value_length);
}

result parameters::set_binary(const char * name, std::size_t name_length,
    const void * value, std::size_t value_length)
{
    result res;

    void * new_buffer = allocator_.allocate(value_length);
    if (new_buffer != NULL)
    {
        std::memcpy(new_buffer, value, value_length);

        res = details::do_set_binary(name, name_length,
            new_buffer, value_length,
            data_, num_of_entries_, allocator_, true);

        if (res != ok)
        {
            allocator_.deallocate(new_buffer);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::set_binary(const char * name,
    const void * value, std::size_t value_length)
{
    return set_binary(name, std::strlen(name), value, value_length);
}

result parameters::set_binary_shallow(
    const char * name, std::size_t name_length,
    const void * value, std::size_t value_length)
{
    const result res = details::do_set_binary(name, name_length,
        value, value_length,
        data_, num_of_entries_, allocator_, false);

    return res;
}

result parameters::set_binary_shallow(const char * name,
    const void * value, std::size_t value_length)
{
    return set_binary_shallow(name, std::strlen(name),
        value, value_length);
}

result parameters::get_binary(const char * name, std::size_t name_length,
    const void * & value, std::size_t & value_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == binary)
        {
            value = e.item.bin.value;
            value_length = e.item.bin.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_binary(const char * name,
    const void * & value, std::size_t & value_length) const
{
    return get_binary(name, std::strlen(name), value, value_length);
}

result parameters::set_boolean_array(
    const char * name, std::size_t name_length,
    const bool * values, std::size_t array_length)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(bool);
    bool * new_array =
        static_cast<bool *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        std::memcpy(new_array, values, raw_length);

        res = details::do_set_boolean_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res != ok)
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::set_boolean_array(const char * name,
    const bool * values, std::size_t array_length)
{
    return set_boolean_array(name, std::strlen(name), values, array_length);
}

result parameters::set_boolean_array_shallow(
    const char * name, std::size_t name_length,
    const bool * values, std::size_t array_length)
{
    const result res = details::do_set_boolean_array(name, name_length,
        values, array_length,
        data_, num_of_entries_, allocator_, false);

    return res;
}

result parameters::set_boolean_array_shallow(const char * name,
    const bool * values, std::size_t array_length)
{
    return set_boolean_array_shallow(name, std::strlen(name),
        values, array_length);
}

result parameters::create_boolean_array(
    const char * name, std::size_t name_length,
    std::size_t array_length, bool * & array)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(bool);
    bool * new_array =
        static_cast<bool *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        res = details::do_set_boolean_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res == ok)
        {
            array = new_array;
        }
        else
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::get_boolean_array(
    const char * name, std::size_t name_length,
    bool * & values, std::size_t & array_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == boolean_array)
        {
            values = e.item.ba.values;
            array_length = e.item.ba.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_boolean_array(const char * name,
    bool * & values, std::size_t & array_length) const
{
    return get_boolean_array(name, std::strlen(name), values, array_length);
}

result parameters::set_integer_array(
    const char * name, std::size_t name_length,
    const int * values, std::size_t array_length)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(int);
    int * new_array =
        static_cast<int *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        std::memcpy(new_array, values, raw_length);

        res = details::do_set_integer_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res != ok)
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::set_integer_array(const char * name,
    const int * values, std::size_t array_length)
{
    return set_integer_array(name, std::strlen(name), values, array_length);
}

result parameters::set_integer_array_shallow(
    const char * name, std::size_t name_length,
    const int * values, std::size_t array_length)
{
    const result res = details::do_set_integer_array(name, name_length,
        values, array_length,
        data_, num_of_entries_, allocator_, false);

    return res;
}

result parameters::set_integer_array_shallow(const char * name,
    const int * values, std::size_t array_length)
{
    return set_integer_array_shallow(name, std::strlen(name),
        values, array_length);
}

result parameters::create_integer_array(
    const char * name, std::size_t name_length,
    std::size_t array_length, int * & array)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(int);
    int * new_array =
        static_cast<int *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        res = details::do_set_integer_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res == ok)
        {
            array = new_array;
        }
        else
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::get_integer_array(
    const char * name, std::size_t name_length,
    int * & values, std::size_t & array_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == integer_array)
        {
            values = e.item.ia.values;
            array_length = e.item.ia.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_integer_array(const char * name,
    int * & values, std::size_t & array_length) const
{
    return get_integer_array(name, std::strlen(name), values, array_length);
}

result parameters::set_long_long_array(
    const char * name, std::size_t name_length,
    const long long * values, std::size_t array_length)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(long long);
    long long * new_array =
        static_cast<long long *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        std::memcpy(new_array, values, raw_length);

        res = details::do_set_long_long_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res != ok)
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::set_long_long_array(const char * name,
    const long long * values, std::size_t array_length)
{
    return set_long_long_array(name, std::strlen(name), values, array_length);
}

result parameters::set_long_long_array_shallow(
    const char * name, std::size_t name_length,
    const long long * values, std::size_t array_length)
{
    const result res = details::do_set_long_long_array(name, name_length,
        values, array_length,
        data_, num_of_entries_, allocator_, false);

    return res;
}

result parameters::set_long_long_array_shallow(const char * name,
    const long long * values, std::size_t array_length)
{
    return set_long_long_array_shallow(name, std::strlen(name),
        values, array_length);
}

result parameters::create_long_long_array(
    const char * name, std::size_t name_length,
    std::size_t array_length, long long * & array)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(long long);
    long long * new_array =
        static_cast<long long *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        res = details::do_set_long_long_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res == ok)
        {
            array = new_array;
        }
        else
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::get_long_long_array(
    const char * name, std::size_t name_length,
    long long * & values, std::size_t & array_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == long_long_array)
        {
            values = e.item.La.values;
            array_length = e.item.La.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_long_long_array(const char * name,
    long long * & values, std::size_t & array_length) const
{
    return get_long_long_array(name, std::strlen(name), values, array_length);
}

result parameters::set_double_float_array(
    const char * name, std::size_t name_length,
    const double * values, std::size_t array_length)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(double);
    double * new_array =
        static_cast<double *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        std::memcpy(new_array, values, raw_length);

        res = details::do_set_double_float_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res != ok)
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::set_double_float_array(const char * name,
    const double * values, std::size_t array_length)
{
    return set_double_float_array(
        name, std::strlen(name), values, array_length);
}

result parameters::set_double_float_array_shallow(
    const char * name, std::size_t name_length,
    const double * values, std::size_t array_length)
{
    const result res = details::do_set_double_float_array(name, name_length,
        values, array_length,
        data_, num_of_entries_, allocator_, false);

    return res;
}

result parameters::set_double_float_array_shallow(const char * name,
    const double * values, std::size_t array_length)
{
    return set_double_float_array_shallow(name, std::strlen(name),
        values, array_length);
}

result parameters::create_double_float_array(
    const char * name, std::size_t name_length,
    std::size_t array_length, double * & array)
{
    result res;

    const std::size_t raw_length = array_length * sizeof(double);
    double * new_array =
        static_cast<double *>(allocator_.allocate(raw_length));
    if (new_array != NULL)
    {
        res = details::do_set_double_float_array(name, name_length,
            new_array, array_length,
            data_, num_of_entries_, allocator_, true);

        if (res == ok)
        {
            array = new_array;
        }
        else
        {
            allocator_.deallocate(new_array);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::get_double_float_array(
    const char * name, std::size_t name_length,
    double * & values, std::size_t & array_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == double_float_array)
        {
            values = e.item.da.values;
            array_length = e.item.da.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_double_float_array(const char * name,
    double * & values, std::size_t & array_length) const
{
    return get_double_float_array(
        name, std::strlen(name), values, array_length);
}

result parameters::create_string_array(
    const char * name, std::size_t name_length,
    std::size_t array_length)
{
    std::size_t dummy_index;
    const result res = details::do_create_string_array(
        name, name_length, array_length, dummy_index,
        data_, num_of_entries_, allocator_);

    return res;
}

result parameters::create_string_array(
    const char * name, std::size_t array_length)
{
    return create_string_array(name, std::strlen(name), array_length);
}

result parameters::set_string_in_array(
    const char * name, std::size_t name_length,
    std::size_t index, const char * value, std::size_t value_length)
{
    result res;
    const std::size_t item_index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (item_index != num_of_entries_)
    {
        res = details::do_set_string_in_array(item_index, index,
            value, value_length, data_, allocator_);
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::set_string_in_array(const char * name,
    std::size_t index, const char * value)
{
    return set_string_in_array(name, std::strlen(name),
        index, value, std::strlen(value));
}

result parameters::get_string_array_length(
    const char * name, std::size_t name_length,
    std::size_t & array_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == string_array)
        {
            array_length = e.item.sa.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_string_in_array(
    const char * name, std::size_t name_length,
    std::size_t index,
    const char * & value, std::size_t & value_length) const
{
    result res;
    const std::size_t item_index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (item_index != num_of_entries_)
    {
        const details::entry & e = data_[item_index];
        if (e.type == string_array)
        {
            if (index < e.item.sa.length)
            {
                value = e.item.sa.values[index].value;
                value_length = e.item.sa.values[index].length;
                res = ok;
            }
            else
            {
                res = no_such_index;
            }
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_string_in_array(const char * name, std::size_t index,
    const char * & value, std::size_t & value_length) const
{
    return get_string_in_array(name, std::strlen(name),
        index, value, value_length);
}

result parameters::create_binary_array(
    const char * name, std::size_t name_length,
    std::size_t array_length)
{
    std::size_t dummy_index;
    const result res = details::do_create_binary_array(
        name, name_length, array_length, dummy_index,
        data_, num_of_entries_, allocator_);

    return res;
}

result parameters::create_binary_array(
    const char * name, std::size_t array_length)
{
    return create_binary_array(name, std::strlen(name), array_length);
}

result parameters::set_binary_in_array(
    const char * name, std::size_t name_length,
    std::size_t index, const void * value, std::size_t value_length)
{
    result res;
    const std::size_t item_index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (item_index != num_of_entries_)
    {
        res = details::do_set_binary_in_array(item_index, index,
            value, value_length, data_, allocator_);
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::set_binary_in_array(const char * name,
    std::size_t index, const void * value, std::size_t value_length)
{
    return set_binary_in_array(name, std::strlen(name),
        index, value, value_length);
}

result parameters::get_binary_array_length(
    const char * name, std::size_t name_length,
    std::size_t & array_length) const
{
    result res;
    const std::size_t index = details::find_entry(data_, num_of_entries_,
        name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == binary_array)
        {
            array_length = e.item.bina.length;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_binary_in_array(
    const char * name, std::size_t name_length,
    std::size_t index,
    const void * & value, std::size_t & value_length) const
{
    result res;
    const std::size_t item_index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (item_index != num_of_entries_)
    {
        const details::entry & e = data_[item_index];
        if (e.type == binary_array)
        {
            if (index < e.item.bina.length)
            {
                value = e.item.bina.values[index].value;
                value_length = e.item.bina.values[index].length;
                res = ok;
            }
            else
            {
                res = no_such_index;
            }
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_binary_in_array(const char * name, std::size_t index,
    const void * & value, std::size_t & value_length) const
{
    return get_binary_in_array(name, std::strlen(name),
        index, value, value_length);
}

result parameters::create_nested_parameters(
    const char * name, std::size_t name_length, parameters * & params)
{
    result res;

    void * new_buffer = allocator_.allocate(sizeof(parameters));
    if (new_buffer != NULL)
    {
        std::size_t index;
        res = details::prepare_for_set(data_, num_of_entries_,
            name, name_length, index, allocator_);

        if (res == ok)
        {
            parameters * nested =
                new (new_buffer) parameters(allocator_, uses_private_area_);

            details::entry & e = data_[index];

            e.type = nested_parameters;
            e.item.nested = nested;

            params = nested;
        }
        else
        {
            allocator_.deallocate(new_buffer);
        }
    }
    else
    {
        res = no_memory;
    }

    return res;
}

result parameters::create_nested_parameters(
    const char * name, parameters * & params)
{
    return create_nested_parameters(name, std::strlen(name), params);
}

result parameters::get_nested_parameters(
    const char * name, std::size_t name_length, parameters * & params) const
{
    result res;
    const std::size_t index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (index != num_of_entries_)
    {
        const details::entry & e = data_[index];
        if (e.type == nested_parameters)
        {
            params = e.item.nested;
            res = ok;
        }
        else
        {
            res = bad_type;
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_nested_parameters(
    const char * name, parameters * & params) const
{
    return get_nested_parameters(name, std::strlen(name), params);
}

result parameters::lock(const char * name, std::size_t name_length,
    long long key)
{
    result res;
    const std::size_t index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (index != num_of_entries_)
    {
        details::entry & e = data_[index];
        if (e.type != nested_parameters)
        {
            // non-nested elements can be locked directly
            res = e.lock(key);
        }
        else
        {
            // nested parameters are locked "deeply" with the state machine

            details::lock_fsm fsm(*this, true, key);
            res = fsm.execute();
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::lock(const char * name, long long key)
{
    return lock(name, std::strlen(name), key);
}

result parameters::unlock(const char * name, std::size_t name_length,
    long long key)
{
    result res;
    const std::size_t index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (index != num_of_entries_)
    {
        details::entry & e = data_[index];
        if (e.type != nested_parameters)
        {
            // non-nested elements can be unlocked directly
            res = e.unlock(key);
        }
        else
        {
            // nested parameters are unlocked "deeply" with the state machine

            details::lock_fsm fsm(*this, false, key);
            res = fsm.execute();
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::unlock(const char * name, long long key)
{
    return unlock(name, std::strlen(name), key);
}

result parameters::is_locked(const char * name, std::size_t name_length,
    bool & value) const
{
    result res;
    const std::size_t index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (index != num_of_entries_)
    {
        details::entry & e = data_[index];
        value = e.locked;

        res = ok;
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::is_locked(const char * name, bool & value) const
{
    return is_locked(name, std::strlen(name), value);
}

result parameters::remove(const char * name, std::size_t name_length)
{
    result res;
    const std::size_t index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (index != num_of_entries_)
    {
        details::entry & e = data_[index];

        bool locked;
        res = e.is_locked_or_contains_locked(locked);

        if (res == ok)
        {
            if (locked == false)
            {
                e.clear_name(allocator_);
                e.clear_item(allocator_);
                e.type = unused;
            }
            else
            {
                // this entry is either locked or is a nested entry
                // that contains some locked entry
                // - cannot be removed as a whole
                
                res = bad_state;
            }
        }
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::remove(const char * name)
{
    return remove(name, std::strlen(name));
}

std::size_t parameters::size() const
{
    std::size_t count = 0;
    for (std::size_t i = 0; i != num_of_entries_; ++i)
    {
        if (data_[i].type != unused)
        {
            ++count;
        }
    }

    return count;
}

result parameters::get_type(const char * name, std::size_t name_length,
    parameter_type & t) const
{
    result res;
    const std::size_t index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (index != num_of_entries_)
    {
        t = data_[index].type;
        res = ok;
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::get_type(const char * name, parameter_type & t) const
{
    return get_type(name, std::strlen(name), t);
}

result parameters::get_iterator(parameter_iterator & it) const
{
    result res;

    std::size_t first_used_index = num_of_entries_;
    for (std::size_t i = 0; i != num_of_entries_; ++i)
    {
        if (data_[i].type != unused)
        {
            first_used_index = i;
            break;
        }
    }

    if (first_used_index != num_of_entries_)
    {
        it.data_ = data_;
        it.current_index_ = first_used_index;
        it.num_of_entries_ = num_of_entries_;
        it.allocator_ = &allocator_;
        res = ok;
    }
    else
    {
        res = no_entries;
    }

    return res;
}

result parameters::find(const char * name, std::size_t name_length,
    parameter_entry & entry) const
{
    result res;
    const std::size_t index = details::find_entry(
        data_, num_of_entries_, name, name_length);
    if (index != num_of_entries_)
    {
        entry.e_ = data_ + index;
        res = ok;
    }
    else
    {
        res = no_such_name;
    }

    return res;
}

result parameters::find(const char * name, parameter_entry & entry) const
{
    return find(name, std::strlen(name), entry);
}

result parameters::merge_from(const parameters & other)
{
    // merge is performed as serialize of other
    // followed by deserialize into this

    result res = ok;

    if (other.size() != 0)
    {
        std::size_t buffer_size;
        res = other.get_serialize_buffer_size(buffer_size);
        if (res == ok)
        {
            char * buffer = static_cast<char *>(
                allocator_.allocate(buffer_size));
            if (buffer != NULL)
            {
                res = other.serialize(&buffer, &buffer_size, 1);
                if (res == ok)
                {
                    const char * buffers[1];
                    buffers[0] = buffer;
                    std::size_t buffer_sizes[1];
                    buffer_sizes[0] = buffer_size;

                    res = deserialize(buffers, buffer_sizes, 1);
                }

                allocator_.deallocate(buffer);
            }
            else
            {
                res = no_memory;
            }
        }
    }

    return res;
}

result parameters::get_serialize_buffer_size(std::size_t & buffer_size) const
{
    // the serialized format is:
    // 1. number of entries
    // 2. for each entry:
    //    1. name length
    //    2. name
    //    3. type
    //    4. value
    // depending on type, value is:
    // - boolean: 0 or 1 in 4 bytes
    // - integer: as is 4 bytets
    // - long_long: as is 8 bytes
    // - double_float: as is 8 bytes
    // - string: length followed by value
    // - boolean_array: length followed by packed (per bit) values
    // - integer_array: length followed by values
    // - long_long_array: length followed by values
    // - double_float_array: length followed by values
    // - string_array: length followed by values as for string
    // - nested: as the whole

    details::get_serialize_buffer_size_fsm fsm(*this);

    const result res = fsm.execute();
    if (res == ok)
    {
        buffer_size = fsm.size();
    }

    return res;
}

result parameters::serialize(char * * buffers,
    const std::size_t * buffer_sizes,
    std::size_t num_of_buffers) const
{
    details::serialize_fsm fsm(*this, buffers, buffer_sizes, num_of_buffers);

    return fsm.execute();
}

result parameters::deserialize(
    const char * * buffers, const std::size_t * buffer_sizes,
    std::size_t num_of_buffers)
{
    details::deserialize_fsm fsm(*this,
        buffers, buffer_sizes, num_of_buffers, allocator_);

    return fsm.execute();
}

void parameters::dump(
    details::dump_sink & sink, std::size_t indent_length) const
{
    for (std::size_t i = 0; i != num_of_entries_; ++i)
    {
        bool add_newline = true;
        sink.indent(indent_length);
        sink.dump("entry ");
        sink.dump(i);
        sink.dump(":\n");
        switch (data_[i].type)
        {
        case unused:
            sink.indent(indent_length);
            sink.dump("unused");
            break;
        case boolean:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("boolean: ");
            sink.dump(data_[i].item.b);
            break;
        case integer:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("integer: ");
            sink.dump(data_[i].item.i);
            break;
        case long_long:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("long_long: ");
            sink.dump(data_[i].item.L);
            break;
        case double_float:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("double: ");
            sink.dump(data_[i].item.d);
            break;
        case string:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("string: ");
            sink.dump(data_[i].item.str.value,
                data_[i].item.str.length);
            break;
        case binary:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("binary of length ");
            sink.dump(data_[i].item.bin.length);
            break;
        case boolean_array:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("boolean array:");
            for (std::size_t j = 0; j != data_[i].item.ba.length; ++j)
            {
                sink.dump(" ");
                sink.dump(data_[i].item.ba.values[j]);
            }
            break;
        case integer_array:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("integer array:");
            for (std::size_t j = 0; j != data_[i].item.ia.length; ++j)
            {
                sink.dump(" ");
                sink.dump(data_[i].item.ia.values[j]);
            }
            break;
        case long_long_array:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("long_long array:");
            for (std::size_t j = 0; j != data_[i].item.La.length; ++j)
            {
                sink.dump(" ");
                sink.dump(data_[i].item.La.values[j]);
            }
            break;
        case double_float_array:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("double array:");
            for (std::size_t j = 0; j != data_[i].item.da.length; ++j)
            {
                sink.dump(" ");
                sink.dump(data_[i].item.da.values[j]);
            }
            break;
        case string_array:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("string array:");
            for (std::size_t j = 0; j != data_[i].item.sa.length; ++j)
            {
                sink.dump(" ");
                sink.dump(data_[i].item.sa.values[j].value,
                    data_[i].item.sa.values[j].length);
            }
            break;
        case binary_array:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("binary array of length ");
            sink.dump(data_[i].item.bina.length);
            break;
        case nested_parameters:
            sink.indent(indent_length);
            sink.dump("name: ");
            data_[i].name.dump(sink);
            sink.dump("\n");
            sink.indent(indent_length);
            sink.dump("nested parameters:\n");
            data_[i].item.nested->dump(sink, indent_length + 2);
            add_newline = false;
            break;
        default:
            details::fatal_failure(__FILE__, __LINE__);
        }

        if (add_newline)
        {
            sink.dump("\n");
        }
    }
}
