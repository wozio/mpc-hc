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
#include "details.h"
#include <yami4-core/parameters.h>
#include <cstring>
#include <sstream>

using namespace yami;

parameters::parameters()
    : own_params_(new core::parameters()), params_(own_params_.get())
{
}

parameters::parameters(core::parameters * external)
    : own_params_(new core::parameters()), params_(external)
{
}

parameters::parameters(const parameters & other)
    : serializable(),
      own_params_(new core::parameters()), params_(own_params_.get())
{
    merge_from(other);
}

void parameters::operator=(const parameters & other)
{
    parameters tmp(other);
    swap(tmp);
}

void parameters::swap(parameters & other)
{
    core::parameters * tmp = own_params_.release();
    own_params_ = other.own_params_;
    other.own_params_.reset(tmp);

    tmp = params_;
    params_ = other.params_;
    other.params_ = tmp;
}

void parameters::set_boolean(const std::string & name, bool value)
{
    details::translate_result_to_exception(
        params_->set_boolean(name.data(), name.size(), value));
}

void parameters::set_boolean(const char * name, bool value)
{
    details::translate_result_to_exception(
        params_->set_boolean(name, std::strlen(name), value));
}

bool parameters::get_boolean(const std::string & name) const
{
    bool value;
    details::translate_result_to_exception(
        params_->get_boolean(name.data(), name.size(), value));
    return value;
}

bool parameters::get_boolean(const char * name) const
{
    bool value;
    details::translate_result_to_exception(
        params_->get_boolean(name, std::strlen(name), value));
    return value;
}

void parameters::set_integer(const std::string & name, int value)
{
    details::translate_result_to_exception(
        params_->set_integer(name.data(), name.size(), value));
}

void parameters::set_integer(const char * name, int value)
{
    details::translate_result_to_exception(
        params_->set_integer(name, std::strlen(name), value));
}

int parameters::get_integer(const std::string & name) const
{
    int value;
    details::translate_result_to_exception(
        params_->get_integer(name.data(), name.size(), value));
    return value;
}

int parameters::get_integer(const char * name) const
{
    int value;
    details::translate_result_to_exception(
        params_->get_integer(name, std::strlen(name), value));
    return value;
}

void parameters::set_long_long(const std::string & name, long long value)
{
    details::translate_result_to_exception(
        params_->set_long_long(name.data(), name.size(), value));
}

void parameters::set_long_long(const char * name, long long value)
{
    details::translate_result_to_exception(
        params_->set_long_long(name, std::strlen(name), value));
}

long long parameters::get_long_long(const std::string & name) const
{
    long long value;
    details::translate_result_to_exception(
        params_->get_long_long(name.data(), name.size(), value));
    return value;
}

long long parameters::get_long_long(const char * name) const
{
    long long value;
    details::translate_result_to_exception(
        params_->get_long_long(name, std::strlen(name), value));
    return value;
}

void parameters::set_double_float(const std::string & name, double value)
{
    details::translate_result_to_exception(
        params_->set_double_float(name.data(), name.size(), value));
}

void parameters::set_double_float(const char * name, double value)
{
    details::translate_result_to_exception(
        params_->set_double_float(name, std::strlen(name), value));
}

double parameters::get_double_float(const std::string & name) const
{
    double value;
    details::translate_result_to_exception(
        params_->get_double_float(name.data(), name.size(), value));
    return value;
}

double parameters::get_double_float(const char * name) const
{
    double value;
    details::translate_result_to_exception(
        params_->get_double_float(name, std::strlen(name), value));
    return value;
}

void parameters::set_string(const std::string & name,
    const std::string & value)
{
    details::translate_result_to_exception(
        params_->set_string(name.data(), name.size(),
            value.data(), value.size()));
}

void parameters::set_string(const char * name, const char * value)
{
    details::translate_result_to_exception(
        params_->set_string(name, std::strlen(name),
            value, std::strlen(value)));
}

