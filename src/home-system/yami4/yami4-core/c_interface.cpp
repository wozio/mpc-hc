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

#include "agent.h"
#include "channel_descriptor.h"
#include "core.h"
#include "fatal_errors.h"
#include "parameter_entry.h"
#include "parameter_iterator.h"
#include "parameters.h"
#include "parameters-details.h"
#include "raw_buffer_data_source.h"
#include <new>

namespace // unnamed
{

int translate_result(yami::core::result res)
{
    int translated = 0; // dummy initialization
    switch (res)
    {
    case yami::core::ok:               translated = 0; break;
    case yami::core::no_such_name:     translated = 1; break;
    case yami::core::bad_type:         translated = 2; break;
    case yami::core::no_such_index:    translated = 3; break;
    case yami::core::no_memory:        translated = 4; break;
    case yami::core::nesting_too_deep: translated = 5; break;
    case yami::core::not_enough_space: translated = 6; break;
    case yami::core::no_entries:       translated = 7; break;
    case yami::core::unexpected_value: translated = 8; break;
    case yami::core::bad_protocol:     translated = 9; break;
    case yami::core::io_error:         translated = 10; break;
    case yami::core::timed_out:        translated = 11; break;
    case yami::core::channel_closed:   translated = 12; break;
    case yami::core::bad_state:        translated = 13; break;
    default:
        yami::details::fatal_failure(__FILE__, __LINE__);
    }

    return translated;
}

template <typename To, typename From>
To forced_cast(From f)
{
    union
    {
        From f;
        To t;
    } converter;

    converter.f = f;
    return converter.t;
}

} // unnamed namespace

extern "C"
{

extern const std::size_t sizeof_parameters = sizeof(yami::core::parameters);
extern const std::size_t sizeof_parameter_iterator =
    sizeof(yami::core::parameter_iterator);
extern const std::size_t sizeof_parameter_entry =
    sizeof(yami::core::parameter_entry);

extern const std::size_t sizeof_raw_buffer_data_source =
    sizeof(yami::core::raw_buffer_data_source);

extern const std::size_t sizeof_agent = sizeof(yami::core::agent);

void register_fatal_error_handler(void * handler)
{
    yami::core::fatal_error_function custom_handler =
        forced_cast<yami::core::fatal_error_function>(handler);

    yami::core::register_fatal_error_handler(custom_handler);
}

void set_bool_in_array(void * a, std::size_t i, int value)
{
    bool * array = static_cast<bool *>(a);
    array[i] = value != 0;
}

int get_bool_from_array(const void * a, std::size_t i)
{
    const bool * array = static_cast<const bool *>(a);
    return array[i] ? 1 : 0;
}

void set_int_in_array(void * a, std::size_t i, int value)
{
    int * array = static_cast<int *>(a);
    array[i] = value;
}

int get_int_from_array(const void * a, std::size_t i)
{
    const int * array = static_cast<const int *>(a);
    return array[i];
}

void set_long_long_in_array(void * a, std::size_t i, long long value)
{
    long long * array = static_cast<long long *>(a);
    array[i] = value;
}

long long get_long_long_from_array(const void * a, std::size_t i)
{
    const long long * array = static_cast<const long long *>(a);
    return array[i];
}

void set_double_in_array(void * a, std::size_t i, double value)
{
    double * array = static_cast<double *>(a);
    array[i] = value;
}

double get_double_from_array(const void * a, std::size_t i)
{
    const double * array = static_cast<const double *>(a);
    return array[i];
}

void parameters_create(void * p, void * working_area, std::size_t size)
{
    new (p) yami::core::parameters(working_area, size);
}

void destroy_parameters(void * p)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    params->~parameters();
}

int parameters_set_boolean(void * p,
    const char * name, std::size_t name_length, int value)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_boolean(name, name_length, value != 0));
}

int parameters_get_boolean(const void * p,
    const char * name, std::size_t name_length, int * value)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    bool v;
    const yami::core::result res = params->get_boolean(name, name_length, v);
    if (res == yami::core::ok)
    {
        *value = v ? 1 : 0;
    }

    return translate_result(res);
}

int parameters_set_integer(void * p,
    const char * name, std::size_t name_length, int value)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_integer(name, name_length, value));
}

int parameters_get_integer(const void * p,
    const char * name, std::size_t name_length, int * value)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(params->get_integer(name, name_length, *value));
}

int parameters_set_long_long(void * p,
    const char * name, std::size_t name_length, long long value)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_long_long(name, name_length, value));
}

int parameters_get_long_long(const void * p,
    const char * name, std::size_t name_length, long long * value)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(params->get_long_long(name, name_length, *value));
}

