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

#include "flag.h"
#include <yami4-core/fatal_errors.h>
#include <sys/timeb.h>

using namespace yami;
using namespace details;

details::flag::flag()
{
    event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (event_ == NULL)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

details::flag::~flag()
{
    CloseHandle(event_);
}

void details::flag::notify()
{
    // note: this object acts as a latch,
    // which means that once notified it stays in this state until suppressed

    BOOL cc = SetEvent(event_);
    if (cc == FALSE)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void details::flag::suppress()
{
    // note: this object acts as a latch,
    // which means that once suppressed it stays in this state until notified

    BOOL cc = ResetEvent(event_);
    if (cc == FALSE)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void details::flag::wait()
{
    DWORD cc = WaitForSingleObject(event_, INFINITE);
    if (cc != WAIT_OBJECT_0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

bool details::flag::wait(std::size_t relative_timeout)
{
    DWORD cc = WaitForSingleObject(event_, relative_timeout);
    if (cc != WAIT_OBJECT_0 && cc != WAIT_TIMEOUT)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    return cc == WAIT_OBJECT_0;
}

bool details::flag::wait_absolute(unsigned long long timeout)
{
    _timeb now;

    _ftime(&now);

    const long long now_millisecs =
        static_cast<long long>(now.time) * 1000 + now.millitm / 1000;

    std::size_t relative_timeout;
    if (timeout > now_millisecs)
    {
        // assume that the difference fits in size_t
        relative_timeout = timeout - now_millisecs;
    }
    else
    {
        relative_timeout = 0;
    }

    return wait(relative_timeout);
}
