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

#include "outgoing_message.h"
#include "outgoing_message_info.h"
#include "outgoing_message_dispatcher_base.h"
#include "mutex_lock.h"

using namespace yami;
using namespace details;

void outgoing_message_info::process_callback(mutex * outer_mtx)
{
    if (message_callback != NULL)
    {
        // this function is called with mutex locked
        // -> unlock for the time of user callback
    
        if (outer_mtx != NULL)
        {
            outer_mtx->unlock();
        }
        mtx.unlock();
        
        try
        {
            message_callback->dispatch(*out_msg);
        }
        catch (...)
        {
            // ignore user exceptions
        }
        
        mtx.lock();
        if (outer_mtx != NULL)
        {
            outer_mtx->lock();
        }
    }
}

void outgoing_message_info::dec_ref_count()
{
    std::size_t new_ref_count;
    {
        mutex_lock lock(mtx);
        new_ref_count = --ref_count;
    }

    if (new_ref_count == 0)
    {
        mtx.clean();
        delete reply_body;
        delete reply_raw_buffer;
        delete out_msg;
        delete message_callback;
        delete this;
    }
}
