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

#ifndef YAMICPP_INCOMING_MESSAGE_QUEUE_H_INCLUDED
#define YAMICPP_INCOMING_MESSAGE_QUEUE_H_INCLUDED

#include "incoming_message_info.h"
#include <deque>
#include <memory>

// selected per platform
#include <mutex.h>
#include <semaphore.h>

namespace yami
{

namespace details
{

class incoming_message_queue
{
public:
    incoming_message_queue();
    ~incoming_message_queue();

    void push(std::auto_ptr<incoming_message_info> incoming);

    std::auto_ptr<incoming_message_info> pop();

private:
    incoming_message_queue(const incoming_message_queue &);
    void operator=(const incoming_message_queue &);

    typedef std::deque<incoming_message_info *> queue_type;

    queue_type queue_;
    bool terminated_;
    semaphore sem_;
    mutex mtx_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_INCOMING_MESSAGE_QUEUE_H_INCLUDED
