// Copyright Maciej Sobczak 2008-2011.
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

#ifndef YAMICORE_PARAMETERS_DETAILS_DUMP_SINK_H_INCLUDED
#define YAMICORE_PARAMETERS_DETAILS_DUMP_SINK_H_INCLUDED

#include <cstddef>

namespace yami
{

namespace details
{

// for unit tests, to avoid dependency on IOStreams from core
class dump_sink
{
public:
    virtual ~dump_sink() {}

    virtual void indent(std::size_t spaces) = 0;
    virtual void dump(std::size_t v) = 0;
    virtual void dump(bool v) = 0;
    virtual void dump(int v) = 0;
    virtual void dump(long long v) = 0;
    virtual void dump(double v) = 0;
    virtual void dump(const char * str) = 0;
    virtual void dump(const char * str, std::size_t str_len) = 0;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_PARAMETERS_DETAILS_DUMP_SINK_H_INCLUDED
