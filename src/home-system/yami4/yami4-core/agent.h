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

#ifndef YAMICORE_AGENT_H_INCLUDED
#define YAMICORE_AGENT_H_INCLUDED

#include "allocator.h"
#include "channel_descriptor.h"
#include "core.h"
#include "dll.h"

namespace yami
{

namespace details
{
class channel_group;
} // namespace details

namespace core
{

class parameters;
class serializable;

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
class DLL agent
{
public:

    /// \brief Constructor.
    ///
    /// Creates an uninitialized agent object.
    ///
    /// The only valid operations that can be executed after constructing
    /// a new agent object are:
    /// - <code>init</code> - to initialize it to the operational state
    /// - <code>clean</code> - which is an empty operation in this case
    /// - destruction
    agent();

    /// \brief Initialization.
    ///
    /// Initializes the agent object to the operational state
    /// with default values for runtime options.
    /// @param dispatch_callback Pointer to the user-defined function
    ///        that will be called when the new incoming message is completed.
    ///        This callback can be NULL.
    /// @param dispatch_hint Any parameter that will be passed to the
    ///        <code>dispatch_callback</code> function.
    /// @param disconnection_hook Pointer to the user-defined function
    ///        that will be called when the existing connection is closed.
    ///        The closing connection can be either the one that was
    ///        explicitly created or the one that was automatically added
    ///        by one of the listeners. In either case, the reason for closing
    ///        the connection is passed to the callback as well.
    ///        This callback can be NULL.
    /// @param disconnection_hook_hint Any parameter that will be passed to
    ///        the <code>disconnection_hook</code> function.
    /// @param working_area If not NULL, forces this object to use the given
    ///        area for all private storage.
    /// @param area_size If different than 0, indicates the size of the
    ///        private memory area that will be used for all dependent
    ///        structures.
    /// @return
    ///         - <code>ok</code> if the operation was successful or the
    ///           appropriate problem description
    ///
    /// <b>Notes:</b>
    /// - If <code>working_area == NULL && area_size == 0</code>
    ///   (the default) then all dependent objects are separately allocated
    ///   on the global store.
    /// - If <code>working_area != NULL && area_size != 0</code> then
    ///   the given block is used as a private area.
    ///
    /// <b>Note:</b>
    /// Do not attempt to create objects sharing the same working area.
    result init(
        incoming_message_dispatch_function dispatch_callback,
        void * dispatch_hint,
        closed_connection_function disconnection_hook = NULL,
        void * disconnection_hook_hint = NULL,
        void * working_area = NULL, std::size_t area_size = 0);

    /// \brief Initialization.
    ///
    /// Initializes the agent object to the operational state
    /// with runtime options that can override default settings.
    ///
    /// See the other <code>init</code> function
    /// for the description of paramters.
    result init(const parameters & configuration_options,
        incoming_message_dispatch_function dispatch_callback,
        void * dispatch_hint,
        closed_connection_function disconnection_hook = NULL,
        void * disconnection_hook_hint = NULL,
        void * working_area = NULL, std::size_t area_size = 0);

    /// \brief Installation of logging notifications callback.
    ///
    /// Installs the logging monitor with the given hint.
    /// The previously installed callback (if any) is overriden.
    ///
    /// <b>Note:</b>
    /// This function should be called after init.
    /// This function is not synchronized.
    void install_event_notifications(
        event_notification_function event_notification_callback,
        void * event_notification_hint);

    /// \brief Installation of I/O error logging callback.
    ///
    /// Installs the I/O error logging callback with the given hint.
    /// The previously installed callback (if any) is overriden.
    ///
    /// <b>Note:</b>
    /// This function should be called after init.
    /// This function is not synchronized.
    void install_io_error_logger(
        io_error_function io_error_callback,
        void * io_error_callback_hint);

    /// \brief Cleanup.
    ///
    /// Cleans up the dependent structures and closes all physical connections
    /// and listeners.
    /// After calling this function the agent object is back in the
    /// uninitialized state and can be reused by initializing it again.
    ///
    /// The memory structures are cleaned up (deallocated)
    /// <i>only</i> if each of them was separately allocated
    /// on the global store.
    /// If the internal structures were created with externally provided
    /// working area (that is, when <code>working_area != NULL</code> when
    /// the agent was initialized) then no memory cleanup is performed.
    void clean();

