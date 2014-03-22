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

#ifndef YAMICPP_IO_ERROR_GENERIC_DISPATCHER_H_INCLUDED
#define YAMICPP_IO_ERROR_GENERIC_DISPATCHER_H_INCLUDED

#include "connection_event_dispatcher_base.h"

namespace yami
{

namespace details
{

template <typename functor>
class io_error_generic_dispatcher
    : public io_error_dispatcher_base
{
public:
    io_error_generic_dispatcher(functor & f) : f_(f) {}

    virtual void dispatch(int error_code, const char * description)
    {
        f_(error_code, description);
    }

private:
    functor & f_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_IO_ERROR_GENERIC_DISPATCHER_H_INCLUDED
