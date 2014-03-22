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

#ifndef YAMICORE_CHANNEL_DESCRIPTOR_H_INCLUDED
#define YAMICORE_CHANNEL_DESCRIPTOR_H_INCLUDED

#include "dll.h"

#include <cstddef>

namespace yami
{

namespace core
{

/// \brief Descriptor handle for the physical channel.
///
/// The descriptor handle for the physical channel that provides
/// immediate access to the underlying channel object.
///
/// The descriptor provides safe access in the sense that
/// dangling descriptors are recognized and reported.
///
class DLL channel_descriptor
{
public:
    /// \brief Constructor.
    ///
    /// Creates a default descriptor that is not associated with
    /// any existing channel.
    ///
    /// Two default descriptors are considered equal.
    channel_descriptor()
        : index_(0), sequence_number_(0)
    {
    }

    // used internally to create the descriptor that is then
    // returned to the user code
    channel_descriptor(std::size_t index, std::size_t sequence_number)
        : index_(index), sequence_number_(sequence_number)
    {
    }

    // used internally
    void get_details(std::size_t & index, std::size_t & sequence_number) const
    {
        index = index_;
        sequence_number = sequence_number_;
    }

    /// \brief Comparison operator.
    ///
    /// Compares two descriptors for equality.
    /// @return
    ///         - <code>true</code> if the other descriptor points
    ///           to the same physical channel
    ///         - <code>false</code> otherwise
    bool operator==(const channel_descriptor & other) const
    {
        return index_ == other.index_ &&
            sequence_number_ == other.sequence_number_;
    }

    /// \brief Comparison operator.
    ///
    /// Compares two descriptors.
    /// @return Reverse to operator==
    bool operator!=(const channel_descriptor & other) const
    {
        return (*this == other) == false;
    }

private:
    std::size_t index_;
    std::size_t sequence_number_;
};

} // namespace core

} // namespace yami

#endif // YAMICORE_CHANNEL_DESCRIPTOR_H_INCLUDED