    /// \brief Destructor.
    ///
    /// Calls <code>clean</code>.
    ~agent();

    /// \brief Creates new channel for the given target.
    ///
    /// Create a new channel for the given target. If the channel
    /// already exists for the given target, this function does nothing.
    ///
    /// The supported target formats are:
    /// - "tcp://host:port" for TCP/IP connections, where <code>host</code>
    ///   can be provided in the symbolic or numeric form
    /// - "udp://host:port" for UDP communication, where <code>host</code>
    ///   can be provided in the symbolic or numeric form
    /// - "unix://path" for Unix connections, where <code>path</code>
    ///   can be relative or absolute
    /// - "file://filename" or "file://filename?write" for writing to
    ///   regular files
    /// - "file://filename?read" for reading from regular files
    /// - "file://filename?append" for appending to regular files
    ///
    /// @param target The target for the new connection.
    /// @return
    ///         - <code>ok</code> if the operation was successful or the
    ///           appropriate problem description
    result open(const char * target);

    /// \brief Create new channel for the given target.
    ///
    /// Create a new channel for the given target or find the existing
    /// channel with the same target.
    ///
    /// See the other <code>open</code> function for the description
    /// of valid target formats.
    ///
    /// @param target The target for the new connection.
    /// @param cd The descriptor that is filled so that it
    ///        refers to the newly created or found channel.
    /// @param created_new_channel set to true if a new channel was created.
    /// @return
    ///         - <code>ok</code> if the operation was successful or the
    ///           appropriate problem description
    result open(const char * target, channel_descriptor & cd,
        bool & created_new_channel);

    /// \brief Create new channel for the given target
    /// with a set of overriding options.
    ///
    /// If the new channel is created, it will use the overriding options
    /// instead of the ones used by the agent.
    /// See the other overload for description.
    result open(const char * target, channel_descriptor & cd,
        bool & created_new_channel, const parameters * overriding_options);

    /// \brief Checks if the given channel is already open.
    ///
    /// Checks if the given channel is already open.
    ///
    /// @param target The target name to check.
    /// @param existing_channel The descriptor that is filled so that it
    ///        refers to the found channel, if it exists.
    /// @return
    ///         - <code>ok</code> if the channel is already open
    ///         - <code>no_such_name</code> if the channel is not open
    result is_open(const char * target,
        channel_descriptor & existing_channel) const;

    /// \brief Closes the given channel.
    ///
    /// Closes the channel identified by the given descriptor.
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
    /// @param cd Descriptor of the channel that should be closed.
    /// @param priority Priority of the request, respects existing
    ///        messages in the outgoing queue.
    /// @return
    ///         - <code>ok</code> if the operation was successful or the
    ///           appropriate problem description
    result close(channel_descriptor cd, std::size_t priority = 0);

    /// \brief Closes the given channel.
    ///
    /// Closes the channel identified by the target name.
    ///
    /// See the other <code>close</code> function for the description
    /// of arguments, return values and the discussion of priorities.
    result close(const char * target, std::size_t priority = 0);

    /// \brief Posts new message for sending.
    ///
    /// Posts a new message to the outgoing queue of the given channel.
    ///
    /// The message is composed of two sets of parameters, one for the
    /// header information and one for the body. This distinction is
    /// supposed to support arbitrary routing conventions defined by
    /// user code. Any of these parts can be empty.
    ///
    /// The priority of the message is taken into account for proper
    /// ordering of the frames in the outgoing queue - frames created
    /// for messages with higher priority will be transmitted before
    /// frames having lower priority. Messages with equal priority are
    /// ordered according to the FIFO regime.
    ///
    /// The callback function can be provided to allow the user code trace
    /// the progress of the message. For each frame that was successfully
    /// pushed for physical transmission the callback is performed with
    /// the number of bytes that were transmitted from the beginning of the
    /// message and the total number of bytes for the whole message.
    /// When these two arguments are equal then it indicates that the whole
    /// message has been transmitted. If both are zero it means that there
    /// was an error and the message was abandoned.
    /// @param cd Descriptor of the channel that should be used for sending.
    /// @param message_header The parameters object containing (arbitrary)
    ///        header information - this object can be empty.
    /// @param message_header The parameters object containing (arbitrary)
    ///        body information - this object can be empty.
    /// @param priority Priority of the request, respects existing
    ///        frames in the outgoing queue.
    /// @param progress_callback Pointer to the user-defined function
    ///        that will be called for tracking transmission progress.
    ///        This callback can be NULL.
    /// @param dispatch_hint Any parameter that will be passed to the
    ///        <code>progress_callback</code> function.
    result post(channel_descriptor cd,
        const serializable & message_header,
        const serializable & message_body,
        std::size_t priority = 0,
        message_progress_function progress_callback = NULL,
        void * progress_hint = NULL);

