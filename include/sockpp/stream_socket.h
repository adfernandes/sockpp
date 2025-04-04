/**
 * @file stream_socket.h
 *
 * Classes for stream sockets.
 *
 * @author Frank Pagliughi
 * @author SoRo Systems, Inc.
 * @author www.sorosys.com
 *
 * @date December 2014
 */

// --------------------------------------------------------------------------
// This file is part of the "sockpp" C++ socket library.
//
// Copyright (c) 2014-2023 Frank Pagliughi
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// --------------------------------------------------------------------------

#ifndef __sockpp_stream_socket_h
#define __sockpp_stream_socket_h

#include <vector>

#include "sockpp/socket.h"
#include "types.h"

namespace sockpp {

/////////////////////////////////////////////////////////////////////////////

/**
 * Base class for streaming sockets, such as TCP and Unix Domain.
 * This is the streaming connection between two peers. It looks like a
 * readable/writeable device.
 */
class stream_socket : public socket
{
    /** The base class   */
    using base = socket;

protected:
    /** Acceptor can create stream sockets. */
    friend class acceptor;

    /**
     * Creates a streaming socket.
     * @return An OS handle to a stream socket.
     */
    static result<socket_t> create_handle(int domain, int protocol = 0) {
        return base::create_handle(domain, COMM_TYPE, protocol);
    }

public:
    /** The socket 'type' for communications semantics. */
    static constexpr int COMM_TYPE = SOCK_STREAM;
    /**
     * Creates an unconnected streaming socket.
     */
    stream_socket() {}
    /**
     * Creates a streaming socket from an existing OS socket handle and
     * claims ownership of the handle.
     * @param handle A socket handle from the operating system.
     */
    explicit stream_socket(socket_t handle) noexcept : base(handle) {}
    /**
     * Creates a stream socket by copying the socket handle from the
     * specified socket object and transfers ownership of the socket.
     */
    stream_socket(stream_socket&& sock) noexcept : base(std::move(sock)) {}
    /**
     * Creates a socket with the specified communications characterics.
     * Not that this is not normally how a socket is creates in the sockpp
     * library. Applications would typically create a connector (client) or
     * acceptor (server) socket which would take care of the details.
     *
     * This is included for completeness or for creating different types of
     * sockets than are supported by the library.
     *
     * @param domain The communications domain for the sockets (i.e. the
     *  			 address family)
     * @param protocol The particular protocol to be used with the sockets
     *
     * @return A stream socket with the requested communications
     *  	   characteristics.
     */
    static result<stream_socket> create(int domain, int protocol = 0);
    /**
     * Move assignment.
     * @param rhs The other socket to move into this one.
     * @return A reference to this object.
     */
    stream_socket& operator=(stream_socket&& rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }
    /**
     * Creates a new stream socket that refers to this one.
     * This creates a new object with an independent lifetime, but refers
     * back to this same socket. On most systems, this duplicates the file
     * handle using the dup() call. A typical use of this is to have
     * separate threads for reading and writing the socket. One thread would
     * get the original socket and the other would get the cloned one.
     * @return A new stream socket object that refers to the same socket as
     *  	   this one.
     */
    result<stream_socket> clone() const {
        if (auto res = base::clone(); !res)
            return res.error();
        else
            return stream_socket{res.release().release()};
    }
    /**
     * Gets the value of the `TCP_NODELAY` option on the socket.
     * @return The value of the `TCP_NODELAY` option on the socket if
     *         successful, an error code on failure.
     */
    result<bool> nodelay() const noexcept {
        return get_option<bool>(IPPROTO_TCP, TCP_NODELAY);
    }
    /**
     * Sets the value of the `TCP_NODELAY` option on the socket.
     * @return The value of the `TCP_NODELAY` option on the socket if
     *         successful, an error code on failure.
     */
    result<> nodelay(bool on) noexcept { return set_option(IPPROTO_TCP, TCP_NODELAY, on); }
    /**
     * Reads from the socket.
     * @param buf Buffer to get the incoming data.
     * @param n The number of bytes to try to read.
     * @return The number of bytes read on success, or @em -1 on error.
     */
    virtual result<size_t> read(void* buf, size_t n);
    /**
     * Best effort attempts to read the specified number of bytes.
     * This will make repeated read attempts until all the bytes are read in
     * or until an error occurs.
     * @param buf Buffer to get the incoming data.
     * @param n The number of bytes to try to read.
     * @return The number of bytes read on success, or @em -1 on error. If
     *  	   successful, the number of bytes read should always be 'n'.
     */
    virtual result<size_t> read_n(void* buf, size_t n);
    /**
     * Reads discontiguous memory ranges from the socket.
     * @param ranges The vector of memory ranges to fill
     * @return The number of bytes read, or @em -1 on error.
     */
    result<size_t> read(const std::vector<iovec>& ranges);
    /**
     * Set a timeout for read operations.
     * Sets the timeout that the device uses for read operations. Not all
     * devices support timeouts, so the caller should prepare for failure.
     * @param to The amount of time to wait for the operation to complete.
     * @return @em true on success, @em false on failure.
     */
    virtual result<> read_timeout(const microseconds& to);
    /**
     * Set a timeout for read operations.
     * Sets the timeout that the device uses for read operations. Not all
     * devices support timeouts, so the caller should prepare for failure.
     * @param to The amount of time to wait for the operation to complete.
     * @return @em true on success, @em false on failure.
     */
    template <class Rep, class Period>
    result<> read_timeout(const duration<Rep, Period>& to) {
        return read_timeout(std::chrono::duration_cast<microseconds>(to));
    }
    /**
     * Writes the buffer to the socket.
     * @param buf The buffer to write
     * @param n The number of bytes in the buffer.
     * @return The number of bytes written, or @em -1 on error.
     */
    virtual result<size_t> write(const void* buf, size_t n);
    /**
     * Best effort attempt to write the whole buffer to the socket.
     * @param buf The buffer to write
     * @param n The number of bytes in the buffer.
     * @return The number of bytes written, or @em -1 on error. On success,
     *  	   the number of bytes written should always be 'n'.
     */
    virtual result<size_t> write_n(const void* buf, size_t n);
    /**
     * Best effort attempt to write a string to the socket.
     * @param s The string to write.
     * @return The number of bytes written, or @em -1 on error. On success,
     *  	   the number of bytes written should always be the length of
     *  	   the string.
     */
    virtual result<size_t> write(const string& s) { return write_n(s.data(), s.size()); }
    /**
     * Writes discontiguous memory ranges to the socket.
     * @param ranges The vector of memory ranges to write
     * @return The number of bytes written, or @em -1 on error.
     */
    virtual result<size_t> write(const std::vector<iovec>& ranges);
    /**
     * Set a timeout for write operations.
     * Sets the timeout that the device uses for write operations. Not all
     * devices support timeouts, so the caller should prepare for failure.
     * @param to The amount of time to wait for the operation to complete.
     * @return @em true on success, @em false on failure.
     */
    virtual result<> write_timeout(const microseconds& to);
    /**
     * Set a timeout for write operations.
     * Sets the timeout that the device uses for write operations. Not all
     * devices support timeouts, so the caller should prepare for failure.
     * @param to The amount of time to wait for the operation to complete.
     * @return @em true on success, @em false on failure.
     */
    template <class Rep, class Period>
    result<> write_timeout(const duration<Rep, Period>& to) {
        return write_timeout(std::chrono::duration_cast<microseconds>(to));
    }
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Template for creating specific stream types (IPv4, IPv6, etc).
 * This just overrides methods that take a generic address and replace them
 * with the address type for a specific family. This doesn't add any
 * runtime functionality, but has compile-time checks that address types
 * aren't accidentally being mixed for an object.
 */
template <typename ADDR>
class stream_socket_tmpl : public stream_socket
{
    /** The base class */
    using base = stream_socket;
    /** This class */
    using self = stream_socket_tmpl;

public:
    /** The address family for this type of address */
    static constexpr sa_family_t ADDRESS_FAMILY = ADDR::ADDRESS_FAMILY;
    /** The type of network address used with this socket. */
    using addr_t = ADDR;
    /** A pair of stream sockets */
    using socket_pair = std::tuple<stream_socket_tmpl, stream_socket_tmpl>;

