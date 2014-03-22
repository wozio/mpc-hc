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

#ifndef YAMICPP_PARAMETERS_H_INCLUDED
#define YAMICPP_PARAMETERS_H_INCLUDED

#include "parameter_entry.h"
#include "parameter_type.h"
#include "serializable.h"
#include <yami4-core/dll.h>
#include <yami4-core/parameter_iterator.h>
#include <memory>
#include <ostream>
#include <string>

namespace yami
{

namespace core
{
class parameters;
} // namespace core

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
/// This class is not thread-safe, although distinct
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
/// <b>Note:</b>
/// For each function that accepts <code>const std::string &</code> parameter
/// there is also an overloaded function accepting <code>const char *</code>.
/// These additional functions are provided to make it more natural to use
/// strings by pointer or as a hard-code literals. They have exactly the same
/// functionality and are therefore not separately documented.
class DLL parameters : public serializable
{
public:
    /// \brief Constructor.
    ///
    /// Creates an empty parameters object.
    parameters();

    /// \brief Constructor.
    ///
    /// Constructor, allows to wrap existing object
    /// from the core part of the YAMI library.
    explicit parameters(core::parameters * external);

    /// \brief Copy constructor
    parameters(const parameters & other);

    /// \brief Assignment operator
    void operator=(const parameters & other);

    /// \brief Swap operation.
    ///
    /// The (no-throw) swap operation.
    void swap(parameters & other);

    /// \brief Destructor.
    ///
    /// The destructor cleans all dependent resources.
    ~parameters() {}

    // single-value operations

    /// \brief Inserts new entry of type bool.
    ///
    /// Inserts a new entry of type bool to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Value to be set.
    void set_boolean(const std::string & name, bool value);
    void set_boolean(const char * name, bool value);

    /// \brief Extracts the bool value from the given entry.
    ///
    /// Extracts the bool value from the entry given by its name.
    /// @param name Name of the entry.
    /// @return Value of the entry if it has correct type.
    bool get_boolean(const std::string & name) const;
    bool get_boolean(const char * name) const;

    /// \brief Inserts new entry of type int.
    ///
    /// Inserts a new entry of type int to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Value to be set.
    void set_integer(const std::string & name, int value);
    void set_integer(const char * name, int value);

    /// \brief Extracts the int value from the given entry.
    ///
    /// Extracts the int value from the entry given by its name.
    /// @param name Name of the entry.
    /// @return Value of the entry if it has correct type.
    int get_integer(const std::string & name) const;
    int get_integer(const char * name) const;

    /// \brief Inserts new entry of type long long.
    ///
    /// Inserts a new entry of type long long to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Value to be set.
    void set_long_long(const std::string & name, long long value);
    void set_long_long(const char * name, long long value);

    /// \brief Extracts the long long value from the given entry.
    ///
    /// Extracts the long long value from the entry given by its name.
    /// @param name Name of the entry.
    /// @return Value of the entry if it has correct type.
    long long get_long_long(const std::string & name) const;
    long long get_long_long(const char * name) const;

    /// \brief Inserts new entry of type double.
    ///
    /// Inserts a new entry of type double to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Value to be set.
    void set_double_float(const std::string & name, double value);
    void set_double_float(const char * name, double value);

    /// \brief Extracts the double value from the given entry.
    ///
    /// Extracts the double value from the entry given by its name.
    /// @param name Name of the entry.
    /// @return Value of the entry if it has correct type.
    double get_double_float(const std::string & name) const;
    double get_double_float(const char * name) const;

    /// \brief Inserts new entry of type string.
    ///
    /// Inserts a new entry of type string to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Value to be set.
    void set_string(const std::string & name, const std::string & value);
    void set_string(const char * name, const char * value);

    /// \brief Inserts new entry of type string.
    ///
    /// Inserts a new entry of type string to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Pointer to the value buffer to be set.
    ///        The buffer can contain zeros.
    /// @param value_length Length of the value buffer.
    ///
    /// <b>Note:</b> The value is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    void set_string_shallow(const std::string & name,
        const char * value, std::size_t value_length);
    void set_string_shallow(const char * name, std::size_t name_length,
        const char * value, std::size_t value_length);