int parameters_set_double_float(void * p,
    const char * name, std::size_t name_length, double value)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_double_float(name, name_length, value));
}

int parameters_get_double_float(const void * p,
    const char * name, std::size_t name_length, double * value)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->get_double_float(name, name_length, *value));
}

int parameters_set_string(void * p,
    const char * name, std::size_t name_length,
    const char * value, std::size_t value_length)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_string(name, name_length, value, value_length));
}

int parameters_get_string(const void * p,
    const char * name, std::size_t name_length,
    const char * * value, std::size_t * value_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->get_string(name, name_length, *value, *value_length));
}

int parameters_set_binary(void * p,
    const char * name, std::size_t name_length,
    const void * value, std::size_t value_length)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_binary(name, name_length, value, value_length));
}

int parameters_get_binary(const void * p,
    const char * name, std::size_t name_length,
    const void * * value, std::size_t * value_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->get_binary(name, name_length, *value, *value_length));
}

int parameters_create_boolean_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t array_length, void * * array)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    bool * new_array;
    const yami::core::result res =
        params->create_boolean_array(name, name_length,
            array_length, new_array);

    if (res == yami::core::ok)
    {
        *array = new_array;
    }

    return translate_result(res);
}

int parameters_get_boolean_array_length(const void * p,
    const char * name, std::size_t name_length, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    bool * array;
    return translate_result(
        params->get_boolean_array(name, name_length, array, *array_length));
}

int parameters_get_boolean_array(const void * p,
    const char * name, std::size_t name_length,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    bool * internal_array;
    const yami::core::result res =
        params->get_boolean_array(
            name, name_length, internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameters_create_integer_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t array_length, void * * array)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    int * new_array;
    const yami::core::result res =
        params->create_integer_array(name, name_length,
            array_length, new_array);

    if (res == yami::core::ok)
    {
        *array = new_array;
    }

    return translate_result(res);
}

int parameters_get_integer_array_length(const void * p,
    const char * name, std::size_t name_length, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    int * array;
    return translate_result(
        params->get_integer_array(name, name_length, array, *array_length));
}

int parameters_get_integer_array(const void * p,
    const char * name, std::size_t name_length,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    int * internal_array;
    const yami::core::result res =
        params->get_integer_array(
            name, name_length, internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameters_create_long_long_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t array_length, void * * array)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    long long * new_array;
    const yami::core::result res =
        params->create_long_long_array(name, name_length,
            array_length, new_array);

    if (res == yami::core::ok)
    {
        *array = new_array;
    }

    return translate_result(res);
}

int parameters_get_long_long_array_length(const void * p,
    const char * name, std::size_t name_length, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    long long * array;
    return translate_result(
        params->get_long_long_array(name, name_length, array, *array_length));
}

int parameters_get_long_long_array(const void * p,
    const char * name, std::size_t name_length,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    long long * internal_array;
    const yami::core::result res =
        params->get_long_long_array(
            name, name_length, internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameters_create_double_float_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t array_length, void * * array)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    double * new_array;
    const yami::core::result res =
        params->create_double_float_array(name, name_length,
            array_length, new_array);

    if (res == yami::core::ok)
    {
        *array = new_array;
    }

    return translate_result(res);
}

int parameters_get_double_float_array_length(const void * p,
    const char * name, std::size_t name_length, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    double * array;
    return translate_result(
        params->get_double_float_array(
            name, name_length, array, *array_length));
}

int parameters_get_double_float_array(const void * p,
    const char * name, std::size_t name_length,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    double * internal_array;
    const yami::core::result res =
        params->get_double_float_array(
            name, name_length, internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameters_create_string_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t array_length)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->create_string_array(name, name_length, array_length));
}

int parameters_get_string_array_length(const void * p,
    const char * name, std::size_t name_length, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->get_string_array_length(
            name, name_length, *array_length));
}

int parameters_set_string_in_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t index, const char * value, std::size_t value_length)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_string_in_array(name, name_length, index,
            value, value_length));
}

int parameters_get_string_in_array(const void * p,
    const char * name, std::size_t name_length,
    std::size_t index,
    const char * * value, std::size_t * value_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->get_string_in_array(name, name_length, index,
            *value, *value_length));
}

int parameters_create_binary_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t array_length)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->create_binary_array(name, name_length, array_length));
}

int parameters_get_binary_array_length(const void * p,
    const char * name, std::size_t name_length, std::size_t * array_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->get_binary_array_length(
            name, name_length, *array_length));
}

