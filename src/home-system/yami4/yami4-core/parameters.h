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

#ifndef YAMICORE_PARAMETERS_H_INCLUDED
#define YAMICORE_PARAMETERS_H_INCLUDED

#include "allocator.h"
#include "core.h"
#include "dll.h"
#include "parameter_type.h"
#include "serializable.h"

namespace yami
{

namespace details
{
struct entry;
class fsm_common;
class fsm_preserving_structure;
class clear_fsm;
class get_serialize_buffer_size_fsm;
class serialize_fsm;
class deserialize_fsm;
class lock_fsm;
class check_locked_fsm;

// for unit tests, to avoid dependency on IOStreams from core
class dump_sink
{
public:
    virtual ~dump_sink() {}

    virtual void indent(std::size_t spaces) = 0;
    virtual void dump(std::size_t v) = 0;
    virtual void dump(bool v) = 0;
    virtual void dump(int v) = 0;
    virtual void dump(long long v) = 0;
    virtual void dump(double v) = 0;
    virtual void dump(const char * str) = 0;
    virtual void dump(const char * str, std::size_t str_len) = 0;
};

} // namespace details

namespace core
{

class parameter_entry;
class parameter_iterator;

/// \brief Collection of message parameters.
///
/// The collection of message parameters, which are typed {name, value} pairs.
///
/// Each entry in this collection has a unique name and can have
/// one of the following types:
/// - bool or bool array
/// - int or int array
/// - long long or long long array
/// - double or double array
/// - string or string array
/// - binary or binary array
/// - nested parameters object, which provides its own scope for naming.
///
/// Each entry has a name that can be either null-terminated
/// or contain embedded zeros and appropriate member functions are provided
/// for both cases. The names are searched for using
/// case-sensitive comparisons.
/// The name of entry is always kept by copy.
///
/// This class is not copyable and not thread-safe, although distinct
/// instances of this class can be used by different threads without
/// synchronization.
///
/// <b>Note:</b>
/// The entries are <i>ordered</i> - the order in which they are created
/// influences the final serialized form of the message payload.<br />
/// Newly created entries are appended to the end of the collection unless
/// there is an existing empty slot that can be reused - the appropriate
/// slot is searched for from the beginning to the end of the collection
/// and if no free slot is found the collection is extended at the end.<br />
/// The above guarantee concerns the user code that relies on
/// predictable serialization.
///
class DLL parameters : public serializable
{
public:
    /// \brief Constructor.
    ///
    /// Creates an empty parameters object.
    /// @param working_area If not NULL, forces this object to use the given
    ///        area for all private storage.
    /// @param area_size If different than 0, indicates the size of the
    ///        private memory area that will be used for all dependent
    ///        structures.
    ///
    /// <b>Notes:</b>
    /// - If <code>working_area == NULL && area_size == 0</code>
    ///   (the default) then all dependent objects are separately allocated
    ///   on the global store.
    /// - If <code>working_area != NULL && area_size != 0</code> then
    ///   the given block is used as a private area.
    ///
    /// <b>Note:</b>
    /// Do not attempt to create objects sharing the same working area.
    explicit parameters(
        void * working_area = NULL, std::size_t area_size = 0);

    /// \brief Destructor.
    ///
    /// The destructor cleans all dependent resources
    /// <i>only</i> if each of them was separately allocated
    /// on the global store.
    /// If the parameters object was created with externally provided
    /// working area (that is, when <code>working_area != NULL</code> when
    /// the object was constructed) then no cleanup is performed.
    ~parameters();

    // single-value operations

    /// \brief Inserts new entry of type bool.
    ///
    /// Inserts a new entry of type bool to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be set.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result set_boolean(const char * name, std::size_t name_length,
        bool value);

    /// \brief Inserts new entry of type bool.
    ///
    /// Inserts a new entry of type bool with null-terminated name.
    /// See the other overloaded function for details.
    result set_boolean(const char * name, bool value);

    /// \brief Extracts the bool value from the given entry.
    ///
    /// Extracts the bool value from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain bool
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_boolean(const char * name, std::size_t name_length,
        bool & value) const;

    /// \brief Extracts the bool value from the given entry.
    ///
    /// Extracts the bool value from the entry given by null-terminated name.
    /// See the other overloaded function for details.
    result get_boolean(const char * name, bool & value) const;

