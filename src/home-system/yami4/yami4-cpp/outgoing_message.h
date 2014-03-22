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

#ifndef YAMICPP_OUTGOING_MESSAGE_H_INCLUDED
#define YAMICPP_OUTGOING_MESSAGE_H_INCLUDED

#include "message_state.h"
#include <yami4-core/dll.h>
#include <cstddef>
#include <string>
#include <vector>

namespace yami
{

namespace details
{
struct outgoing_message_info;
class outgoing_message_manager;
} // namespace details

class parameters;

/// \brief Outgoing message.
///
/// The handler allowing to track the progress of outgoing message,
/// inspect its state and to obtain the reply content.
///
/// <b>Note:</b>
/// The objects of this class can be safely used from multiple threads.
class DLL outgoing_message
{
public:

    /// \brief Constructor.
    ///
    /// Creates uninitialized outgoing message object.
    outgoing_message();

    ~outgoing_message();

    void reset(details::outgoing_message_manager & manager,
        details::outgoing_message_info & info);
    
    void disown_info_object();

    /// \brief Returns the state of this message.
    message_state get_state() const;

    /// \brief Returns the state of this message.
    ///
    /// This function allows to inspect the progress of the message
    /// transmission. During transmission the <code>sent_bytes</code> value
    /// is always smaller than <code>total_byte_count</code>.
    /// When these two values become equal, it means that the transmission
    /// was either succesful or abandoned.
    ///
    /// @param sent_bytes The number of bytes that were already sent
    ///                   for this message.
    /// @param total_byte_count The total number of bytes that should be sent
    ///                   for this message.
    message_state get_state(std::size_t & sent_bytes,
        std::size_t & total_byte_count) const;

    /// \brief Waits for the transmission to finish.
    ///
    /// Waits for the transmission to finish - that is, to either send all
    /// the message data or to abandon it.
    /// After this function returns the state of the message is either
    /// <code>transmitted</code>, <code>abandoned</code>,
    /// <code>replied</code> or <code>rejected</code>.
    void wait_for_transmission() const;

    /// \brief Waits for the transmission to finish or until timeout expires.
    ///
    /// Waits for the transmission to finish or until the given relative
    /// timeout expires.
    ///
    /// @param relative_timeout The relative timeout in milliseconds.
    /// @return
    ///         - <code>true</code> if the wait completed before the timeout
    ///         - <code>false</code> if the timeout expired before
    ///           the message was fully transmitted
    bool wait_for_transmission(std::size_t relative_timeout) const;

    /// \brief Waits for the transmission to finish or until timeout expires.
    ///
    /// Waits for the transmission to finish or until the given absolute
    /// timeout expires.
    ///
    /// @param timeout The absolute timeout expressed in milliseconds
    ///                from the beginning of the epoch (UTC).
    /// @return
    ///         - <code>true</code> if the wait completed before the timeout
    ///         - <code>false</code> if the timeout expired before
    ///           the message was fully transmitted
    bool wait_for_transmission_absolute(unsigned long long timeout) const;

    /// \brief Waits for the full message roundtrip.
    ///
    /// Waits for the full message roundtrip - that is, for some confirmation
    /// that the message has been received and reacted upon by the
    /// target agent.
    /// After this function returns the state of the message is either
    /// <code>abandoned</code>, <code>replied</code> or <code>rejected</code>.
    ///
    /// <b>Note:</b>
    /// This function should not be called if the intended semantics of the
    /// message is "one-way" - in this case this function would block
    /// indefinitely.
    void wait_for_completion() const;

    /// \brief Waits for the message roundtrip or until timeout expires.
    ///
    /// Waits for the full message roundtrip or until the given relative
    /// timeout expires.
    ///
    /// @param relative_timeout The relative timeout in milliseconds.
    /// @return
    ///         - <code>true</code> if the wait completed before the timeout
    ///         - <code>false</code> if the timeout expired before
    ///           the message was completed
    bool wait_for_completion(std::size_t relative_timeout) const;

    /// \brief Waits for the message roundtrip or until timeout expires.
    ///
    /// Waits for the full message roundtrip or until the given absolute
    /// timeout expires.
    ///
    /// @param timeout The absolute timeout expressed in milliseconds
    ///                from the beginning of the epoch (UTC).
    /// @return
    ///         - <code>true</code> if the wait completed before the timeout
    ///         - <code>false</code> if the timeout expired before
    ///           the message was completed
    bool wait_for_completion_absolute(unsigned long long timeout) const;

    /// \brief Provides access to the reply content.
    const parameters & get_reply() const;

    /// \brief Provides access to the reply content.
    ///
    /// Note: this function can be used only when the data
    /// is delivered in the raw buffer form.
    const std::vector<char> & get_raw_reply() const;

    /// \brief Extracts the reply content.
    ///
    /// The reply content is moved to the caller and the caller
    /// becomes the owner of the <code>parameters</code> object.
    /// This object itself is no longer holding any references to
    /// the content and therefore any future attempts to access it
    /// will result in the exception.
    parameters * extract_reply();

    /// \brief Returns the exception message.
    ///
    /// This function can be called when the state of message is "rejected".
    const std::string & get_exception_msg() const;

private:
    outgoing_message(const outgoing_message &);
    void operator=(const outgoing_message &);

    void clean();

    details::outgoing_message_manager * manager_;
    details::outgoing_message_info * info_;
    bool owner_of_info_;
};

} // namespace yami

#endif // YAMICPP_OUTGOING_MESSAGE_H_INCLUDED
