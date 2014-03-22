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

#ifndef YAMICPP_AGENT_IMPL_BASE_H_INCLUDED
#define YAMICPP_AGENT_IMPL_BASE_H_INCLUDED

#include "connection_event_dispatcher_base.h"
#include "incoming_message_dispatcher_base.h"
#include "io_error_dispatcher_base.h"
#include "outgoing_message_dispatcher_base.h"
#include <yami4-core/dll.h>

#include <memory>
#include <string>

namespace yami
{

class serializable;

namespace details
{

class DLL agent_impl_base
{
public:
    virtual ~agent_impl_base() {}

    virtual void register_object(
        const std::string & object_name,
        std::auto_ptr<incoming_message_dispatcher_base> object) = 0;

    virtual long long send(
        std::auto_ptr<outgoing_message_dispatcher_base>
            outgoing_message_callback,
        const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content,
        std::size_t priority,
        bool auto_connect) = 0;

    virtual void register_connection_event_monitor(
        std::auto_ptr<connection_event_dispatcher_base> monitor) = 0;

    virtual void register_io_error_logger(
        std::auto_ptr<io_error_dispatcher_base> logger) = 0;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_AGENT_IMPL_BASE_H_INCLUDED
