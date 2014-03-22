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

#include "parameters-details.h"
#include "allocator.h"
#include "fatal_errors.h"
#include "parameters.h"
#include "serialization.h"
#include <cstring>

using namespace yami;
using namespace yami::details;

int details::type_code(core::parameter_type t)
{
    int res = 0;  // dummy initialization to please the compiler
    switch (t)
    {
    case core::boolean:            res = 1;  break;
    case core::integer:            res = 2;  break;
    case core::long_long:          res = 3;  break;
    case core::double_float:       res = 4;  break;
    case core::string:             res = 5;  break;
    case core::binary:             res = 6;  break;
    case core::boolean_array:      res = 7;  break;
    case core::integer_array:      res = 8;  break;
    case core::long_long_array:    res = 9;  break;
    case core::double_float_array: res = 10; break;
    case core::string_array:       res = 11; break;
    case core::binary_array:       res = 12; break;
    case core::nested_parameters:  res = 13; break;
    default:
        fatal_failure(__FILE__, __LINE__);
    }

    return res;
}

core::result details::get_type_from_code(
    int code, core::parameter_type & type)
{
    core::result res = core::ok;
    switch (code)
    {
    case 1:  type = core::boolean;            break;
    case 2:  type = core::integer;            break;
    case 3:  type = core::long_long;          break;
    case 4:  type = core::double_float;       break;
    case 5:  type = core::string;             break;
    case 6:  type = core::binary;             break;
    case 7:  type = core::boolean_array;      break;
    case 8:  type = core::integer_array;      break;
    case 9:  type = core::long_long_array;    break;
    case 10: type = core::double_float_array; break;
    case 11: type = core::string_array;       break;
    case 12: type = core::binary_array;       break;
    case 13: type = core::nested_parameters;  break;
    default:
        res = core::unexpected_value;
    }

    return res;
}

core::result entry_name::set(const char * name, std::size_t length,
    allocator & alloc)
{
    core::result res;
    name_length = length;
    if (length <= short_name_optimization_threshold)
    {
        std::memcpy(buffer.short_value, name, length);
        res = core::ok;
    }
    else
    {
        char * allocated =
            static_cast<char *>(alloc.allocate(length));
        if (allocated != NULL)
        {
            std::memcpy(allocated, name, length);
            buffer.long_value = allocated;
            res = core::ok;
        }
        else
        {
            res = core::no_memory;
        }
    }

    return res;
}

const char * entry_name::value() const
{
    const char * res;
    if (name_length <= short_name_optimization_threshold)
    {
        res = buffer.short_value;
    }
    else
    {
        res = buffer.long_value;
    }

    return res;
}

void entry_name::clear(allocator & alloc)
{
    if (name_length > short_name_optimization_threshold)
    {
        alloc.deallocate(buffer.long_value);
    }

    name_length = 0;
}

bool entry_name::equals(const char * str, std::size_t length) const
{
    bool res = false;

    if (length == name_length)
    {
        if (name_length <= short_name_optimization_threshold)
        {
            res = std::memcmp(buffer.short_value, str, length) == 0;
        }
        else
        {
            res = std::memcmp(buffer.long_value, str, length) == 0;
        }
    }

    return res;
}

void entry_name::dump(dump_sink & sink) const
{
    if (name_length <= short_name_optimization_threshold)
    {
        sink.dump(buffer.short_value, name_length);
    }
    else
    {
        sink.dump(buffer.long_value, name_length);
    }
}

void string_array_element::clear(allocator & alloc)
{
    if (value != NULL)
    {
        alloc.deallocate(value);
        value = NULL;
        length = 0;
    }
}

void binary_array_element::clear(allocator & alloc)
{
    if (value != NULL)
    {
        alloc.deallocate(value);
        value = NULL;
        length = 0;
    }
}

// note: this function does not clear nested entries
void entry::clear_item(allocator & alloc)
{
    if (type == core::string && item.str.own)
    {
        alloc.deallocate(item.str.value);
    }
    else if (type == core::binary && item.bin.own)
    {
        alloc.deallocate(item.bin.value);
    }
    else if (type == core::boolean_array && item.ba.own)
    {
        alloc.deallocate(item.ba.values);
    }
    else if (type == core::integer_array && item.ia.own)
    {
        alloc.deallocate(item.ia.values);
    }
    else if (type == core::long_long_array && item.La.own)
    {
        alloc.deallocate(item.La.values);
    }
    else if (type == core::double_float_array && item.da.own)
    {
        alloc.deallocate(item.da.values);
    }
    else if (type == core::string_array && item.sa.own)
    {
        for (std::size_t i = 0; i != item.sa.length; ++i)
        {
            item.sa.values[i].clear(alloc);
        }

        alloc.deallocate(item.sa.values);
    }
    else if (type == core::binary_array && item.bina.own)
    {
        for (std::size_t i = 0; i != item.bina.length; ++i)
        {
            item.bina.values[i].clear(alloc);
        }

        alloc.deallocate(item.bina.values);
    }

    locked = false;
}

