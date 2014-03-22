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

#ifndef YAMICPP_INCOMING_MESSAGE_INFO_H_INCLUDED
#define YAMICPP_INCOMING_MESSAGE_INFO_H_INCLUDED

#include "parameters.h"
#include <string>
#include <vector>

namespace yami
{

namespace details
{

struct incoming_message_info
{
    incoming_message_info()
        : body(NULL), raw_buffer(NULL) {}

    std::string source;
    std::string object_name;
    std::string message_name;
    long long message_id;

    // either body or raw_buffer has the content
    parameters * body;
    std::vector<char> * raw_buffer;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_INCOMING_MESSAGE_INFO_H_INCLUDED
