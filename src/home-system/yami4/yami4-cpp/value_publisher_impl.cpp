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

#include "value_publisher_impl.h"
#include "agent.h"
#include "details.h"
#include "incoming_message.h"
#include "incoming_message_dispatcher_base.h"
#include "mutex_lock.h"
#include "outgoing_message.h"
#include "parameters.h"
#include "parameter_entry.h"
#include "value_publisher.h"
#include "value_publisher_overflow_dispatcher_base.h"

#include <algorithm>

using namespace yami;
using namespace yami::details;

value_publisher_impl::value_publisher_impl(
    incoming_message_dispatcher_base * imd,
    std::size_t max_queue_length,
    value_publisher_overflow_dispatcher_base * qod)
    : max_queue_length_(max_queue_length),
      incoming_message_dispatcher_(imd),
      overflow_dispatcher_(qod),
      controlling_agent_(NULL)
{
    mtx_.init();
}

value_publisher_impl::~value_publisher_impl()
{
    if (controlling_agent_ != NULL)
    {
        controlling_agent_->unregister_object(object_name_);
    }

    subscriptions_map_type::iterator it = subscriptions_.begin();
    subscriptions_map_type::iterator end = subscriptions_.end();
    for (; it != end; ++it)
    {
        release_last_messages(it);
    }

    mtx_.clean();
}