    /**
     * Creates an unconnected streaming socket.
     */
    stream_socket_tmpl() {}
    /**
     * Creates a streaming socket from an existing OS socket handle and
     * claims ownership of the handle.
     * @param handle A socket handle from the operating system.
     */
    explicit stream_socket_tmpl(socket_t handle) : base(handle) {}
    /**
     * Move constructor.
     * Creates a stream socket by moving the other socket to this one.
     * @param sock Another stream socket.
     */
    stream_socket_tmpl(stream_socket&& sock) : base(std::move(sock)) {}
    /**
     * Creates a stream socket by copying the socket handle from the
     * specified socket object and transfers ownership of the socket.
     */
    stream_socket_tmpl(stream_socket_tmpl&& sock) : base(std::move(sock)) {}
    /**
     * Move assignment.
     * @param rhs The other socket to move into this one.
     * @return A reference to this object.
     */
    stream_socket_tmpl& operator=(stream_socket_tmpl&& rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }
    /**
     * Creates a stream socket.
     * @param protocol The particular protocol to be used with the sockets
     * @return A stream socket
     */
    stream_socket_tmpl create(int protocol = 0) {
        return stream_socket_tmpl(std::move(base::create(ADDRESS_FAMILY, protocol)));
    }
    /**
     * Creates a pair of connected stream sockets.
     *
     * Whether this will work at all is highly system and domain dependent.
     * Currently it is only known to work for Unix-domain sockets on *nix
     * systems.
     *
     * @param protocol The protocol to be used with the socket. (Normally 0)
     *
     * @return A pair (std::tuple) of stream sockets on success. An error
     *             code on failure.
     */
    static result<socket_pair> pair(int protocol = 0) {
        if (auto res = base::pair(ADDRESS_FAMILY, COMM_TYPE, protocol); !res) {
            return res.error();
        }
        else {
            auto [s1, s2] = res.release();
            return std::make_tuple<self, self>(self{s1.release()}, self{s2.release()});
        }
    }
    /**
     * Gets the local address to which the socket is bound.
     * @return The local address to which the socket is bound.
     */
    addr_t address() const { return addr_t(socket::address()); }
    /**
     * Gets the address of the remote peer, if this socket is connected.
     * @return The address of the remote peer, if this socket is connected.
     */
    addr_t peer_address() const { return addr_t(socket::peer_address()); }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace sockpp

#endif  // __sockpp_socket_h
