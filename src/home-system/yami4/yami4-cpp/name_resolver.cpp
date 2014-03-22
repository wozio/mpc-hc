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

#include "name_resolver.h"
#include "mutex_lock.h"

using namespace yami;
using namespace details;

name_resolver::name_resolver()
    : any_callback_(NULL)
{
    mtx_.init();
}

name_resolver::~name_resolver()
{
    map_type::iterator it = map_.begin();
    map_type::iterator end = map_.end();
    for ( ; it != end; ++it)
    {
        delete it->second;
    }

    mtx_.clean();
}

void name_resolver::register_object(const std::string & name,
    std::auto_ptr<incoming_message_dispatcher_base> object)
{
    mutex_lock lock(mtx_);

    if (name == "*")
    {
        any_callback_ = object.release();
    }
    else
    {
        const map_type::iterator it = map_.find(name);
        if (it != map_.end())
        {
            // replace existing registration

            delete it->second;

            it->second = object.release();
        }
        else
        {
            // create new registration

            map_[name] = object.release();
        }
    }
}

void name_resolver::unregister_object(const std::string & name)
{
    mutex_lock lock(mtx_);

    if (name == "*")
    {
        any_callback_ = NULL;
    }
    else
    {
        const map_type::iterator it = map_.find(name);
        if (it != map_.end())
        {
            delete it->second;

            map_.erase(it);
        }
    }
}

incoming_message_dispatcher_base * name_resolver::resolve(
    const std::string & name) const
{
    mutex_lock lock(mtx_);

    const map_type::const_iterator it = map_.find(name);
    if (it != map_.end())
    {
        return it->second;
    }
    else
    {
        return any_callback_;
    }
}