void parameters::set_string_shallow(const std::string & name,
    const char * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_string_shallow(name.data(), name.size(),
            value, value_length));
}

void parameters::set_string_shallow(const char * name,
    std::size_t name_length, const char * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_string_shallow(name, name_length,
            value, value_length));
}

std::string parameters::get_string(const std::string & name) const
{
    const char * value;
    std::size_t value_length;
    details::translate_result_to_exception(
        params_->get_string(name.data(), name.size(),
            value, value_length));
    return value != NULL ?
        std::string(value, value_length) : std::string();
}

std::string parameters::get_string(const char * name) const
{
    const char * value;
    std::size_t value_length;
    details::translate_result_to_exception(
        params_->get_string(name, std::strlen(name),
            value, value_length));
    return value != NULL ?
        std::string(value, value_length) : std::string();
}

const char * parameters::get_string(const std::string & name,
    std::size_t & length) const
{
    const char * value;
    details::translate_result_to_exception(
        params_->get_string(name.data(), name.size(),
            value, length));
    return value;
}

const char * parameters::get_string(const char * name,
    std::size_t & length) const
{
    const char * value;
    details::translate_result_to_exception(
        params_->get_string(name, std::strlen(name),
            value, length));
    return value;
}

void parameters::set_binary(const std::string & name,
    const void * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_binary(name.data(), name.size(),
            value, value_length));
}

void parameters::set_binary(const char * name,
    const void * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_binary(name, std::strlen(name),
            value, value_length));
}

void parameters::set_binary_shallow(const std::string & name,
    const void * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_binary_shallow(name.data(), name.size(),
            value, value_length));
}

void parameters::set_binary_shallow(const char * name,
    std::size_t name_length, const void * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_binary_shallow(name, name_length,
            value, value_length));
}

const void * parameters::get_binary(const std::string & name,
    std::size_t & length) const
{
    const void * value;
    details::translate_result_to_exception(
        params_->get_binary(name.data(), name.size(),
            value, length));
    return value;
}

const void * parameters::get_binary(const char * name,
    std::size_t & length) const
{
    const void * value;
    details::translate_result_to_exception(
        params_->get_binary(name, std::strlen(name),
            value, length));
    return value;
}

void parameters::set_boolean_array(const std::string & name,
    const bool * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_boolean_array(name.data(), name.size(),
            values, array_length));
}

void parameters::set_boolean_array(const char * name,
    const bool * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_boolean_array(name, std::strlen(name),
            values, array_length));
}

void parameters::set_boolean_array_shallow(const std::string & name,
    const bool * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_boolean_array_shallow(name.data(), name.size(),
            values, array_length));
}

void parameters::set_boolean_array_shallow(const char * name,
    const bool * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_boolean_array(name, std::strlen(name),
            values, array_length));
}

bool * parameters::get_boolean_array(const std::string & name,
    std::size_t & length) const
{
    bool * values;
    details::translate_result_to_exception(
        params_->get_boolean_array(name.data(), name.size(),
            values, length));
    return values;
}

bool * parameters::get_boolean_array(const char * name,
    std::size_t & array_length) const
{
    bool * values;
    details::translate_result_to_exception(
        params_->get_boolean_array(name, std::strlen(name),
            values, array_length));
    return values;
}

void parameters::set_integer_array(const std::string & name,
    const int * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_integer_array(name.data(), name.size(),
            values, array_length));
}

void parameters::set_integer_array(const char * name,
    const int * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_integer_array(name, std::strlen(name),
            values, array_length));
}

void parameters::set_integer_array_shallow(const std::string & name,
    const int * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_integer_array_shallow(name.data(), name.size(),
            values, array_length));
}

void parameters::set_integer_array_shallow(const char * name,
    const int * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_integer_array(name, std::strlen(name),
            values, array_length));
}

