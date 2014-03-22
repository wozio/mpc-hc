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

#include "allocator.h"
#include "fatal_errors.h"
#include <cstdlib>

using namespace yami;
using namespace details;

namespace // unnamed
{

struct segment_header
{
    segment_header * next_;
    segment_header * prev_;

    bool free_;

    // used only when free_ == true
    segment_header * next_free_;
    segment_header * prev_free_;

    union alignment
    {
        double dummy_;
        char data_buffer_;
    } aligned;
};

// the smallest segment that can be be split off (including header)
const std::size_t smallest_split_segment_size = 64;

// the round-up amount for alignment of split segments
const std::size_t round_up_amount = sizeof(double);

std::size_t buffer_size(
    segment_header * segment, void * base, std::size_t total_size)
{
    std::size_t result;
    if (segment->next_ != NULL)
    {
        // this is not the last segment

        result = reinterpret_cast<char *>(segment->next_) -
            &segment->aligned.data_buffer_;
    }
    else
    {
        // this is the last segment

        result = total_size -
            (&segment->aligned.data_buffer_ - reinterpret_cast<char *>(base));
    }

    return result;
}

} // namespace unnamed

allocator::allocator()
    : base_(NULL), size_(0), first_free_segment_(NULL)
{
}

void allocator::set_working_area(void * buf, std::size_t size)
{
    if (base_ != NULL)
    {
        fatal_failure(__FILE__, __LINE__);
    }

    if (buf != NULL && size != 0)
    {
        base_ = buf;
        size_ = size;

        segment_header * initial_segment =
            reinterpret_cast<segment_header *>(base_);
        initial_segment->next_ = NULL;
        initial_segment->prev_ = NULL;
        initial_segment->free_ = true;

        initial_segment->next_free_ = NULL;
        initial_segment->prev_free_ = NULL;

        first_free_segment_ = initial_segment;
    }
}

void * allocator::allocate(std::size_t requested_size)
{
    void * result = NULL;

    if (base_ == NULL)
    {
        result = std::malloc(requested_size);
    }
    else
    {
        // iterate over the list of free segments
        // and find the first that is big enough

        requested_size += round_up_amount - 1;
        requested_size /= round_up_amount;
        requested_size *= round_up_amount;

        segment_header * segment =
            reinterpret_cast<segment_header *>(first_free_segment_);
        while (segment != NULL && result == NULL)
        {
            const std::size_t buf_size = buffer_size(segment, base_, size_);
            if (buf_size >= requested_size)
            {
                std::size_t remaining_space = buf_size - requested_size;

                if (remaining_space >= smallest_split_segment_size)
                {
                    // the remaining space is enough
                    // to contain another segment
                    // -> split segments

                    segment_header * new_segment =
                        reinterpret_cast<segment_header *>(
                            &segment->aligned.data_buffer_
                            + buf_size - remaining_space);

                    new_segment->next_ = segment->next_;
                    new_segment->prev_ = segment;
                    new_segment->free_ = true;
                    segment->next_ = new_segment;
                    if (new_segment->next_ != NULL)
                    {
                        new_segment->next_->prev_ = new_segment;
                    }

                    new_segment->next_free_ = segment->next_free_;
                    new_segment->prev_free_ = segment;
                    segment->next_free_ = new_segment;
                }

                result = &segment->aligned.data_buffer_;
            }
            else
            {
                segment = segment->next_free_;
            }
        }

        if (result != NULL)
        {
            // appropriate free segment was found
            // -> mark it as not free and adjust free list links

            segment->free_ = false;

            if (segment->prev_free_ != NULL)
            {
                segment->prev_free_->next_free_ = segment->next_free_;
            }
            else
            {
                // this was the first free segment
                first_free_segment_ = segment->next_free_;
            }

            if (segment->next_free_ != NULL)
            {
                segment->next_free_->prev_free_ = segment->prev_free_;
            }
        }
    }

    return result;
}

void allocator::deallocate(const void * p)
{
    if (base_ == NULL)
    {
        std::free(const_cast<void *>(p));
    }
    else
    {
        segment_header * segment =
            reinterpret_cast<segment_header *>(
                static_cast<char *>(const_cast<void *>(p)) -
                offsetof(segment_header, aligned));
        segment->free_ = true;

        // reestablish free list links

        if (segment == first_free_segment_)
        {
            fatal_failure(__FILE__, __LINE__);
        }

        if (first_free_segment_ == NULL ||
            segment < static_cast<segment_header *>(first_free_segment_))
        {
            // this segment should be the new first known free segment

            segment->prev_free_ = NULL;
            segment->next_free_ =
                static_cast<segment_header *>(first_free_segment_);
            if (first_free_segment_ != NULL)
            {
                static_cast<segment_header *>(
                    first_free_segment_)->prev_free_ = segment;
            }

            first_free_segment_ = segment;
        }
        else
        {
            // this segment is further than the first known free segment
            // -> find the closest earlier free segment

            segment_header * iter = segment->prev_;
            while (iter->free_ == false)
            {
                iter = iter->prev_;
            }

            segment->prev_free_ = iter;
            iter->next_free_ = segment;

            // -> find the closest further free segment

            iter = segment->next_;
            while (iter != NULL && iter->free_ == false)
            {
                iter = iter->next_;
            }

            segment->next_free_ = iter;
            if (iter != NULL)
            {
                iter->prev_free_ = segment;
            }
        }

        if (segment->next_ != NULL)
        {
            // this is not the last segment
            // -> attempt to join with the next one, if it is free

            segment_header * next_segment = segment->next_;
            if (next_segment->free_)
            {
                segment->next_ = next_segment->next_;
                if (next_segment->next_ != NULL)
                {
                    next_segment->next_->prev_ = segment;
                }

                segment->next_free_ = next_segment->next_free_;
                if (next_segment->next_free_ != NULL)
                {
                    next_segment->next_free_->prev_free_ = segment;
                }
            }
        }

        if (segment->prev_ != NULL)
        {
            // this is not the first segment
            // -> attempt to join with the previous one, if it is free

            segment_header * previous_segment = segment->prev_;
            if (previous_segment->free_)
            {
                previous_segment->next_ = segment->next_;
                if (segment->next_ != NULL)
                {
                    segment->next_->prev_ = previous_segment;
                }

                previous_segment->next_free_ = segment->next_free_;
                if (segment->next_free_ != NULL)
                {
                    segment->next_free_->prev_free_ = previous_segment;
                }
            }
        }
    }
}

void allocator::get_free_size(std::size_t & biggest, std::size_t & all) const
{
    biggest = 0;
    all = 0;

    if (base_ != NULL)
    {
        segment_header * segment =
            reinterpret_cast<segment_header *>(first_free_segment_);
        while (segment != NULL)
        {
            const std::size_t buf_size = buffer_size(segment, base_, size_);
            if (buf_size > biggest)
            {
                biggest = buf_size;
            }

            all += buf_size;

            segment = segment->next_free_;
        }
    }
}
