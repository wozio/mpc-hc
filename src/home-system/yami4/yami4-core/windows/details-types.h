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

#ifndef YAMICORE_DETAILS_TYPES_H_INCLUDED
#define YAMICORE_DETAILS_TYPES_H_INCLUDED

#include <Winsock2.h>

namespace yami
{

namespace details
{

typedef SOCKET io_descriptor_type;
const io_descriptor_type empty_io_descriptor = INVALID_SOCKET;

} // namespace details

} // namespace yami

#endif // YAMICORE_DETAILS_TYPES_H_INCLUDED
