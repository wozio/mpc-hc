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

#ifndef YAMICORE_FATAL_ERRORS_H_INCLUDED
#define YAMICORE_FATAL_ERRORS_H_INCLUDED

#include "dll.h"

namespace yami
{

namespace core
{

/// Type of function callback for reporting internal fatal errors.
///
/// Note: This function is supposed to be a customization point for
/// reporting assertion errors; even if this function returns,
/// the code calling it will abort immediately after that.
/// The default handler prints the message on standard error channel.
///
/// @param source_file Name of the source file where the error occured.
/// @param line_number Line number of the place where assertion failed.
extern "C" typedef void (*fatal_error_function)(
    const char * source_file, int line_number);

/// Registers the custom handler for reporting fatal errors.
///
/// Note: this function is not thread-safe and should be called (if at all)
/// before any agent is initialized.
DLL void register_fatal_error_handler(fatal_error_function handler);

} // namespace core

namespace details
{

DLL void fatal_failure(const char * source_file, int line_number);

} // namespace details

} // namespace yami

#endif // YAMICORE_FATAL_ERRORS_H_INCLUDED