    /// \brief Extracts the string value from the given entry.
    ///
    /// Extracts the string value from the entry given by its name.
    /// @param name Name of the entry.
    /// @return Value of the entry if it has correct type.
    std::string get_string(const std::string & name) const;
    std::string get_string(const char * name) const;

    /// \brief Extracts the string value from the given entry.
    ///
    /// Extracts the string value from the entry given by its name
    /// by accessing the buffer directly.
    /// @param name Name of the entry.
    /// @param length Length of the internal buffer.
    /// @return Value of the entry if it has correct type.
    const char * get_string(const std::string & name,
        std::size_t & length) const;
    const char * get_string(const char * name,
        std::size_t & length) const;

    /// \brief Inserts new entry of type binary.
    ///
    /// Inserts a new entry of type binary to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Value to be set.
    void set_binary(const std::string & name,
        const void * value, std::size_t value_length);
    void set_binary(const char * name,
        const void * value, std::size_t value_length);

    /// \brief Inserts new entry of type binary.
    ///
    /// Inserts a new entry of type binary to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param value Pointer to the value buffer to be set.
    ///        The buffer can contain zeros.
    /// @param value_length Length of the value buffer.
    ///
    /// <b>Note:</b> The value is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    void set_binary_shallow(const std::string & name,
        const void * value, std::size_t value_length);
    void set_binary_shallow(const char * name, std::size_t name_length,
        const void * value, std::size_t value_length);

    /// \brief Extracts the binary value from the given entry.
    ///
    /// Extracts the binary value from the entry given by its name
    /// by accessing the buffer directly.
    /// @param name Name of the entry.
    /// @param length Length of the internal buffer.
    /// @return Value of the entry if it has correct type.
    const void * get_binary(const std::string & name,
        std::size_t & length) const;
    const void * get_binary(const char * name,
        std::size_t & length) const;

    // array operations

    /// \brief Inserts new entry of type bool array.
    ///
    /// Inserts a new entry of type bool array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    void set_boolean_array(const std::string & name,
        const bool * values, std::size_t array_length);
    void set_boolean_array(const char * name,
        const bool * values, std::size_t array_length);

    /// \brief Inserts new entry of type bool array.
    ///
    /// Inserts a new entry of type bool array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    void set_boolean_array_shallow(const std::string & name,
        const bool * values, std::size_t array_length);
    void set_boolean_array_shallow(const char * name,
        const bool * values, std::size_t array_length);

    /// \brief Extracts the array of bool values from the given entry.
    ///
    /// Extracts the array of bool values from the entry given by its name.
    /// @param name Name of the entry.
    /// @param array_length Length of the internal array.
    /// @return Pointer to the internal array buffer.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    bool * get_boolean_array(const std::string & name,
        std::size_t & length) const;
    bool * get_boolean_array(const char * name,
        std::size_t & array_length) const;

    /// \brief Inserts new entry of type int array.
    ///
    /// Inserts a new entry of type int array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    void set_integer_array(const std::string & name,
        const int * values, std::size_t array_length);
    void set_integer_array(const char * name,
        const int * values, std::size_t array_length);

    /// \brief Inserts new entry of type int array.
    ///
    /// Inserts a new entry of type int array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    void set_integer_array_shallow(const std::string & name,
        const int * values, std::size_t array_length);
    void set_integer_array_shallow(const char * name,
        const int * values, std::size_t array_length);

    /// \brief Extracts the array of int values from the given entry.
    ///
    /// Extracts the array of int values from the entry given by its name.
    /// @param name Name of the entry.
    /// @param length Length of the internal array.
    /// @return Pointer to the internal array buffer.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    int * get_integer_array(const std::string & name,
        std::size_t & array_length) const;
    int * get_integer_array(const char * name,
        std::size_t & array_length) const;

    /// \brief Inserts new entry of type long long array.
    ///
    /// Inserts a new entry of type long long array
    /// to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    void set_long_long_array(const std::string & name,
        const long long * values, std::size_t array_length);
    void set_long_long_array(const char * name,
        const long long * values, std::size_t array_length);

    /// \brief Inserts new entry of type long long array.
    ///
    /// Inserts a new entry of type long long array
    /// to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    void set_long_long_array_shallow(const std::string & name,
        const long long * values, std::size_t array_length);
    void set_long_long_array_shallow(const char * name,
        const long long * values, std::size_t array_length);

    /// \brief Extracts the array of long long values from the given entry.
    ///
    /// Extracts the array of long long values
    /// from the entry given by its name.
    /// @param name Name of the entry.
    /// @param length Length of the internal array.
    /// @return Pointer to the internal array buffer.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    long long * get_long_long_array(const std::string & name,
        std::size_t & array_length) const;
    long long * get_long_long_array(const char * name,
        std::size_t & array_length) const;

    /// \brief Inserts new entry of type double array.
    ///
    /// Inserts a new entry of type double array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i>copied</i>
    /// to the internal buffer.
    void set_double_float_array(const std::string & name,
        const double * values, std::size_t array_length);
    void set_double_float_array(const char * name,
        const double * values, std::size_t array_length);

    /// \brief Inserts new entry of type double array.
    ///
    /// Inserts a new entry of type double array to the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param values Pointer to the array of values to be set.
    /// @param array_length Length of the array of values.
    ///
    /// <b>Note:</b> The array of values is <i><b>not</b></i> <i>copied</i>
    /// to the internal buffer and the user has to ensure that the given
    /// buffer is valid as long as this object refers to it.
    void set_double_float_array_shallow(const std::string & name,
        const double * values, std::size_t array_length);
    void set_double_float_array_shallow(const char * name,
        const double * values, std::size_t array_length);

    /// \brief Extracts the array of double values from the given entry.
    ///
    /// Extracts the array of double values from the entry given by its name.
    /// @param name Name of the entry.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    double * get_double_float_array(const std::string & name,
        std::size_t & array_length) const;
    double * get_double_float_array(const char * name,
        std::size_t & array_length) const;

    /// \brief Creates new empty entry of type string array.
    ///
    /// Creates a new empty entry of type string array.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param array_length Length of the newly created array.
    ///
    /// <b>Note:</b> After creation the array contains empty strings
    /// (that is, strings which length is 0).
    void create_string_array(const std::string & name,
        std::size_t array_length);
    void create_string_array(const char * name, std::size_t array_length);

    /// \brief Inserts new string value to string array.
    ///
    /// Inserts a new string value (possibly replacing the old one)
    /// to already existing string array at the given index.
    /// @param name Name of the entry containing string array.
    /// @param index The array index (array slot, starting from 0).
    /// @param value Value to be set.
    ///
    /// <b>Note:</b> The value is <i>copied</i> to the internal buffer.
    void set_string_in_array(const std::string & name, std::size_t index,
        const std::string & value);
    void set_string_in_array(const char * name, std::size_t index,
        const char * value);

    /// \brief Extracts the length of string array.
    ///
    /// Extracts the length of string array
    /// that is located at the given entry.
    ///
    /// @param name Name of the entry containing string array.
    /// @return Length of the array.
    std::size_t get_string_array_length(const std::string & name) const;
    std::size_t get_string_array_length(const char * name) const;

    /// \brief Extracts string value from string array.
    ///
    /// Extracts the string value from the given index of string array.
    /// @param name Name of the entry containing string array.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @return Value from array.
    std::string get_string_in_array(const std::string & name,
        std::size_t index) const;
    std::string get_string_in_array(const char * name,
        std::size_t index) const;

    /// \brief Extracts string value from string array.
    ///
    /// Extracts the string value from the given index of string array
    /// by accessing the internal buffer directly.
    /// @param name Name of the entry containing string array.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param length Length of the value buffer.
    /// @return Value from array.
    const char * get_string_in_array(const std::string & name,
        std::size_t index, std::size_t & length) const;
    const char * get_string_in_array(const char * name,
        std::size_t index, std::size_t & length) const;

    /// \brief Creates new empty entry of type binary array.
    ///
    /// Creates a new empty entry of type binary array.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @param array_length Length of the newly created array.
    ///
    /// <b>Note:</b> After creation the array contains empty binaries
    /// (that is, binaries which length is 0).
    void create_binary_array(const std::string & name,
        std::size_t array_length);
    void create_binary_array(const char * name, std::size_t array_length);

    /// \brief Inserts new binary value to binary array.
    ///
    /// Inserts a new binary value (possibly replacing the old one)
    /// to already existing binary array at the given index.
    /// @param name Name of the entry containing string array.
    /// @param index The array index (array slot, starting from 0).
    /// @param value Pointer to the value buffer to be set.
    /// @param value_length Length of the value buffer.
    ///
    /// <b>Note:</b> The value is <i>copied</i> to the internal buffer.
    void set_binary_in_array(const std::string & name, std::size_t index,
        const void * value, std::size_t value_length);
    void set_binary_in_array(const char * name, std::size_t index,
        const void * value, std::size_t value_length);

    /// \brief Extracts the length of binary array.
    ///
    /// Extracts the length of binary array
    /// that is located at the given entry.
    ///
    /// @param name Name of the entry containing binary array.
    /// @return Length of the array.
    std::size_t get_binary_array_length(const std::string & name) const;
    std::size_t get_binary_array_length(const char * name) const;

    /// \brief Extracts binary value from binary array.
    ///
    /// Extracts the binary value from the given index of binary array
    /// by accessing the internal buffer directly.
    /// @param name Name of the entry containing binary array.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param length Length of the value buffer.
    /// @return Value from array.
    const void * get_binary_in_array(const std::string & name,
        std::size_t index, std::size_t & length) const;
    const void * get_binary_in_array(const char * name,
        std::size_t index, std::size_t & length) const;

    // support for data nesting

    /// \brief Creates nested parameters entry.
    ///
    /// Creates a new nested parameters entry in the first available slot.
    /// If the entry with the given name already exists it is replaced
    /// without changing the order of entries.
    /// @param name Name of the new entry or the entry to be replaced.
    /// @return Pointer to the internally created parameters object;
    ///         the return value can be used to initialize wrapper
    ///         <code>parameters</code> object.
    core::parameters * create_nested_parameters(const std::string & name);

    /// \brief Extracts the nested parameters object from the given entry.
    ///
    /// Extracts nested parameters from the entry given by its name.
    /// @param name Name of the entry.
    /// @return Pointer to the internally stored parameters object;
    ///         the return value can be used to initialize wrapper
    ///         <code>parameters</code> object.
    core::parameters * get_nested_parameters(const std::string & name) const;
    core::parameters * get_nested_parameters(const char * name) const;

    /// \brief Locks the given entry and disallows future modifications.
    ///
    /// Marks the given entry as locked with the provided unlock key.
    /// This operation is possible only when the entry is in the unlocked
    /// state and prevents future modifications of that entry.
    /// The entry can be unlocked if the proper key value is provided.
    /// @param name Name of the entry to lock.
    /// @param key Key value to be used in future unlock operation.
    void lock(const std::string & name, long long key);
    void lock(const char * name, long long key);

    /// \brief Unlocks the given entry.
    ///
    /// Marks the given entry as unlocked with the provided unlock key.
    /// This operation is possible only when the entry is in the locked
    /// state. The provided key value must be equal to the value that
    /// was provided when the entry was locked.
    /// @param name Name of the entry to unlock.
    /// @param key Key value.
    void unlock(const std::string & name, long long key);
    void unlock(const char * name, long long key);

    /// \brief Checks whether a given entry is locked.
    ///
    /// Checks whether a given entry is locked and whether the modifications
    /// are disallowed.
    /// @param name Name of the entry to check.
    /// @return
    ///         - <code>true</code> if the given entry is locked and
    ///           <code>false</code> otherwise
    bool is_locked(const std::string & name) const;
    bool is_locked(const char * name) const;

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
    /// @param name Name of the entry.
    /// @return type of the given entry.
    parameter_type type(const std::string & name) const;
    parameter_type type(const char * name) const;

    /// \brief Iterator class for inspecting entries in the collection.
    ///
    /// Iterator class for inspecting entries in the collection.
    ///
    /// This iterator is of the <i>InputIterator</i> category, except for the
    /// <code>operator-></code>, which is not supported
    /// (the parameter_entry is a proxy object
    /// that is created on the fly and it is not possible to
    /// return its address).
    class iterator
    {
    public:
        iterator() : empty_(true), it_() {}
        iterator(core::parameter_iterator it) : empty_(false), it_(it) {}

        bool operator==(const iterator & rhs) const
        {
            // this is enough for the input iterator,
            // because the user cannot legally have two iterators
            // in the middle of the collection and therefore the comparison
            // can be used only to check for the end-of-sequence condition
            return empty_ && rhs.empty_;
        }

        bool operator!=(const iterator & rhs) const
        {
            return operator==(rhs) == false;
        }

        iterator & operator++()
        {
            if (it_.has_next())
            {
                it_.move_next();
            }
            else
            {
                empty_ = true;
            }

            return *this;
        }

        iterator operator++(int)
        {
            iterator old(*this);
            operator++();
            return old;
        }

        parameter_entry operator*() const
        {
            parameter_entry entry;
            entry.entry_ = it_.current();
            return entry;
        }

    private:
        friend class parameters;
        bool empty_;
        core::parameter_iterator it_;
    };

    /// \brief Extracts the starting iterator for this collection.
    ///
    /// Extracts the iterator pointing to the beginning of the collection,
    /// which means the first used slot.
    /// @return the iterator pointing to the beginning of the collection
    iterator begin() const;

    /// \brief Extracts the ending iterator for this collection.
    ///
    /// Extracts the iterator pointing past the end of the collection.
    /// @return the iterator pointing past the end of the collection
    iterator end() const;

    /// \brief Finds the given entry.
    ///
    /// Extracts the view on the entry specified by its name.
    /// @param name Name of the entry.
    /// @param entry The entry view to be returned.
    /// @return
    ///         - <code>true</code> if the entry has been found
    ///         - <code>false</code> if the given name cannot be found
    bool find(const std::string & name, parameter_entry & entry) const;
    bool find(const char * name, parameter_entry & entry) const;

    // cleanup

    /// \brief Removes the given entry.
    ///
    /// Removes the entry given by its name.
    /// @param name Name of the entry to remove.
    ///
    /// <b>Note:</b> The removed entry leaves a <i>hole</i> (empty slot) in
    /// the collection that can be reused by newly added entries.
    void remove(const std::string & name);
    void remove(const char * name);

    /// \brief Removes the entry given by its iterator.
    ///
    /// Removes the entry given by its iterator.
    /// @param it Iterator pointing to the entry to remove.
    ///
    /// <b>Note:</b> The removed entry leaves a <i>hole</i> (empty slot) in
    /// the collection that can be reused by newly added entries.
    void remove(iterator it);

    /// \brief Merges entries from the given parameters object.
    ///
    /// Merges the entries from another parameters object.
    /// The merged entries can have the same names,
    /// in which case the new entries replace existing ones.
    /// The merging is deep in the sense that no data is shared between
    /// this and other object after the merge - this applies also to
    /// those entries that were shallow references in the other object.
    /// @param The object to be merged into this object.
    void merge_from(const parameters & other);

    /// \brief Clears the collection of entries.
    ///
    /// Clears the collection of entries and deallocates dependent structures.
    /// After executing the state of this object is as it was
    /// immediately after construction.
    void clear();

    // serialization

    /// \brief Returns the total size of serialization buffer.
    ///
    /// Computes the total size of serialization buffer(s) for the current
    /// content of this object.
    virtual std::size_t serialize_buffer_size() const;

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
    virtual void serialize(char * * buffers, std::size_t * buffer_sizes,
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
    ///
    /// <b>Note:</b> The current content of this object is not cleared
    /// before attempting deserialization and each retrieved data element
    /// is <i>merged</i> into the current content as if done by individual
    /// calls to appropriate <code>set_XYZ</code> functions.<br />
    /// In most cases deserialization will be performed to the empty
    /// parameters object (to reconstruct it to the form that was used
    /// for serialization), but deserialization onto non-empty object
    /// might be occasionally useful as a way of merging two collections.
    void deserialize(const char * * buffers, std::size_t * buffer_sizes,
        std::size_t num_of_buffers);

    /// \brief Provides access to the underlying core object.
    virtual const core::parameters & get_core_object() const;

    // used from unit tests
    void dump(std::ostream & os) const;

private:

    // values that are owned and controlled by this object
    std::auto_ptr<core::parameters> own_params_;

    // delegated object (not necessarily own_params_, can be external)
    // note: it is not owned
    core::parameters * params_;
};

} // namespace yami

#endif // YAMICPP_PARAMETERS_H_INCLUDED
