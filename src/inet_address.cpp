// inet_address.cpp
//
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

#include "sockpp/inet_address.h"

#include "sockpp/error.h"

using namespace std;

namespace sockpp {

// --------------------------------------------------------------------------

inet_address::inet_address(uint32_t addr, in_port_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(addr);
    addr_.sin_port = htons(port);
#if defined(__APPLE__) || defined(BSD)
    addr_.sin_len = (uint8_t)SZ;
#endif
}

// --------------------------------------------------------------------------

inet_address::inet_address(const string& saddr, in_port_t port) {
    auto res = create(saddr, port);
    if (!res)
        throw system_error{res.error()};

    addr_ = res.value().addr_;
}

inet_address::inet_address(const string& saddr, in_port_t port, error_code& ec) noexcept {
    auto res = create(saddr, port);
    ec = res.error();

    if (res)
        addr_ = res.value().addr_;
}

// --------------------------------------------------------------------------

result<in_addr_t> inet_address::resolve_name(const string& saddr) noexcept {
#if !defined(_WIN32)
    in_addr ia;
    if (::inet_pton(ADDRESS_FAMILY, saddr.c_str(), &ia) == 1)
        return ia.s_addr;
#endif

    addrinfo *res, hints = addrinfo{};
    hints.ai_family = ADDRESS_FAMILY;
    hints.ai_socktype = SOCK_STREAM;

    int err = ::getaddrinfo(saddr.c_str(), NULL, &hints, &res);

    if (err != 0) {
        error_code ec{};
#if defined(_WIN32)
        ec = error_code{errno, system_category()};
#else
        if (err == EAI_SYSTEM)
            ec = result<>::last_error();
        else
            ec = make_error_code(static_cast<gai_errc>(err));
#endif
        return ec;
    }

    auto ipv4 = reinterpret_cast<sockaddr_in*>(res->ai_addr);
    auto addr = ipv4->sin_addr.s_addr;
    freeaddrinfo(res);
    return addr;
}

// --------------------------------------------------------------------------

result<inet_address> inet_address::create(const string& saddr, in_port_t port) noexcept {
    auto res = resolve_name(saddr.c_str());
    if (!res)
        return res.error();

    auto addr = sockaddr_in{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = res.value();
    addr.sin_port = htons(port);
#if defined(__APPLE__) || defined(BSD)
    addr.sin_len = (uint8_t)SZ;
#endif
    return inet_address{addr};
}

// --------------------------------------------------------------------------

string inet_address::to_string() const {
    char buf[INET_ADDRSTRLEN];
    auto str = inet_ntop(AF_INET, (void*)&(addr_.sin_addr), buf, INET_ADDRSTRLEN);
    return string(str ? str : "<unknown>") + ":" + std::to_string(unsigned(port()));
}

/////////////////////////////////////////////////////////////////////////////

ostream& operator<<(ostream& os, const inet_address& addr) {
    char buf[INET_ADDRSTRLEN];
    auto str =
        inet_ntop(AF_INET, (void*)&(addr.sockaddr_in_ptr()->sin_addr), buf, INET_ADDRSTRLEN);
    os << (str ? str : "<unknown>") << ":" << unsigned(addr.port());
    return os;
}

/////////////////////////////////////////////////////////////////////////////
// End namespace sockpp
}  // namespace sockpp
