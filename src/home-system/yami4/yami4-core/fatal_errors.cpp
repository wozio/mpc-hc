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

#include "fatal_errors.h"
#include <cstdio>
#include <cstdlib>

using namespace yami;
using namespace yami::core;
using namespace yami::details;

namespace // unnamed
{

fatal_error_function custom_handler = NULL;

} // unnamed namespace

void yami::core::register_fatal_error_handler(fatal_error_function handler)
{
    custom_handler = handler;
}

void yami::details::fatal_failure(const char * source_file, int line_number)
{
    if (custom_handler != NULL)
    {
        try
        {
            custom_handler(source_file, line_number);
        }
        catch (...)
        {
            // ignore
        }
    }
    else
    {
        std::printf("YAMI4 fatal error, see %s:%d\n",
            source_file, line_number);
    }

    std::abort();
}