int * parameters::get_integer_array(const std::string & name,
    std::size_t & array_length) const
{
    int * values;
    details::translate_result_to_exception(
        params_->get_integer_array(name.data(), name.size(),
            values, array_length));
    return values;
}

int * parameters::get_integer_array(const char * name,
    std::size_t & array_length) const
{
    int * values;
    details::translate_result_to_exception(
        params_->get_integer_array(name, std::strlen(name),
            values, array_length));
    return values;
}

void parameters::set_long_long_array(const std::string & name,
    const long long * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_long_long_array(name.data(), name.size(),
            values, array_length));
}

void parameters::set_long_long_array(const char * name,
    const long long * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_long_long_array(name, std::strlen(name),
            values, array_length));
}

void parameters::set_long_long_array_shallow(const std::string & name,
    const long long * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_long_long_array_shallow(name.data(), name.size(),
            values, array_length));
}

void parameters::set_long_long_array_shallow(const char * name,
    const long long * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_long_long_array(name, std::strlen(name),
            values, array_length));
}

long long * parameters::get_long_long_array(const std::string & name,
    std::size_t & array_length) const
{
    long long * values;
    details::translate_result_to_exception(
        params_->get_long_long_array(name.data(), name.size(),
            values, array_length));
    return values;
}

long long * parameters::get_long_long_array(const char * name,
    std::size_t & array_length) const
{
    long long * values;
    details::translate_result_to_exception(
        params_->get_long_long_array(name, std::strlen(name),
            values, array_length));
    return values;
}

void parameters::set_double_float_array(const std::string & name,
    const double * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_double_float_array(name.data(), name.size(),
            values, array_length));
}

void parameters::set_double_float_array(const char * name,
    const double * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_double_float_array(name, std::strlen(name),
            values, array_length));
}

void parameters::set_double_float_array_shallow(const std::string & name,
    const double * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_double_float_array_shallow(name.data(), name.size(),
            values, array_length));
}

void parameters::set_double_float_array_shallow(const char * name,
    const double * values, std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->set_double_float_array(name, std::strlen(name),
            values, array_length));
}

double * parameters::get_double_float_array(const std::string & name,
    std::size_t & array_length) const
{
    double * values;
    details::translate_result_to_exception(
        params_->get_double_float_array(name.data(), name.size(),
            values, array_length));
    return values;
}

double * parameters::get_double_float_array(const char * name,
    std::size_t & array_length) const
{
    double * values;
    details::translate_result_to_exception(
        params_->get_double_float_array(name, std::strlen(name),
            values, array_length));
    return values;
}

void parameters::create_string_array(const std::string & name,
    std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->create_string_array(name.data(), name.size(),
            array_length));
}

void parameters::create_string_array(const char * name,
    std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->create_string_array(name, std::strlen(name),
            array_length));
}

void parameters::set_string_in_array(const std::string & name,
    std::size_t index, const std::string & value)
{
    details::translate_result_to_exception(
        params_->set_string_in_array(name.data(), name.size(), index,
            value.data(), value.size()));
}

void parameters::set_string_in_array(const char * name, std::size_t index,
    const char * value)
{
    details::translate_result_to_exception(
        params_->set_string_in_array(name, std::strlen(name), index,
            value, std::strlen(value)));
}

std::size_t parameters::get_string_array_length(
    const std::string & name) const
{
    std::size_t length;
    details::translate_result_to_exception(
        params_->get_string_array_length(name.data(), name.size(),
            length));
    return length;
}

std::size_t parameters::get_string_array_length(const char * name) const
{
    std::size_t length;
    details::translate_result_to_exception(
        params_->get_string_array_length(name, std::strlen(name),
            length));
    return length;
}

std::string parameters::get_string_in_array(const std::string & name,
    std::size_t index) const
{
    const char * value;
    std::size_t value_length;
    details::translate_result_to_exception(
        params_->get_string_in_array(name.data(), name.size(), index,
            value, value_length));
    return value != NULL ?
        std::string(value, value_length) : std::string();
}