void value_publisher_impl::register_at(agent & controlling_agent,
    const std::string & object_name)
{
    if (controlling_agent_ != NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    controlling_agent.register_object(object_name, *this);

    controlling_agent_ = &controlling_agent;
    object_name_ = object_name;
}

void value_publisher_impl::unregister()
{
    if (controlling_agent_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    controlling_agent_->unregister_object(object_name_);

    controlling_agent_ = NULL;
}

void value_publisher_impl::operator()(incoming_message & message)
{
    const std::string & message_name = message.get_message_name();

    if (message_name == "subscribe" || message_name == "unsubscribe")
    {
        // extract the destination target

        const parameters & content = message.get_parameters();

        std::string destination_target;

        parameter_entry entry;
        if (content.find("destination_target", entry))
        {
            if (entry.type() == string)
            {
                destination_target = entry.get_string();
            }
        }

        if (destination_target.empty())
        {
            // if the destination target is not specified
            // in the subscription message, use the
            // message source as a default

            destination_target = message.get_source();
        }

        if (message_name == "subscribe")
        {
            // extract the destination object name

            std::string destination_object;

            if (content.find("destination_object", entry))
            {
                if (entry.type() == string)
                {
                    destination_object = entry.get_string();
                }
            }

            if (destination_object.empty())
            {
                // if the destination object is not specified
                // in the subscription message, use the
                // local object name as a default

                destination_object = message.get_object_name();
            }

            subscribe(destination_target, destination_object);
        }
        else // message_name == "unsubscribe"
        {
            unsubscribe(destination_target);
        }
    }

    // any message - delegate to parent

    if (incoming_message_dispatcher_ != NULL)
    {
        try
        {
            incoming_message_dispatcher_->dispatch(message);
        }
        catch (...)
        {
            // ignore user exceptions here
        }
    }
    else
    {
        // in the absence of user callback, just confirm this operation

        message.reply();
    }
}

void value_publisher_impl::subscribe(
    const std::string & destination_target,
    const std::string & destination_object)
{
    details::mutex_lock lock(mtx_);

    const subscriptions_map_type::iterator it =
        subscriptions_.find(destination_target);
    if (it == subscriptions_.end())
    {
        // this is a new subscription

        // make sure the channel exists,
        // so that further sends will not have to create it

        controlling_agent_->open_connection(destination_target);

        subscriptions_[destination_target] =
            std::make_pair(destination_object,
                std::deque<outgoing_message *>());
    }
    else
    {
        // there is already a subscription for this target
        // -> refresh it

        release_last_messages(it);

        it->second.first = destination_object;
    }
}

void value_publisher_impl::unsubscribe(const std::string & destination_target)
{
    details::mutex_lock lock(mtx_);

    const subscriptions_map_type::iterator it =
        subscriptions_.find(destination_target);
    if (it != subscriptions_.end())
    {
        do_unsubscribe(it);
    }
}

void value_publisher_impl::publish(
    const serializable & value, std::size_t priority)
{
    if (controlling_agent_ == NULL)
    {
        details::translate_result_to_exception(core::bad_state);
    }

    details::mutex_lock lock(mtx_);

    subscriptions_map_type::iterator it = subscriptions_.begin();
    subscriptions_map_type::iterator end = subscriptions_.end();
    while (it != end)
    {
        const std::string & destination_target = it->first;
        const std::string & destination_object = it->second.first;
        std::deque<outgoing_message *> & last_sent_messages =
            it->second.second;

        // check all previous messages that were still not processed
        // by this subscriber

        bool abandon_subscription = false;

        std::deque<outgoing_message *>::iterator omitbegin =
            last_sent_messages.begin();
        std::deque<outgoing_message *>::iterator omitend =
            last_sent_messages.end();
        for (std::deque<outgoing_message *>::iterator omit = omitbegin;
             abandon_subscription == false && omit != omitend; ++omit)
        {
            outgoing_message * msg = *omit;
            message_state state = msg->get_state();
            switch (state)
            {
            case transmitted:
            case replied:
            case rejected:

                // this previous message has been successfully sent
                // -> remove it from the list

                delete msg;
                *omit = NULL;
                break;

            case abandoned:

                // the whole channel is broken
                // - abandon the subscription

                abandon_subscription = true;
                break;

            default:
                break;
            }
        }

        last_sent_messages.erase(
            std::remove(omitbegin, omitend,
                static_cast<outgoing_message *>(NULL)),
            last_sent_messages.end());

        if (abandon_subscription)
        {
            do_unsubscribe(it++);
            continue;
        }

        // check if there is a place in the queue

        if (last_sent_messages.size() >= max_queue_length_)
        {
            // the queue is full - ask user for decision

            value_publisher_overflow_action decision =
                wait_for_previous_message;
            if (overflow_dispatcher_ != NULL)
            {
                try
                {
                    decision = overflow_dispatcher_->dispatch(
                        destination_target,
                        destination_object,
                        value);
                }
                catch (...)
                {
                    // threat user exceptions as "abandon message"

                    decision = abandon_message;
                }
            }

            if (decision == wait_for_previous_message)
            {
                outgoing_message * msg = last_sent_messages[0];
                msg->wait_for_transmission();
                delete msg;
                last_sent_messages.pop_front();
            }
            else if (decision == abandon_message)
            {
                ++it;
                continue;
            }
            else // decision == abandon_subscription
            {
                do_unsubscribe(it++);
                continue;
            }
        }

        // send the message

        try
        {
            const bool auto_connect = false;

            std::auto_ptr<outgoing_message> message(
                controlling_agent_->send(destination_target,
                    destination_object, "subscription_update",
                    value, priority, auto_connect));

            last_sent_messages.push_back(message.get());
            message.release();

            ++it;
        }
        catch (...)
        {
            // in case of any error drop this subscription

            do_unsubscribe(it++);
        }
    }
}

std::size_t value_publisher_impl::get_number_of_subscribers() const
{
    details::mutex_lock lock(mtx_);

    return subscriptions_.size();
}

std::vector<std::pair<std::string, std::string> >
value_publisher_impl::get_subscribers() const
{
    std::vector<std::pair<std::string, std::string> > result;

    details::mutex_lock lock(mtx_);

    subscriptions_map_type::const_iterator it = subscriptions_.begin();
    subscriptions_map_type::const_iterator end = subscriptions_.end();
    for (; it != end; ++it)
    {
        const std::string & destination_target = it->first;
        const std::string & destination_object = it->second.first;

        result.push_back(
            std::make_pair(destination_target, destination_object));
    }

    return result;
}

// synchronized by caller
void value_publisher_impl::do_unsubscribe(subscriptions_map_type::iterator it)
{
    release_last_messages(it);
    subscriptions_.erase(it);
}

// synchronized by caller
void value_publisher_impl::release_last_messages(
    subscriptions_map_type::iterator it)
{
    std::deque<outgoing_message *> & last_sent_messages = it->second.second;
    std::deque<outgoing_message *>::iterator omit =
        last_sent_messages.begin();
    std::deque<outgoing_message *>::iterator end =
        last_sent_messages.end();
    for (; omit != end; ++omit)
    {
        outgoing_message * msg = *omit;
        delete msg;
    }

    last_sent_messages.clear();
}