    /// \brief Inserts new entry of type int.
    ///
    /// Inserts a new entry of type int to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be set.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result set_integer(const char * name, std::size_t name_length, int value);

    /// \brief Inserts new entry of type int.
    ///
    /// Inserts a new entry of type int with null-terminated name.
    /// See the other overloaded function for details.
    result set_integer(const char * name, int value);

    /// \brief Extracts the int value from the given entry.
    ///
    /// Extracts the int value from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain int
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_integer(const char * name, std::size_t name_length,
        int & value) const;

    /// \brief Extracts the int value from the given entry.
    ///
    /// Extracts the int value from the entry given by null-terminated name.
    /// See the other overloaded function for details.
    result get_integer(const char * name, int & value) const;

    /// \brief Inserts new entry of type long long.
    ///
    /// Inserts a new entry of type long long to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be set.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result set_long_long(const char * name, std::size_t name_length,
        long long value);

    /// \brief Inserts new entry of type long long.
    ///
    /// Inserts a new entry of type long long with null-terminated name.
    /// See the other overloaded function for details.
    result set_long_long(const char * name, long long value);

    /// \brief Extracts the long long value from the given entry.
    ///
    /// Extracts the long long value from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain long long
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_long_long(const char * name, std::size_t name_length,
        long long & value) const;

    /// \brief Extracts the long long value from the given entry.
    ///
    /// Extracts the long long value from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_long_long(const char * name, long long & value) const;

