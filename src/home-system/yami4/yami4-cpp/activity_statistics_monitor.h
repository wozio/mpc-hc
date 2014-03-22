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

#ifndef YAMICPP_ACTIVITY_STATISTICS_MONITOR_H_INCLUDED
#define YAMICPP_ACTIVITY_STATISTICS_MONITOR_H_INCLUDED

#include "event_callback.h"
#include <yami4-core/dll.h>

namespace yami
{

class incoming_message;
class parameters;

namespace details
{
class activity_statistics_monitor_impl;
} // namespace details

/// \brief Simple activity statistics monitor.
///
/// This class defines the basic stats monitor that is based on
/// the event notification mechanism. The monitor can also be used for
/// remote inspection of the collected data, as it can directly
/// play the role of regular object that implements the "get" command.
class DLL activity_statistics_monitor : public event_callback
{
public:
    
    activity_statistics_monitor();
    virtual ~activity_statistics_monitor();

    /// \brief Retrieves all collected statistics.
    ///
    /// Retrieves the collected statistics by filling
    /// the given parameters object.
    /// Optional atomic reset of the counters is possible.
    ///
    /// @param params The parameters object to be filled with data.
    /// @reset_counters Request atomic reset of the counters.
    void get(parameters & params, bool reset_counters = false);

    /// \brief Standard reply to the incoming message.
    ///
    /// Implements the standard reply to the remote "get" command
    /// and replies with a parameters object filled with collected statistics.
    ///
    /// @params Incoming message object.
    void operator()(incoming_message & msg);

private:

    virtual void agent_created();
    virtual void agent_closed();
    virtual void listener_added(const char * target);
    virtual void listener_removed(const char * target);
    virtual void incoming_connection_open(const char * target);
    virtual void outgoing_connection_open(const char * target);
    virtual void connection_closed(const char * target);
    virtual void connection_error(const char * target);
    virtual void object_registered(const char * name);
    virtual void object_unregistered(const char * name);
    virtual void message_sent(const char * target, std::size_t size);
    virtual void message_received(const char * target, std::size_t size);
    
    details::activity_statistics_monitor_impl * pimpl_;
};

} // namespace yami

#endif // YAMICPP_ACTIVITY_STATISTICS_MONITOR_H_INCLUDED
