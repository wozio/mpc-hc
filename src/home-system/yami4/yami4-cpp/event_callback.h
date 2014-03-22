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

#ifndef YAMICPP_EVENT_CALLBACK_H_INCLUDED
#define YAMICPP_EVENT_CALLBACK_H_INCLUDED

#include <yami4-core/dll.h>
#include <cstddef>

namespace yami
{

/// \brief Event notification callback interface.
///
/// This class defined the base interface for
/// the internal event notification callback.
/// Users can implement this interface and install its instance
/// in the given agent to receive notifications about internal events.
///
/// <b>Note:</b>
/// The event callback is supposed to be used only to gather statistics
/// and for similar state monitoring - in particular the implementation
/// of this interface should not attempt to use any agent functionality.
class DLL event_callback
{
public:
    virtual ~event_callback() {}

    virtual void agent_created() {}
    virtual void agent_closed() {}
    virtual void listener_added(const char * /* target */) {}
    virtual void listener_removed(const char * /* target */) {}
    virtual void incoming_connection_open(const char * /* target */) {}
    virtual void outgoing_connection_open(const char * /* target */) {}
    virtual void connection_closed(const char * /* target */) {}
    virtual void connection_error(const char * /* target */) {}
    virtual void object_registered(const char * /* name */) {}
    virtual void object_unregistered(const char * /* name */) {}
    virtual void message_sent(
        const char * /* target */, std::size_t /* size */) {}
    virtual void message_received(
        const char * /* target */, std::size_t /* size */) {}
};

} // namespace yami

#endif // YAMICPP_EVENT_CALLBACK_H_INCLUDED
