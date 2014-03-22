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

#include "../channel.h"
#include "../allocator.h"
#include "../io_error_handler.h"
#include "../network_utils.h"
#include "../options.h"
#include "mutex.h"
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace yami;
using namespace details;

namespace // unnamed
{

// helper function for trimming connection strings
// length is in characters from source, the result is 1 char longer
// for null-termination
// result is NULL if no memory can be allocated
const char * trim(allocator * alloc, const char * source, std::size_t length)
{
    char * trimmed = static_cast<char *>(alloc->allocate(length + 1));
    if (trimmed != NULL)
    {
        std::strncpy(trimmed, source, length);
        trimmed[length] = '\0';
    }

    return trimmed;
}

} // unnamed namespace

void channel::close_connection()
{
    ::close(fd_);
}

core::result channel::connect_to_file(const char * file_name)
{
    core::result res = core::ok;

    if (*file_name == '\0')
    {
        res = core::bad_protocol;
    }
    else
    {
        protocol_ = proto_file;

        //const char write_suffix[] = "?write";
        //const std::size_t write_suffix_size = sizeof(write_suffix) - 1;

        const char append_suffix[] = "?append";
        const std::size_t append_suffix_size = sizeof(append_suffix) - 1;

        const char read_suffix[] = "?read";
        const std::size_t read_suffix_size = sizeof(read_suffix) - 1;

        const char * trimmed_file_name = NULL;
        const char * used_file_name = file_name;

        // write (output) by default
        int open_flags = O_CREAT | O_TRUNC | O_WRONLY;
        direction_ = output;

        const std::size_t file_name_len = std::strlen(file_name);
        if (file_name_len > append_suffix_size)
        {
            const std::size_t trimmed_length =
                file_name_len - append_suffix_size;

            if (std::strcmp(file_name + trimmed_length,
                    append_suffix) == 0)
            {
                trimmed_file_name = trim(alloc_, file_name, trimmed_length);

                if (trimmed_file_name != NULL)
                {
                    used_file_name = trimmed_file_name;

                    open_flags = O_CREAT | O_APPEND;
                    direction_ = output;
                }
                else
                {
                    res = core::no_memory;
                }
            }
        }
        if (file_name_len > read_suffix_size)
        {
            const std::size_t trimmed_length =
                file_name_len - read_suffix_size;

            if (std::strcmp(file_name + trimmed_length,
                    read_suffix) == 0)
            {
                trimmed_file_name = trim(alloc_, file_name, trimmed_length);

                if (trimmed_file_name != NULL)
                {
                    used_file_name = trimmed_file_name;

                    open_flags = O_RDONLY;
                    direction_ = input;
                }
                else
                {
                    res = core::no_memory;
                }
            }
        }

        if (res == core::ok)
        {
            fd_ = ::open(used_file_name, open_flags, 0666);
            if (fd_ != -1)
            {
                preferred_frame_size_ =
                    configuration_options_.file_frame_size;

                res = core::ok;
            }
            else
            {
                handle_io_error("connect to file",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }

            if (res == core::ok)
            {
                if (configuration_options_.file_nonblocking)
                {
                    res = set_nonblocking(fd_);
                }
            }
        }

        if (trimmed_file_name != NULL)
        {
            alloc_->deallocate(trimmed_file_name);
        }
    }

    return res;
}

core::result channel::connect_to_tcp(const char * tcp_address)
{
    ip_address ipa;
    core::result res = parse_address(*alloc_, tcp_address, ipa,
        io_error_callback_, io_error_callback_hint_);
    if (res == core::ok)
    {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ < 0)
        {
            handle_io_error("create tcp socket",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }
    }

    if (res == core::ok)
    {
        protocol_ = proto_tcp;

        struct sockaddr_in servaddr;

        std::memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = ipa.port;
        std::memcpy(&(servaddr.sin_addr), &ipa.host, sizeof(int));

        if (configuration_options_.tcp_nonblocking)
        {
            // perform non-blocking connection
            // as part of non-blocking setting

            res = set_nonblocking(fd_);

            if (res == core::ok)
            {
                int cc = ::connect(fd_,
                    (struct sockaddr*)&servaddr, sizeof(servaddr));
                if (cc < 0 && errno == EINPROGRESS)
                {
                    // wait with configured timeout
                    // for completing the connection process
                    // timeout 0ms means wait until complete

                    struct timeval tv;
                    struct timeval * tv_to_use;
                    fd_set write_set;
                    FD_ZERO(&write_set);
                    FD_SET(fd_, &write_set);
                    int num_of_descriptors_to_test = fd_ + 1;

                    const std::size_t timeout =
                        configuration_options_.tcp_connect_timeout;

                    if (timeout > 0)
                    {
                        tv.tv_sec = timeout / 1000;
                        tv.tv_usec = 1000 * (timeout % 1000);
                        tv_to_use = &tv;
                    }
                    else
                    {
                        tv_to_use = NULL;
                    }

                    cc = ::select(num_of_descriptors_to_test,
                        NULL, &write_set, NULL, tv_to_use);
                    if (cc > 0)
                    {
                        // socket was either connected in time
                        // or there was an error

                        int error = 0;
                        socklen_t error_len = sizeof(error);
                        cc = ::getsockopt(fd_,
                            SOL_SOCKET, SO_ERROR, &error, &error_len);
                        if (error != 0)
                        {
                            handle_io_error_with_code(
                                "non-blocking tcp connect",
                                error,
                                io_error_callback_, io_error_callback_hint_);

                            res = core::io_error;
                        }
                    }
                    else if (cc == 0)
                    {
                        res = core::timed_out;
                    }
                    else
                    {
                        handle_io_error("wait for non-blocking tcp connect",
                            io_error_callback_, io_error_callback_hint_);

                        res = core::io_error;
                    }
                }
                else if (cc < 0)
                {
                    handle_io_error("non-blocking tcp connect",
                        io_error_callback_, io_error_callback_hint_);

                    res = core::io_error;
                }
            }
        }
        else
        {
            // blocking connection

            const int cc = ::connect(fd_,
                (struct sockaddr*)&servaddr, sizeof(servaddr));
            if (cc < 0)
            {
                handle_io_error("blocking tcp connect",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }
        }
    }

    if (res == core::ok)
    {
        if (configuration_options_.tcp_nodelay)
        {
            res = set_nodelay(fd_);
        }
    }

    if (res == core::ok)
    {
        if (configuration_options_.tcp_keepalive)
        {
            res = set_keepalive(fd_);
        }
    }

    if (res == core::ok)
    {
        preferred_frame_size_ =
            configuration_options_.tcp_frame_size;

        direction_ = inout;
    }
    else
    {
        ::close(fd_);
    }

    return res;
}

core::result channel::connect_to_udp(const char * udp_address)
{
    ip_address ipa;
    core::result res = parse_address(*alloc_, udp_address, ipa,
        io_error_callback_, io_error_callback_hint_);
    if (res == core::ok)
    {
        fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (fd_ < 0)
        {
            handle_io_error("create udp socket",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }
        else
        {
            protocol_ = proto_udp;
            direction_ = inout;

            // remember the target address and use it
            // for each frame when it is sent

            target_address_size_ = sizeof(sockaddr_in);
            struct sockaddr_in * servaddr =
                static_cast<sockaddr_in *>(
                    alloc_->allocate(target_address_size_));

            if (servaddr != NULL)
            {
                std::memset(servaddr, 0, target_address_size_);
                servaddr->sin_family = AF_INET;
                servaddr->sin_port = ipa.port;
                std::memcpy(&(servaddr->sin_addr), &ipa.host, sizeof(int));

                preferred_frame_size_ =
                    configuration_options_.udp_frame_size;

                datagram_buffer_ = static_cast<char *>(
                    alloc_->allocate(preferred_frame_size_));
                if (datagram_buffer_ != NULL)
                {
                    target_address_ = servaddr;
                }
                else
                {
                    alloc_->deallocate(servaddr);
                    res = core::no_memory;
                }
            }
            else
            {
                res = core::no_memory;
            }
        }
    }

    return res;
}

core::result channel::connect_to_unix(const char * path)
{
    core::result res = core::ok;

    struct sockaddr_un servaddr;

    const std::size_t max_path_length = sizeof(servaddr.sun_path) - 1;
    const std::size_t path_length = ::strlen(path);
    if (path_length >= max_path_length)
    {
        res = core::unexpected_value;
    }
    else
    {
        fd_ = socket(AF_LOCAL, SOCK_STREAM, 0);
        if (fd_ < 0)
        {
            handle_io_error("create unix socket",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }
        else
        {
            protocol_ = proto_unix;

            std::memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sun_family = AF_LOCAL;
            std::strcpy(servaddr.sun_path, path);

            const int cc = ::connect(fd_,
                (struct sockaddr*)&servaddr, sizeof(servaddr));
            if (cc < 0)
            {
                handle_io_error("unix connect",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }

            if (res == core::ok)
            {
                if (configuration_options_.unix_nonblocking)
                {
                    res = set_nonblocking(fd_);
                }
            }

            if (res == core::ok)
            {
                preferred_frame_size_ =
                    configuration_options_.unix_frame_size;

                direction_ = inout;
            }
            else
            {
                ::close(fd_);
            }
        }
    }

    return res;
}

core::result channel::read_bytes(char * buf, std::size_t size,
    std::size_t & readn)
{
    core::result res;

    if (protocol_ == proto_udp)
    {
        // the UDP frame has to be read as a single operation

        struct sockaddr_in remote_address;
        socklen_t address_size = sizeof(remote_address);

        ssize_t rn = ::recvfrom(fd_, buf, size, 0,
            reinterpret_cast<sockaddr *>(&remote_address), &address_size);

        if (rn >= 0)
        {
            readn = static_cast<std::size_t>(rn);
            res = core::ok;
        }
        else
        {
            handle_io_error("udp receive",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }
    }
    else
    {
        ssize_t rn = ::read(fd_, buf, size);
        if (rn == -1)
        {
            if (errno == EINTR)
            {
                readn = 0;
                res = core::ok;
            }
            else
            {
                handle_io_error("stream read",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }
        }
        else
        {
            if (rn != 0)
            {
                readn = static_cast<std::size_t>(rn);
                res = core::ok;
            }
            else
            {
                res = core::channel_closed;
            }
        }
    }

    return res;
}

core::result channel::write_bytes(const char * buf, std::size_t size,
    std::size_t & written)
{
    core::result res;

    if (protocol_ == proto_udp)
    {
        // the UDP frame is either successfully pushed as a whole
        // or the operation is considered as failing

        socklen_t address_size = static_cast<socklen_t>(target_address_size_);
        ssize_t wn = ::sendto(fd_, buf, size, 0,
            reinterpret_cast<sockaddr *>(target_address_), address_size);
        if (static_cast<std::size_t>(wn) == size)
        {
            res = core::ok;
        }
        else
        {
            handle_io_error("udp send",
                io_error_callback_, io_error_callback_hint_);

            res = core::io_error;
        }
    }
    else
    {
        // this is a stream channel

        ssize_t wn = ::write(fd_, buf, size);
        if (wn == -1)
        {
            if (errno == EINTR)
            {
                written = 0;
                res = core::ok;
            }
            else
            {
                handle_io_error("stream write",
                    io_error_callback_, io_error_callback_hint_);

                res = core::io_error;
            }
        }
        else
        {
            written = static_cast<std::size_t>(wn);
            res = core::ok;
        }
    }

    return res;
}