core::result entry::lock(long long key)
{
    core::result res;
    if (locked)
    {
        res = core::bad_state;
    }
    else
    {
        locked = true;
        lock_key = key;
        res = core::ok;
    }

    return res;
}

core::result entry::unlock(long long key)
{
    core::result res;
    if (locked == false)
    {
        res = core::bad_state;
    }
    else
    {
        if (lock_key == key)
        {
            locked = false;
            res = core::ok;
        }
        else
        {
            res = core::unexpected_value;
        }
    }

    return res;
}

core::result entry::is_locked_or_contains_locked(bool & value)
{
    core::result res;

    if (locked)
    {
        value = true;
        res = core::ok;
    }
    else
    {
        if (type == core::nested_parameters)
        {
            // nested entries need to be deeply checked
            // to take into account possible entries
            // that exist further down the tree

            check_locked_fsm fsm(*item.nested, value);
            res = fsm.execute();
        }
        else
        {
            value = false;
            res = core::ok;
        }
    }

    return res;
}

fsm_common::fsm_common()
    : current_nesting_level_(0),
      state_(&state_stack_[0]),
      index_(&index_stack_[0]),
      done_(false),
      res_(core::ok)
{
    *state_ = processing_preamble;
}

core::result fsm_common::execute()
{
    while (done_ == false && res_ == core::ok)
    {
        switch (*state_)
        {
        case processing_preamble:
            process_preamble();
            break;
        case processing_entries:
            process_entries();
            break;
        case finished:
            finish();
            break;
        }
    }

    return res_;
}

fsm_preserving_structure::fsm_preserving_structure(
    const core::parameters & params)
    : current_object_(&object_stack_[0]),
      base_(&base_stack_[0]),
      changed_frame_(false)
{
    *current_object_ = &params;
    *base_ = params.data_;
}

void fsm_preserving_structure::pre_process_entries()
{
    changed_frame_ = false;
}

void fsm_preserving_structure::push_stack_frame(const entry & e)
{
    // create new frame on the stack for the nested object
    if (current_nesting_level_ < max_nesting_level - 1)
    {
        core::parameters * nested = e.item.nested;

        ++current_nesting_level_;
        changed_frame_ = true;

        current_object_ = &object_stack_[current_nesting_level_];
        base_ = &base_stack_[current_nesting_level_];
        state_ = &state_stack_[current_nesting_level_];
        index_ = &index_stack_[current_nesting_level_];

        *current_object_ = nested;
        *base_ = nested->data_;
        *state_ = processing_preamble;
    }
    else
    {
        res_ = core::nesting_too_deep;
    }
}

void fsm_preserving_structure::advance()
{
    // if there was no frame change on the "stack" due to nested parameters,
    // continue iteration over the current list
    // (if case of frame change the iteration is continued when the frame
    // is popped in finished state)

    if (res_ == core::ok && changed_frame_ == false)
    {
        if (*index_ != (*current_object_)->num_of_entries_ - 1)
        {
            ++(*index_);
        }
        else
        {
            *state_ = finished;
        }
    }
}

void fsm_preserving_structure::finish()
{
    // finished with entries in the current object
    // -> go up one level or quit

    if (current_nesting_level_ == 0)
    {
        done_ = true;
    }
    else
    {
        --current_nesting_level_;

        current_object_ = &object_stack_[current_nesting_level_];
        base_ = &base_stack_[current_nesting_level_];
        state_ = &state_stack_[current_nesting_level_];
        index_ = &index_stack_[current_nesting_level_];

        finish_nested();

        if (*index_ != (*current_object_)->num_of_entries_ - 1)
        {
            ++(*index_);
        }
        else
        {
            *state_ = finished;
        }
    }
}

clear_fsm::clear_fsm(core::parameters & params, allocator & alloc)
    : fsm_preserving_structure(params), alloc_(alloc)
{
}

void clear_fsm::process_preamble()
{
    if ((*current_object_)->num_of_entries_ != 0)
    {
        *state_ = processing_entries;
        *index_ = 0;
    }
    else
    {
        *state_ = finished;
    }
}

void clear_fsm::process_entries()
{
    pre_process_entries();

    entry & e = const_cast<entry &>((*base_)[*index_]);

    if (e.type != core::unused)
    {
        e.clear_name(alloc_);
        e.clear_item(alloc_);

        if (e.type == core::nested_parameters)
        {
            push_stack_frame(e);
        }
    }

    advance();
}

