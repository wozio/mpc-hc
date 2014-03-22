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

#ifndef YAMICORE_CORE_H_INCLUDED
#define YAMICORE_CORE_H_INCLUDED

#include "channel_descriptor.h"
#include <cstddef>

/// Namespace devoted for everything related to YAMI4.
namespace yami
{

/// Namespace defining the YAMI4-core communication module.
namespace core
{

/// General type for reporting success and error states.
enum result
{
    ok,               ///< Operation completed successfully.
    no_such_name,     ///< The given name was not found.
    bad_type,         ///< The expected type is different than the actual.
    no_such_index,    ///< Index out of range.
    no_memory,        ///< Not enough memory.
    nesting_too_deep, ///< The nesting of parameters is too deep.
    not_enough_space, ///< There is not enough space in the buffer.
    no_entries,       ///< There are no entries.
    unexpected_value, ///< The given value was not recognized.
    bad_protocol,     ///< The connection protocol is incorrect.
    io_error,         ///< Unable to perform the I/O operation.
    timed_out,        ///< The requested operation timed out.
    channel_closed,   ///< The operation was not possible due to EOF.
    bad_state         ///< The given object is in the wrong state.
};

/// Type of function callback for incoming message dispatch.
/// @param hint Arbitrary (any) argument provided at the time
///        when the callback was installed.
/// @param source Name of the originating channel.
/// @param header_buffers Array of pointers to data buffers that can be used
///        to deserialize the message header.
/// @param header_buffer_sizes Array of sizes of header data buffers.
/// @param num_of_header_buffers Number of elements in the
///        <code>header_buffers</code> and <code>header_buffer_sizes</code>
///        arrays.
/// @param body_buffers Array of pointers to data buffers that can be used
///        to deserialize the message body.
/// @param body_buffer_sizes Array of sizes of body data buffers.
/// @param num_of_body_buffers Number of elements in the
///        <code>body_buffers</code> and <code>body_buffer_sizes</code>
///        arrays.
///
/// <b>Note</b>: The arguments to this callback can be directly used with
/// the <code>deserialize</code> function of the <code>parameters</code>
/// class.
extern "C" typedef void (*incoming_message_dispatch_function)(
    void * hint,
    const char * source,
    const char * header_buffers[],
    std::size_t header_buffer_sizes[],
    std::size_t num_of_header_buffers,
    const char * body_buffers[],
    std::size_t body_buffer_sizes[],
    std::size_t num_of_body_buffers);

/// Type of function callback (hook) for new incoming connection.
/// This function sees the new channel already in the proper state
/// and can use it (in particular it can close it).
/// @param hint Arbitrary (any) argument provided at the time
///        when the callback was installed.
/// @param source The source (remote) name of the newly created channel.
/// @param index Index for the channel descriptor.
/// @param sequence_number Sequence number for the channel descriptor.
/// @return <code>true</code> if the connection is accepted and
///         <code>false</code> otherwise.
extern "C" typedef void (*new_incoming_connection_function)(
    void * hint,
    const char * source,
    std::size_t index, std::size_t sequence_number);

/// Type of function callback (hook) for closing connection.
/// @param hint Arbitrary (any) argument provided at the time
///        when the callback was installed.
/// @param name The name of the closing channel.
/// @param reason The reason for closing the connection, this is either
///        <code>channel_closed</code> for regular end-of-stream condition
///        or an appropriate error value that was reported while operating
///        on the given channel.
extern "C" typedef void (*closed_connection_function)(
    void * hint,
    const char * name, result reason);

/// Type of function callback for outgoing message progress report.
/// If both size values are zero, it means that there was an
/// error while sending the message -
/// the message itself will be abandoned and channel will be closed.
/// @param hint Arbitrary (any) argument provided at the time
///        when the callback was installed.
/// @param sent_bytes The number of bytes accumulated from the beginning
///        of the message that have been sent.
/// @param total_byte_count The overall size of the message.
///
/// <b>Note</b>: When <code>sent_bytes</code> and
/// <code>total_byte_count</code> are equal it means that the whole message
/// has been sent; if they are both zero it indicates that there was an error
/// and the message was abandoned.
extern "C" typedef void (*message_progress_function)(
    void * hint,
    std::size_t sent_bytes,
    std::size_t total_byte_count);

/// Type of internal event notification.
enum event_notification
{
    agent_closed,              // (NULL, 0)
    listener_added,            // (target, 0)
    listener_removed,          // (target, 0)
    incoming_connection_open,  // (target, 0)
    outgoing_connection_open,  // (target, 0)
    connection_closed,         // (target, 0)
    connection_error,          // (target, 0)
    message_sent,              // (target, bytes)
    message_received           // (target, bytes)
};

/// Type of function callback for internal event notifications (logging).
/// Depending on the event type, some callback parameters are used.
extern "C" typedef void (*event_notification_function)(
    void * hint,
    event_notification e,
    const char * str,
    std::size_t size);

/// Type of function callback for internal I/O error logging.
extern "C" typedef void (*io_error_function)(
    void * hint,
    int error_code,
    const char * description);

} // namespace core

} // namespace yami

#endif // YAMICORE_CORE_H_INCLUDED