    /// \brief Posts new message for sending.
    ///
    /// Posts a new message to the outgoing queue of the given channel
    /// where the channel is identified by its target.
    ///
    /// See the other <code>post</code> function for the description of
    /// arguments and their semantics.
    result post(const char * target,
        const serializable & message_header,
        const serializable & message_body,
        std::size_t priority = 0,
        message_progress_function progress_callback = NULL,
        void * progress_hint = NULL);

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
    /// @param target The target for the new listener.
    /// @param connection_hook Pointer to the user-defined function
    ///        that will be called when there is a new incoming connection
    ///        accepted by this listener. This function can intercept
    ///        and reject the incoming connection.
    ///        This callback can be NULL.
    /// @param connection_hook_hint Any parameter that will be passed to the
    ///        <code>connection_hook</code> function.
    /// @param resolved_target The pointer to actual (resolved) target
    ///        after the given target is bound.
    ///        If NULL, the resolved target is not propagated back to caller.
    /// @return
    ///         - <code>ok</code> if the operation was successful or the
    ///           appropriate problem description
    ///
    /// <b>Note</b>: The agent does not check whether the given
    /// target is already used and if there is a conflict it can be reported
    /// as an I/O error, depending on the listener protocol.
    result add_listener(const char * target,
        new_incoming_connection_function connection_hook = NULL,
        void * connection_hook_hint = NULL,
        const char * * resolved_target = NULL);

    /// \brief Removes existing listener.
    ///
    /// Removes the listener denoted by its actual target name.
    /// Note that the actual target name might be different from the name
    /// provided when the listener was created, due to target resolution.
    /// @param target Target identifying the listener to remove.
    ///        If the listener is not found, this function does nothing.
    /// @return
    ///         - <code>ok</code> if the operation was successful or the
    ///           appropriate problem description
    result remove_listener(const char * target);

    /// \brief Performs a portion of I/O or internal management work.
    ///
    /// Performs a portion of work with the given timeout.
    /// If there is some pending work at the call time it is performed
    /// immediately and function returns without waiting for further work;
    /// otherwise the call blocks waiting for the work with the given timeout.
    ///
    /// The pending work can include any of:
    /// - any of the listeners is ready to accept new connection
    /// - any of the channels is ready for reading data
    /// - any of the channels is ready for output operation and there are
    ///   pending frames in its outgoing queue
    /// - there was some change in the internal data structures that needs
    ///   to be acted upon
    ///
    /// @param timeout Timeout in milliseconds.
    /// @param allow_outgoing_traffic Flow control flag.
    /// @param allow_incoming_traffic Flow control flag.
    ///
    /// <b>Note</b>: All callbacks initiated by the agent are executed in
    /// the context of the thread that calls this function.
    /// The thread calling this function is also the only one that performs
    /// actual data transfer.
    ///
    /// <b>Note</b>: The timeout value is subject to system limits
    /// as defined for the select function.
    ///
    /// <b>Note</b>: In the typical usage scenario this function should be
    /// called in a tight loop.
    result do_some_work(std::size_t timeout,
        bool allow_outgoing_traffic = true,
        bool allow_incoming_traffic = true);

    /// \brief Artificially interrupts the wait state of do_some_work.
    result interrupt_work_waiter();

    /// \brief Returns the selector's channel usage counters.
    void get_channel_usage(int & max_allowed, int & used);
    
private:

    agent(const agent &);
    void operator=(const agent &);

    result do_init(const parameters * configuration_options,
        core::incoming_message_dispatch_function dispatch_callback,
        void * dispatch_hint,
        core::closed_connection_function disconnection_hook,
        void * disconnection_hook_hint,
        void * working_area, std::size_t area_size);

    bool initialized_;

    details::allocator alloc_;
    bool uses_private_area_;

    details::channel_group * ch_group_;
    
    int max_channels_allowed_;
    int channels_used_;
};

} // namespace core

} // namespace yami

#endif // YAMICORE_AGENT_H_INCLUDED
