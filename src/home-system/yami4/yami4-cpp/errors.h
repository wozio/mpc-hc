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

#ifndef YAMICPP_ERRORS_H_INCLUDED
#define YAMICPP_ERRORS_H_INCLUDED

#include <yami4-core/dll.h>
#include <string>
#include <stdexcept>

namespace yami
{

/// \brief General exception class for reporting logic errors.
///
/// General exception class for reporting logic errors.
///
/// Logic errors represent misuses of the API
/// like invalid arguments, out of range indexes or type mismatches.
class DLL yami_logic_error : public std::logic_error
{
public:
    explicit yami_logic_error(const std::string & message)
    : std::logic_error(message)
    {
    }
};

/// \brief General exception class for reporting run-time errors.
///
/// General exception class for reporting run-time errors.
///
/// Runtime errors represent problems that might not result
/// from incorrect library usage, but can be related to resource constraints
/// or communication problems.
class DLL yami_runtime_error : public std::runtime_error
{
public:
    explicit yami_runtime_error(const std::string & message)
    : std::runtime_error(message)
    {
    }
};

} // namespace yami

#endif // YAMICPP_ERRORS_H_INCLUDED
