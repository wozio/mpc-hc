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

#include "outgoing_message_manager.h"
#include "mutex_lock.h"
#include "outgoing_message_info.h"
#include <yami4-core/fatal_errors.h>

using namespace yami;
using namespace details;

outgoing_message_manager::outgoing_message_manager()
{
    mtx_.init();
}

outgoing_message_manager::~outgoing_message_manager()
{
    map_type::iterator it = map_.begin();
    map_type::iterator end = map_.end();
    for ( ; it != end; ++it)
    {
        outgoing_message_info * outgoing = it->second;
        it->second = NULL;

        outgoing->dec_ref_count();
    }

    mtx_.clean();
}

void outgoing_message_manager::put(long long message_id,
    outgoing_message_info * outgoing)
{
    mutex_lock lock(mtx_);

    map_type::const_iterator it = map_.find(message_id);

    // it is not possible to create the same outgoing message twice
    if (it != map_.end())
    {
        fatal_failure(__FILE__, __LINE__);
    }

    map_[message_id] = outgoing;
    ++(outgoing->ref_count);
}

void outgoing_message_manager::remove(long long message_id)
{
    mutex_lock lock(mtx_);

    map_type::iterator it = map_.find(message_id);

    // it is acceptable to have a spurious remove - this can be a result
    // of combination of exception handling and regular cleanup
    if (it != map_.end())
    {
        do_remove(it);
    }
}

void outgoing_message_manager::do_remove(map_type::iterator it)
{
    outgoing_message_info * outgoing = it->second;
    it->second = NULL;

    outgoing->dec_ref_count();

    map_.erase(it);
}

void outgoing_message_manager::report_replied(long long message_id,
    std::auto_ptr<parameters> body)
{
    mutex_lock lock(mtx_);

    map_type::iterator it = map_.find(message_id);

    // it is not a bug if the user tries to operate on non-existing message
    // (it might be a network junk that should be ignored)

    if (it != map_.end())
    {
        outgoing_message_info & outgoing = *(it->second);
        bool should_remove = false;
        {
            mutex_lock lock_outgoing(outgoing.mtx);

            // it is possible to get several replies to the same message
            // (this might be a junk network content),
            // but only the first reply is taken into account

            if (outgoing.state == posted || outgoing.state == transmitted)
            {
                message_state previous_state = outgoing.state;

                outgoing.state = replied;

                if (outgoing.reply_body != NULL)
                {
                    fatal_failure(__FILE__, __LINE__);
                }

                outgoing.reply_body = body.release();

                if (previous_state == posted)
                {
                    outgoing.transmitted.notify();
                }
                outgoing.completed.notify();
                
                outgoing.process_callback(&mtx_);
                
                // there will be no more interaction with this message
                should_remove = true;
            }
        }
        
        if (should_remove)
        {
            do_remove(it);
        }
    }
}

void outgoing_message_manager::report_replied(long long message_id,
    std::auto_ptr<std::vector<char> > raw_buffer)
{
    mutex_lock lock(mtx_);

    map_type::iterator it = map_.find(message_id);

    // it is not a bug if the user tries to operate on non-existing message
    // (it might be a network junk that should be ignored)

    if (it != map_.end())
    {
        outgoing_message_info & outgoing = *(it->second);
        bool should_remove = false;
        {
            mutex_lock lock_outgoing(outgoing.mtx);

            // it is possible to get several replies to the same message
            // (this might be a junk network content),
            // but only the first reply is taken into account

            if (outgoing.state == posted || outgoing.state == transmitted)
            {
                message_state previous_state = outgoing.state;

                outgoing.state = replied;

                if (outgoing.reply_body != NULL)
                {
                    fatal_failure(__FILE__, __LINE__);
                }

                outgoing.reply_raw_buffer = raw_buffer.release();

                if (previous_state == posted)
                {
                    outgoing.transmitted.notify();
                }
                outgoing.completed.notify();

                outgoing.process_callback(&mtx_);
                
                // there will be no more interaction with this message
                should_remove = true;
            }
        }
        
        if (should_remove)
        {
            do_remove(it);
        }
    }
}

void outgoing_message_manager::report_rejected(
    long long message_id, const std::string & reason)
{
    mutex_lock lock(mtx_);

    map_type::iterator it = map_.find(message_id);

    // it is not a bug if the user tries to operate on non-existing message
    // (it might be a network junk that should be ignored)

    if (it != map_.end())
    {
        outgoing_message_info & outgoing = *(it->second);
        bool should_remove = false;
        {
            mutex_lock lock_outgoing(outgoing.mtx);

            // it is possible to get several rejections to the same message
            // (this might be a junk network content),
            // but only the first one is taken into account

            if (outgoing.state == posted || outgoing.state == transmitted)
            {
                message_state previous_state = outgoing.state;

                outgoing.state = rejected;
                outgoing.exception_msg = reason;

                if (previous_state == posted)
                {
                    outgoing.transmitted.notify();
                }
                outgoing.completed.notify();
 
                outgoing.process_callback(&mtx_);
                
                // there will be no more interaction with this message
                should_remove = true;
           }
        }
        
        if (should_remove)
        {
            do_remove(it);
        }
    }
}
