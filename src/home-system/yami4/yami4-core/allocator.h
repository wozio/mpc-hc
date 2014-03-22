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

#ifndef YAMICORE_ALLOCATOR_H_INCLUDED
#define YAMICORE_ALLOCATOR_H_INCLUDED

#include "dll.h"

#include <cstddef>

namespace yami
{

namespace details
{

class DLL allocator
{
public:
    allocator();

    void set_working_area(void * buf, std::size_t size);

    void * allocate(std::size_t requested_size);

    void deallocate(const void * p);

    void get_free_size(std::size_t & biggest, std::size_t & all) const;

private:
    allocator(const allocator &);
    void operator=(const allocator &);

    void * base_;
    std::size_t size_;

    void * first_free_segment_;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_ALLOCATOR_H_INCLUDED
