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

#ifndef YAMICPP_OUTGOING_MESSAGE_INFO_H_INCLUDED
#define YAMICPP_OUTGOING_MESSAGE_INFO_H_INCLUDED

#include "message_state.h"
#include "parameters.h"
#include <cstddef>
#include <vector>

// selected per platform
#include <mutex.h>
#include <semaphore.h>
#include <flag.h>

namespace yami
{

class outgoing_message;

namespace details
{

class agent_impl;
class outgoing_message_dispatcher_base;

struct outgoing_message_info
{
    message_state state;
    long long message_id;
    std::size_t sent_bytes;
    std::size_t total_byte_count;
    parameters * reply_body;
    std::vector<char> * reply_raw_buffer;
    std::string exception_msg;

    mutex mtx;
    mutable flag transmitted;
    mutable flag completed;

    agent_impl * agent;
    
    // for callback (if used), owned by this info object
    outgoing_message * out_msg;
    outgoing_message_dispatcher_base * message_callback;

    // this is always called with mtx locked,
    // possibly with some outer_mtx locked as well
    void process_callback(mutex * outer_mtx = NULL);

    std::size_t ref_count;

    void dec_ref_count();
};

} // namespace details

} // namespace yami

#endif // YAMICPP_OUTGOING_MESSAGE_INFO_H_INCLUDED
