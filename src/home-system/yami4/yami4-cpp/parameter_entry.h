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

#ifndef YAMICPP_PARAMETER_ENTRY_H_INCLUDED
#define YAMICPP_PARAMETER_ENTRY_H_INCLUDED

#include "parameter_type.h"
#include <yami4-core/dll.h>
#include <yami4-core/parameter_entry.h>
#include <yami4-core/parameters.h>
#include <string>

namespace yami
{

/// \brief Read-only view on the parameters entry.
///
/// Read-only view on the parameters entry.
/// \sa parameters
///
/// This view is a lightweight proxy that itself is copyable, but
/// the copying of this class does not create new copies of
/// the underlying entry.
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
    /// @return Name of the entry.
    std::string name() const;

    /// \brief Extracts the bool value from the current entry.
    ///
    /// Extracts the bool value from the current entry.
    /// @return Value of the entry if it has correct type.
    bool get_boolean() const;

    /// \brief Extracts the int value from the current entry.
    ///
    /// Extracts the int value from the current entry.
    /// @return Value of the entry if it has correct type.
    int get_integer() const;

    /// \brief Extracts the long long value from the current entry.
    ///
    /// Extracts the long long value from the current entry.
    /// @return Value of the entry if it has correct type.
    long long get_long_long() const;

    /// \brief Extracts the double float value from the current entry.
    ///
    /// Extracts the double float value from the current entry.
    /// @return Value of the entry if it has correct type.
    double long get_double_float() const;

    /// \brief Extracts the string value from the current entry.
    ///
    /// Extracts the string value from the current entry.
    /// @return Value of the entry if it has correct type.
    std::string get_string() const;

    /// \brief Extracts the string value from the current entry.
    ///
    /// Extracts the string value from the current entry
    /// by accessing the buffer directly.
    /// @param length Length of the internal buffer.
    /// @return Pointer to the internal buffer if it has correct type.
    const char * get_string(std::size_t & length) const;

    /// \brief Extracts the binary value from the current entry.
    ///
    /// Extracts the binary value from the current entry
    /// by accessing the buffer directly.
    /// @param length Length of the internal buffer.
    /// @return Pointer to the internal buffer if it has correct type.
    const void * get_binary(std::size_t & length) const;

    /// \brief Extracts the nested parameters value from the current entry.
    ///
    /// Extracts the nested parameters value from the current entry.
    /// @return Value of the entry if it has correct type.
    core::parameters * get_nested_parameters() const;

    /// \brief Extracts the array of bool values from the current entry.
    ///
    /// Extracts the array of bool values from the current entry.
    /// @param array_length Length of the internal array.
    /// @return Pointer to the internal array buffer.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    bool * get_boolean_array(std::size_t & array_length) const;

    /// \brief Extracts the array of int values from the current entry.
    ///
    /// Extracts the array of int values from the current entry.
    /// @param array_length Length of the internal array.
    /// @return Pointer to the internal array buffer.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    int * get_integer_array(std::size_t & array_length) const;

    /// \brief Extracts the array of long long values from the current entry.
    ///
    /// Extracts the array of long long values from the current entry.
    /// @param array_length Length of the internal array.
    /// @return Pointer to the internal array buffer.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    long long * get_long_long_array(std::size_t & array_length) const;

    /// \brief Extracts the array of double values from the current entry.
    ///
    /// Extracts the array of double values from the current entry.
    /// @param array_length Length of the internal array.
    /// @return Pointer to the internal array buffer.
    ///
    /// <b>Note:</b> this function gives read-write access to
    /// the underlying array, even if it was set as a shallow-copy.
    double * get_double_float_array(std::size_t & array_length) const;

    /// \brief Extracts the length of string array.
    ///
    /// Extracts the length of string array
    /// that is located at the current entry.
    ///
    /// @return Length of the array.
    std::size_t get_string_array_length() const;

    /// \brief Extracts string value from string array.
    ///
    /// Extracts the string value from the given index of string array
    /// that is located at the current entry.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @return Value at the given index.
    std::string get_string_in_array(std::size_t index) const;

    /// \brief Extracts string value from string array.
    ///
    /// Extracts the string value from the given index of string array
    /// that is located at the current entry.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param value_length Length of the internal value buffer.
    /// @return Pointer to the internal value buffer.
    const char * get_string_in_array(std::size_t index,
        std::size_t & value_length) const;

    /// \brief Extracts the length of binary array.
    ///
    /// Extracts the length of binary array
    /// that is located at the current entry.
    ///
    /// @return Length of the array.
    std::size_t get_binary_array_length() const;

    /// \brief Extracts binary value from binary array.
    ///
    /// Extracts the binary value from the given index of binary array
    /// that is located at the current entry.
    /// @param index The array index (array slot to be read, starting from 0).
    /// @param value_length Length of the internal value buffer.
    /// @return Pointer to the internal value buffer.
    const void * get_binary_in_array(std::size_t index,
        std::size_t & value_length) const;

    core::parameter_entry entry_;
};

} // namespace yami

#endif // YAMICPP_PARAMETER_ENTRY_H_INCLUDED
