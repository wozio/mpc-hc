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

#include "options.h"
#include "option_names.h"
#include "parameters.h"

using namespace yami;
using namespace details;

namespace // unnamed
{

void override_if_defined(const parameters & params, const char * name,
    std::size_t & value)
{
    parameter_entry entry;
    const bool found = params.find(name, entry);
    if (found)
    {
        const parameter_type type = entry.type();
        if (type == integer)
        {
            int tmp = entry.get_integer();
            if (tmp >= 0)
            {
                value = static_cast<std::size_t>(tmp);
            }
        }
    }
}

void override_if_defined(const parameters & params, const char * name,
    bool & value)
{
    parameter_entry entry;
    const bool found = params.find(name, entry);
    if (found)
    {
        const parameter_type type = entry.type();
        if (type == boolean)
        {
            value = entry.get_boolean();
        }
    }
}

} // unnamed namespace

void cpp_options::init(const parameters & params)
{
    // set default values

    // there is just one dispatcher thread
    dispatcher_threads = 1;

    // try to connect 5 times if not successful
    connection_retries = 5;

    // before retrying the connection wait random time
    // between 0 and this number of milliseconds
    connection_retry_delay_spread = 100;

    // there can be at most 100 messages waiting in the
    // outgoing queues and if this limit is reached, the queue blocks
    // and allows more messages only after the backlog is reduced to 20
    outgoing_high_water_mark = 100;
    outgoing_low_water_mark = 20;

    // there can be at most 100 messages waiting for dispatch
    // in the incoming queue and if this limit is reached, the
    // incoming traffic is suppressed until the queue is reduced to 20
    incoming_high_water_mark = 100;
    incoming_low_water_mark = 20;

    // by default the message content is delivered
    // as already deserialized parameters object
    deliver_as_raw_binary = false;

    // extract user-provided options and override default settings

    override_if_defined(params,
        option_names::dispatcher_threads, dispatcher_threads);

    override_if_defined(params,
        option_names::connection_retries, connection_retries);

    override_if_defined(params,
        option_names::connection_retry_delay_spread,
        connection_retry_delay_spread);

    override_if_defined(params,
        option_names::outgoing_high_water_mark, outgoing_high_water_mark);

    override_if_defined(params,
        option_names::outgoing_low_water_mark, outgoing_low_water_mark);

    if (outgoing_low_water_mark > outgoing_high_water_mark)
    {
        outgoing_low_water_mark = outgoing_high_water_mark;
    }

    override_if_defined(params,
        option_names::incoming_high_water_mark, incoming_high_water_mark);

    override_if_defined(params,
        option_names::incoming_low_water_mark, incoming_low_water_mark);

    if (incoming_low_water_mark > incoming_high_water_mark)
    {
        incoming_low_water_mark = incoming_high_water_mark;
    }

    override_if_defined(params,
        option_names::deliver_as_raw_binary, deliver_as_raw_binary);
}
