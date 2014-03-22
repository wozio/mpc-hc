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

#ifndef YAMICPP_VALUE_PUBLISHER_OVERFLOW_ACTION_H_INCLUDED
#define YAMICPP_VALUE_PUBLISHER_OVERFLOW_ACTION_H_INCLUDED

namespace yami
{

/// User-defined reaction to the overflow condition in value publisher.
enum value_publisher_overflow_action
{
    /// block and wait until previous message is transmitted
    wait_for_previous_message,

    /// abandon the current message and continue with other subscribers
    abandon_message,

    /// abandon the overflowing subscription altogether
    /// and continue with other subscribers
    abandon_subscription
};

} // namespace yami

#endif // YAMICPP_VALUE_PUBLISHER_OVERFLOW_ACTION_H_INCLUDED
