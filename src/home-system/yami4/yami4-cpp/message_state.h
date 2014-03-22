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

#ifndef YAMICPP_MESSAGE_STATE_H_INCLUDED
#define YAMICPP_MESSAGE_STATE_H_INCLUDED

namespace yami
{

/// Outgoing message state.
enum message_state
{
    posted,      ///< Message was posted for transmission.
    transmitted, ///< Message was fully transmitted.
    abandoned,   ///< Message was abandoned due to error or channel closing.
    replied,     ///< The reply was received for the given message.
    rejected     ///< Message was rejected.
};

} // namespace yami

#endif // YAMICPP_MESSAGE_STATE_H_INCLUDED
