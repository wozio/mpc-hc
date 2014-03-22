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

#include "id_generator.h"
#include "mutex_lock.h"

using namespace yami;
using namespace details;

id_generator::id_generator()
{
    mtx_.init();
    next_value_ = 1;
}

id_generator::~id_generator()
{
    mtx_.clean();
}

long long id_generator::get_next_id()
{
    mutex_lock lock(mtx_);

    return next_value_++;
}
