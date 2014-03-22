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

#include "water_flow_manager.h"
#include "mutex_lock.h"
#include <yami4-core/fatal_errors.h>

using namespace yami;
using namespace details;

water_flow_manager::water_flow_manager()
{
    mtx_.init();
    current_level_ = 0;
}

water_flow_manager::~water_flow_manager()
{
    mtx_.clean();
}

void water_flow_manager::set_limits(
    std::size_t high_mark, std::size_t low_mark)
{
    if (high_mark < low_mark)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    high_water_mark_ = high_mark;
    low_water_mark_ = low_mark;
}

water_flow_control water_flow_manager::increase()
{
    water_flow_control result = no_change;

    mutex_lock lock(mtx_);

    ++current_level_;
    if (current_level_ >= high_water_mark_)
    {
        result = suppress;
    }

    return result;
}

water_flow_control water_flow_manager::decrease()
{
    water_flow_control result = no_change;

    mutex_lock lock(mtx_);

    if (current_level_ <= 0)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    --current_level_;
    if (current_level_ < low_water_mark_)
    {
        result = allow;
    }

    return result;
}

void water_flow_manager::get_state(std::size_t & current_level,
    std::size_t & high_water_mark, std::size_t & low_water_mark) const
{
    mutex_lock lock(mtx_);

    current_level = current_level_;
    high_water_mark = high_water_mark_;
    low_water_mark = low_water_mark_;
}
