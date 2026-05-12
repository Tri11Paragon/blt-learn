#pragma once
/*
 *  Copyright (C) 2024  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BLT_SOCKET_H
#define BLT_SOCKET_H
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "blt/std/memory_util.h"
#include "blt/std/ranges.h"

#if __has_include(<sys/socket.h>) && __has_include(<sys/un.h>)
#define HAS_SOCKET
#else
#undef HAS_SOCKET
#endif

#ifdef HAS_SOCKET

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace blt::net
{
    namespace error
    {
        struct error_t
        {
            static std::string what(const int error)
            {
                return "Unknown error: " + std::to_string(error) + " (" + std::strerror(error) + ")";
            }
        };

        struct socket_error_t
        {
            static std::string what(int error);
        };

        struct bind_error_t
        {
            static std::string what(int error);
        };

        struct listen_error_t
        {
            static std::string what(int error);
        };

        struct accept_error_t
        {
            static std::string what(int error);
        };

        struct connect_error_t
        {
            static std::string what(int error);
        };

        struct write_error_t
        {
            static std::string what(int error);
        };

        template <typename T = error_t>
        struct c_error_exception : std::exception
        {
            c_error_exception() : error_message(T::what(errno)), error{errno}
            {
            }

            explicit c_error_exception(int error) : error_message(T::what(error)), error{error}
            {
            }

            [[nodiscard]] const char* what() const noexcept override
            {
                return error_message.c_str();
            }

            std::string error_message;
            int error;
        };
    }

    namespace detail
    {
    }

    struct socket_domain_t
    {
        enum socket_domain : int
        {
            UNIX = AF_UNIX,
            INET = AF_INET,
            INET6 = AF_INET6,
            PACKET = AF_PACKET,
        };

        socket_domain_t(const socket_domain domain) : family{domain}
        {
        }

        operator int() const
        {
            return family;
        }

        operator unsigned int() const
        {
            return family;
        }

        operator std::make_unsigned_t<sa_family_t>() const
        {
            return static_cast<std::make_unsigned_t<sa_family_t>>(family);
        }

        operator std::make_signed_t<sa_family_t>() const
        {
            return static_cast<std::make_signed_t<sa_family_t>>(family);
        }

        socket_domain family;
    };

    struct socket_type_t
    {
        enum socket_type : int
        {
            STREAM = SOCK_STREAM,
            DGRAM = SOCK_DGRAM,
            SEQ = SOCK_SEQPACKET,
            RAW = SOCK_RAW,
            RDM = SOCK_RDM,
        };

        socket_type_t(const socket_type type) : type{type}
        {
        }

        operator int() const
        {
            return type;
        }

        operator unsigned int() const
        {
            return type;
        }

        int type;
    };

    enum class socket_extras: int
    {
        NONE = 0,
        NONBLOCK = SOCK_NONBLOCK,
        CLOEXEC = SOCK_CLOEXEC,
    };

    struct client_t;
    struct server_t;

    struct socket_t
    {
        socket_t(socket_domain_t domain, socket_type_t type, std::optional<socket_extras> extras = std::nullopt,
                 std::optional<int> protocol = std::nullopt);

        [[nodiscard]] client_t client() const;
        [[nodiscard]] server_t server() const;

        size_t write(const void* ptr, const size_t size) const
        {
            return ::write(fd, ptr, size);
        }

        template <typename T>
        void write(span<T> data) const
        {
            size_t written = 0;
            do
            {
                auto wr = ::write(fd, data.data() + written, data.size() - written);
                if (wr == -1)
                    throw error::c_error_exception<error::write_error_t>{};
                written += wr;
            }
            while (written != data.size());
        }

        template <typename T>
        void write(T&& t) const
        {
            write(span<unsigned char>(reinterpret_cast<unsigned char*>(&t), sizeof(T)));
        }

        int fd;
        socket_domain_t domain;
        socket_type_t type;
    };

    struct server_t : socket_t
    {
        explicit server_t(const socket_t& socket) : socket_t(socket)
        {
        }

        template <typename Address>
        server_t& bind(const Address& address)
        {
            address_data.set(address);
            if (::bind(fd, reinterpret_cast<sockaddr*>(address_data.data()), sizeof(Address)))
                throw error::c_error_exception<error::bind_error_t>{};
            return *this;
        }

        server_t& listen(const int backlog = SOMAXCONN)
        {
            if (::listen(fd, backlog))
                throw error::c_error_exception<error::listen_error_t>{};
            return *this;
        }

        [[nodiscard]] std::optional<client_t> accept(socket_extras flags = socket_extras::NONE) const;

        template <typename Address>
        [[nodiscard]] std::optional<client_t> accept(Address& address, socket_extras flags = socket_extras::NONE) const;

        mem::storage_t address_data;

    private:
        std::optional<int> accept_internal(sockaddr* addr, socklen_t* addrlen, int flags) const;

        [[nodiscard]] client_t generate_client(int new_fd) const;
    };

    struct client_t : socket_t
    {
        client_t(const socket_t& socket, const int new_fd) : socket_t{socket}
        {
            this->fd = new_fd;
        }

        explicit client_t(const socket_t& socket) : socket_t(socket)
        {
        }

        template <typename Address>
        client_t& connect(const Address& address)
        {
            address_data = std::make_optional(mem::storage_t{});
            address_data->set(address);
            if (::connect(fd, reinterpret_cast<sockaddr*>(address_data->data()), sizeof(address)) == -1)
                throw error::c_error_exception<error::connect_error_t>{};
            return *this;
        }

        std::optional<mem::storage_t> address_data;
    };

    template <typename Address>
    [[nodiscard]] std::optional<client_t> server_t::accept(Address& address, socket_extras flags) const
    {
        socklen_t len = sizeof(Address);
        const auto lfd = accept_internal(reinterpret_cast<sockaddr*>(&address), &len, static_cast<int>(flags));
        if (len != sizeof(Address))
            throw std::runtime_error{
                "Address length doesn't match provided structure. Got " + std::to_string(len) + " expected " +
                std::to_string(sizeof(Address))
            };
        if (!lfd)
            return {};
        auto client = generate_client(lfd.value());
        client.address_data = std::make_optional(mem::storage_t{});
        client.address_data->set(address);
        return client;
    }
}

#endif

#endif //BLT_SOCKET_H
