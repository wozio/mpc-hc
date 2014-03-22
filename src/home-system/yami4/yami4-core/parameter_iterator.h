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

#ifndef YAMICORE_PARAMETER_ITERATOR_H_INCLUDED
#define YAMICORE_PARAMETER_ITERATOR_H_INCLUDED

#include "core.h"
#include "dll.h"
#include "parameter_entry.h"

namespace yami
{

namespace details
{
class allocator;
struct entry;
} // namespace details

namespace core
{

class parameters;

/// \brief Iterator to parameter entries.
///
/// The iterator to all parameter entries.
/// \sa parameters
///
/// <b>Note:</b> The iterator object can be used only as long as it is valid
/// and every <i>modifying</i> operation on the parameters object should be
/// assumed as invalidating all existing iterators.
class DLL parameter_iterator
{
public:
    /// \brief Checks if there is a subsequent entry.
    ///
    /// Checks if there is a subsequent entry in the associated parameters
    /// object.
    /// @return true if there is some further entry, false otherwise.
    bool has_next() const;

    /// \brief Moves the iterator to the next entry.
    ///
    /// Moves the iterator to the next non-empty entry in the associated
    /// parameters.<br />
    /// This function can be called only when some next entry exists.
    void move_next();

    /// \brief Gets the current entry.
    ///
    /// Gets the current entry.
    parameter_entry current() const;

    /// \brief Removes the current entry.
    ///
    /// Removes the current entry.
    ///
    /// <b>Note:</b> The iterator is not automatically moved
    /// to the next entry after the current one is removed.
    /// The only operations that are allowed after that are
    /// has_next and move_next.
    void remove();

private:
    friend class parameters;

    details::entry * data_;
    std::size_t current_index_;
    std::size_t num_of_entries_;
    details::allocator * allocator_;
};

} // namespace core

} // namespace yami

#endif // YAMICORE_PARAMETER_ITERATOR_H_INCLUDED
