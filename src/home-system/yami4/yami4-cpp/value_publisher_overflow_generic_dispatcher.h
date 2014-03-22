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

#ifndef YAMICPP_VALUE_PUBLISHER_OVERFLOW_GENERIC_DISPATCHER_H_INCLUDED
#define YAMICPP_VALUE_PUBLISHER_OVERFLOW_GENERIC_DISPATCHER_H_INCLUDED

#include "value_publisher_overflow_dispatcher_base.h"
#include <yami4-core/dll.h>

namespace yami
{

namespace details
{

template <typename functor>
class DLL value_publisher_overflow_generic_dispatcher
    : public value_publisher_overflow_dispatcher_base
{
public:
    value_publisher_overflow_generic_dispatcher(functor & f) : f_(f) {}

    virtual value_publisher_overflow_action dispatch(
        const std::string & server_name,
        const std::string & object_name,
        const serializable & value)
    {
        return f_(server_name, object_name, value);
    }

private:
    functor & f_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_VALUE_PUBLISHER_OVERFLOW_GENERIC_DISPATCHER_H_INCLUDED