int parameters_set_binary_in_array(void * p,
    const char * name, std::size_t name_length,
    std::size_t index, const void * value, std::size_t value_length)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->set_binary_in_array(name, name_length, index,
            value, value_length));
}

int parameters_get_binary_in_array(const void * p,
    const char * name, std::size_t name_length,
    std::size_t index,
    const void * * value, std::size_t * value_length)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->get_binary_in_array(name, name_length, index,
            *value, *value_length));
}

int parameters_create_nested(void * p,
    const char * name, std::size_t name_length,
    void * * nested)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    yami::core::parameters * temp;
    const yami::core::result res =
        params->create_nested_parameters(name, name_length, temp);
    if (res == yami::core::ok)
    {
        *nested = temp;
    }

    return translate_result(res);
}

int parameters_get_nested(const void * p,
    const char * name, std::size_t name_length, void * * nested)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    yami::core::parameters * tmp;
    const yami::core::result res =
        params->get_nested_parameters(name, name_length, tmp);
    if (res == yami::core::ok)
    {
        *nested = tmp;
    }

    return translate_result(res);
}

int parameters_lock(void * p,
    const char * name, std::size_t name_length, long long key)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(params->lock(name, name_length, key));
}

int parameters_unlock(void * p,
    const char * name, std::size_t name_length, long long key)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(params->unlock(name, name_length, key));
}

int parameters_is_locked(void * p, const char * name, std::size_t name_length,
    int * value)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    bool v;
    const yami::core::result res =
        params->is_locked(name, name_length, v);

    if (res == yami::core::ok)
    {
        *value = v ? 1 : 0;
    }

    return translate_result(res);
}

std::size_t parameters_size(const void * p)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return params->size();
}

int parameters_remove(void * p, const char * name, std::size_t name_length)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(params->remove(name, name_length));
}

void parameters_clear(void * p)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    params->clear();
}

int parameters_get_type(const void * p,
    const char * name, std::size_t name_length,
    int * type)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    yami::core::parameter_type t;
    const yami::core::result res = params->get_type(name, name_length, t);
    if (res == yami::core::ok)
    {
        *type = yami::details::type_code(t);
    }

    return translate_result(res);
}

void parameters_get_iterator(const void * p, void * it, int * result)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    yami::core::parameter_iterator * iterator =
        static_cast<yami::core::parameter_iterator *>(it);

    const yami::core::result r = params->get_iterator(*iterator);
    *result = translate_result(r);
}

void parameters_find(const void * p,
    const char * name, std::size_t name_length, void * e, int * result)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    yami::core::parameter_entry * entry =
        static_cast<yami::core::parameter_entry *>(e);

    const yami::core::result r = params->find(name, name_length, *entry);
    *result = (r == yami::core::ok) ? 1 : 0;
}

int parameters_serialize_buffer_size(const void * p, std::size_t * size)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(params->get_serialize_buffer_size(*size));
}

int parameters_serialize(const void * p,
    char * * buffers, std::size_t * buffer_sizes, std::size_t num_of_buffers)
{
    const yami::core::parameters * params =
        static_cast<const yami::core::parameters *>(p);

    return translate_result(
        params->serialize(buffers, buffer_sizes, num_of_buffers));
}

int parameters_deserialize(void * p,
    const char * * buffers, const std::size_t * buffer_sizes,
    std::size_t num_of_buffers)
{
    yami::core::parameters * params =
        static_cast<yami::core::parameters *>(p);

    return translate_result(
        params->deserialize(buffers, buffer_sizes, num_of_buffers));
}

int parameter_iterator_has_next(const void * it)
{
    const yami::core::parameter_iterator * iterator =
        static_cast<const yami::core::parameter_iterator *>(it);

    return iterator->has_next() ? 1 : 0;
}

void parameter_iterator_move_next(void * it)
{
    yami::core::parameter_iterator * iterator =
        static_cast<yami::core::parameter_iterator *>(it);

    iterator->move_next();
}

void parameter_iterator_current(const void * it, void * e)
{
    const yami::core::parameter_iterator * iterator =
        static_cast<const yami::core::parameter_iterator *>(it);

    yami::core::parameter_entry * entry =
        static_cast<yami::core::parameter_entry *>(e);

    *entry = iterator->current();
}

int parameter_entry_type(const void * e)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return yami::details::type_code(entry->type());
}

void parameter_entry_name(const void * e,
    const char * * name, std::size_t * name_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    entry->get_name(*name, *name_length);
}

int parameter_entry_get_boolean(const void * e, int * value)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    bool v;
    const yami::core::result res = entry->get_boolean(v);
    if (res == yami::core::ok)
    {
        *value = v ? 1 : 0;
    }

    return translate_result(res);
}