std::string parameters::get_string_in_array(const char * name,
    std::size_t index) const
{
    const char * value;
    std::size_t value_length;
    details::translate_result_to_exception(
        params_->get_string_in_array(name, std::strlen(name), index,
            value, value_length));
    return value != NULL ?
        std::string(value, value_length) : std::string();
}

const char * parameters::get_string_in_array(const std::string & name,
    std::size_t index, std::size_t & length) const
{
    const char * value;
    details::translate_result_to_exception(
        params_->get_string_in_array(name.data(), name.size(), index,
            value, length));
    return value;
}

const char * parameters::get_string_in_array(const char * name,
    std::size_t index, std::size_t & length) const
{
    const char * value;
    details::translate_result_to_exception(
        params_->get_string_in_array(name, std::strlen(name), index,
            value, length));
    return value;
}

void parameters::create_binary_array(const std::string & name,
    std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->create_binary_array(name.data(), name.size(),
            array_length));
}

void parameters::create_binary_array(const char * name,
    std::size_t array_length)
{
    details::translate_result_to_exception(
        params_->create_binary_array(name, std::strlen(name),
            array_length));
}

void parameters::set_binary_in_array(const std::string & name,
    std::size_t index, const void * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_binary_in_array(name.data(), name.size(), index,
            value, value_length));
}

void parameters::set_binary_in_array(const char * name, std::size_t index,
    const void * value, std::size_t value_length)
{
    details::translate_result_to_exception(
        params_->set_binary_in_array(name, std::strlen(name), index,
            value, value_length));
}

std::size_t parameters::get_binary_array_length(
    const std::string & name) const
{
    std::size_t length;
    details::translate_result_to_exception(
        params_->get_binary_array_length(name.data(), name.size(),
            length));
    return length;
}

std::size_t parameters::get_binary_array_length(const char * name) const
{
    std::size_t length;
    details::translate_result_to_exception(
        params_->get_binary_array_length(name, std::strlen(name),
            length));
    return length;
}

const void * parameters::get_binary_in_array(const std::string & name,
    std::size_t index, std::size_t & length) const
{
    const void * value;
    details::translate_result_to_exception(
        params_->get_binary_in_array(name.data(), name.size(), index,
            value, length));
    return value;
}

const void * parameters::get_binary_in_array(const char * name,
    std::size_t index, std::size_t & length) const
{
    const void * value;
    details::translate_result_to_exception(
        params_->get_binary_in_array(name, std::strlen(name), index,
            value, length));
    return value;
}

core::parameters * parameters::create_nested_parameters(
    const std::string & name)
{
    core::parameters * nested;
    details::translate_result_to_exception(
        params_->create_nested_parameters(name.data(), name.size(),
            nested));
    return nested;
}

core::parameters * parameters::get_nested_parameters(
    const std::string & name) const
{
    core::parameters * nested;
    details::translate_result_to_exception(
        params_->get_nested_parameters(name.data(), name.size(),
            nested));
    return nested;
}

core::parameters * parameters::get_nested_parameters(const char * name) const
{
    core::parameters * nested;
    details::translate_result_to_exception(
        params_->get_nested_parameters(name, std::strlen(name),
            nested));
    return nested;
}

void parameters::lock(const std::string & name, long long key)
{
    details::translate_result_to_exception(
        params_->lock(name.data(), name.size(), key));
}

void parameters::lock(const char * name, long long key)
{
    details::translate_result_to_exception(
        params_->lock(name, std::strlen(name), key));
}

void parameters::unlock(const std::string & name, long long key)
{
    details::translate_result_to_exception(
        params_->unlock(name.data(), name.size(), key));
}

void parameters::unlock(const char * name, long long key)
{
    details::translate_result_to_exception(
        params_->unlock(name, std::strlen(name), key));
}

