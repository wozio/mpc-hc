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
#include "details.h"
#include "mutex_lock.h"
#include "outgoing_message_info.h"
#include "outgoing_message_manager.h"

using namespace yami;

outgoing_message::outgoing_message()
    : manager_(NULL), info_(NULL)
{
}

outgoing_message::~outgoing_message()
{
    clean();
}

void outgoing_message::clean()
{
    if (owner_of_info_ && manager_ != NULL && info_ != NULL)
    {
        manager_->remove(info_->message_id);
        info_->dec_ref_count();
    }
}

void outgoing_message::reset(
    details::outgoing_message_manager & manager,
    details::outgoing_message_info & info)
{
    clean();

    manager_ = &manager;
    info_ = &info;
    owner_of_info_ = true;

    ++(info.ref_count);
}

void outgoing_message::disown_info_object()
{
    info_->dec_ref_count();
    owner_of_info_ = false;
}

message_state outgoing_message::get_state() const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    details::mutex_lock lock(info_->mtx);

    return info_->state;
}

message_state outgoing_message::get_state(
    std::size_t & sent_bytes,
    std::size_t & total_byte_count) const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    details::mutex_lock lock(info_->mtx);

    sent_bytes = info_->sent_bytes;
    total_byte_count = info_->total_byte_count;

    return info_->state;
}

void outgoing_message::wait_for_transmission() const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    info_->transmitted.wait();
}

bool outgoing_message::wait_for_transmission(
    std::size_t relative_timeout) const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return info_->transmitted.wait(relative_timeout);
}

bool outgoing_message::wait_for_transmission_absolute(
    unsigned long long timeout) const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return info_->transmitted.wait_absolute(timeout);
}

void outgoing_message::wait_for_completion() const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    info_->completed.wait();
}

bool outgoing_message::wait_for_completion(
    std::size_t relative_timeout) const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return info_->completed.wait(relative_timeout);
}

bool outgoing_message::wait_for_completion_absolute(
    unsigned long long timeout) const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return info_->completed.wait_absolute(timeout);
}

const parameters & outgoing_message::get_reply() const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    details::mutex_lock lock(info_->mtx);

    if (info_->reply_body == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return *(info_->reply_body);
}

const std::vector<char> & outgoing_message::get_raw_reply() const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    details::mutex_lock lock(info_->mtx);

    if (info_->reply_raw_buffer == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return *(info_->reply_raw_buffer);
}

parameters * outgoing_message::extract_reply()
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    details::mutex_lock lock(info_->mtx);

    parameters * result = info_->reply_body;
    info_->reply_body = NULL;

    return result;
}

const std::string & outgoing_message::get_exception_msg() const
{
    if (info_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    details::mutex_lock lock(info_->mtx);

    return info_->exception_msg;
}
