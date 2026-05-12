/*
 *  <Short Description>
 *  Copyright (C) 2026  Brett Terpstra
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
#include <net/socket.h>

namespace blt::net
{
    std::string error::socket_error_t::what(const int error)
    {
        switch (error)
        {
        case EACCES:
            return "Permission to create a socket of the specified type and/or protocol is denied.";
        case EAFNOSUPPORT:
            return "The implementation does not support the specified address family.";
        case EINVAL:
            return "Unknown protocol, or protocol family not available. Invalid flags in type.";
        case EMFILE:
            return "The per-process limit on the number of open file descriptors has been reached.";
        case ENFILE:
            return "The system-wide limit on the total number of open files has been reached.";
        case ENOBUFS:
        case ENOMEM:
            return
                "Insufficient memory is available. The socket cannot be created until sufficient resources are freed.";
        case EPROTONOSUPPORT:
            return "The protocol type or the specified protocol is not supported within this domain.";
        default:
            return error_t::what(error);
        }
    }

    std::string error::bind_error_t::what(const int error)
    {
        switch (error)
        {
        case EACCES:
            return
                "The address is protected, and the user is not the superuser. Or search permission is denied on a component of the path prefix.";
        case EADDRINUSE:
            return
                "The given address is already in use, or all port numbers in the ephemeral port range are currently in use.";
        case EBADF:
            return "sockfd is not a valid file descriptor.";
        case EINVAL:
            return
                "The socket is already bound to an address, or addrlen is wrong, or addr is not a valid address for this socket's domain.";
        case ENOTSOCK:
            return "The file descriptor sockfd does not refer to a socket.";
        case EADDRNOTAVAIL:
            return "A nonexistent interface was requested or the requested address was not local.";
        case EFAULT:
            return "addr points outside the user's accessible address space.";
        case ELOOP:
            return "Too many symbolic links were encountered in resolving addr.";
        case ENAMETOOLONG:
            return "addr is too long.";
        case ENOENT:
            return "A component in the directory prefix of the socket pathname does not exist.";
        case ENOMEM:
            return "Insufficient kernel memory was available.";
        case ENOTDIR:
            return "A component of the path prefix is not a directory.";
        case EROFS:
            return "The socket inode would reside on a read-only filesystem.";
        default:
            return "Protocol error. or " + error_t::what(error);
        }
    }

    std::string error::listen_error_t::what(const int error)
    {
        switch (error)
        {
        case EADDRINUSE:
            return
                "Another socket is already listening on the same port, or all port numbers in the ephemeral port range are currently in use.";
        case EBADF:
            return "The argument sockfd is not a valid file descriptor.";
        case ENOTSOCK:
            return "The file descriptor sockfd does not refer to a socket.";
        case EOPNOTSUPP:
            return "The socket is not of a type that supports the listen() operation.";
        default:
            return error_t::what(error);
        }
    }

    std::string error::accept_error_t::what(const int error)
    {
        switch (error)
        {
        case EBADF:
            return "sockfd is not an open file descriptor.";
        case ECONNABORTED:
            return "A connection has been aborted.";
        case EFAULT:
            return "The addr argument is not in a writable part of the user address space.";
        case EINTR:
            return "The system call was interrupted by a signal that was caught before a valid connection arrived.";
        case EINVAL:
            return
                "Socket is not listening for connections, or addrlen is invalid (e.g., is negative), or invalid value in flags.";
        case EMFILE:
            return "The per-process limit on the number of open file descriptors has been reached.";
        case ENFILE:
            return "The system-wide limit on the total number of open files has been reached.";
        case ENOBUFS:
        case ENOMEM:
            return
                "Not enough free memory. This often means that the memory allocation is limited by the socket buffer limits, not by the system memory.";
        case ENOTSOCK:
            return "The file descriptor sockfd does not refer to a socket.";
        case EOPNOTSUPP:
            return "The referenced socket is not of type SOCK_STREAM.";
        case EPERM:
            return "Firewall rules forbid connection.";
        case EPROTO:
            return "Protocol error.";
        default:
            return error_t::what(error);
        }
    }

    std::string error::connect_error_t::what(const int error)
    {
        switch (error)
        {
        case EACCES:
        case EPERM:
            return
                "Permission denied. Write permission is denied on the socket file, "
                "search permission is denied for one of the directories in the path prefix, "
                "the user tried to connect to a broadcast address without having the socket broadcast flag enabled, "
                "the connection request failed because of a local firewall rule, or an SELinux policy denied a connection.";
        case EADDRINUSE:
            return "Local address is already in use.";
        case EADDRNOTAVAIL:
            return
                "The socket had not previously been bound to an address "
                "and all port numbers in the ephemeral port range are currently in use.";
        case EAFNOSUPPORT:
            return "The passed address didn't have the correct address family in its sa_family field.";
        case EAGAIN:
            return
                "For nonblocking UNIX domain sockets, the socket is nonblocking "
                "and the connection cannot be completed immediately. For other socket families, "
                "there are insufficient entries in the routing cache.";
        case EALREADY:
            return "The socket is nonblocking and a previous connection attempt has not yet been completed.";
        case EBADF:
            return "sockfd is not a valid open file descriptor.";
        case ECONNREFUSED:
            return "A connect() on a stream socket found no one listening on the remote address.";
        case EFAULT:
            return "The socket structure address is outside the user's address space.";
        case EINPROGRESS:
            return
                "The socket is nonblocking and the connection cannot be completed immediately. "
                "It is possible to select() or poll() for completion by selecting the socket for writing.";
        case EINTR:
            return "The system call was interrupted by a signal that was caught.";
        case EISCONN:
            return "The socket is already connected.";
        case ENETUNREACH:
            return "Network is unreachable.";
        case ENOTSOCK:
            return "The file descriptor sockfd does not refer to a socket.";
        case EPROTOTYPE:
            return "The socket type does not support the requested communications protocol.";
        case ETIMEDOUT:
            return "Timeout while attempting connection. The server may be too busy to accept new connections.";
        default:
            return error_t::what(error);
        }
    }

    std::string error::write_error_t::what(int error)
    {
        switch (error)
        {
        case EAGAIN:
        case EWOULDBLOCK:
            return
                "The file descriptor refers to a file other than a socket and has been marked nonblocking (O_NONBLOCK), and the write would block. "
                "Or the file descriptor refers to a socket and has been marked nonblocking (O_NONBLOCK), and the write would block.";
        case EBADF:
            return "fd is not a valid file descriptor or is not open for writing.";
        case EDESTADDRREQ:
            return "fd refers to a datagram socket for which a peer address has not been set using connect().";
        case EDQUOT:
            return
                "The user's quota of disk blocks on the filesystem containing the file referred to by fd has been exhausted.";
        case EFAULT:
            return "buf is outside your accessible address space.";
        case EFBIG:
            return
                "An attempt was made to write a file that exceeds the implementation-defined maximum file size or the process's file size limit, "
                "or to write at a position past the maximum allowed offset.";
        case EINTR:
            return "The call was interrupted by a signal before any data was written.";
        case EINVAL:
            return
                "fd is attached to an object which is unsuitable for writing; or the file was opened with the O_DIRECT flag, "
                "and either the address specified in buf, the value specified in count, or the file offset is not suitably aligned.";
        case EIO:
            return
                "A low-level I/O error occurred while modifying the inode. This error may relate to the write-back of data written by an earlier write(), "
                "which may have been issued to a different file descriptor on the same file. "
                "An alternate cause of EIO on networked filesystems is when an advisory lock had been taken out on the file descriptor and this lock has been lost.";
        case ENOSPC:
            return "The device containing the file referred to by fd has no room for the data.";
        case EPERM:
            return "The operation was prevented by a file seal.";
        case EPIPE:
            return
                "fd is connected to a pipe or socket whose reading end is closed. When this happens the writing process will also receive a SIGPIPE signal.";
        default:
            return error_t::what(error);
        }
    }

    socket_t::socket_t(const socket_domain_t domain, const socket_type_t type,
                       const std::optional<socket_extras> extras,
                       const std::optional<int> protocol) : domain{domain}, type{type}
    {
        int converted_type = type;
        if (extras.has_value())
        {
            converted_type = converted_type | static_cast<int>(extras.value());
        }
        fd = socket(domain, converted_type, protocol.value_or(0));
        if (fd)
            throw error::c_error_exception<error::socket_error_t>{};
    }

    client_t socket_t::client() const
    {
        return client_t{*this};
    }

    server_t socket_t::server() const
    {
        return server_t{*this};
    }

    std::optional<client_t> server_t::accept(const socket_extras flags) const
    {
        const auto lfd = accept_internal(nullptr, nullptr, static_cast<int>(flags));
        if (!lfd)
            return {};
        return generate_client(lfd.value());
    }

    std::optional<int> server_t::accept_internal(sockaddr* addr, socklen_t* addrlen, int flags) const
    {
        const auto lfd = ::accept4(fd, addr, addrlen, flags);
        if (lfd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return {};
            throw error::c_error_exception<error::accept_error_t>{};
        }
        return lfd;
    }

    client_t server_t::generate_client(const int new_fd) const
    {
        return client_t{*this, new_fd};
    }
}
