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
#include <errno.h>
#include <sys/time.h>

using namespace yami;
using namespace details;

details::flag::flag()
{
    int cc = pthread_mutex_init(&mtx_, NULL);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    cc = pthread_cond_init(&cond_, NULL);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    state_ = false;
}

details::flag::~flag()
{
    int cc = pthread_mutex_destroy(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    cc = pthread_cond_destroy(&cond_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void details::flag::notify()
{
    // note: this object acts as a latch,
    // which means that once notified it stays in this state until suppressed

    int cc = pthread_mutex_lock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    state_ = true;

    cc = pthread_mutex_unlock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    cc = pthread_cond_broadcast(&cond_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void details::flag::suppress()
{
    // note: this object acts as a latch,
    // which means that once suppressed it stays in this state until notified

    int cc = pthread_mutex_lock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    state_ = false;

    cc = pthread_mutex_unlock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

void details::flag::wait()
{
    int cc = pthread_mutex_lock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    while (state_ == false)
    {
        cc = pthread_cond_wait(&cond_, &mtx_);
        if (cc != 0)
        {
            fatal_failure(__FILE__, __LINE__);
        }
    }

    cc = pthread_mutex_unlock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }
}

bool details::flag::wait(std::size_t relative_timeout)
{
    struct timeval now;
    (void)gettimeofday(&now, NULL);

    const unsigned long long now_ms =
        static_cast<unsigned long long>(now.tv_sec) * 1000 +
        (now.tv_usec / 1000);

    const unsigned long long deadline = now_ms + relative_timeout;

    return wait_absolute(deadline);
}

bool details::flag::wait_absolute(unsigned long long timeout)
{
    bool result;

    struct timespec tm;

    tm.tv_sec = timeout / 1000;
    tm.tv_nsec = (timeout % 1000) * 1000000;

    int cc = pthread_mutex_lock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    while (state_ == false && cc == 0)
    {
        cc = pthread_cond_timedwait(&cond_, &mtx_, &tm);
    }

    if (cc != 0 && cc != ETIMEDOUT)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    result = cc == 0;

    cc = pthread_mutex_unlock(&mtx_);
    if (cc != 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    return result;
}
