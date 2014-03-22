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

#ifndef YAMICPP_AGENT_H_INCLUDED
#define YAMICPP_AGENT_H_INCLUDED

#include "agent_impl_base.h"
#include "connection_event_generic_dispatcher.h"
#include "event_callback.h"
#include "incoming_message_generic_dispatcher.h"
#include "io_error_generic_dispatcher.h"
#include "outgoing_message.h"
#include "outgoing_message_generic_dispatcher.h"
#include "parameters.h"
#include <yami4-core/dll.h>
#include <memory>
#include <string>

namespace yami
{

class value_publisher;

namespace details
{
class agent_impl;
} // namespace details

/// \brief Message broker.
///
/// The message broker that encapsulates physical channel management,
/// incoming and outgoing message queues, listeners and resource
/// management.
///
/// A single agent object can manage many listeners, which are responsible
/// for accepting remote connections, and many incoming and outgoing
/// connections.
///
/// The agent objects can be created and destroyed without constraints
/// on the stack, on the free store or as static objects.
///
/// The objects of this class can be safely used by multiple threads.
class DLL agent
{
public:

    /// Outgoing message identifier type.
    typedef long long outgoing_message_id;

    /// \brief Constructor.
    ///
    /// Creates the message broker and starts its internal threads.
    /// The broker created with this constructor has no listener.
    agent(const parameters & options = parameters());

    /// \brief Constructor.
    ///
    /// Creates the message broker with event notification callback.
    /// The broker created with this constructor has no listener.
    agent(event_callback & event_listener,
        const parameters & options = parameters());

    /// \brief Destructor.
    ///
    /// The destructor stops the internal threads and cleans up all
    /// internal data structures.
    ///
    /// <b>Note:</b>
    /// The messages and replies that were posted for transmission and that
    /// have not yet been fully transmitted are abandoned; in the case of
    /// outgoing messages their state is properly notified about that fact.
    ~agent();

    /// \brief Adds new listener.
    ///
    /// Adds a new listener for the given target address.
    ///
    /// The supported target formats are:
    /// - "tcp://host:port" for TCP/IP connections, where <code>host</code>
    ///   can be provided in the symbolic or numeric form
    /// - "tcp://*:port" for TCP/IP connections, for "any" local address
    /// - "tcp://port" for TCP/IP connections, for "any" local address
    /// - "udp://host:port" for UDP communication, with rules as for TCP/IP
    /// - "unix://path" for Unix connections
    ///
    /// The port for TCP/IP and UDP protocols can be
    /// <code>0</code> or <code>*</code>,
    /// in which case the actual port number is assigned by the system.
    ///
    /// @param listener The target name for the new listener.
    /// @return The locally resolved listener name. This name can be used
    ///         to remove the listener later on.
    std::string add_listener(const std::string & listener);

    /// \brief Removes existing listener.
    ///
    /// Removes the listener denoted by its actual target name.
    /// Note that the actual target name might be different from the name
    /// provided when the listener was created, due to target resolution.
    /// The name which should be used for listener removal is the name
    /// that is returned by the <code>add_listener</code> function.
    void remove_listener(const std::string & listener);

    /// \brief Registers the new logical destination object.
    ///
    /// Registers the new "object" that can be a logical destination
    /// for incoming messages.
    ///
    /// @param object_name The name of the newly registered object.
    ///                    If an object with this name is already registered,
    ///                    the registration data is replaced.
    /// @param f The callable entity that can accept the
    ///          <code>incoming_message</code> as the invocation parameter.
    template <typename functor>
    void register_object(const std::string & object_name, functor & f)
    {
        std::auto_ptr<details::incoming_message_dispatcher_base> object(
            new details::incoming_message_generic_dispatcher<functor>(f));

        pimpl_base_->register_object(object_name, object);
    }

    void register_raw_object(const std::string & object_name,
        void (* callback)(incoming_message & im, void * hint), void * hint);

    /// \brief Registers the value publisher as a new logical object.
    ///
    /// @param object_name The name of the newly registered object.
    ///                    If an object with this name is already registered,
    ///                    the registration data is replaced.
    /// @param publisher The value publisher to be registered.
    void register_value_publisher(const std::string & object_name,
        value_publisher & publisher);

