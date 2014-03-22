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

#include "activity_statistics_monitor.h"
#include "incoming_message.h"
#include "mutex_lock.h"
#include "parameters.h"

#include <ctime>
#include <map>
#include <set>
#include <vector>

namespace yami
{
namespace details
{

enum connection_type { incoming, outgoing };

struct connection_stats
{
    connection_stats(connection_type t = incoming)
        : type(t), messages_sent(0), messages_received(0),
          bytes_sent(0), bytes_received(0)
    {
    }

    void reset()
    {
        messages_sent = 0;
        messages_received = 0;
        bytes_sent = 0;
        bytes_received = 0;
    }

    connection_type type;
    std::size_t messages_sent;
    std::size_t messages_received;
    std::size_t bytes_sent;
    std::size_t bytes_received;
};

class activity_statistics_monitor_impl
{
public:

    activity_statistics_monitor_impl()
    {
        mtx_.init();
    }

    ~activity_statistics_monitor_impl()
    {
        mtx_.clean();
    }

    void agent_created()
    {
        started_ = std::time(NULL);
    }

    void agent_closed()
    {
    }

    void listener_added(const char * target)
    {
        mutex_lock lck(mtx_);

        listeners_.insert(target);
    }

    void listener_removed(const char * target)
    {
        mutex_lock lck(mtx_);

        listeners_.erase(target);
    }

    void incoming_connection_open(const char * target)
    {
        mutex_lock lck(mtx_);

        connections_[target] = connection_stats(incoming);
    }
        
    void outgoing_connection_open(const char * target)
    {
        mutex_lock lck(mtx_);

        connections_[target] = connection_stats(outgoing);
    }

    void connection_closed(const char * target)
    {
        mutex_lock lck(mtx_);

        connections_.erase(target);
    }

    void connection_error(const char * target)
    {
        mutex_lock lck(mtx_);

        ++(connection_errors_[target]);
    }

    void object_registered(const char * name)
    {
        mutex_lock lck(mtx_);

        objects_.insert(name);
    }

    void object_unregistered(const char * name)
    {
        mutex_lock lck(mtx_);

        objects_.erase(name);
    }

    void message_sent(const char * target, std::size_t size)
    {
        mutex_lock lck(mtx_);

        connection_stats & stats = connections_[target];

        ++stats.messages_sent;
        stats.bytes_sent += size;
    }

    void message_received(const char * target, std::size_t size)
    {
        mutex_lock lck(mtx_);

        connection_stats & stats = connections_[target];

        ++stats.messages_received;
        stats.bytes_received += size;
    }

    void get(parameters & params, bool reset_counters)
    {
        mutex_lock lck(mtx_);

        // uptime information

        double uptime = std::difftime(std::time(NULL), started_);
        params.set_integer("uptime", static_cast<int>(uptime));

        // list of active listeners

        const char * listeners_field = "listeners";
        params.create_string_array(listeners_field, listeners_.size());
        std::set<std::string>::iterator lst_it = listeners_.begin();
        std::set<std::string>::iterator lst_end = listeners_.end();
        std::size_t index = 0;
        for ( ; lst_it != lst_end; ++lst_it, ++index)
        {
            params.set_string_in_array(listeners_field, index, *lst_it);
        }

        // list of registered objects

        const char * objects_field = "objects";
        params.create_string_array(objects_field, objects_.size());
        std::set<std::string>::iterator obj_it = objects_.begin();
        std::set<std::string>::iterator obj_end = objects_.end();
        index = 0;
        for ( ; obj_it != obj_end; ++obj_it, ++index)
        {
            params.set_string_in_array(objects_field, index, *obj_it);
        }

        // connection statistics

        std::vector<long long> conn_messages_sent(connections_.size());
        std::vector<long long> conn_messages_received(connections_.size());
        std::vector<long long> conn_bytes_sent(connections_.size());
        std::vector<long long> conn_bytes_received(connections_.size());

        const char * conn_names_field = "connection_names";
        params.create_string_array(conn_names_field, connections_.size());
        connection_map::iterator conn_it = connections_.begin();
        connection_map::iterator conn_end = connections_.end();
        index = 0;
        for ( ; conn_it != conn_end; ++conn_it, ++index)
        {
            params.set_string_in_array(
                conn_names_field, index, conn_it->first);

            conn_messages_sent[index] = conn_it->second.messages_sent;
            conn_messages_received[index] = conn_it->second.messages_received;
            conn_bytes_sent[index] = conn_it->second.bytes_sent;
            conn_bytes_received[index] = conn_it->second.bytes_received;
        }

        params.set_long_long_array("messages_sent",
            &conn_messages_sent[0], conn_messages_sent.size());
        params.set_long_long_array("messages_received",
            &conn_messages_received[0], conn_messages_received.size());
        params.set_long_long_array("bytes_sent",
            &conn_bytes_sent[0], conn_bytes_sent.size());
        params.set_long_long_array("bytes_received",
            &conn_bytes_received[0], conn_bytes_received.size());

        // connection errors

        std::vector<long long> connection_errors(connection_errors_.size());

        const char * conn_error_names_field = "error_names";
        params.create_string_array(conn_error_names_field,
            connection_errors_.size());
        connection_error_map::iterator err_it = connection_errors_.begin();
        connection_error_map::iterator err_end = connection_errors_.end();
        index = 0;
        for ( ; err_it != err_end; ++err_it, ++index)
        {
            params.set_string_in_array(
                conn_error_names_field, index, err_it->first);

            connection_errors[index] = err_it->second;
        }

        params.set_long_long_array("errors",
            &connection_errors[0], connection_errors.size());

        // atomic reset, if requested

        if (reset_counters)
        {
            connection_errors_.clear();

            connection_map::iterator it = connections_.begin();
            connection_map::iterator end = connections_.end();
            for ( ; it != end; ++it)
            {
                it->second.reset();
            }
        }
    }

private:

