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

#ifndef YAMICPP_CONNECTION_EVENT_GENERIC_DISPATCHER_H_INCLUDED
#define YAMICPP_CONNECTION_EVENT_GENERIC_DISPATCHER_H_INCLUDED

#include "connection_event_dispatcher_base.h"
#include <yami4-core/dll.h>

namespace yami
{

namespace details
{

template <typename functor>
class DLL connection_event_generic_dispatcher
    : public connection_event_dispatcher_base
{
public:
    connection_event_generic_dispatcher(functor & f) : f_(f) {}

    virtual void dispatch(const std::string & name, connection_event event)
    {
        f_(name, event);
    }

private:
    functor & f_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_CONNECTION_EVENT_GENERIC_DISPATCHER_H_INCLUDED
