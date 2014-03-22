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

#include "agent.h"
#include "agent_impl.h"
#include "value_publisher.h"

using namespace yami;

agent::agent(const parameters & options)
{
    pimpl_ = new details::agent_impl(options);
    pimpl_base_ = pimpl_;
}

agent::agent(event_callback & event_listener,
    const parameters & options)
{
    pimpl_ = new details::agent_impl(options, event_listener);
    pimpl_base_ = pimpl_;
}

agent::~agent()
{
    delete pimpl_;
}

std::string agent::add_listener(const std::string & listener)
{
    return pimpl_->add_listener(listener);
}

void agent::remove_listener(const std::string & listener)
{
    return pimpl_->remove_listener(listener);
}

void agent::register_raw_object(const std::string & object_name,
    void (* callback)(incoming_message & im, void * hint), void * hint)
{
    std::auto_ptr<details::incoming_message_dispatcher_base> object(
        new details::incoming_message_raw_dispatcher(callback, hint));

    pimpl_base_->register_object(object_name, object);
}

void agent::register_value_publisher(const std::string & object_name,
    value_publisher & publisher)
{
    publisher.register_at(*this, object_name);
}

void agent::unregister_object(const std::string & object_name)
{
    pimpl_->unregister_object(object_name);
}

void agent::open_connection(const std::string & target)
{
    pimpl_->open_connection(target);
}

void agent::open_connection(const std::string & target,
    const parameters & options)
{
    pimpl_->open_connection(target, options);
}

std::auto_ptr<outgoing_message> agent::send(
    const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect)
{
    return pimpl_->send(
        target, object_name, message_name, content, priority, auto_connect);
}

void agent::send(
    outgoing_message & message,
    const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect)
{
    pimpl_->send(message,
        target, object_name, message_name, content, priority, auto_connect);
}

void agent::clean_outgoing_message_callback(outgoing_message_id id)
{
    pimpl_->clean_outgoing_message_callback(id);
}

void agent::send_one_way(const std::string & target,
    const std::string & object_name,
    const std::string & message_name,
    const serializable & content,
    std::size_t priority,
    bool auto_connect)
{
    pimpl_->send_one_way(
        target, object_name, message_name, content, priority, auto_connect);
}

void agent::close_connection(const std::string & target, std::size_t priority)
{
    pimpl_->close_connection(target, priority);
}

void agent::get_outgoing_flow_state(std::size_t & current_level,
    std::size_t & high_water_mark, std::size_t & low_water_mark) const
{
    pimpl_->get_outgoing_flow_state(
        current_level, high_water_mark, low_water_mark);
}

void agent::get_channel_usage(int & max_allowed, int & used)
{
    pimpl_->get_channel_usage(max_allowed, used);
}

