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

#include "mutex.h"
#include "../fatal_errors.h"

using namespace yami;
using namespace details;

void mutex::init()
{
    const int cc = pthread_mutex_init(&mtx_, NULL);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void mutex::clean()
{
    const int cc = pthread_mutex_destroy(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void mutex::lock()
{
    const int cc = pthread_mutex_lock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void mutex::unlock()
{
    const int cc = pthread_mutex_unlock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}
