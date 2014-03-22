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

#ifndef YAMICORE_CHANNEL_HOLDER_H_INCLUDED
#define YAMICORE_CHANNEL_HOLDER_H_INCLUDED

#include <cstddef>

namespace yami
{

namespace details
{

class channel;
class channel_holder
{
public:

    void init();
    void set(channel * ch, std::size_t & new_sequence_number);
    channel * get_channel() const { return ch_; }
    std::size_t get_sequence_number() const { return sequence_number_; }

    void reserve() { reserved_ = true; }
    bool is_reserved() const { return reserved_; }
    void clean();

private:

    std::size_t sequence_number_;
    channel * ch_;
    bool reserved_;
};

} // namespace details

} // namespace yami

#endif // YAMICORE_CHANNEL_HOLDER_H_INCLUDED
