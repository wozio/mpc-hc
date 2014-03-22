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
#include "details.h"

using namespace yami;

parameter_type parameter_entry::type() const
{
    return static_cast<parameter_type>(
        static_cast<int>(entry_.type()));
}

std::string parameter_entry::name() const
{
    const char * name_value;
    std::size_t name_length;
    entry_.get_name(name_value, name_length);
    return name_value != NULL ?
        std::string(name_value, name_length) : std::string();
}

bool parameter_entry::get_boolean() const
{
    bool value;
    details::translate_result_to_exception(
        entry_.get_boolean(value));
    return value;
}

int parameter_entry::get_integer() const
{
    int value;
    details::translate_result_to_exception(
        entry_.get_integer(value));
    return value;
}

long long parameter_entry::get_long_long() const
{
    long long value;
    details::translate_result_to_exception(
        entry_.get_long_long(value));
    return value;
}

double long parameter_entry::get_double_float() const
{
    double value;
    details::translate_result_to_exception(
        entry_.get_double_float(value));
    return value;
}

std::string parameter_entry::get_string() const
{
    const char * value;
    std::size_t value_length;
    details::translate_result_to_exception(
        entry_.get_string(value, value_length));
    return value != NULL ?
        std::string(value, value_length) : std::string();
}

const char * parameter_entry::get_string(std::size_t & length) const
{
    const char * value;
    details::translate_result_to_exception(
        entry_.get_string(value, length));
    return value;
}

const void * parameter_entry::get_binary(std::size_t & length) const
{
    const void * value;
    details::translate_result_to_exception(
        entry_.get_binary(value, length));
    return value;
}

core::parameters * parameter_entry::get_nested_parameters() const
{
    core::parameters * nested;
    details::translate_result_to_exception(
        entry_.get_nested_parameters(nested));
    return nested;
}

bool * parameter_entry::get_boolean_array(std::size_t & array_length) const
{
    bool * values;
    details::translate_result_to_exception(
        entry_.get_boolean_array(values, array_length));
    return values;
}

int * parameter_entry::get_integer_array(std::size_t & array_length) const
{
    int * values;
    details::translate_result_to_exception(
        entry_.get_integer_array(values, array_length));
    return values;
}

long long * parameter_entry::get_long_long_array(
    std::size_t & array_length) const
{
    long long * values;
    details::translate_result_to_exception(
        entry_.get_long_long_array(values, array_length));
    return values;
}

double * parameter_entry::get_double_float_array(
    std::size_t & array_length) const
{
    double * values;
    details::translate_result_to_exception(
        entry_.get_double_float_array(values, array_length));
    return values;
}

std::size_t parameter_entry::get_string_array_length() const
{
    std::size_t array_length;
    details::translate_result_to_exception(
        entry_.get_string_array_length(array_length));
    return array_length;
}

std::string parameter_entry::get_string_in_array(std::size_t index) const
{
    const char * value;
    std::size_t value_length;
    details::translate_result_to_exception(
        entry_.get_string_in_array(index, value, value_length));
    return value != NULL ?
        std::string(value, value_length) : std::string();
}

const char * parameter_entry::get_string_in_array(std::size_t index,
    std::size_t & value_length) const
{
    const char * value;
    details::translate_result_to_exception(
        entry_.get_string_in_array(index, value, value_length));
    return value;
}

std::size_t parameter_entry::get_binary_array_length() const
{
    std::size_t array_length;
    details::translate_result_to_exception(
        entry_.get_binary_array_length(array_length));
    return array_length;
}

const void * parameter_entry::get_binary_in_array(std::size_t index,
    std::size_t & value_length) const
{
    const void * value;
    details::translate_result_to_exception(
        entry_.get_binary_in_array(index, value, value_length));
    return value;
}
