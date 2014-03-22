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

#ifndef YAMICPP_INCOMING_MESSAGE_H_INCLUDED
#define YAMICPP_INCOMING_MESSAGE_H_INCLUDED

#include "parameters.h"
#include <yami4-core/dll.h>
#include <string>
#include <vector>

namespace yami
{

namespace details
{
class agent_impl;
struct incoming_message_info;
} // namespace details

/// \brief Incoming message.
///
/// The handler allowing to inspect the details of the incoming message
/// and sent back replies or rejection notifications.
///
/// The user code interacts with objects of this type mainly in the
/// functors that are provided during object registration and that are later
/// called back when the incoming message arrives. The handler objects
/// can be stored aside for further processing even after the callback
/// returns, but should not be kept alive longer than the agent itself.
///
/// <b>Note:</b>
/// The objects of this class are not supposed to be used
/// from multiple threads.
class DLL incoming_message
{
public:
    incoming_message(details::agent_impl & agent,
        details::incoming_message_info & info);

    /// \brief Moving constructor.
    ///
    /// This class was designed to be non-copyable, but allows
    /// "destructive copy" similar to that of <code>std::auto_ptr</code>.
    /// This functionality is enough for the user to store aside the
    /// handler for further processing, but the original object is no
    /// longer valid and all attempts to use it will result in the exception.
    incoming_message(incoming_message & other);

    ~incoming_message();

    /// \brief Returns the source of this incoming message.
    ///
    /// @return The name of the originating endpoint from where this
    ///         incoming message has been sent.
    const std::string & get_source() const;

    /// \brief Returns the destination object name.
    const std::string & get_object_name() const;

    /// \brief Returns the message name.
    const std::string & get_message_name() const;

    /// \brief Provides access to the message content.
    const parameters & get_parameters() const;

    /// \brief Extracts the message content.
    ///
    /// The message content is moved to the caller and the caller
    /// becomes the owner of the <code>parameters</code> object.
    /// This object itself is no longer holding any references to
    /// the content and therefore any future attempts to access it
    /// will result in the exception.
    parameters * extract_parameters();

    /// \brief Provides access to the raw buffer of message content.
    ///
    /// Note: this function can be called only if the messages are delivered
    /// in the form of raw (not already serialized) buffers.
    const std::vector<char> & get_raw_content() const;

    /// \brief Sends back the reply.
    ///
    /// Sends back the reply to the message identified by this object.
    /// The reply (or rejection) can be sent only once.
    ///
    /// @param body The content of the reply.
    /// @param priority The priority of the reply.
    void reply(const serializable & body = parameters(),
        std::size_t priority = 0);

    /// \brief Sends back the rejection (exception) notification.
    ///
    /// Sends back the rejection to the message identified by this object.
    /// The rejection (or reply) can be sent only once.
    ///
    /// @param reason Arbitrary text that will be visible by the message
    ///               sender as a reason for rejection.
    /// @param priority The priority of the rejection.
    void reject(const std::string & reason = std::string(),
        std::size_t priority = 0);

private:
    incoming_message(const incoming_message &);
    void operator=(const incoming_message &);

    details::agent_impl & agent_;
    std::string source_;
    std::string object_name_;
    std::string message_name_;
    long long message_id_;
    parameters * params_;
    std::vector<char> * raw_buffer_;
    bool already_used_;
};

} // namespace yami

#endif // YAMICPP_INCOMING_MESSAGE_H_INCLUDED