int parameter_entry_get_integer(const void * e, int * value)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(entry->get_integer(*value));
}

int parameter_entry_get_long_long(const void * e, long long * value)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(entry->get_long_long(*value));
}

int parameter_entry_get_double_float(const void * e, double * value)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(entry->get_double_float(*value));
}

int parameter_entry_get_string(const void * e,
    const char * * value, std::size_t * value_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(entry->get_string(*value, *value_length));
}

int parameter_entry_get_binary(const void * e,
    const void * * value, std::size_t * value_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(entry->get_binary(*value, *value_length));
}

int parameter_entry_get_boolean_array_length(const void * e,
    std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    bool * array;
    return translate_result(
        entry->get_boolean_array(array, *array_length));
}

int parameter_entry_get_boolean_array(const void * e,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    bool * internal_array;
    const yami::core::result res =
        entry->get_boolean_array(internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameter_entry_get_integer_array_length(const void * e,
    std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    int * array;
    return translate_result(
        entry->get_integer_array(array, *array_length));
}

int parameter_entry_get_integer_array(const void * e,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    int * internal_array;
    const yami::core::result res =
        entry->get_integer_array(internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameter_entry_get_long_long_array_length(const void * e,
    std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    long long * array;
    return translate_result(
        entry->get_long_long_array(array, *array_length));
}

int parameter_entry_get_long_long_array(const void * e,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    long long * internal_array;
    const yami::core::result res =
        entry->get_long_long_array(internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameter_entry_get_double_float_array_length(const void * e,
    std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    double * array;
    return translate_result(
        entry->get_double_float_array(array, *array_length));
}

int parameter_entry_get_double_float_array(const void * e,
    void * * array, std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    double * internal_array;
    const yami::core::result res =
        entry->get_double_float_array(internal_array, *array_length);
    if (res == yami::core::ok)
    {
        *array = internal_array;
    }

    return translate_result(res);
}

int parameter_entry_get_string_array_length(const void * e,
    std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(
        entry->get_string_array_length(*array_length));
}

int parameter_entry_get_string_in_array(const void * e,
    std::size_t index,
    const char * * value, std::size_t * value_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(
        entry->get_string_in_array(index, *value, *value_length));
}

int parameter_entry_get_binary_array_length(const void * e,
    std::size_t * array_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(
        entry->get_binary_array_length(*array_length));
}

int parameter_entry_get_binary_in_array(const void * e,
    std::size_t index,
    const void * * value, std::size_t * value_length)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    return translate_result(
        entry->get_binary_in_array(index, *value, *value_length));
}

int parameter_entry_get_nested(const void * e, void * * nested)
{
    const yami::core::parameter_entry * entry =
        static_cast<const yami::core::parameter_entry *>(e);

    yami::core::parameters * tmp;
    const yami::core::result res =
        entry->get_nested_parameters(tmp);
    if (res == yami::core::ok)
    {
        *nested = tmp;
    }

    return translate_result(res);
}

void raw_buffer_data_source_create(void * p,
    const char * * buffers, const std::size_t * buffer_sizes,
    std::size_t num_of_buffers)
{
    new (p) yami::core::raw_buffer_data_source(
        buffers, buffer_sizes, num_of_buffers);
}

void destroy_raw_buffer_data_source(void * p)
{
    yami::core::raw_buffer_data_source * raw_buffer =
        static_cast<yami::core::raw_buffer_data_source *>(p);

    raw_buffer->~raw_buffer_data_source();
}

int raw_buffer_data_source_serialize_buffer_size(
    const void * p, std::size_t * size)
{
    const yami::core::raw_buffer_data_source * raw =
        static_cast<const yami::core::raw_buffer_data_source *>(p);

    return translate_result(raw->get_serialize_buffer_size(*size));
}

int raw_buffer_data_source_serialize(const void * p,
    char * * buffers, std::size_t * buffer_sizes, std::size_t num_of_buffers)
{
    const yami::core::raw_buffer_data_source * raw =
        static_cast<const yami::core::raw_buffer_data_source *>(p);

    return translate_result(
        raw->serialize(buffers, buffer_sizes, num_of_buffers));
}

void agent_create(void * p,
    void * options,
    void * incoming_callback, void * incoming_callback_hint,
    void * closed_callback, void * closed_callback_hint,
    void * working_area, std::size_t size, int * result)
{
    yami::core::agent * the_agent = new (p) yami::core::agent();

    yami::core::incoming_message_dispatch_function dispatch_callback =
        forced_cast<yami::core::incoming_message_dispatch_function>(
            incoming_callback);
    yami::core::closed_connection_function closed_function =
        forced_cast<yami::core::closed_connection_function>(
            closed_callback);
    yami::core::result r;
    if (options != NULL)
    {
        const yami::core::parameters * option_params =
            static_cast<yami::core::parameters *>(options);

        r = the_agent->init(*option_params,
            dispatch_callback, incoming_callback_hint,
            closed_function, closed_callback_hint,
            working_area, size);
    }
    else
    {
        r = the_agent->init(
            dispatch_callback, incoming_callback_hint,
            closed_function, closed_callback_hint,
            working_area, size);
    }

    *result = translate_result(r);
}

void destroy_agent(void * p)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    the_agent->~agent();
}

void agent_install_event_notifications(void * p,
    void * event_callback, void * event_callback_hint)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    yami::core::event_notification_function event_function =
        forced_cast<yami::core::event_notification_function>(
            event_callback);

    the_agent->install_event_notifications(
        event_function, event_callback_hint);
}

void agent_open(void * p, const char * target, int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    *result = translate_result(the_agent->open(target));
}

void agent_open_descr(void * p, const char * target,
    std::size_t * index, std::size_t * seq_num,
    int * created_new, int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    yami::core::channel_descriptor cd;
    bool created_new_channel;
    const yami::core::result r =
        the_agent->open(target, cd, created_new_channel);
    if (r == yami::core::ok)
    {
        cd.get_details(*index, *seq_num);
        *created_new = created_new_channel ? 1 : 0;
    }

    *result = translate_result(r);
}

void agent_is_open(void * p, const char * target,
    std::size_t * index, std::size_t * seq_num, int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    yami::core::channel_descriptor cd;
    const yami::core::result r = the_agent->is_open(target, cd);
    if (r == yami::core::ok)
    {
        cd.get_details(*index, *seq_num);
    }

    *result = translate_result(r);
}

void agent_close_cd(void * p,
    std::size_t index, std::size_t seq_num,
    std::size_t priority, int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    const yami::core::channel_descriptor cd(index, seq_num);
    *result = translate_result(the_agent->close(cd, priority));
}

void agent_close_str(void * p, const char * target,
    std::size_t priority, int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    *result = translate_result(the_agent->close(target, priority));
}

void agent_post_cd(void * p,
    std::size_t index, std::size_t seq_num,
    void * header, void * body,
    std::size_t priority,
    void * progress_callback, void * progress_callback_hint,
    int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    yami::core::message_progress_function progress_function =
        forced_cast<yami::core::message_progress_function>(
            progress_callback);

    const yami::core::channel_descriptor cd(index, seq_num);

    const yami::core::serializable * header_ser =
        static_cast<yami::core::serializable *>(header);
    const yami::core::serializable * body_ser =
        static_cast<yami::core::serializable *>(body);

    *result = translate_result(
        the_agent->post(
            cd, *header_ser, *body_ser, priority,
            progress_function, progress_callback_hint));
}

void agent_post_str(void * p,
    const char * target,
    void * header, void * body,
    std::size_t priority,
    void * progress_callback, void * progress_callback_hint,
    int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    yami::core::message_progress_function progress_function =
        forced_cast<yami::core::message_progress_function>(
            progress_callback);

    const yami::core::serializable * header_ser =
        static_cast<yami::core::serializable *>(header);
    const yami::core::serializable * body_ser =
        static_cast<yami::core::serializable *>(body);

    *result = translate_result(
        the_agent->post(
            target, *header_ser, *body_ser, priority,
            progress_function, progress_callback_hint));
}

void agent_add_listener(void * p,
    const char * target,
    const char * * resolved_target,
    void * connection_callback, void * connection_callback_hint,
    int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    yami::core::new_incoming_connection_function connection_function =
        forced_cast<yami::core::new_incoming_connection_function>(
            connection_callback);

    *result = translate_result(
        the_agent->add_listener(target,
            connection_function, connection_callback_hint,
            resolved_target));
}

void agent_remove_listener(void * p, const char * target, int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    *result = translate_result(the_agent->remove_listener(target));
}

void agent_do_some_work(void * p,
    std::size_t timeout, int allow_outgoing, int allow_incoming,
    int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    *result = translate_result(
        the_agent->do_some_work(timeout,
            allow_outgoing != 0, allow_incoming != 0));
}

void agent_interrupt_work_waiter(void * p, int * result)
{
    yami::core::agent * the_agent =
        static_cast<yami::core::agent *>(p);

    *result = translate_result(the_agent->interrupt_work_waiter());
}

} // extern "C"
