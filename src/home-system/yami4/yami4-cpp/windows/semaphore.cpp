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

#include "semaphore.h"
#include <yami4-core/fatal_errors.h>

using namespace yami;
using namespace details;

details::semaphore::semaphore()
{
    const long max_value = 1000000000; // arbitrary
    sem_ = CreateSemaphore(NULL, 0, max_value, NULL);
    if (sem_ == NULL)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

details::semaphore::~semaphore()
{
    CloseHandle(sem_);
}

void details::semaphore::release()
{
    BOOL cc = ReleaseSemaphore(sem_, 1, NULL);
    if (cc == FALSE)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void details::semaphore::acquire()
{
    DWORD cc = WaitForSingleObject(sem_, INFINITE);
    if (cc != WAIT_OBJECT_0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}
