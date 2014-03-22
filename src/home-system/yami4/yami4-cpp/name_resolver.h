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

#ifndef YAMICPP_NAME_RESOLVER_H_INCLUDED
#define YAMICPP_NAME_RESOLVER_H_INCLUDED

#include "incoming_message_dispatcher_base.h"
#include <map>
#include <memory>
#include <string>

// selected per platform
#include <mutex.h>

namespace yami
{

namespace details
{

class name_resolver
{
public:
    name_resolver();
    ~name_resolver();

    void register_object(const std::string & name,
        std::auto_ptr<incoming_message_dispatcher_base> object);

    void unregister_object(const std::string & name);

    incoming_message_dispatcher_base * resolve(
        const std::string & name) const;

private:
    name_resolver(const name_resolver &);
    void operator=(const name_resolver &);

    typedef std::map<std::string, incoming_message_dispatcher_base *>
        map_type;

    map_type map_;
    incoming_message_dispatcher_base * any_callback_;
    mutable mutex mtx_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_NAME_RESOLVER_H_INCLUDED
