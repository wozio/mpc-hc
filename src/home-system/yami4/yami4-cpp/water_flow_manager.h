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

#ifndef YAMICPP_WATER_FLOW_MANAGER_H_INCLUDED
#define YAMICPP_WATER_FLOW_MANAGER_H_INCLUDED

#include <mutex.h>
#include <cstddef>

namespace yami
{

namespace details
{

enum water_flow_control { suppress, allow, no_change };

class water_flow_manager
{
public:
    water_flow_manager();
    ~water_flow_manager();

    void set_limits(std::size_t high_mark, std::size_t low_mark);

    water_flow_control increase();
    water_flow_control decrease();

    void get_state(std::size_t & current_level,
        std::size_t & high_water_mark,
        std::size_t & low_water_mark) const;

private:
    std::size_t current_level_;
    std::size_t high_water_mark_;
    std::size_t low_water_mark_;
    mutable mutex mtx_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_WATER_FLOW_MANAGER_H_INCLUDED
