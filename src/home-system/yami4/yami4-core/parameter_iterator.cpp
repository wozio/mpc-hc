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

#include "parameter_iterator.h"
#include "allocator.h"
#include "parameters-details.h"
#include <cstring>

using namespace yami;
using namespace yami::core;

bool parameter_iterator::has_next() const
{
    return details::find_next_used(data_, num_of_entries_, current_index_) !=
        num_of_entries_;
}

void parameter_iterator::move_next()
{
    const std::size_t next_used =
        details::find_next_used(data_, num_of_entries_, current_index_);
    if (next_used != num_of_entries_)
    {
        current_index_ = next_used;
    }
}

parameter_entry parameter_iterator::current() const
{
    parameter_entry e;
    e.e_ = data_ + current_index_;
    return e;
}

void parameter_iterator::remove()
{
    details::entry & e = data_[current_index_];
    e.clear_name(*allocator_);
    e.clear_item(*allocator_);
    e.type = unused;
}
