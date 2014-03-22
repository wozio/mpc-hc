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

#ifndef YAMICPP_VALUE_PUBLISHER_IMPL_H_INCLUDED
#define YAMICPP_VALUE_PUBLISHER_IMPL_H_INCLUDED

// selected per platform
#include <mutex.h>

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace yami
{

class agent;
class incoming_message;
class outgoing_message;
class serializable;
class value_publisher;

namespace details
{

class incoming_message_dispatcher_base;
class value_publisher_overflow_dispatcher_base;

class value_publisher_impl
{
public:

    value_publisher_impl(
        incoming_message_dispatcher_base * imd,
        std::size_t max_queue_length,
        value_publisher_overflow_dispatcher_base * qod);
    ~value_publisher_impl();

    void register_at(agent & controlling_agent,
        const std::string & object_name);

    void unregister();

    void operator()(incoming_message & message);

    void subscribe(const std::string & destination_target,
        const std::string & destination_object);

    void unsubscribe(const std::string & destination_target);

    void publish(const serializable & value, std::size_t priority);

    std::size_t get_number_of_subscribers() const;

    std::vector<std::pair<std::string, std::string> >
    get_subscribers() const;

private:

    value_publisher_impl(const value_publisher_impl &);
    void operator=(const value_publisher_impl &);

    std::size_t max_queue_length_;

    // destination target ->
    //      {destination object, previously sent message (or NULL)}
    typedef std::map<std::string,
                     std::pair<std::string,
                               std::deque<outgoing_message *> > >
        subscriptions_map_type;

    void do_unsubscribe(subscriptions_map_type::iterator it);
    void release_last_messages(subscriptions_map_type::iterator it);

    incoming_message_dispatcher_base * incoming_message_dispatcher_;
    value_publisher_overflow_dispatcher_base * overflow_dispatcher_;

    agent * controlling_agent_;
    std::string object_name_;

    subscriptions_map_type subscriptions_;
    mutable details::mutex mtx_;
};

} // namespace details

} // namespace yami

#endif // YAMICPP_VALUE_PUBLISHER_IMPL_H_INCLUDED
