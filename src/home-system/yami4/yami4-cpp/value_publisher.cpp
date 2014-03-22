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

#include "value_publisher.h"
#include "value_publisher_impl.h"

using namespace yami;

value_publisher::value_publisher()
{
    init();
}

value_publisher::~value_publisher()
{
    delete pimpl_;
}

void value_publisher::init(
    details::incoming_message_dispatcher_base * imd,
    std::size_t max_queue_length,
    details::value_publisher_overflow_dispatcher_base * qod)
{
    pimpl_ = new details::value_publisher_impl(imd, max_queue_length, qod);
}

void value_publisher::register_at(agent & controlling_agent,
    const std::string & object_name)
{
    pimpl_->register_at(controlling_agent, object_name);
}

void value_publisher::unregister()
{
    pimpl_->unregister();
}

void value_publisher::subscribe(const std::string & destination_target,
    const std::string & destination_object)
{
    pimpl_->subscribe(destination_target, destination_object);
}

void value_publisher::unsubscribe(const std::string & destination_target)
{
    pimpl_->unsubscribe(destination_target);
}

void value_publisher::publish(const serializable & value, std::size_t priority)
{
    pimpl_->publish(value, priority);
}

std::size_t value_publisher::get_number_of_subscribers() const
{
    return pimpl_->get_number_of_subscribers();
}

std::vector<std::pair<std::string, std::string> >
value_publisher::get_subscribers() const
{
    return pimpl_->get_subscribers();
}
