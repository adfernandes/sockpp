// unecho.cpp
//
// Simple Unix-domain echo client
//
// --------------------------------------------------------------------------
// This file is part of the "sockpp" C++ socket library.
//
// Copyright (c) 2014-2017 Frank Pagliughi
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

#include <iostream>
#include <string>

#include "sockpp/unix_connector.h"
#include "sockpp/version.h"

using namespace std;

// --------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    cout << "Sample Unix-domain echo client for 'sockpp' " << sockpp::SOCKPP_VERSION << '\n'
         << endl;

#if defined(_WIN32)
    const string DFLT_PATH = "C:\\TEMP\\unechosvr.sock"s;
#else
    string DFLT_PATH = "/tmp/unechosvr.sock"s;
#endif

    const string path = (argc > 1) ? argv[1] : DFLT_PATH;

    sockpp::initialize();
    sockpp::unix_connector conn;

    if (auto res = conn.connect(sockpp::unix_address(path)); !res) {
        cerr << "Error connecting to UNIX socket at " << path << "\n\t" << res.error_message()
             << endl;
        return 1;
    }

    cout << "Created a connection to '" << conn.peer_address() << "'" << endl;

    string s, sret;
    while (getline(cin, s) && !s.empty()) {
        const size_t N = s.length();

        if (auto res = conn.write(s); !res || res != N) {
            cerr << "Error writing to the UNIX stream" << endl;
            break;
        }

        sret.resize(N);
        if (auto res = conn.read_n(&sret[0], N); !res || res != N) {
            cerr << "Error reading from UNIX stream" << endl;
            break;
        }

        cout << sret << endl;
    }

    return (!conn) ? 1 : 0;
}
