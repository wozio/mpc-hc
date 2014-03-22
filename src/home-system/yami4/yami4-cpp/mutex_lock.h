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

#ifndef YAMICPP_MUTEX_LOCK_H_INCLUDED
#define YAMICPP_MUTEX_LOCK_H_INCLUDED

#include <mutex.h> // from core/platform

namespace yami
{

namespace details
{

class mutex_lock
{
public:
    mutex_lock(mutex & mtx) : mtx_(mtx)
    {
        mtx_.lock();
    }

    ~mutex_lock()
    {
        mtx_.unlock();
    }

private:
    mutex & mtx_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_MUTEX_LOCK_H_INCLUDED