void clear_fsm::finish_nested()
{
    entry & e = const_cast<entry &>((*base_)[*index_]);

    if (e.item.nested->data_ != NULL)
    {
        alloc_.deallocate(e.item.nested->data_);
    }

    alloc_.deallocate(e.item.nested);
}

get_serialize_buffer_size_fsm::get_serialize_buffer_size_fsm(
    const core::parameters & params)
    : fsm_preserving_structure(params), size_(0)
{
}

void get_serialize_buffer_size_fsm::process_preamble()
{
    size_ += 4; // to store number of entries for current object
    if ((*current_object_)->num_of_entries_ != 0)
    {
        *state_ = processing_entries;
        *index_ = 0;
    }
    else
    {
        *state_ = finished;
    }
}

void get_serialize_buffer_size_fsm::process_entries()
{
    pre_process_entries();

    const entry & e = (*base_)[*index_];

    if (e.type != core::unused)
    {
        // name length and the name itself:
        size_ += 4;
        size_ += round_up_4(e.name.name_length);

        // type
        size_ += 4;

        // value
        switch (e.type)
        {
        case core::boolean:
        case core::integer:
            size_ += 4;
            break;
        case core::long_long:
            size_ += 8;
            break;
        case core::double_float:
            size_ += 8;
            break;
        case core::string:
            // length and value
            size_ += 4;
            size_ += round_up_4(e.item.str.length);
            break;
        case core::binary:
            // length and value
            size_ += 4;
            size_ += round_up_4(e.item.bin.length);
            break;
        case core::boolean_array:
            // length and packed values
            size_ += 4;
            size_ += round_up_4((e.item.ba.length + 7) / 8);
            break;
        case core::integer_array:
            // length and values (each value 4 bytes)
            size_ += 4;
            size_ += round_up_4(e.item.ia.length * 4);
            break;
        case core::long_long_array:
            // length and values (each value 8 bytes)
            size_ += 4;
            size_ += round_up_4(e.item.La.length * 8);
            break;
        case core::double_float_array:
            // length and values (each value 8 bytes)
            size_ += 4;
            size_ += round_up_4(e.item.da.length * 8);
            break;
        case core::string_array:
            // length and then each entry as a string
            size_ += 4;
            for (std::size_t j = 0; j != e.item.sa.length; ++j)
            {
                // length of array entry and its value
                size_ += 4;
                size_ += round_up_4(e.item.sa.values[j].length);
            }
            break;
        case core::binary_array:
            // length and then each entry separately
            size_ += 4;
            for (std::size_t j = 0; j != e.item.bina.length; ++j)
            {
                // length of array entry and its value
                size_ += 4;
                size_ += round_up_4(e.item.bina.values[j].length);
            }
            break;
        case core::nested_parameters:
            push_stack_frame(e);
            break;
        default:
            fatal_failure(__FILE__, __LINE__);
        }
    }

    advance();
}

serialize_fsm::serialize_fsm(const core::parameters & params,
    char * * buffers, const std::size_t * buffer_sizes,
    std::size_t num_of_buffers)
    : fsm_preserving_structure(params),
      buffers_(buffers), buffer_sizes_(buffer_sizes),
      num_of_buffers_(num_of_buffers),
      current_buffer_(0), buffer_position_(buffers[0])
{
}


void serialize_fsm::process_preamble()
{
    // store number of entries for current object
    res_ = put_integer(buffers_, buffer_sizes_, num_of_buffers_,
        current_buffer_, buffer_position_,
        static_cast<int>((*current_object_)->size()));
    if (res_ == core::ok)
    {
        if ((*current_object_)->num_of_entries_ != 0)
        {
            *state_ = processing_entries;
            *index_ = 0;
        }
        else
        {
            *state_ = finished;
        }
    }
}

