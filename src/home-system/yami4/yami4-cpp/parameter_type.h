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

#ifndef YAMICPP_PARAMETER_TYPE_H_INCLUDED
#define YAMICPP_PARAMETER_TYPE_H_INCLUDED

namespace yami
{

/// Type of parameter entry.
enum parameter_type
{
    unused,             ///< Internal mark for unused slots
    boolean,            ///< bool
    integer,            ///< int
    long_long,          ///< long long
    double_float,       ///< double
    string,             ///< string
    binary,             ///< Binary block
    boolean_array,      ///< Array of bool
    integer_array,      ///< Array of int
    long_long_array,    ///< Array of long long
    double_float_array, ///< Array of double
    string_array,       ///< Array of strings
    binary_array,       ///< Array of binary blocks
    nested_parameters   ///< Nested parameters object
};

} // namespace yami

#endif // YAMICPP_PARAMETER_TYPE_H_INCLUDED
