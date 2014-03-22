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

#ifndef YAMICPP_OUTGOING_MESSAGE_MANAGER_H_INCLUDED
#define YAMICPP_OUTGOING_MESSAGE_MANAGER_H_INCLUDED

#include "parameters.h"
#include <map>
#include <memory>
#include <vector>

// selected per platform
#include <mutex.h>

namespace yami
{

namespace details
{

struct outgoing_message_info;

class outgoing_message_manager
{
public:
    outgoing_message_manager();
    ~outgoing_message_manager();

    void put(long long message_id, outgoing_message_info * outgoing);

    void remove(long long message_id);

    void report_replied(long long message_id,
        std::auto_ptr<parameters> body);
    void report_replied(long long message_id,
        std::auto_ptr<std::vector<char> > raw_buffer);

    void report_rejected(long long message_id, const std::string & reason);

private:
    outgoing_message_manager(const outgoing_message_manager &);
    void operator=(const outgoing_message_manager &);

    typedef std::map<long long, outgoing_message_info *> map_type;

    void do_remove(map_type::iterator it);

    map_type map_;
    mutex mtx_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_OUTGOING_MESSAGE_MANAGER_H_INCLUDED
