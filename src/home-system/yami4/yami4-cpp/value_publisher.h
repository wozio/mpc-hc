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

#ifndef YAMICPP_VALUE_PUBLISHER_H_INCLUDED
#define YAMICPP_VALUE_PUBLISHER_H_INCLUDED

#include "incoming_message_generic_dispatcher.h"
#include "value_publisher_overflow_generic_dispatcher.h"
#include <yami4-core/dll.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace yami
{

class agent;
class incoming_message;
class serializable;

namespace details
{
class value_publisher_impl;
}

/// \brief Simple subscription publisher.
///
/// The subscription publisher that notifies remote listeners
/// with published value updates.
///
/// Remote listeners can subscribe and unsubscribe at any time.
class DLL value_publisher
{
public:

    /// \brief Constructor.
    ///
    /// Creates the subscription publisher
    /// that is not registered at any agent.
    value_publisher();

    /// \brief Destructor.
    ///
    /// <b>Note:</b>
    /// The destructor automatically unregisters the publisher from
    /// the agent it was registered at, which might cause hazards
    /// if some updates are pending in the incoming message queue.
    ~value_publisher();

    /// \brief Constructor.
    ///
    /// Creates the subscription publisher
    /// that is not registered at any agent and that has a handler
    /// for arbitrary remote commands.
    ///
    /// <b>Note:</b>
    /// The "subscribe" and "unsubscribe" messages are also forwarded
    /// to the user-provided callback, but these two messages are already
    /// "replied-to" by the publisher's implementation.
    ///
    /// @param f The callable entity that can accept the
    ///          <code>incoming_message</code> as the invocation parameter.
    template <typename functor>
    value_publisher(functor & f)
    {
        user_command_handler_.reset(
            new details::incoming_message_generic_dispatcher<functor>(f));
        init(user_command_handler_.get());
    }

    /// \brief Constructor.
    ///
    /// Creates the subscription publisher
    /// that is not registered at any agent and that has a handler
    /// for arbitrary remote commands
    /// as well as for the queue overflow conditions.
    ///
    /// <b>Note:</b>
    /// The "subscribe" and "unsubscribe" messages are also forwarded
    /// to the user-provided callback, but these two messages are already
    /// processed by the publisher's implementation.
    ///
    /// @param f The callable entity that can accept the
    ///          <code>incoming_message</code> as the invocation parameter.
    /// @param max_queue_length Length of message queue for each subscriber.
    /// @param qof The callable entity that can accept the
    ///            <code>server_name</code>, <code>object_name</code> and
    ///            <code>value</code> as invocation parameters.
    template <typename incoming_message_functor,
              typename queue_overflow_functor>
    value_publisher(incoming_message_functor & f,
        std::size_t max_queue_length, queue_overflow_functor & qof)
    {
        user_command_handler_.reset(
            new details::incoming_message_generic_dispatcher<
                incoming_message_functor>(f));
        user_overflow_handler_.reset(
            new details::value_publisher_overflow_generic_dispatcher<
                queue_overflow_functor>(qof));
        init(user_command_handler_.get(),
            max_queue_length, user_overflow_handler_.get());
    }

    /// \brief Registers the publisher at the given agent.
    ///
    /// @param controlling_agent The agent which should manage
    ///                          the communication for this publisher.
    /// @param object_name The name of object that should be visible to
    ///                    remote subscribers.
    void register_at(agent & controlling_agent,
        const std::string & object_name);

    /// \brief Unregisters the publisher from its associated agent.
    void unregister();

    /// \brief Subscribes the new listener.
    ///
    /// This function is usually called internally as a result of
    /// processing the remote "subscribe" message, but can be also
    /// used locally if the listener's location is obtained via
    /// other means.
    ///
    /// @param destination_target Target of the remote listener.
    /// @param destination_object Name of the remote listener's object.
    void subscribe(const std::string & destination_target,
        const std::string & destination_object);

    /// \brief Unsubscribes the given listener.
    ///
    /// @param destination_target Target of the remote listener.
    void unsubscribe(const std::string & destination_target);

    /// \brief Publishes the new value.
    ///
    /// Sends the update message to all active listeners with the given value.
    /// In case of any errors or communication problems, the problematic
    /// listener is automatically unsubscribed.
    ///
    /// @param value New value that is to be sent as update to all listeners.
    /// @param priority The priority of the update message.
    void publish(const serializable & value, std::size_t priority = 0);

    /// \brief Returns the number of active subscribers.
    std::size_t get_number_of_subscribers() const;

    /// \brief Returns the information about all active subscribers.
    ///
    /// The first component of each vector entry is a destination target
    /// and the second component is a destination object for
    /// the given subscriber.
    std::vector<std::pair<std::string, std::string> >
    get_subscribers() const;

private:

    value_publisher(const value_publisher &);
    void operator=(const value_publisher &);

    void init(details::incoming_message_dispatcher_base * imd = NULL,
        std::size_t max_queue_length = 1,
        details::value_publisher_overflow_dispatcher_base * qod = NULL);

    details::value_publisher_impl * pimpl_;

    std::auto_ptr<details::incoming_message_dispatcher_base>
        user_command_handler_;
    std::auto_ptr<details::value_publisher_overflow_dispatcher_base>
        user_overflow_handler_;
};

} // namespace yami

#endif // YAMICPP_VALUE_PUBLISHER_H_INCLUDED
