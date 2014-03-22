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

#ifndef YAMICORE_PARAMETER_ENTRY_H_INCLUDED
#define YAMICORE_PARAMETER_ENTRY_H_INCLUDED

#include "core.h"
#include "dll.h"
#include "parameter_type.h"
#include <cstddef>

namespace yami
{

namespace details
{
struct entry;
} // namespace details

namespace core
{

class parameters;

/// \brief Read-only view on the parameters entry.
///
/// Read-only view on the parameters entry.
/// \sa parameters
class DLL parameter_entry
{
public:
    /// \brief Returns the type of underlying (current) entry.
    ///
    /// Returns the type of the underlying entry in the associated
    /// parameters object.
    /// @return Type of the entry.
    parameter_type type() const;

    /// \brief Extracts the name of current entry.
    ///
    /// Extracts the name of the underlying entry in the associated
    /// parameters object.
    /// @param value Pointer to the internal name buffer to be returned.
    /// @param value_length Length of the internal name buffer.
    void get_name(const char * & name, std::size_t & name_length) const;

    /// \brief Extracts the bool value from the current entry.
    ///
    /// Extracts the bool value from the current entry.
    /// @param value The value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain the bool value
    result get_boolean(bool & value) const;

    /// \brief Extracts the int value from the current entry.
    ///
    /// Extracts the int value from the current entry.
    /// @param value The value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain the int value
    result get_integer(int & value) const;

    /// \brief Extracts the long long value from the current entry.
    ///
    /// Extracts the long long value from the current entry.
    /// @param value The value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain the long long value
    result get_long_long(long long & value) const;

    /// \brief Extracts the double value from the current entry.
    ///
    /// Extracts the double value from the current entry.
    /// @param value The value to be returned.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain the double value
    result get_double_float(double & value) const;

    /// \brief Extracts the string value from the current entry.
    ///
    /// Extracts the string value from the current entry.
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain the string value
    result get_string(const char * & value, std::size_t & value_length) const;

    /// \brief Extracts the binary value from the current entry.
    ///
    /// Extracts the binary value from the current entry.
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain the binary value
    result get_binary(const void * & value, std::size_t & value_length) const;

    /// \brief Extracts the nested parameters object from the current entry.
    ///
    /// Extracts nested parameters from the current entry.
    /// @param params Pointer to the internally created parameters object.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain nested parameters
    result get_nested_parameters(parameters * & params) const;

    /// \brief Extracts the array of bool values from the current entry.
    ///
    /// Extracts the array of bool values from the current entry.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain bool array
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_boolean_array(
        bool * & values, std::size_t & array_length) const;

    /// \brief Extracts the array of int values from the current entry.
    ///
    /// Extracts the array of int values from the current entry.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain int array
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_integer_array(
        int * & values, std::size_t & array_length) const;

    /// \brief Extracts the array of long long values from the current entry.
    ///
    /// Extracts the array of long long values from the current entry.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain long long array
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_long_long_array(
        long long * & values, std::size_t & array_length) const;

    /// \brief Extracts the array of double values from the current entry.
    ///
    /// Extracts the array of double values from the current entry.
    /// @param values Pointer to the internal array buffer to be returned.
    /// @param array_length Length of the internal array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain double array
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    result get_double_float_array(
        double * & values, std::size_t & array_length) const;

    /// \brief Extracts the length of string array.
    ///
    /// Extracts the length of string array
    /// that is located at the current entry.
    ///
    /// @param length Length of the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain string array
    result get_string_array_length(std::size_t & length) const;

    /// \brief Extracts string value from string array.
    ///
    /// Extracts the string value from the given index of string array
    /// that is located at the current entry.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain string array
    ///         - <code>no_such_index</code>
    ///           if the given index is out of range
    result get_string_in_array(std::size_t index,
        const char * & value, std::size_t & value_length) const;

    /// \brief Extracts the length of binary array.
    ///
    /// Extracts the length of binary array
    /// that is located at the current entry.
    ///
    /// @param length Length of the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain binary array
    result get_binary_array_length(std::size_t & length) const;

    /// \brief Extracts binary value from binary array.
    ///
    /// Extracts the binary value from the given index of binary array
    /// that is located at the current entry.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param value Pointer to the internal value buffer to be returned.
    /// @param value_length Length of the internal value buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>bad_type</code> if the current entry
    ///           does not contain binary array
    ///         - <code>no_such_index</code>
    ///           if the given index is out of range
    result get_binary_in_array(std::size_t index,
        const void * & value, std::size_t & value_length) const;

private:
    friend class parameters;
    friend class parameter_iterator;

    details::entry * e_;
};

} // namespace core

} // namespace yami

#endif // YAMICORE_PARAMETER_ENTRY_H_INCLUDED