    /// \brief Unregisters the logical destination object.
    ///
    /// It is permitted to request unregistration for an object
    /// that does not exist - such operation has no effect.
    ///
    /// <b>Note:</b>
    /// Due to performance and design tradeoffs it is <b>not</b> guaranteed
    /// that no more messages will be ever dispatched to the given object
    /// when this function returns.
    /// In fact, some of the messages that have been received by agent and not
    /// yet dispatched might be still dispatched shortly after
    /// this function returns.
    /// Only those messages that are received by agent after
    /// this function returns are guaranteed not to be dispatched to the
    /// unregistered object.
    /// This might be particularly important with regard
    /// to the lifetime of the callable entity that was provided when
    /// the given object has been registered.
    ///
    /// @param object_name The name of the object to be unregistered.
    void unregister_object(const std::string & object_name);

    /// \brief Opens the new connection.
    ///
    /// Opens the new channel or does nothing if the channel already exists.
    ///
    /// This function is not necessary with automatic connection
    /// recovery option in <code>send</code> and <code>send_one_way</code>.
    ///
    /// @param target The name of the target endpoint.
    ///               This name should correspond to the listener name
    ///               in some target agent object.
    void open_connection(const std::string & target);

    /// \brief Opens the new connection with overriding options.
    ///
    /// Opens the new channel or does nothing if the channel already exists.
    /// If the new channel is created, it will use the overriding options
    /// from those which are defined.
    ///
    /// This function is not necessary with automatic connection
    /// recovery option in <code>send</code> and <code>send_one_way</code>.
    ///
    /// @param target The name of the target endpoint.
    ///               This name should correspond to the listener name
    ///               in some target agent object.
    /// @param options The set of options that will override agent's values.
    void open_connection(const std::string & target,
        const parameters & options);

    /// \brief Sends the new outgoing message.
    ///
    /// Sends the new outgoing message to the given destination.
    ///
    /// @param target The name of the target endpoint.
    ///               This name should correspond to the listener name
    ///               in some target agent object.
    /// @param object_name The name of the logical destination object
    ///                    in the target agent.
    /// @param message_name The name of the message.
    /// @param content The content of the message.
    /// @param priority The priority of the message.
    /// @param auto_connect The flag controlling automatic (re)connection.
    /// @return The <code>outgoing_message</code> object that allows to
    ///         track the progress of this message, its status and obtain
    ///         response data.
    ///
    /// <b>Note:</b>
    /// This function implicitly opens a new communication channel
    /// if it is not already open. This channel is kept open until
    /// it is explicitly closed
    /// (see the <code>close_connection</code> function)
    /// or until the agent is destroyed or the communication error
    /// is detected.
    std::auto_ptr<outgoing_message> send(
        const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content = parameters(),
        std::size_t priority = 0,
        bool auto_connect = true);

    /// \brief Sends the outgoing message.
    ///
    /// Sends the outgoing message to the given destination and reinitializes
    /// the message object in-place.
    ///
    /// This function behaves as the other version of send, except for the
    /// outgoing_message object creation policy.
    void send(
        outgoing_message & message,
        const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content = parameters(),
        std::size_t priority = 0,
        bool auto_connect = true);

    /// \brief Sends the outgoing message.
    ///
    /// Sends the outgoing message to the given destination and executes
    /// the callback whenever the status of the message changes.
    /// The callback should accept the outgoing_message object
    /// as its parameter.
    /// Note: the callback should not block the processing unnecessarily.
    ///
    /// This function behaves as the other version of send, except for the
    /// outgoing_message object creation policy.
    template <typename functor>
    outgoing_message_id send(
        functor & f,
        const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content = parameters(),
        std::size_t priority = 0,
        bool auto_connect = true)
    {
        std::auto_ptr<details::outgoing_message_dispatcher_base>
            outgoing_message_callback(
            new details::outgoing_message_generic_dispatcher<functor>(f));

        return pimpl_base_->send(outgoing_message_callback,
            target, object_name, message_name,
            content, priority, auto_connect);
    }
    
