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

#ifndef YAMICPP_VERSION_H_INCLUDED
#define YAMICPP_VERSION_H_INCLUDED

#include <yami4-core/dll.h>

namespace yami
{

/// Library version name.
extern DLL const char * const version_name;

/// Library version number (X * 10000 + Y * 100 + Z).
extern DLL const int version_number;

} // namespace yami

#endif // YAMICPP_VERSION_H_INCLUDED