    typedef std::map<std::string, connection_stats> connection_map;
    typedef std::map<std::string, std::size_t> connection_error_map;

    details::mutex mtx_;

    std::time_t started_;
    
    std::set<std::string> listeners_;
    std::set<std::string> objects_;

    connection_map connections_;

    // counter of errors per target
    // this structure can contain targets
    // that are not active in the main structure
    connection_error_map connection_errors_;
};

} // namespace details

} // namespace yami

using namespace yami;

activity_statistics_monitor::activity_statistics_monitor()
{
    pimpl_ = new details::activity_statistics_monitor_impl();
}

activity_statistics_monitor::~activity_statistics_monitor()
{
    delete pimpl_;
}

void activity_statistics_monitor::get(
    parameters & params, bool reset_counters)
{
    pimpl_->get(params, reset_counters);
}

void activity_statistics_monitor::operator()(incoming_message & msg)
{
    if (msg.get_message_name() != "get")
    {
        msg.reject("Unknown message name.");
        return;
    }

    const parameters & msg_params = msg.get_parameters();

    bool reset_counters = false;
    parameter_entry e;
    const char * reset_field = "reset";
    if (msg_params.find(reset_field, e) && e.type() == boolean)
    {
        reset_counters = msg_params.get_boolean(reset_field);
    }

    parameters reply_params;
    get(reply_params, reset_counters);

    msg.reply(reply_params);
}

void activity_statistics_monitor::agent_created()
{
    pimpl_->agent_created();
}

void activity_statistics_monitor::agent_closed()
{
    pimpl_->agent_closed();
}

void activity_statistics_monitor::listener_added(const char * target)
{
    pimpl_->listener_added(target);
}

void activity_statistics_monitor::listener_removed(const char * target)
{
    pimpl_->listener_removed(target);
}

void activity_statistics_monitor::incoming_connection_open(
    const char * target)
{
    pimpl_->incoming_connection_open(target);
}

void activity_statistics_monitor::outgoing_connection_open(
    const char * target)
{
    pimpl_->outgoing_connection_open(target);
}

void activity_statistics_monitor::connection_closed(const char * target)
{
    pimpl_->connection_closed(target);
}

void activity_statistics_monitor::connection_error(const char * target)
{
    pimpl_->connection_error(target);
}

void activity_statistics_monitor::object_registered(const char * name)
{
    pimpl_->object_registered(name);
}

void activity_statistics_monitor::object_unregistered(const char * name)
{
    pimpl_->object_unregistered(name);
}

void activity_statistics_monitor::message_sent(
    const char * target, std::size_t size)
{
    pimpl_->message_sent(target, size);
}

void activity_statistics_monitor::message_received(
    const char * target, std::size_t size)
{
    pimpl_->message_received(target, size);
}
