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

#include "../listener.h"
#include "../allocator.h"
#include "../channel_group.h"
#include "../fatal_errors.h"
#include "../io_error_handler.h"
#include "../network_utils.h"
#include "../options.h"
#include <stdio.h>
#include <cstdlib>
#include <cstring>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace yami;
using namespace details;

namespace // unnamed
{
// artificial counter for disambiguating client Unix names
// so that Unix client connections have different channel names
long long unix_client_id = 0;
}

core::result listener::prepare_tcp(const char * address)
{
    protocol_ = proto_tcp;

    ip_address ipa;
    core::result res = parse_address(*alloc_, address, ipa,
        io_error_callback_, io_error_callback_hint_);

    if (res == core::ok)
    {
        const int SOCKET_ERROR = -1;

        io_descriptor_type fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd != SOCKET_ERROR)
        {
            res = core::ok;
            if (group_->get_options().tcp_reuseaddr)
            {
                res = set_reuseaddr(fd);
            }
        }
        else
        {
            handle_io_error("create tcp listener socket",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }

        if (res == core::ok)
        {
            sockaddr_in local;

            std::memset(&local, 0, sizeof(local));

            local.sin_family = AF_INET;
            local.sin_port = ipa.port;

            if (ipa.host != 0)
            {
                local.sin_addr.s_addr = ipa.host;
            }
            else
            {
                local.sin_addr.s_addr = htonl(INADDR_ANY);
            }

            int cc = ::bind(fd,
                reinterpret_cast<sockaddr *>(&local), sizeof(local));
            if (cc != SOCKET_ERROR)
            {
                const int backlog =
                    group_->get_options().tcp_listen_backlog;

                cc = ::listen(fd, backlog);
                if (cc != SOCKET_ERROR)
                {
                    fd_ = fd;
                }
                else
                {
                    handle_io_error("tcp listen",
                        io_error_callback_, io_error_callback_hint_);

                    res = core::io_error;
                }

                if (res == core::ok)
                {
                    // recreate the target based on (possibly)
                    // system-assigned values

                    // max length of the string representing the target
                    // (arbitrary, but should allow for prefix,
                    // address and port)
                    // note that address alone can be 256 chars
                    const std::size_t max_target_length = 270;

                    char * buf = static_cast<char *>(
                        alloc_->allocate(max_target_length));

                    if (buf != NULL)
                    {
                        socklen_t dummy = sizeof(local);
                        cc = ::getsockname(fd,
                            reinterpret_cast<sockaddr*>(&local), &dummy);
                        if (cc != SOCKET_ERROR)
                        {
                            const int assigned_port = ntohs(local.sin_port);

                            // assigned_addr is in network byte order
                            const int assigned_addr = local.sin_addr.s_addr;

                            if (assigned_addr == 0)
                            {
                                // if assigned address is local ANY,
                                // recreate it from gethostname

                                char this_host[max_target_length];
                                cc = ::gethostname(
                                    this_host, max_target_length);
                                if (cc == 0)
                                {
                                    ::snprintf(buf, max_target_length,
                                        "tcp://%s:%d",
                                        this_host, assigned_port);
                                }
                                else
                                {
                                    handle_io_error(
                                        "tcp listener gethostname",
                                        io_error_callback_,
                                        io_error_callback_hint_);

                                    res = core::io_error;
                                }
                            }
                            else
                            {
                                // the address was provided by user
                                // and resolved to number in the parse phase

                                const unsigned char * addr_bytes =
                                    reinterpret_cast<const unsigned char *>(
                                        &assigned_addr);

                                // note: address is in network byte order
                                ::snprintf(buf, max_target_length,
                                    "tcp://%d.%d.%d.%d:%d",
                                    static_cast<int>(addr_bytes[0]),
                                    static_cast<int>(addr_bytes[1]),
                                    static_cast<int>(addr_bytes[2]),
                                    static_cast<int>(addr_bytes[3]),
                                    assigned_port);
                            }

                            target_ = buf;
                        }
                        else
                        {
                            alloc_->deallocate(buf);

                            handle_io_error("tcp listener getsockname",
                                io_error_callback_, io_error_callback_hint_);

                            res = core::io_error;
                        }
                    }
                    else
                    {
                        res = core::no_memory;
                    }
                }
            }
            else
            {
                handle_io_error("tcp listener bind",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }
        }

        if (res != core::ok)
        {
            if (fd != SOCKET_ERROR)
            {
                ::close(fd);
            }

            if (target_ != NULL)
            {
                alloc_->deallocate(target_);
            }
        }
    }

    return res;
}

core::result listener::prepare_udp(const char * address)
{
    protocol_ = proto_udp;

    ip_address ipa;
    core::result res = parse_address(*alloc_, address, ipa,
        io_error_callback_, io_error_callback_hint_);

    if (res == core::ok)
    {
        const int SOCKET_ERROR = -1;

        io_descriptor_type fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (fd != SOCKET_ERROR)
        {
            sockaddr_in local;

            std::memset(&local, 0, sizeof(local));

            local.sin_family = AF_INET;
            local.sin_port = ipa.port;

            if (ipa.host != 0)
            {
                local.sin_addr.s_addr = ipa.host;
            }
            else
            {
                local.sin_addr.s_addr = htonl(INADDR_ANY);
            }

            int cc = ::bind(fd,
                reinterpret_cast<sockaddr *>(&local), sizeof(local));
            if (cc != SOCKET_ERROR)
            {
                fd_ = fd;

                // recreate the target based on (possibly)
                // system-assigned values

                // max length of the string representing the target
                // (arbitrary, but should allow for prefix,
                // address and port)
                // note that address alone can be 256 chars
                const std::size_t max_target_length = 270;

                char * buf = static_cast<char *>(
                    alloc_->allocate(max_target_length));

                if (buf != NULL)
                {
                    socklen_t dummy = sizeof(local);
                    cc = ::getsockname(fd,
                        reinterpret_cast<sockaddr*>(&local), &dummy);
                    if (cc != SOCKET_ERROR)
                    {
                        const int assigned_port = ntohs(local.sin_port);

                        // assigned_addr is in network byte order
                        const int assigned_addr = local.sin_addr.s_addr;

                        if (assigned_addr == 0)
                        {
                            // if assigned address is local ANY,
                            // recreate it from gethostname

                            char this_host[max_target_length];
                            cc = ::gethostname(
                                this_host, max_target_length);
                            if (cc == 0)
                            {
                                ::snprintf(buf, max_target_length,
                                    "udp://%s:%d",
                                    this_host, assigned_port);
                            }
                            else
                            {
                                handle_io_error(
                                    "udp listener gethostname",
                                    io_error_callback_,
                                    io_error_callback_hint_);

                                res = core::io_error;
                            }
                        }
                        else
                        {
                            // the address was provided by user
                            // and resolved to number in the parse phase

                            const unsigned char * addr_bytes =
                                reinterpret_cast<const unsigned char *>(
                                    &assigned_addr);

                            // note: address is in network byte order
                            ::snprintf(buf, max_target_length,
                                "udp://%d.%d.%d.%d:%d",
                                static_cast<int>(addr_bytes[0]),
                                static_cast<int>(addr_bytes[1]),
                                static_cast<int>(addr_bytes[2]),
                                static_cast<int>(addr_bytes[3]),
                                assigned_port);
                        }

                        target_ = buf;
                    }
                    else
                    {
                        alloc_->deallocate(buf);

                        handle_io_error("udp listener getsockname",
                            io_error_callback_, io_error_callback_hint_);

                        res = core::io_error;
                    }
                }
                else
                {
                    res = core::no_memory;
                }
            }
            else
            {
                handle_io_error("udp listener bind",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }
        }
        else
        {
            handle_io_error("create udp listener socket",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }

        if (res == core::ok)
        {
            // allocate udp frame buffer of appropriate size

            frame_buffer_size_ = group_->get_options().udp_frame_size;

            frame_buffer_ = static_cast<char *>(
                alloc_->allocate(frame_buffer_size_));
            if (frame_buffer_ == NULL)
            {
                res = core::no_memory;
            }
        }

        if (res != core::ok)
        {
            if (fd != SOCKET_ERROR)
            {
                ::close(fd);
            }

            if (target_ != NULL)
            {
                alloc_->deallocate(target_);
            }

            if (frame_buffer_ != NULL)
            {
                alloc_->deallocate(frame_buffer_);
            }
        }
    }

    return res;
}

core::result listener::prepare_unix(const char * path)
{
    core::result res;

    protocol_ = proto_unix;

    sockaddr_un local;

    const std::size_t max_path_length = sizeof(local.sun_path) - 1;
    const std::size_t path_length = ::strlen(path);
    if (path_length >= max_path_length)
    {
        res = core::unexpected_value;
    }
    else
    {
        const int SOCKET_ERROR = -1;

        (void) ::unlink(path);

        io_descriptor_type fd = ::socket(AF_LOCAL, SOCK_STREAM, 0);
        if (fd != SOCKET_ERROR)
        {
            std::memset(&local, 0, sizeof(local));

            local.sun_family = AF_LOCAL;
            ::strcpy(local.sun_path, path);

            int cc = ::bind(fd,
                reinterpret_cast<sockaddr *>(&local), sizeof(local));
            if (cc != SOCKET_ERROR)
            {
                const int backlog =
                    group_->get_options().unix_listen_backlog;

                cc = ::listen(fd, backlog);
                if (cc != SOCKET_ERROR)
                {
                    fd_ = fd;

                    // the resolved target is exactly the same
                    // as the required one

                    const char unix_prefix[] = "unix://";

                    char * buf = static_cast<char *>(
                        alloc_->allocate(path_length + sizeof(unix_prefix)));
                    if (buf != NULL)
                    {
                        ::sprintf(buf, "unix://%s", path);
                        target_ = buf;

                        res = core::ok;
                    }
                    else
                    {
                        res = core::no_memory;
                    }
                }
                else
                {
                    handle_io_error("unix listen",
                        io_error_callback_, io_error_callback_hint_);

                    res = core::io_error;
                }
            }
            else
            {
                handle_io_error("unix listener bind",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }
        }
        else
        {
            handle_io_error("create unix listener socket",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }

        if (res != core::ok)
        {
            ::close(fd);

            if (target_ != NULL)
            {
                alloc_->deallocate(target_);
            }
        }
    }

    return res;
}

void listener::close_socket()
{
    ::close(fd_);
}

// synchronized by caller
core::result listener::do_some_work()
{
    // accept new connection from the listening socket
    // assume that the listening socket is already ready for "reading"
    // (otherwise this operation can block)

    core::result res;

    switch (protocol_)
    {
    case proto_tcp:
        res = accept_tcp();
        break;
    case proto_udp:
        res = accept_udp();
        break;
    case proto_unix:
        res = accept_unix();
        break;

    default:
        // the file protocol is impossible here
        fatal_failure(__FILE__, __LINE__);
    }

    return res;
}

core::result listener::accept_tcp()
{
    core::result res;

    sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    const int SOCKET_ERROR = -1;

    const io_descriptor_type new_fd = ::accept(fd_,
        reinterpret_cast<sockaddr *>(&client_addr), &client_addr_size);
    if (new_fd != SOCKET_ERROR)
    {
        res = core::ok;

        if (group_->get_options().tcp_nonblocking)
        {
            res = set_nonblocking(new_fd);
        }

        if (res == core::ok)
        {
            if (group_->get_options().tcp_nodelay)
            {
                res = set_nodelay(new_fd);
            }
        }

        if (res == core::ok)
        {
            if (group_->get_options().tcp_keepalive)
            {
                res = set_keepalive(new_fd);
            }
        }

        if (res == core::ok)
        {
            // extract the client address and create new channel
            // with that address as the target

            // target created for the new channel will have the form:
            // "tcp://xxx.xxx.xxx.xxx:yyyyyyy"
            const std::size_t target_size = 30;
            char * target = static_cast<char *>(
                alloc_->allocate(target_size));
            if (target != NULL)
            {
                const unsigned char * tmp =
                    reinterpret_cast<unsigned char *>(&client_addr.sin_addr);

                ::snprintf(target, target_size, "tcp://%d.%d.%d.%d:%d",
                    static_cast<int>(tmp[0]),
                    static_cast<int>(tmp[1]),
                    static_cast<int>(tmp[2]),
                    static_cast<int>(tmp[3]),
                    ntohs(client_addr.sin_port));

                const std::size_t preferred_frame_size =
                    group_->get_options().tcp_frame_size;

                core::channel_descriptor new_descriptor;

                res = group_->add_existing(target, new_fd, proto_tcp,
                    preferred_frame_size, new_descriptor);

                // at this point the new channel is already seen
                // in the proper state

                if (res == core::ok && connection_hook_ != NULL)
                {
                    mtx_->unlock();

                    std::size_t index;
                    std::size_t sequence_number;
                    new_descriptor.get_details(index, sequence_number);

                    try
                    {
                        connection_hook_(connection_hook_hint_,
                            target, index, sequence_number);
                    }
                    catch (...)
                    {
                        // ignore exceptions from user code
                    }

                    mtx_->lock();
                }

                if (res != core::ok)
                {
                    alloc_->deallocate(target);
                }
            }
            else
            {
                res = core::no_memory;
            }
        }

        if (res != core::ok)
        {
            ::close(new_fd);
        }
    }
    else
    {
        handle_io_error("tcp listener accept",
            io_error_callback_, io_error_callback_hint_);

        res = core::io_error;
    }

    return res;
}

// synchronized by caller
core::result listener::accept_udp()
{
    // "Accepting" at the UDP listener is an artificial term
    // that really relates to receiving of regular datagram
    // and not to establishing any connection.
    // The datagram is received and placed in the appropriate channel
    // according to the sender address information.

    core::result res;

    sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    const int SOCKET_ERROR = -1;

    const ssize_t readn =
        ::recvfrom(fd_, frame_buffer_, frame_buffer_size_, 0,
            reinterpret_cast<sockaddr *>(&client_addr), &client_addr_size);
    if (readn != SOCKET_ERROR)
    {
        // extract the client address and make sure that a channel
        // with that address as the target exists

        // target created for the channel will have the form:
        // "udp://xxx.xxx.xxx.xxx:yyyyyyy"
        const std::size_t target_size = 30;
        char * target = static_cast<char *>(
            alloc_->allocate(target_size));
        if (target != NULL)
        {
            const unsigned char * tmp =
                reinterpret_cast<unsigned char *>(&client_addr.sin_addr);

            ::snprintf(target, target_size, "udp://%d.%d.%d.%d:%d",
                static_cast<int>(tmp[0]),
                static_cast<int>(tmp[1]),
                static_cast<int>(tmp[2]),
                static_cast<int>(tmp[3]),
                ntohs(client_addr.sin_port));

            core::channel_descriptor descriptor;

            // make sure the channel for the client exists
            // (create if it is a new channel)

            bool created_new_channel;
            res = group_->open(target, descriptor,
                created_new_channel, false);

            alloc_->deallocate(target);

            // at this point the new channel is already seen
            // in the proper state

            if (created_new_channel)
            {
                // this channel was freshly created
                // -> call the connection hook

                if (res == core::ok && connection_hook_ != NULL)
                {
                    mtx_->unlock();

                    std::size_t index;
                    std::size_t sequence_number;
                    descriptor.get_details(index, sequence_number);

                    try
                    {
                        connection_hook_(connection_hook_hint_,
                            target, index, sequence_number);
                    }
                    catch (...)
                    {
                        // ignore exceptions from user code
                    }

                    mtx_->lock();
                }
            }

            if (res == core::ok)
            {
                // inject the received datagram into the channel

                res = group_->process_complete_incoming_frame(
                    descriptor, frame_buffer_, readn);
            }
        }
        else
        {
            res = core::no_memory;
        }
    }
    else
    {
        handle_io_error("udp listener receive",
            io_error_callback_, io_error_callback_hint_);

        res = core::io_error;
    }

    return res;
}

core::result listener::accept_unix()
{
    core::result res;

    sockaddr_un client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    const int SOCKET_ERROR = -1;

    const io_descriptor_type new_fd = ::accept(fd_,
        reinterpret_cast<sockaddr *>(&client_addr), &client_addr_size);
    if (new_fd != SOCKET_ERROR)
    {
        res = core::ok;

        if (group_->get_options().unix_nonblocking)
        {
            res = set_nonblocking(new_fd);
        }

        if (res == core::ok)
        {
            // extract the path and create new channel
            // with that address as the target

            // target created for the new channel will have the form:
            // "unix://path"
            // buf size for target should be at least Unix path + prefix + id
            const std::size_t target_size = sizeof(client_addr.sun_path) + 28;
            char * target = static_cast<char *>(
                alloc_->allocate(target_size));
            if (target != NULL)
            {
                int cc = ::getsockname(new_fd,
                    reinterpret_cast<sockaddr*>(&client_addr),
                    &client_addr_size);
                if (cc != SOCKET_ERROR)
                {
                    ::snprintf(target, target_size, "unix://%s$%lld",
                        client_addr.sun_path, ++unix_client_id);

                    const std::size_t preferred_frame_size =
                        group_->get_options().unix_frame_size;

                    core::channel_descriptor new_descriptor;

                    res = group_->add_existing(target, new_fd, proto_unix,
                        preferred_frame_size, new_descriptor);

                    // at this point the new channel is already seen
                    // in the proper state

                    if (res == core::ok && connection_hook_ != NULL)
                    {
                        mtx_->unlock();

                        std::size_t index;
                        std::size_t sequence_number;
                        new_descriptor.get_details(index, sequence_number);

                        try
                        {
                            connection_hook_(connection_hook_hint_,
                                target, index, sequence_number);
                        }
                        catch (...)
                        {
                            // ignore exceptions from user code
                        }

                        mtx_->lock();
                    }
                }
                else
                {
                    handle_io_error("unix listener getsockname",
                        io_error_callback_, io_error_callback_hint_);

                    res = core::io_error;
                }

                if (res != core::ok)
                {
                    alloc_->deallocate(target);
                }
            }
            else
            {
                res = core::no_memory;
            }
        }

        if (res != core::ok)
        {
            ::close(new_fd);
        }
    }
    else
    {
        handle_io_error("unix listener accept",
            io_error_callback_, io_error_callback_hint_);

        res = core::io_error;
    }

    return res;
}
