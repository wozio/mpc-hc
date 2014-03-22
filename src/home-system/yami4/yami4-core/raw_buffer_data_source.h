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

#ifndef YAMICORE_RAW_BUFFER_DATA_SOURCE_H_INCLUDED
#define YAMICORE_RAW_BUFFER_DATA_SOURCE_H_INCLUDED

#include "core.h"
#include "dll.h"
#include "serializable.h"

namespace yami
{

namespace core
{

/// \brief Serializable wrapper for the raw binary data.
///
/// Serializable wrapper for the raw binary data allows to use
/// already serialized content for message sending.
/// The two major use cases for this class are efficient message forwarding
/// (when the message is received and its content is used for another message)
/// and support for custom data models and serializers.
class DLL raw_buffer_data_source : public serializable
{
public:

    /// \brief Constructor.
    ///
    /// Constructs the buffer wrapper for the given set of buffers.
    /// The data buffer does not have to be contiguous and any number
    /// of buffer segments is allowed, provided that the size of each buffer
    /// segment is a multiple of 4 (32 bits).<br />
    /// The wrapper gathers the binary data from subsequent buffers
    /// as they are serialized - serialization of the wrapper copies the data
    /// from source set of buffers to the target set, which is possibly
    /// structured in a different way.<br />
    /// The buffers are provided as array of buffer pointers and their sizes.
    /// @param buffers Pointer to the array of buffer pointers
    ///        (each of type <code>const char *</code>).
    /// @param buffer_sizes Pointer to the array of buffer sizes.
    /// @param num_of_buffers Number of buffers described by the array.
    raw_buffer_data_source(const char * * buffers,
        const std::size_t * buffer_sizes,
        std::size_t num_of_buffers);

    /// \brief Constructor.
    ///
    /// Constructs the buffer wrapper for the single contiguous source buffer.
    /// @param buffer Pointer to the source buffer.
    /// @param buffer_size Source buffer size.
    raw_buffer_data_source(const char * buffer, std::size_t buffer_size);

    /// \brief Finds the total size of serialization buffer.
    ///
    /// Computes the total size of serialization buffer(s) for the current
    /// content of this object.
    /// @param size The computed size of buffer.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>nesting_too_deep</code> if the level of nesting
    ///           in this object is deeper than the limit
    virtual result get_serialize_buffer_size(std::size_t & size) const;

    /// \brief Serializes current content into given buffer(s).
    ///
    /// Serializes the current content of this object into the given
    /// buffer(s).
    /// The serialization buffer does not have to be contiguous and any number
    /// of buffer segments is allowed, provided that the size of each buffer
    /// segment is a multiple of 4 (32 bits).<br />
    /// The function scatters the serialized data into subsequent buffers
    /// as they become filled.<br />
    /// The buffers are provided as array of buffer pointers and their sizes.
    /// @param target_buffers Pointer to the array of target buffer pointers
    ///        (each of type <code>char *</code>).
    /// @param target_buffer_sizes Pointer to the array of target buffer
    ///        sizes.
    /// @param num_of_target_buffers Number of target buffers
    ///        described by the array.
    /// @return
    ///         - <code>ok</code> if operation was successful
    ///         - <code>not_enough_space</code> if the buffers are not
    ///           big enough for all the data
    virtual result serialize(char * * target_buffers,
        const std::size_t * target_buffer_sizes,
        std::size_t num_of_target_buffers) const;

private:

    raw_buffer_data_source(const raw_buffer_data_source &);
    void operator=(const raw_buffer_data_source &);

    const char * * buffers_;
    const std::size_t * buffer_sizes_;
    std::size_t num_of_buffers_;

    // helpers for single buffer case
    const char * single_buffer[1];
    std::size_t single_buffer_size[1];
};

} // namespace core

} // namespace yami

#endif // YAMICORE_RAW_BUFFER_DATA_SOURCE_H_INCLUDED
