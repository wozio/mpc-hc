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

#include "incoming_message_queue.h"
#include "mutex_lock.h"

using namespace yami;
using namespace details;

incoming_message_queue::incoming_message_queue()
    : terminated_(false)
{
    mtx_.init();
}

incoming_message_queue::~incoming_message_queue()
{
    while (queue_.empty() == false)
    {
        incoming_message_info * info = queue_.front();
        queue_.pop_front();

        if (info->body != NULL)
        {
            delete info->body;
        }

        delete info;
    }

    mtx_.clean();
}

void incoming_message_queue::push(std::auto_ptr<incoming_message_info> info)
{
    {
        mutex_lock lock(mtx_);

        if (terminated_ == false)
        {
            if (info.get() != NULL)
            {
                queue_.push_back(info.release());
            }
            else
            {
                terminated_ = true;
            }
        }
    }

    sem_.release();
}

std::auto_ptr<incoming_message_info> incoming_message_queue::pop()
{
    sem_.acquire();

    std::auto_ptr<incoming_message_info> info;
    {
        mutex_lock lock(mtx_);

        // return NULL if the queue is terminated
        if (terminated_ == false)
        {
            info.reset(queue_.front());
            queue_.pop_front();
        }
    }

    return info;
}