    /// \brief Inserts new entry of type double.
    ///
    /// Inserts a new entry of type double to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be set.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result set_double_float(const char * name, std::size_t name_length,
        double value);

    /// \brief Inserts new entry of type double.
    ///
    /// Inserts a new entry of type double with null-terminated name.
    /// See the other overloaded function for details.
    result set_double_float(const char * name, double value);

    /// \brief Extracts the double value from the given entry.
    ///
    /// Extracts the double value from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain double
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_double_float(const char * name, std::size_t name_length,
        double & value) const;

    /// \brief Extracts the double value from the given entry.
    ///
    /// Extracts the double value from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_double_float(const char * name, double & value) const;

    /// \brief Inserts new entry of type string.
    ///
    /// Inserts a new entry of type string to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Pointer to the value buffer to be set.
    ///        The buffer can contain zeros.
    /// @param value_length Length of the value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The value is <i>copied</i> to the internal buffer.
    result set_string(const char * name, std::size_t name_length,
        const char * value, std::size_t value_length);

    /// \brief Inserts new entry of type string.
    ///
    /// Inserts a new entry of type string with null-terminated
    /// name and value. See the other overloaded function for details.
    result set_string(const char * name, const char * value);

    /// \brief Inserts new entry of type string.
    ///
    /// Inserts a new entry of type string to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Pointer to the value buffer to be set.
    ///        The buffer can contain zeros.
    /// @param value_length Length of the value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The value is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    result set_string_shallow(const char * name, std::size_t name_length,
        const char * value, std::size_t value_length);

    /// \brief Inserts new entry of type string.
    ///
    /// Inserts (without copy) a new entry of type string with null-terminated
    /// name and value. See the other overloaded function for details.
    result set_string_shallow(const char * name, const char * value);

    /// \brief Extracts the string value from the given entry.
    ///
    /// Extracts the string value from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain string
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_string(const char * name, std::size_t name_length,
        const char * & value, std::size_t & value_length) const;

    /// \brief Extracts the string value from the given entry.
    ///
    /// Extracts the string value from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_string(const char * name,
        const char * & value, std::size_t & value_length) const;

    /// \brief Inserts new entry of type binary.
    ///
    /// Inserts a new entry of type binary to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Pointer to the value buffer to be set.
    /// @param value_length Length of the value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The value is <i>copied</i> to the internal buffer.
    result set_binary(const char * name, std::size_t name_length,
        const void * value, std::size_t value_length);

    /// \brief Inserts new entry of type binary.
    ///
    /// Inserts a new entry of type binary with null-terminated name.
    /// See the other overloaded function for details.
    result set_binary(const char * name,
        const void * value, std::size_t value_length);

    /// \brief Inserts new entry of type binary.
    ///
    /// Inserts a new entry of type binary to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Pointer to the value buffer to be set.
    /// @param value_length Length of the value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The value is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    result set_binary_shallow(const char * name, std::size_t name_length,
        const void * value, std::size_t value_length);

    /// \brief Inserts new entry of type binary.
    ///
    /// Inserts (without copy) a new entry of type binary with null-terminated
    /// name. See the other overloaded function for details.
    result set_binary_shallow(const char * name,
        const void * value, std::size_t value_length);

    /// \brief Extracts the binary value from the given entry.
    ///
    /// Extracts the binary value from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain binary
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_binary(const char * name, std::size_t name_length,
        const void * & value, std::size_t & value_length) const;

    /// \brief Extracts the binary value from the given entry.
    ///
    /// Extracts the binary value from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_binary(const char * name,
        const void * & value, std::size_t & value_length) const;

    // array operations

    /// \brief Inserts new entry of type bool array.
    ///
    /// Inserts a new entry of type bool array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    result set_boolean_array(const char * name, std::size_t name_length,
        const bool * values, std::size_t array_length);

    /// \brief Inserts new entry of type bool array.
    ///
    /// Inserts a new entry of type bool array with null-terminated name.
    /// See the other overloaded function for details.
    result set_boolean_array(const char * name,
        const bool * values, std::size_t array_length);

    /// \brief Inserts new entry of type bool array.
    ///
    /// Inserts a new entry of type bool array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    result set_boolean_array_shallow(
        const char * name, std::size_t name_length,
        const bool * values, std::size_t array_length);

    /// \brief Inserts new entry of type bool array.
    ///
    /// Inserts (without copy) a new entry of type bool array
    /// with null-terminated name and value.
    /// See the other overloaded function for details.
    result set_boolean_array_shallow(const char * name,
        const bool * values, std::size_t array_length);

    /// \brief Creates new uninitialized entry of type bool array.
    ///
    /// Creates a new entry of type bool array in the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the new array.
    /// @param values Returned pointer to the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result create_boolean_array(const char * name, std::size_t name_length,
        std::size_t array_length, bool * & array);

    /// \brief Extracts the array of bool values from the given entry.
    ///
    /// Extracts the array of bool values from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain bool array
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_boolean_array(const char * name, std::size_t name_length,
        bool * & values, std::size_t & array_length) const;

    /// \brief Extracts the array of bool values from the given entry.
    ///
    /// Extracts the array of bool values from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_boolean_array(const char * name,
        bool * & values, std::size_t & array_length) const;

    /// \brief Inserts new entry of type int array.
    ///
    /// Inserts a new entry of type int array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    result set_integer_array(const char * name, std::size_t name_length,
        const int * values, std::size_t array_length);

    /// \brief Inserts new entry of type int array.
    ///
    /// Inserts a new entry of type int array with null-terminated name.
    /// See the other overloaded function for details.
    result set_integer_array(const char * name,
        const int * values, std::size_t array_length);

    /// \brief Inserts new entry of type int array.
    ///
    /// Inserts a new entry of type int array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    result set_integer_array_shallow(
        const char * name, std::size_t name_length,
        const int * values, std::size_t array_length);

    /// \brief Inserts new entry of type int array.
    ///
    /// Inserts (without copy) a new entry of type int array
    /// with null-terminated name and value.
    /// See the other overloaded function for details.
    result set_integer_array_shallow(const char * name,
        const int * values, std::size_t array_length);

    /// \brief Creates new uninitialized entry of type int array.
    ///
    /// Creates a new entry of type int array in the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the new array.
    /// @param values Returned pointer to the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result create_integer_array(const char * name, std::size_t name_length,
        std::size_t array_length, int * & array);

    /// \brief Extracts the array of int values from the given entry.
    ///
    /// Extracts the array of int values from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain int array
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_integer_array(const char * name, std::size_t name_length,
        int * & values, std::size_t & array_length) const;

    /// \brief Extracts the array of int values from the given entry.
    ///
    /// Extracts the array of int values from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_integer_array(const char * name,
        int * & values, std::size_t & array_length) const;

    /// \brief Inserts new entry of type long long array.
    ///
    /// Inserts a new entry of type long long array
    /// to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    result set_long_long_array(const char * name, std::size_t name_length,
        const long long * values, std::size_t array_length);

    /// \brief Inserts new entry of type long long array.
    ///
    /// Inserts a new entry of type long long array with null-terminated name.
    /// See the other overloaded function for details.
    result set_long_long_array(const char * name,
        const long long * values, std::size_t array_length);

    /// \brief Inserts new entry of type long long array.
    ///
    /// Inserts a new entry of type long long array
    /// to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    result set_long_long_array_shallow(
        const char * name, std::size_t name_length,
        const long long * values, std::size_t array_length);

    /// \brief Inserts new entry of type long long array.
    ///
    /// Inserts (without copy) a new entry of type long long array
    /// with null-terminated name and value.
    /// See the other overloaded function for details.
    result set_long_long_array_shallow(const char * name,
        const long long * values, std::size_t array_length);

    /// \brief Creates new uninitialized entry of type long long array.
    ///
    /// Creates a new entry of type long long array
    /// in the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the new array.
    /// @param values Returned pointer to the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result create_long_long_array(const char * name, std::size_t name_length,
        std::size_t array_length, long long * & array);

    /// \brief Extracts the array of long long values from the given entry.
    ///
    /// Extracts the array of long long values from the entry
    /// given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain long long array
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_long_long_array(const char * name, std::size_t name_length,
        long long * & values, std::size_t & array_length) const;

    /// \brief Extracts the array of long long values from the given entry.
    ///
    /// Extracts the array of long long values from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_long_long_array(const char * name,
        long long * & values, std::size_t & array_length) const;

    /// \brief Inserts new entry of type double array.
    ///
    /// Inserts a new entry of type double array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    result set_double_float_array(const char * name, std::size_t name_length,
        const double * values, std::size_t array_length);

    /// \brief Inserts new entry of type double array.
    ///
    /// Inserts a new entry of type double array with null-terminated name.
    /// See the other overloaded function for details.
    result set_double_float_array(const char * name,
        const double * values, std::size_t array_length);

    /// \brief Inserts new entry of type double array.
    ///
    /// Inserts a new entry of type double array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    result set_double_float_array_shallow(
        const char * name, std::size_t name_length,
        const double * values, std::size_t array_length);

    /// \brief Inserts new entry of type double array.
    ///
    /// Inserts (without copy) a new entry of type double array
    /// with null-terminated name and value.
    /// See the other overloaded function for details.
    result set_double_float_array_shallow(const char * name,
        const double * values, std::size_t array_length);

    /// \brief Creates new uninitialized entry of type double float array.
    ///
    /// Creates a new entry of type double float array
    /// in the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the new array.
    /// @param values Returned pointer to the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    result create_double_float_array(
        const char * name, std::size_t name_length,
        std::size_t array_length, double * & array);

    /// \brief Extracts the array of double values from the given entry.
    ///
    /// Extracts the array of double values from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain double array
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_double_float_array(const char * name, std::size_t name_length,
        double * & values, std::size_t & array_length) const;

    /// \brief Extracts the array of double values from the given entry.
    ///
    /// Extracts the array of double values from the entry given by
    /// null-terminated name. See the other overloaded function for details.
    result get_double_float_array(const char * name,
        double * & values, std::size_t & array_length) const;

    /// \brief Creates new empty entry of type string array.
    ///
    /// Creates a new empty entry of type string array.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the newly created array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> After creation the array contains empty strings
    /// (that is, strings which length is 0).
    result create_string_array(const char * name, std::size_t name_length,
        std::size_t array_length);

    /// \brief Creates new empty entry of type string array.
    ///
    /// Creates a new empty entry of type string array
    /// with null-terminated name.
    /// See the other overloaded function for details.
    result create_string_array(const char * name, std::size_t array_length);

    /// \brief Inserts new string value to string array.
    ///
    /// Inserts a new string value (possibly replacing the old one)
    /// to already existing string array at the given index.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param index The array index (array slot, starting from 0).
    /// @param value Pointer to the value buffer to be set.
    ///        The buffer can contain zeros.
    /// @param value_length Length of the value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain string array
    ///         - <code>no_such_index</code>
    ///           if the given index is out of range
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///
    /// <b>Note:</b> The value is <i>copied</i> to the internal buffer.
    result set_string_in_array(const char * name, std::size_t name_length,
        std::size_t index, const char * value, std::size_t value_length);

    /// \brief Inserts new string value to string array.
    ///
    /// Inserts a new null-terminated string value
    /// to already existing string array (given by null-terminated name)
    /// at the given index. See the other overloaded function for details.
    result set_string_in_array(const char * name, std::size_t index,
        const char * value);

    /// \brief Extracts the length of string array.
    ///
    /// Extracts the length of string array
    /// that is located at the given entry.
    ///
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain string array
    result get_string_array_length(const char * name, std::size_t name_length,
        std::size_t & array_length) const;

    /// \brief Extracts string value from string array.
    ///
    /// Extracts the string value from the given index of string array.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain string array
    ///         - <code>no_such_index</code>
    ///           if the given index is out of range
    result get_string_in_array(const char * name, std::size_t name_length,
        std::size_t index,
        const char * & value, std::size_t & value_length) const;

    /// \brief Extracts string value from string array.
    ///
    /// Extracts the string value from the given index of string array
    /// identified by null-terminated name.
    /// See the other overloaded function for details.
    result get_string_in_array(const char * name, std::size_t index,
        const char * & value, std::size_t & value_length) const;

    /// \brief Creates new empty entry of type binary array.
    ///
    /// Creates a new empty entry of type binary array.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the newly created array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///
    /// <b>Note:</b> After creation the array contains empty binaries
    /// (that is, binaries which length is 0).
    result create_binary_array(const char * name, std::size_t name_length,
        std::size_t array_length);

    /// \brief Creates new empty entry of type binary array.
    ///
    /// Creates a new empty entry of type binary array
    /// with null-terminated name.
    /// See the other overloaded function for details.
    result create_binary_array(const char * name, std::size_t array_length);

    /// \brief Inserts new binary value to binary array.
    ///
    /// Inserts a new binary value (possibly replacing the old one)
    /// to already existing binary array at the given index.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param index The array index (array slot, starting from 0).
    /// @param value Pointer to the value buffer to be set.
    /// @param value_length Length of the value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain binary array
    ///         - <code>no_such_index</code>
    ///           if the given index is out of range
    ///         - <code>no_memory</code> if the new entry cannot be allocated
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///
    /// <b>Note:</b> The value is <i>copied</i> to the internal buffer.
    result set_binary_in_array(const char * name, std::size_t name_length,
        std::size_t index, const void * value, std::size_t value_length);

    /// \brief Inserts new binary value to binary array.
    ///
    /// Inserts a new binary value
    /// to already existing binary array (given by null-terminated name)
    /// at the given index. See the other overloaded function for details.
    result set_binary_in_array(const char * name, std::size_t index,
        const void * value, std::size_t value_length);

    /// \brief Extracts the length of binary array.
    ///
    /// Extracts the length of binary array
    /// that is located at the given entry.
    ///
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param array_length Length of the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain binary array
    result get_binary_array_length(const char * name, std::size_t name_length,
        std::size_t & array_length) const;

    /// \brief Extracts binary value from binary array.
    ///
    /// Extracts the binary value from the given index of binary array.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain binary array
    ///         - <code>no_such_index</code>
    ///           if the given index is out of range
    result get_binary_in_array(const char * name, std::size_t name_length,
        std::size_t index,
        const void * & value, std::size_t & value_length) const;

    /// \brief Extracts binary value from binary array.
    ///
    /// Extracts the binary value from the given index of binary array
    /// identified by null-terminated name.
    /// See the other overloaded function for details.
    result get_binary_in_array(const char * name, std::size_t index,
        const void * & value, std::size_t & value_length) const;

    // support for data nesting

    /// \brief Creates nested parameters entry.
    ///
    /// Creates a new nested parameters entry in the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param params Pointer to the internally created parameters object.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_memory</code>
    ///           if the new entry cannot be allocated
    result create_nested_parameters(
        const char * name, std::size_t name_length,
        parameters * & params);

    /// \brief Creates nested parameters entry.
    ///
    /// Creates a new nested parameters entry with null-terminated name.
    /// See the other overloaded function for details.
    result create_nested_parameters(const char * name, parameters * & params);

    /// \brief Extracts the nested parameters object from the given entry.
    ///
    /// Extracts nested parameters from the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param params Pointer to the internally stored parameters object.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the given entry
    ///           does not contain nested parameters
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_nested_parameters(
        const char * name, std::size_t name_length,
        parameters * & params) const;

    /// \brief Extracts the nested parameters object from the given entry.
    ///
    /// Extracts nested parameters from the entry given by its
    /// null-terminated name. See the other overloaded function for details.
    result get_nested_parameters(
        const char * name, parameters * & params) const;

    /// \brief Locks the given entry and disallows future modifications.
    ///
    /// Marks the given entry as locked with the provided unlock key.
    /// This operation is possible only when the entry is in the unlocked
    /// state and prevents future modifications of that entry.
    /// The entry can be unlocked if the proper key value is provided.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param key Key value to be used in future unlock operation.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///         - <code>bad_state</code> if the entry is already locked
    result lock(const char * name, std::size_t name_length,
        long long key);

    /// \brief Locks the given entry and disallows future modifications.
    ///
    /// Marks the given entry as locked with the provided unlock key.
    /// See the other overloaded function for details.
    result lock(const char * name, long long key);

    /// \brief Unlocks the given entry.
    ///
    /// Marks the given entry as unlocked with the provided unlock key.
    /// This operation is possible only when the entry is in the locked
    /// state. The provided key value must be equal to the value that
    /// was provided when the entry was locked.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param key Key value.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///         - <code>bad_state</code> if the entry is not locked
    ///         - <code>unexpected_value</code> if the key is wrong
    result unlock(const char * name, std::size_t name_length,
        long long key);

    /// \brief Unlocks the given entry.
    ///
    /// Marks the given entry as unlocked with the provided unlock key.
    /// See the other overloaded function for details.
    result unlock(const char * name, long long key);

    /// \brief Checks whether a given entry is locked.
    ///
    /// Checks whether a given entry is locked and whether the modifications
    /// are disallowed.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param value The locked flag value.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_such_name</code> if the given name cannot be found
    result is_locked(const char * name, std::size_t name_length,
        bool & value) const;

    /// \brief Checks whether a given entry is locked.
    ///
    /// Checks whether a given entry is locked and whether the modifications
    /// are disallowed.
    /// See the other overloaded function for details.
    result is_locked(const char * name, bool & value) const;

    /// \brief Removes the given entry.
    ///
    /// Removes the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_such_name</code> if the given name cannot be found
    ///
    /// <b>Note:</b> The removed entry leaves a <i>hole</i> (empty slot) in
    /// the collection that can be reused by newly added entries.
    result remove(const char * name, std::size_t name_length);

    /// \brief Removes the given entry.
    ///
    /// Removes the entry given by its null-terminated name.
    /// See the other overloaded function for details.
    result remove(const char * name);

    // inspection

    /// \brief Returns the size of the collection.
    ///
    /// Returns the size of the collection - that is,
    /// the number of all non-empty slots.
    /// @return the size of the collection
    std::size_t size() const;

    /// \brief Extracts the type of the given entry.
    ///
    /// Extracts the type of the entry given by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param t The type of the given entry.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_such_name</code> if the given name cannot be found
    result get_type(const char * name, std::size_t name_length,
        parameter_type & t) const;

    /// \brief Extracts the type of the given entry.
    ///
    /// Extracts the type of the entry given by its null-terminated name.
    /// See the other overloaded function for details.
    result get_type(const char * name, parameter_type & t) const;

    // iterator support

    /// \brief Extracts the iterator to all entries.
    ///
    /// Extracts the iterator pointing to the beginning of the collection,
    /// which means the first used slot.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_entries</code> if this object contains no entries
    result get_iterator(parameter_iterator & it) const;

    /// \brief Finds the given entry.
    ///
    /// Extracts the view on the entry specified by its name.
    /// @param name Pointer to the name buffer. The buffer can contain zeros.
    /// @param name_length Length of the name buffer.
    /// @param entry The entry view to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>no_such_name</code> if the given name cannot be found
    result find(const char * name, std::size_t name_length,
        parameter_entry & entry) const;

    /// \brief Finds the given entry.
    ///
    /// Extracts the on the entry specified by its
    /// null-terminated name.
    /// See the other overloaded function for details.
    result find(const char * name, parameter_entry & entry) const;

    /// \brief Merges entries from the given parameters object.
    ///
    /// Merges the entries from another parameters object.
    /// The merged entries can have the same names,
    /// in which case the new entries replace existing ones.
    /// The merging is deep in the sense that no data is shared between
    /// this and other object after the merge - this applies also to
    /// those entries that were shallow references in the other object.
    /// @param other The object to be merged into this object.
    result merge_from(const parameters & other);

    /// \brief Clears the collection of entries.
    ///
    /// Clears the collection of entries and deallocates dependent structures.
    /// After executing the state of this object is as it was
    /// immediately after construction.
    void clear();

    // serialization

    /// \brief Finds the total size of serialization buffer.
    ///
    /// Computes the total size of serialization buffer(s) for the current
    /// content of this object.
    /// @param size The computed size of buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>nesting_too_deep</code> if the level of nesting
    ///           in this object is deeper than the limit
    virtual result get_serialize_buffer_size(std::size_t & size) const;

    /// \brief Serializes current content into given buffer(s).
    ///
    /// Serializes the current content of this object into the given
    /// buffer(s).
    /// The serialization buffer does not have to be contiguous and any number
    /// of buffer segments is allowed, provided that the size of each buffer
    /// segment is a multiple of 4 (32 bits).<br />
    /// The function scatters the serialized data into subsequent buffers
    /// as they become filled.<br />
    /// The buffers are provided as array of buffer pointers and their sizes.
    /// @param buffers Pointer to the array of buffer pointers
    ///        (each of type <code>char *</code>).
    /// @param buffer_sizes Pointer to the array of buffer sizes.
    /// @param num_of_buffers Number of buffers described by the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>nesting_too_deep</code> if the level of nesting
    ///           in this object is deeper than the limit
    ///         - <code>not_enough_space</code> if the buffers are not
    ///           big enough for all the data
    virtual result serialize(char * * buffers,
        const std::size_t * buffer_sizes,
        std::size_t num_of_buffers) const;

    /// \brief Deserializes from the given buffer(s).
    ///
    /// Deserializes content from the given buffer(s).
    /// The data buffer does not have to be contiguous and any number
    /// of buffer segments is allowed, provided that the size of each buffer
    /// segment is a multiple of 4 (32 bits).<br />
    /// The function gathers the serialized data from subsequent buffers
    /// as they are consumed.<br />
    /// The buffers are provided as array of buffer pointers and their sizes.
    /// @param buffers Pointer to the array of buffer pointers
    ///        (each of type <code>const char *</code>).
    /// @param buffer_sizes Pointer to the array of buffer sizes.
    /// @param num_of_buffers Number of buffers described by the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>nesting_too_deep</code> if the level of nesting
    ///           in the data stream is deeper than the limit
    ///         - <code>not_enough_space</code> if the buffers do not
    ///           contain all expected data
    ///         - <code>unexpected_value</code> if the data stream contains
    ///           a value that cannot be properly interpreted
    ///
    /// <b>Note:</b> The current content of this object is not cleared
    /// before attempting deserialization and each retrieved data element
    /// is <i>merged</i> into the current content as if done by individual
    /// calls to appropriate <code>set_XYZ</code> functions.<br />
    /// In most cases deserialization will be performed to the empty
    /// parameters object (to reconstruct it to the form that was used
    /// for serialization), but deserialization onto non-empty object
    /// might be occasionally useful as a way of merging two collections.
    result deserialize(const char * * buffers,
        const std::size_t * buffer_sizes,
        std::size_t num_of_buffers);

    // for unit tests
    void dump(details::dump_sink & sink, std::size_t indent_length = 0) const;

private:
    friend class details::fsm_common;
    friend class details::fsm_preserving_structure;
    friend class details::clear_fsm;
    friend class details::get_serialize_buffer_size_fsm;
    friend class details::serialize_fsm;
    friend class details::deserialize_fsm;
    friend class details::lock_fsm;
    friend class details::check_locked_fsm;

    parameters(const parameters &);
    void operator=(const parameters &);

    parameters(details::allocator & allocator, bool private_area);

    details::allocator own_allocator_;
    details::allocator & allocator_;
    bool uses_private_area_;

    details::entry * data_;
    std::size_t num_of_entries_;
};

} // namespace core

} // namespace yami

#endif // YAMICORE_PARAMETERS_H_INCLUDED