bool parameters::is_locked(const std::string & name) const
{
    bool result;
    details::translate_result_to_exception(
        params_->is_locked(name.data(), name.size(), result));
    return result;
}

bool parameters::is_locked(const char * name) const
{
    bool result;
    details::translate_result_to_exception(
        params_->is_locked(name, std::strlen(name), result));
    return result;
}

std::size_t parameters::size() const
{
    return params_->size();
}

parameter_type parameters::type(const std::string & name) const
{
    core::parameter_type t;
    details::translate_result_to_exception(
        params_->get_type(name.data(), name.size(), t));
    return static_cast<parameter_type>(static_cast<int>(t));
}

parameter_type parameters::type(const char * name) const
{
    core::parameter_type t;
    details::translate_result_to_exception(
        params_->get_type(name, std::strlen(name), t));
    return static_cast<parameter_type>(static_cast<int>(t));
}

parameters::iterator parameters::begin() const
{
    core::parameter_iterator it;
    const core::result res = params_->get_iterator(it);
    if (res == core::no_entries)
    {
        return iterator();
    }
    else
    {
        return iterator(it);
    }
}

parameters::iterator parameters::end() const
{
    return iterator();
}

bool parameters::find(const std::string & name, parameter_entry & entry) const
{
    core::parameter_entry e;
    const core::result res = params_->find(name.data(), name.size(), e);
    if (res == core::ok)
    {
        entry.entry_ = e;
    }
    return res == core::ok;
}

bool parameters::find(const char * name, parameter_entry & entry) const
{
    core::parameter_entry e;
    const core::result res = params_->find(name, std::strlen(name), e);
    if (res == core::ok)
    {
        entry.entry_ = e;
    }
    return res == core::ok;
}

void parameters::remove(const std::string & name)
{
    details::translate_result_to_exception(
        params_->remove(name.data(), name.size()));
}

void parameters::remove(const char * name)
{
    details::translate_result_to_exception(
        params_->remove(name, std::strlen(name)));
}

void parameters::remove(iterator it)
{
    it.it_.remove();
}

void parameters::merge_from(const parameters & other)
{
    details::translate_result_to_exception(
        params_->merge_from(*other.params_));
}

void parameters::clear()
{
    params_->clear();
}

std::size_t parameters::serialize_buffer_size() const
{
    std::size_t buffer_size;
    details::translate_result_to_exception(
        params_->get_serialize_buffer_size(buffer_size));
    return buffer_size;
}

void parameters::serialize(char * * buffers, std::size_t * buffer_sizes,
    std::size_t num_of_buffers) const
{
    details::translate_result_to_exception(
        params_->serialize(buffers, buffer_sizes, num_of_buffers));
}

void parameters::deserialize(const char * * buffers,
    std::size_t * buffer_sizes, std::size_t num_of_buffers)
{
    details::translate_result_to_exception(
        params_->deserialize(buffers, buffer_sizes, num_of_buffers));
}

const core::parameters & parameters::get_core_object() const
{
    return * params_;
}

// helper class for the dump function
class ostream_dump : public yami::details::dump_sink
{
public:
    ostream_dump(std::ostream & os) : os_(os) {}

    virtual void indent(std::size_t spaces)
    {
        os_ << std::string(spaces, ' ');
    }

    virtual void dump(std::size_t v)
    {
        os_ << v;
    }

    virtual void dump(bool v)
    {
        os_ << v;
    }

    virtual void dump(int v)
    {
        os_ << v;
    }

    virtual void dump(long long v)
    {
        os_ << v;
    }

    virtual void dump(double v)
    {
        os_ << v;
    }

    virtual void dump(const char * str)
    {
        os_ << str;
    }

    virtual void dump(const char * str, std::size_t str_len)
    {
        os_ << std::string(str, str_len);
    }

private:
    std::ostream & os_;
};

void parameters::dump(std::ostream & os) const
{
    ostream_dump osd(os);
    params_->dump(osd);
}
