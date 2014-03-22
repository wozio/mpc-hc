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

#include "details.h"
#include "errors.h"

using namespace yami;
using namespace details;

void details::translate_result_to_exception(core::result res)
{
    switch (res)
    {
    case core::ok:
        // operation completed successfully
        break;
    case core::no_such_name:
        throw yami_logic_error("No such name.");
    case core::bad_type:
        throw yami_logic_error("Bad type.");
    case core::no_such_index:
        throw yami_logic_error("No such index.");
    case core::no_memory:
        throw yami_runtime_error("Not enough memory.");
    case core::nesting_too_deep:
        throw yami_logic_error("Nesting of parameters is too deep.");
    case core::not_enough_space:
        throw yami_runtime_error(
            "Not enough space or not enough data in the buffer.");
    case core::no_entries:
        throw yami_logic_error("No entries found.");
    case core::unexpected_value:
        throw yami_runtime_error(
            "The value that was given or received is incorrect.");
    case core::bad_protocol:
        throw yami_logic_error("The given protocol is not supported.");
    case core::io_error:
        throw yami_runtime_error("I/O error.");
    case core::timed_out:
        throw yami_runtime_error("Operation timed out.");
    case core::channel_closed:
        throw yami_runtime_error("The channel was closed.");
    case core::bad_state:
        throw yami_logic_error("The given object is in the wrong state.");
    }
}