    /// \brief Cleans internal resources for the given message callback.
    ///
    /// Cleans internal resources associated with the given message callback.
    ///
    /// @param id Outgoing message identifier, as obtained from send.
    void clean_outgoing_message_callback(outgoing_message_id id);

    /// \brief Sends the new outgoing message.
    ///
    /// Sends the new outgoing message to the given destination, without
    /// the possibility to track its progress.
    ///
    /// See the description and notes for the <code>send</code> function.
    void send_one_way(const std::string & target,
        const std::string & object_name,
        const std::string & message_name,
        const serializable & content = parameters(),
        std::size_t priority = 0,
        bool auto_connect = true);

    /// \brief Closes the given communication channel.
    ///
    /// Closes the channel identified by name.
    ///
    /// The priority allows to properly handle the existing outgoing
    /// messages that are waiting in the outgoing queue for transmission.
    /// The existing messages with lower priority are
    /// abandoned, whereas the existing messages with priority equal
    /// or higher to the one provided as parameter are retained in the
    /// outgoing queue and are properly pushed for transmission
    /// before the channel is physically closed.
    /// The channel is closed immediately only if there are no
    /// messages waiting in its outgoing queue.
    ///
    /// @param target The name of the target endpoint.
    /// @param priority Proprity of the request, respects existing
    ///        messages in the outgoing queue.
    void close_connection(const std::string & target,
        std::size_t priority = 0);

    /// \brief Registers the monitor for connection-related events.
    ///
    /// Registers the monitor for connection events.
    ///
    /// <b>Note:</b>
    /// The monitor callback is intentionally not synchronized.
    /// Use this function after constructing the agent, but before
    /// opening any connections.
    ///
    /// @param f The callable entity that can accept the
    ///          <code>std::string</code> as the connection name
    ///          and <code>connection_event</code> as event description.
    template <typename functor>
    void register_connection_event_monitor(functor & f)
    {
        std::auto_ptr<details::connection_event_dispatcher_base> monitor(
            new details::connection_event_generic_dispatcher<functor>(f));

        pimpl_base_->register_connection_event_monitor(monitor);
    }

    /// \brief Registers the logger for I/O errors.
    ///
    /// Registers the logger for I/O errors.
    ///
    /// <b>Note:</b>
    /// The logger callback is intentionally not synchronized.
    /// Use this function after constructing the agent, but before
    /// opening any connections.
    ///
    /// @param f The callable entity that can accept the
    ///          <code>int</code> as the error code
    ///          and <code>const char *</code> as error description.
    template <typename functor>
    void register_io_error_logger(functor & f)
    {
        std::auto_ptr<details::io_error_dispatcher_base> logger(
            new details::io_error_generic_dispatcher<functor>(f));

        pimpl_base_->register_io_error_logger(logger);
    }

    /// \brief Obtains the state of overall outgoing flow.
    ///
    /// Obtains the state of overall outgoing flow.
    ///
    /// <b>Note:</b>
    /// The outgoing flow is a combination of all outgoing traffic,
    /// and is not tied to any particular communication channel.
    ///
    /// @param current_level The current level of the outgoing flow.
    /// @param high_water_mark The high water mark.
    /// @param low_water_mark The low water mark.
    void get_outgoing_flow_state(std::size_t & current_level,
        std::size_t & high_water_mark, std::size_t & low_water_mark) const;

    /// \brief Returns the selector's channel usage counters.
    ///
    /// <b>Note:</b>
    /// The information obtained with this function can be in constant flow,
    /// as incoming and outgoing channels are created and closed.
    ///
    /// @param max_allowed The maximum number of channels that the agent
    ///                    can handle internall or 0 if the limit is unknown.
    /// @param used Total number of handled channels and listeners.
    void get_channel_usage(int & max_allowed, int & used);
    
private:
    agent(const agent &);
    void operator=(const agent &);

    details::agent_impl * pimpl_;
    details::agent_impl_base * pimpl_base_;
};

} // namespace yami

#endif // YAMICPP_AGENT_H_INCLUDED
