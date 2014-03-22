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

#include "delay.h"
#include <time.h>

using namespace yami;
using namespace details;

void details::delay(unsigned long long pause)
{
    struct timespec tm;
    tm.tv_sec = pause / 1000;
    tm.tv_nsec = (pause % 1000) * 1000000;

    (void)nanosleep(&tm, NULL);
}
