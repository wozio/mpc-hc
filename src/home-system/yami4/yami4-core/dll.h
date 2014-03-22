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

#ifndef YAMICORE_DLL_H_INCLUDED
#define YAMICORE_DLL_H_INCLUDED

#ifdef _WIN32
#  ifdef YAMI4_DLL_COMPILATION
#    define DLL __declspec(dllexport)
#  endif // YAMI4_DLL_COMPILATION
#  ifdef YAMI4_USE_AS_DLL
#    define DLL __declspec(dllimport)
#  endif // YAMI4_USE_AS_DLL
#endif // _WIN32

#ifndef DLL
#  define DLL
#endif // DLL

#endif // YAMICORE_DLL_H_INCLUDED