void serialize_fsm::process_entries()
{
    pre_process_entries();

    const entry & e = (*base_)[*index_];

    if (e.type != core::unused)
    {
        // name length and the name itself:
        res_ = put_string(buffers_, buffer_sizes_, num_of_buffers_,
            current_buffer_, buffer_position_,
            e.name.value(), e.name.name_length);
        if (res_ == core::ok)
        {
            // type
            res_ = put_integer(buffers_, buffer_sizes_, num_of_buffers_,
                current_buffer_, buffer_position_,
                type_code(e.type));
            if (res_ == core::ok)
            {
                // value
                switch (e.type)
                {
                case core::boolean:
                    res_ = put_integer(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        static_cast<int>(e.item.b));
                    break;
                case core::integer:
                    res_ = put_integer(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.i);
                    break;
                case core::long_long:
                    res_ = put_long_long(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.L);
                    break;
                case core::double_float:
                    res_ = put_double_float(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.d);
                    break;
                case core::string:
                    res_ = put_string(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.str.value, e.item.str.length);
                    break;
                case core::binary:
                    res_ = put_string(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        reinterpret_cast<const char *>(e.item.bin.value),
                        e.item.bin.length);
                    break;
                case core::boolean_array:
                    res_ = put_boolean_array(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.ba.values, e.item.ba.length);
                    break;
                case core::integer_array:
                    res_ = put_integer_array(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.ia.values, e.item.ia.length);
                    break;
                case core::long_long_array:
                    res_ = put_long_long_array(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.La.values, e.item.La.length);
                    break;
                case core::double_float_array:
                    res_ = put_double_float_array(
                        buffers_, buffer_sizes_, num_of_buffers_,
                        current_buffer_, buffer_position_,
                        e.item.da.values, e.item.da.length);
                    break;
                case core::string_array:
                    {
                        const std::size_t array_length = e.item.sa.length;
                        const string_array_element * values =
                            e.item.sa.values;
                        res_ = put_integer(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            static_cast<int>(array_length));
                        for (std::size_t i = 0;
                             res_ == core::ok && i != array_length; ++i)
                        {
                            res_ = put_string(
                                buffers_, buffer_sizes_, num_of_buffers_,
                                current_buffer_, buffer_position_,
                                values[i].value, values[i].length);
                        }
                    }
                    break;
                case core::binary_array:
                    {
                        const std::size_t array_length = e.item.bina.length;
                        const binary_array_element * values =
                            e.item.bina.values;
                        res_ = put_integer(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            static_cast<int>(array_length));
                        for (std::size_t i = 0;
                             res_ == core::ok && i != array_length; ++i)
                        {
                            res_ = put_string(
                                buffers_, buffer_sizes_, num_of_buffers_,
                                current_buffer_, buffer_position_,
                                reinterpret_cast<const char *>(
                                    values[i].value),
                                values[i].length);
                        }
                    }
                    break;
                case core::nested_parameters:
                    push_stack_frame(e);
                    break;
                default:
                    fatal_failure(__FILE__, __LINE__);
                }
            }
        }
    }

    advance();
}

deserialize_fsm::deserialize_fsm(core::parameters & params,
    const char * * buffers, const std::size_t * buffer_sizes,
    std::size_t num_of_buffers,
    allocator & alloc)
    : current_object_(&object_stack_[0]),
      size_(&size_stack_[0]),
      buffers_(buffers), buffer_sizes_(buffer_sizes),
      num_of_buffers_(num_of_buffers),
      current_buffer_(0), buffer_position_(buffers[0]),
      allocator_(alloc)
{
    *current_object_ = &params;
}

void deserialize_fsm::process_preamble()
{
    // read number of entries for current object
    int raw_size;
    res_ = get_integer(buffers_, buffer_sizes_, num_of_buffers_,
        current_buffer_, buffer_position_,
        raw_size);
    if (res_ == core::ok)
    {
        *size_ = static_cast<std::size_t>(raw_size);
        if (*size_ != 0)
        {
            *state_ = processing_entries;
            *index_ = 0;
        }
        else
        {
            *state_ = finished;
        }
    }
}

