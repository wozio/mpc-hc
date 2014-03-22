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

#ifndef YAMICPP_FLAG_H_INCLUDED
#define YAMICPP_FLAG_H_INCLUDED

#include <cstddef>
#include <pthread.h>

namespace yami
{

namespace details
{

class flag
{
public:
    flag();
    ~flag();

    void notify();
    void suppress();
    void wait();
    bool wait(std::size_t relative_timeout);
    bool wait_absolute(unsigned long long timeout);

private:
    pthread_mutex_t mtx_;
    pthread_cond_t cond_;
    bool state_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_FLAG_H_INCLUDED
