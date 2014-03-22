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

#include "parameter_entry.h"
#include "parameters-details.h"
#include <cstring>

using namespace yami;
using namespace yami::core;

parameter_type parameter_entry::type() const
{
    return e_->type;
}

void parameter_entry::get_name(
    const char * & name, std::size_t & name_length) const
{
    name = e_->name.value();
    name_length = e_->name.name_length;
}

result parameter_entry::get_boolean(bool & value) const
{
    result res = bad_type;
    if (e_->type == boolean)
    {
        value = e_->item.b;
        res = ok;
    }

    return res;
}

result parameter_entry::get_integer(int & value) const
{
    result res = bad_type;
    if (e_->type == integer)
    {
        value = e_->item.i;
        res = ok;
    }

    return res;
}

result parameter_entry::get_long_long(long long & value) const
{
    result res = bad_type;
    if (e_->type == long_long)
    {
        value = e_->item.L;
        res = ok;
    }

    return res;
}

result parameter_entry::get_double_float(double & value) const
{
    result res = bad_type;
    if (e_->type == double_float)
    {
        value = e_->item.d;
        res = ok;
    }

    return res;
}

result parameter_entry::get_string(
    const char * & value, std::size_t & value_length) const
{
    result res = bad_type;
    if (e_->type == string)
    {
        value = e_->item.str.value;
        value_length = e_->item.str.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_binary(
    const void * & value, std::size_t & value_length) const
{
    result res = bad_type;
    if (e_->type == binary)
    {
        value = e_->item.bin.value;
        value_length = e_->item.bin.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_nested_parameters(parameters * & params) const
{
    result res = bad_type;
    if (e_->type == nested_parameters)
    {
        params = e_->item.nested;
        res = ok;
    }

    return res;
}

result parameter_entry::get_boolean_array(
    bool * & values, std::size_t & array_length) const
{
    result res = bad_type;
    if (e_->type == boolean_array)
    {
        values = e_->item.ba.values;
        array_length = e_->item.ba.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_integer_array(
    int * & values, std::size_t & array_length) const
{
    result res = bad_type;
    if (e_->type == integer_array)
    {
        values = e_->item.ia.values;
        array_length = e_->item.ia.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_long_long_array(
    long long * & values, std::size_t & array_length) const
{
    result res = bad_type;
    if (e_->type == long_long_array)
    {
        values = e_->item.La.values;
        array_length = e_->item.La.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_double_float_array(
    double * & values, std::size_t & array_length) const
{
    result res = bad_type;
    if (e_->type == double_float_array)
    {
        values = e_->item.da.values;
        array_length = e_->item.da.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_string_array_length(std::size_t & length) const
{
    result res = bad_type;
    if (e_->type == string_array)
    {
        length = e_->item.sa.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_string_in_array(std::size_t index,
    const char * & value, std::size_t & value_length) const
{
    result res = bad_type;
    if (e_->type == string_array)
    {
        if (index < e_->item.sa.length)
        {
            value = e_->item.sa.values[index].value;
            value_length = e_->item.sa.values[index].length;
            res = ok;
        }
        else
        {
            res = no_such_index;
        }
    }

    return res;
}

result parameter_entry::get_binary_array_length(std::size_t & length) const
{
    result res = bad_type;
    if (e_->type == binary_array)
    {
        length = e_->item.bina.length;
        res = ok;
    }

    return res;
}

result parameter_entry::get_binary_in_array(std::size_t index,
    const void * & value, std::size_t & value_length) const
{
    result res = bad_type;
    if (e_->type == binary_array)
    {
        if (index < e_->item.bina.length)
        {
            value = e_->item.bina.values[index].value;
            value_length = e_->item.bina.values[index].length;
            res = ok;
        }
        else
        {
            res = no_such_index;
        }
    }

    return res;
}