void deserialize_fsm::process_entries()
{
    bool changed_frame = false;

    // name length and the name itself:
    const char * name;
    std::size_t name_length;
    res_ = get_string(
        buffers_, buffer_sizes_, num_of_buffers_,
        current_buffer_, buffer_position_,
        name, name_length, allocator_);
    if (res_ == core::ok)
    {
        // type
        int type_code;
        res_ = get_integer(
            buffers_, buffer_sizes_, num_of_buffers_,
            current_buffer_, buffer_position_,
            type_code);
        if (res_ == core::ok)
        {
            core::parameter_type type;
            res_ = get_type_from_code(type_code, type);
            if (res_ == core::ok)
            {
                // value
                switch (type)
                {
                case core::boolean:
                    {
                        int raw_value;
                        res_ = get_integer(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            raw_value);
                        if (res_ == core::ok)
                        {
                            bool value = raw_value != 0;
                            res_ = (*current_object_)->set_boolean(
                                name, name_length, value);
                        }
                    }
                    break;
                case core::integer:
                    {
                        int value;
                        res_ = get_integer(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            value);
                        if (res_ == core::ok)
                        {
                            res_ = (*current_object_)->set_integer(
                                name, name_length, value);
                        }
                    }
                    break;
                case core::long_long:
                    {
                        long long value;
                        res_ = get_long_long(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            value);
                        if (res_ == core::ok)
                        {
                            res_ = (*current_object_)->set_long_long(
                                name, name_length, value);
                        }
                    }
                    break;
                case core::double_float:
                    {
                        double value;
                        res_ = get_double_float(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            value);
                        if (res_ == core::ok)
                        {
                            res_ = (*current_object_)->set_double_float(
                                name, name_length, value);
                        }
                    }
                    break;
                case core::string:
                    {
                        const char * value;
                        std::size_t value_length;
                        res_ = get_string(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            value, value_length, allocator_);
                        if (res_ == core::ok)
                        {
                            res_ = do_set_string(
                                name, name_length,
                                value, value_length,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_, true);
                        }
                    }
                    break;
                case core::binary:
                    {
                        const char * value;
                        std::size_t value_length;
                        res_ = get_string(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            value, value_length, allocator_);
                        if (res_ == core::ok)
                        {
                            res_ = do_set_binary(
                                name, name_length,
                                value, value_length,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_, true);
                        }
                    }
                    break;
                case core::boolean_array:
                    {
                        bool * values;
                        std::size_t values_length;
                        res_ = get_boolean_array(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            values, values_length, allocator_);
                        if (res_ == core::ok)
                        {
                            res_ = do_set_boolean_array(
                                name, name_length,
                                values, values_length,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_, true);
                        }
                    }
                    break;
                case core::integer_array:
                    {
                        int * values;
                        std::size_t values_length;
                        res_ = get_integer_array(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            values, values_length, allocator_);
                        if (res_ == core::ok)
                        {
                            res_ = do_set_integer_array(
                                name, name_length,
                                values, values_length,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_, true);
                        }
                    }
                    break;
                case core::long_long_array:
                    {
                        long long * values;
                        std::size_t values_length;
                        res_ = get_long_long_array(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            values, values_length, allocator_);
                        if (res_ == core::ok)
                        {
                            res_ = do_set_long_long_array(
                                name, name_length,
                                values, values_length,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_, true);
                        }
                    }
                    break;
                case core::double_float_array:
                    {
                        double * values;
                        std::size_t values_length;
                        res_ = get_double_float_array(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            values, values_length, allocator_);
                        if (res_ == core::ok)
                        {
                            res_ = do_set_double_float_array(
                                name, name_length,
                                values, values_length,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_, true);
                        }
                    }
                    break;
                case core::string_array:
                    {
                        int tmp;
                        res_ = get_integer(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            tmp);
                        if (res_ == core::ok)
                        {
                            const std::size_t array_length =
                                static_cast<std::size_t>(tmp);
                            std::size_t item_index;
                            res_ = do_create_string_array(
                                name, name_length,
                                array_length, item_index,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_);
                            for (std::size_t i = 0;
                                 res_ == core::ok && i != array_length;
                                 ++i)
                            {
                                const char * value;
                                std::size_t value_length;
                                res_ = get_string(
                                    buffers_, buffer_sizes_,
                                    num_of_buffers_,
                                    current_buffer_, buffer_position_,
                                    value, value_length,
                                    allocator_);
                                if (res_ == core::ok)
                                {
                                    res_ = do_place_string_in_array(
                                        item_index, i,
                                        value, value_length,
                                        (*current_object_)->data_,
                                        allocator_);
                                    if (res_ != core::ok)
                                    {
                                        allocator_.deallocate(value);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case core::binary_array:
                    {
                        int tmp;
                        res_ = get_integer(
                            buffers_, buffer_sizes_, num_of_buffers_,
                            current_buffer_, buffer_position_,
                            tmp);
                        if (res_ == core::ok)
                        {
                            const std::size_t array_length =
                                static_cast<std::size_t>(tmp);
                            std::size_t item_index;
                            res_ = do_create_binary_array(
                                name, name_length,
                                array_length, item_index,
                                (*current_object_)->data_,
                                (*current_object_)->num_of_entries_,
                                allocator_);
                            for (std::size_t i = 0;
                                 res_ == core::ok && i != array_length;
                                 ++i)
                            {
                                const char * value;
                                std::size_t value_length;
                                res_ = get_string(
                                    buffers_, buffer_sizes_,
                                    num_of_buffers_,
                                    current_buffer_, buffer_position_,
                                    value, value_length,
                                    allocator_);
                                if (res_ == core::ok)
                                {
                                    res_ = do_place_binary_in_array(
                                        item_index, i,
                                        value, value_length,
                                        (*current_object_)->data_,
                                        allocator_);
                                    if (res_ != core::ok)
                                    {
                                        allocator_.deallocate(value);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case core::nested_parameters:
                    // create new frame on the stack
                    // for the nested object
                    if (current_nesting_level_ < max_nesting_level - 1)
                    {
                        core::parameters * nested;
                        res_ = (*current_object_)->create_nested_parameters(
                            name, name_length, nested);
                        if (res_ == core::ok)
                        {
                            ++current_nesting_level_;
                            changed_frame = true;

                            current_object_ =
                                &object_stack_[current_nesting_level_];
                            state_ = &state_stack_[current_nesting_level_];
                            size_ = &size_stack_[current_nesting_level_];
                            index_ = &index_stack_[current_nesting_level_];

                            *current_object_ = nested;
                            *state_ = processing_preamble;
                        }
                    }
                    else
                    {
                        res_ = core::nesting_too_deep;
                    }
                    break;
                default:
                    fatal_failure(__FILE__, __LINE__);
                }
            }
        }

        // optimization opportunity:
        // reuse the name buffer between iterations
        // or even fuse it directly into the entry
        allocator_.deallocate(name);
    }

    if (res_ == core::ok)
    {
        // if there was no frame change on the "stack" due to nesting,
        // continue iteration over the current list
        // (if case of frame change the iteration is continued when the frame
        // is popped in finished state)

        if (changed_frame == false)
        {
            ++(*index_);
            if (*index_ == *size_)
            {
                *state_ = finished;
            }
        }
    }
}

void deserialize_fsm::finish()
{
    // finished with entries in the current object
    // -> go up one level or quit

    if (current_nesting_level_ == 0)
    {
        done_ = true;
    }
    else
    {
        --current_nesting_level_;

        current_object_ = &object_stack_[current_nesting_level_];
        state_ = &state_stack_[current_nesting_level_];
        size_ = &size_stack_[current_nesting_level_];
        index_ = &index_stack_[current_nesting_level_];

        if (*index_ != *size_)
        {
            ++(*index_);
            if (*index_ == *size_)
            {
                *state_ = finished;
            }
        }
    }
}

lock_fsm::lock_fsm(core::parameters & params, bool locking, long long key)
    : fsm_preserving_structure(params),
      locking_(locking), key_(key)
{
}

void lock_fsm::process_preamble()
{
    if ((*current_object_)->num_of_entries_ != 0)
    {
        *state_ = processing_entries;
        *index_ = 0;
    }
    else
    {
        *state_ = finished;
    }
}

void lock_fsm::process_entries()
{
    pre_process_entries();

    entry & e = const_cast<entry &>((*base_)[*index_]);

    if (e.type != core::unused)
    {
        // note: when processing nested parameters,
        // the errors from lock/unlock are ignored
        // (existing locks should be retained)

        if (locking_)
        {
            (void)e.lock(key_);
        }
        else
        {
            (void)e.unlock(key_);
        }

        if (e.type == core::nested_parameters)
        {
            push_stack_frame(e);
        }
    }

    advance();
}

check_locked_fsm::check_locked_fsm(core::parameters & params, bool & locked)
    : fsm_preserving_structure(params),
      locked_(locked)
{
    locked = false;
}

void check_locked_fsm::process_preamble()
{
    if ((*current_object_)->num_of_entries_ != 0)
    {
        *state_ = processing_entries;
        *index_ = 0;
    }
    else
    {
        *state_ = finished;
    }
}

void check_locked_fsm::process_entries()
{
    pre_process_entries();

    entry & e = const_cast<entry &>((*base_)[*index_]);

    if (e.type != core::unused)
    {
        if (e.locked)
        {
            locked_ = true;
            done_ = true;
        }

        if (e.type == core::nested_parameters)
        {
            push_stack_frame(e);
        }
    }

    advance();
}

std::size_t details::find_entry(
    const entry * data, std::size_t num_of_entries,
    const char * name, std::size_t name_length)
{
    std::size_t index = num_of_entries;

    if (data != NULL)
    {
        for (std::size_t i = 0; i != num_of_entries; ++i)
        {
            if (data[i].type != core::unused &&
                data[i].name_equals(name, name_length))
            {
                index = i;
                break;
            }
        }
    }

    return index;
}

core::result details::find_empty_entry(
    entry * & data, std::size_t & num_of_entries,
    std::size_t & index, allocator & alloc)
{
    // note: initialization here is not needed
    // (the logic later on guarantees that it will be always set)
    // but exists to please less smart compilers

    core::result res = core::ok;

    if (data == NULL)
    {
        // no data entries, this will be the first one

        data = static_cast<entry*>(
            alloc.allocate(sizeof(entry) * initial_number_of_entries));
        if (data != NULL)
        {
            num_of_entries = initial_number_of_entries;
            for (std::size_t i = 0; i != num_of_entries; ++i)
            {
                data[i].type = core::unused;
            }
            index = 0;
            res = core::ok;
        }
        else
        {
            res = core::no_memory;
        }
    }
    else
    {
        // some entries already exist,
        // try to find an empty one

        for (std::size_t i = 0; i != num_of_entries; ++i)
        {
            if (data[i].type == core::unused)
            {
                index = i;
                res = core::ok;
                break;
            }
        }

        if (index == num_of_entries)
        {
            // no empty entry found, expand the set

            const std::size_t new_capacity = 2 * num_of_entries;
            entry * new_data = static_cast<entry *>(
                alloc.allocate(sizeof(entry) * new_capacity));
            if (new_data != NULL)
            {
                std::memcpy(new_data, data, sizeof(entry) * num_of_entries);
                alloc.deallocate(data);
                data = new_data;

                for (std::size_t i = num_of_entries; i != new_capacity; ++i)
                {
                    data[i].type = core::unused;
                }
                index = num_of_entries;
                num_of_entries = new_capacity;

                res = core::ok;
            }
            else
            {
                res = core::no_memory;
            }
        }
    }

    return res;
}

core::result details::prepare_for_set(
    entry * & data, std::size_t & num_of_entries,
    const char * name, std::size_t name_length,
    std::size_t & index, allocator & alloc)
{
    core::result res;

    index = find_entry(data, num_of_entries, name, name_length);
    if (index != num_of_entries)
    {
        // existing entry found

        entry & e = data[index];

        bool locked;
        res = e.is_locked_or_contains_locked(locked);

        if (res == core::ok)
        {
            if (locked == false)
            {
                if (e.type != core::nested_parameters)
                {
                    // simple entries and arrays

                    e.clear_item(alloc);
                }
                else
                {
                    // nested entry needs to be deeply destroyed

                    e.item.nested->clear();
                    alloc.deallocate(e.item.nested);
                }
            }
            else
            {
                // this entry is either locked or is a nested entry
                // that contains some locked entry
                // - cannot be reset as a whole to some other value
                
                res = core::bad_state;
            }
        }
    }
    else
    {
        res = find_empty_entry(data, num_of_entries, index, alloc);
        if (res == core::ok)
        {
            data[index].locked = false;
            res = data[index].set_name(name, name_length, alloc);
        }
    }

    return res;
}

std::size_t details::find_next_used(
    const entry * data, std::size_t num_of_entries,
    std::size_t current_index)
{
    std::size_t res = num_of_entries;
    for (std::size_t i = current_index + 1; i != num_of_entries; ++i)
    {
        if (data[i].type != core::unused)
        {
            res = i;
            break;
        }
    }

    return res;
}

core::result details::do_set_string(
    const char * name, std::size_t name_length,
    const char * value, std::size_t value_length,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc, bool own)
{
    std::size_t index;
    const core::result res = prepare_for_set(data, num_of_entries,
        name, name_length, index, alloc);

    if (res == core::ok)
    {
        entry * e = data + index;

        e->type = core::string;
        e->item.str.value = value;
        e->item.str.length = value_length;
        e->item.str.own = own;
    }

    return res;
}

core::result details::do_set_binary(
    const char * name, std::size_t name_length,
    const void * value, std::size_t value_length,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc, bool own)
{
    std::size_t index;
    const core::result res = prepare_for_set(data, num_of_entries,
        name, name_length, index, alloc);

    if (res == core::ok)
    {
        entry * e = data + index;

        e->type = core::binary;
        e->item.bin.value = value;
        e->item.bin.length = value_length;
        e->item.bin.own = own;
    }

    return res;
}

core::result details::do_set_boolean_array(
    const char * name, std::size_t name_length,
    const bool * values, std::size_t array_length,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc, bool own)
{
    std::size_t index;
    const core::result res = prepare_for_set(data, num_of_entries,
        name, name_length, index, alloc);

    if (res == core::ok)
    {
        entry * e = data + index;

        e->type = core::boolean_array;
        e->item.ba.values = const_cast<bool *>(values);
        e->item.ba.length = array_length;
        e->item.ba.own = own;
    }

    return res;
}

core::result details::do_set_integer_array(
    const char * name, std::size_t name_length,
    const int * values, std::size_t array_length,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc, bool own)
{
    std::size_t index;
    const core::result res = prepare_for_set(data, num_of_entries,
        name, name_length, index, alloc);

    if (res == core::ok)
    {
        entry * e = data + index;

        e->type = core::integer_array;
        e->item.ia.values = const_cast<int *>(values);
        e->item.ia.length = array_length;
        e->item.ia.own = own;
    }

    return res;
}

core::result details::do_set_long_long_array(
    const char * name, std::size_t name_length,
    const long long * values, std::size_t array_length,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc, bool own)
{
    std::size_t index;
    const core::result res = prepare_for_set(data, num_of_entries,
        name, name_length, index, alloc);

    if (res == core::ok)
    {
        entry * e = data + index;

        e->type = core::long_long_array;
        e->item.La.values = const_cast<long long *>(values);
        e->item.La.length = array_length;
        e->item.La.own = own;
    }

    return res;
}

core::result details::do_set_double_float_array(
    const char * name, std::size_t name_length,
    const double * values, std::size_t array_length,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc, bool own)
{
    std::size_t index;
    const core::result res = prepare_for_set(data, num_of_entries,
        name, name_length, index, alloc);

    if (res == core::ok)
    {
        entry * e = data + index;

        e->type = core::double_float_array;
        e->item.da.values = const_cast<double *>(values);
        e->item.da.length = array_length;
        e->item.da.own = own;
    }

    return res;
}

core::result details::do_create_string_array(
    const char * name, std::size_t name_length,
    std::size_t array_length, std::size_t & index,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc)
{
    core::result res;

    const std::size_t raw_length =
        array_length * sizeof(string_array_element);
    string_array_element * new_array =
        static_cast<string_array_element *>(
            alloc.allocate(raw_length));
    if (new_array != NULL)
    {
        for (std::size_t i = 0; i != array_length; ++i)
        {
            new_array[i].value = NULL;
            new_array[i].length = 0;
        }

        res = prepare_for_set(data, num_of_entries,
            name, name_length, index, alloc);

        if (res == core::ok)
        {
            entry * e = data + index;

            e->type = core::string_array;
            e->item.sa.values = new_array;
            e->item.sa.length = array_length;
            e->item.sa.own = true;
        }
        else
        {
            alloc.deallocate(new_array);
        }
    }
    else
    {
        res = core::no_memory;
    }

    return res;
}

core::result details::do_create_binary_array(
    const char * name, std::size_t name_length,
    std::size_t array_length, std::size_t & index,
    entry * & data, std::size_t & num_of_entries,
    allocator & alloc)
{
    core::result res;

    const std::size_t raw_length =
        array_length * sizeof(binary_array_element);
    binary_array_element * new_array =
        static_cast<binary_array_element *>(
            alloc.allocate(raw_length));
    if (new_array != NULL)
    {
        for (std::size_t i = 0; i != array_length; ++i)
        {
            new_array[i].value = NULL;
            new_array[i].length = 0;
        }

        res = prepare_for_set(data, num_of_entries,
            name, name_length, index, alloc);

        if (res == core::ok)
        {
            entry * e = data + index;

            e->type = core::binary_array;
            e->item.bina.values = new_array;
            e->item.bina.length = array_length;
            e->item.bina.own = true;
        }
        else
        {
            alloc.deallocate(new_array);
        }
    }
    else
    {
        res = core::no_memory;
    }

    return res;
}

core::result details::do_set_string_in_array(
    std::size_t item_index, std::size_t array_index,
    const char * value, std::size_t value_length,
    entry * data, allocator & alloc)
{
    core::result res;

    char * new_buffer =
        static_cast<char *>(alloc.allocate(value_length));
    if (new_buffer != NULL)
    {
        std::memcpy(new_buffer, value, value_length);

        res = do_place_string_in_array(
            item_index, array_index,
            new_buffer, value_length,
            data, alloc);

        if (res != core::ok)
        {
            alloc.deallocate(new_buffer);
        }
    }
    else
    {
        res = core::no_memory;
    }

    return res;
}

core::result details::do_set_binary_in_array(
    std::size_t item_index, std::size_t array_index,
    const void * value, std::size_t value_length,
    entry * data, allocator & alloc)
{
    core::result res;

    void * new_buffer = alloc.allocate(value_length);
    if (new_buffer != NULL)
    {
        std::memcpy(new_buffer, value, value_length);

        res = do_place_binary_in_array(
            item_index, array_index,
            new_buffer, value_length,
            data, alloc);

        if (res != core::ok)
        {
            alloc.deallocate(new_buffer);
        }
    }
    else
    {
        res = core::no_memory;
    }

    return res;
}

core::result details::do_place_string_in_array(
    std::size_t item_index, std::size_t array_index,
    const char * value, std::size_t value_length,
    entry * data, allocator & alloc)
{
    core::result res;

    const entry * e = data + item_index;
    if (e->type == core::string_array)
    {
        if (array_index < e->item.sa.length)
        {
            if (e->locked)
            {
                res = core::bad_state;
            }
            else
            {
                e->item.sa.values[array_index].clear(alloc);
                e->item.sa.values[array_index].value = value;
                e->item.sa.values[array_index].length = value_length;
                res = core::ok;
            }
        }
        else
        {
            res = core::no_such_index;
        }
    }
    else
    {
        res = core::bad_type;
    }

    return res;
}

core::result details::do_place_binary_in_array(
    std::size_t item_index, std::size_t array_index,
    const void * value, std::size_t value_length,
    entry * data, allocator & alloc)
{
    core::result res;

    const entry * e = data + item_index;
    if (e->type == core::binary_array)
    {
        if (array_index < e->item.sa.length)
        {
            if (e->locked)
            {
                res = core::bad_state;
            }
            else
            {
                e->item.bina.values[array_index].clear(alloc);
                e->item.bina.values[array_index].value = value;
                e->item.bina.values[array_index].length = value_length;
                res = core::ok;
            }
        }
        else
        {
            res = core::no_such_index;
        }
    }
    else
    {
        res = core::bad_type;
    }

    return res;
}
