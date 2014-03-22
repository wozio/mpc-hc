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

#include "incoming_message.h"
#include "agent_impl.h"
#include "details.h"
#include "incoming_message_info.h"
#include <yami4-core/core.h>

using namespace yami;

incoming_message::incoming_message(details::agent_impl & agent,
    details::incoming_message_info & info)
    : agent_(agent)
{
    source_.swap(info.source);
    object_name_.swap(info.object_name);
    message_name_.swap(info.message_name);
    message_id_ = info.message_id;

    params_ = info.body;
    info.body = NULL;
    raw_buffer_ = info.raw_buffer;
    info.raw_buffer = NULL;

    already_used_ = false;
}

incoming_message::incoming_message(incoming_message & other)
    : agent_(other.agent_)
{
    source_.swap(other.source_);
    object_name_.swap(other.object_name_);
    message_name_.swap(other.message_name_);
    message_id_ = other.message_id_;
    params_ = other.params_;
    other.params_ = NULL;
    raw_buffer_ = other.raw_buffer_;
    other.raw_buffer_ = NULL;
    already_used_ = other.already_used_;
    other.already_used_ = true;
}

incoming_message::~incoming_message()
{
    delete params_;
    delete raw_buffer_;
}

const std::string & incoming_message::get_source() const
{
    return source_;
}

const std::string & incoming_message::get_object_name() const
{
    return object_name_;
}

const std::string & incoming_message::get_message_name() const
{
    return message_name_;
}

const parameters & incoming_message::get_parameters() const
{
    if (params_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return *params_;
}

parameters * incoming_message::extract_parameters()
{
    parameters * p = params_;
    params_ = NULL;
    return p;
}

const std::vector<char> & incoming_message::get_raw_content() const
{
    if (raw_buffer_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    return *raw_buffer_;
}

void incoming_message::reply(const serializable & body, std::size_t priority)
{
    if (already_used_)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    agent_.send_reply(source_, message_id_, body, priority);
    already_used_ = true;
}

void incoming_message::reject(const std::string & reason,
    std::size_t priority)
{
    if (already_used_)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    agent_.send_rejection(source_, message_id_, reason, priority);
    already_used_ = true;
}
