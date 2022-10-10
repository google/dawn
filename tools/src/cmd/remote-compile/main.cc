// Copyright 2021 The Tint Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "tools/src/cmd/remote-compile/compile.h"
#include "tools/src/cmd/remote-compile/socket.h"

namespace {

#if 0
#define DEBUG(msg, ...) printf(msg "\n", ##__VA_ARGS__)
#else
#define DEBUG(...)
#endif

/// Print the tool usage, and exit with 1.
void ShowUsage() {
    const char* name = "tint-remote-compile";
    printf(R"(%s is a tool for compiling a shader on a remote machine

usage as server:
  %s -s [-p port-number]

usage as client:
  %s [-p port-number] [server-address] shader-file-path

  [server-address] can be omitted if the TINT_REMOTE_COMPILE_ADDRESS environment
  variable is set.
  Alternatively, you can pass xcrun arguments so %s can be used as a
  drop-in replacement.
)",
           name, name, name, name);
    exit(1);
}

/// The protocol version code. Bump each time the protocol changes
constexpr uint32_t kProtocolVersion = 1;

/// Supported shader source languages
enum SourceLanguage {
    MSL,
};

/// Stream is a serialization wrapper around a socket
struct Stream {
    /// The underlying socket
    Socket* const socket;
    /// Error state
    std::string error;

    /// Writes a uint32_t to the socket
    Stream operator<<(uint32_t v) {
        if (error.empty()) {
            Write(&v, sizeof(v));
        }
        return *this;
    }

    /// Reads a uint32_t from the socket
    Stream operator>>(uint32_t& v) {
        if (error.empty()) {
            Read(&v, sizeof(v));
        }
        return *this;
    }

    /// Writes a std::string to the socket
    Stream operator<<(const std::string& v) {
        if (error.empty()) {
            uint32_t count = static_cast<uint32_t>(v.size());
            *this << count;
            if (count) {
                Write(v.data(), count);
            }
        }
        return *this;
    }

    /// Reads a std::string from the socket
    Stream operator>>(std::string& v) {
        uint32_t count = 0;
        *this >> count;
        if (count) {
            std::vector<char> buf(count);
            if (Read(buf.data(), count)) {
                v = std::string(buf.data(), buf.size());
            }
        } else {
            v.clear();
        }
        return *this;
    }

    /// Writes an enum value to the socket
    template <typename T>
    std::enable_if_t<std::is_enum<T>::value, Stream> operator<<(T e) {
        return *this << static_cast<uint32_t>(e);
    }

    /// Reads an enum value from the socket
    template <typename T>
    std::enable_if_t<std::is_enum<T>::value, Stream> operator>>(T& e) {
        uint32_t v;
        *this >> v;
        e = static_cast<T>(v);
        return *this;
    }

  private:
    bool Write(const void* data, size_t size) {
        if (error.empty()) {
            if (!socket->Write(data, size)) {
                error = "Socket::Write() failed";
            }
        }
        return error.empty();
    }

    bool Read(void* data, size_t size) {
        auto buf = reinterpret_cast<uint8_t*>(data);
        while (size > 0 && error.empty()) {
            if (auto n = socket->Read(buf, size)) {
                if (n > size) {
                    error = "Socket::Read() returned more bytes than requested";
                    return false;
                }
                size -= n;
                buf += n;
            } else {
                error = "Socket::Read() failed";
            }
        }
        return error.empty();
    }
};

////////////////////////////////////////////////////////////////////////////////
// Messages
////////////////////////////////////////////////////////////////////////////////

/// Base class for all messages
struct Message {
    /// The type of the message
    enum class Type {
        ConnectionRequest,
        ConnectionResponse,
        CompileRequest,
        CompileResponse,
    };

    explicit Message(Type ty) : type(ty) {}

    const Type type;
};

struct ConnectionResponse : Message {  // Server -> Client
    ConnectionResponse() : Message(Type::ConnectionResponse) {}

    template <typename T>
    void Serialize(T&& f) {
        f(error);
    }

    std::string error;
};

struct ConnectionRequest : Message {  // Client -> Server
    using Response = ConnectionResponse;

    explicit ConnectionRequest(uint32_t proto_ver = kProtocolVersion)
        : Message(Type::ConnectionRequest), protocol_version(proto_ver) {}

    template <typename T>
    void Serialize(T&& f) {
        f(protocol_version);
    }

    uint32_t protocol_version;
};

struct CompileResponse : Message {  //  Server -> Client
    CompileResponse() : Message(Type::CompileResponse) {}

    template <typename T>
    void Serialize(T&& f) {
        f(error);
    }

    std::string error;
};

struct CompileRequest : Message {  // Client -> Server
    using Response = CompileResponse;

    CompileRequest() : Message(Type::CompileRequest) {}
    CompileRequest(SourceLanguage lang, std::string src)
        : Message(Type::CompileRequest), language(lang), source(src) {}

    template <typename T>
    void Serialize(T&& f) {
        f(language);
        f(source);
    }

    SourceLanguage language;
    std::string source;
};

/// Writes the message `m` to the stream `s`
template <typename MESSAGE>
std::enable_if_t<std::is_base_of<Message, MESSAGE>::value, Stream>& operator<<(Stream& s,
                                                                               const MESSAGE& m) {
    s << m.type;
    const_cast<MESSAGE&>(m).Serialize([&s](const auto& value) { s << value; });
    return s;
}

/// Reads the message `m` from the stream `s`
template <typename MESSAGE>
std::enable_if_t<std::is_base_of<Message, MESSAGE>::value, Stream>& operator>>(Stream& s,
                                                                               MESSAGE& m) {
    Message::Type ty;
    s >> ty;
    if (s.error.empty()) {
        if (ty == m.type) {
            m.Serialize([&s](auto& value) { s >> value; });
        } else {
            std::stringstream ss;
            ss << "expected message type " << static_cast<int>(m.type) << ", got "
               << static_cast<int>(ty);
            s.error = ss.str();
        }
    }
    return s;
}

/// Writes the request message `req` to the stream `s`, then reads and returns
/// the response message from the same stream.
template <typename REQUEST, typename RESPONSE = typename REQUEST::Response>
RESPONSE Send(Stream& s, const REQUEST& req) {
    s << req;
    if (s.error.empty()) {
        RESPONSE resp;
        s >> resp;
        if (s.error.empty()) {
            return resp;
        }
    }
    return {};
}

}  // namespace

bool RunServer(std::string port);
bool RunClient(std::string address, std::string port, std::string file);

int main(int argc, char* argv[]) {
    bool run_server = false;
    std::string port = "19000";

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-s" || arg == "--server") {
            run_server = true;
            continue;
        }
        if (arg == "-p" || arg == "--port") {
            if (i < argc - 1) {
                i++;
                port = argv[i];
            } else {
                printf("expected port number");
                exit(1);
            }
            continue;
        }

        // xcrun flags are ignored so this executable can be used as a replacement for xcrun.
        if ((arg == "-x" || arg == "-sdk") && (i < argc - 1)) {
            i++;
            continue;
        }
        if (arg == "metal") {
            for (; i < argc; i++) {
                if (std::string(argv[i]) == "-c") {
                    break;
                }
            }
            continue;
        }

        args.emplace_back(arg);
    }

    bool success = false;

    if (run_server) {
        success = RunServer(port);
    } else {
        std::string address;
        std::string file;
        switch (args.size()) {
            case 1:
                if (auto* addr = getenv("TINT_REMOTE_COMPILE_ADDRESS")) {
                    address = addr;
                }
                file = args[0];
                break;
            case 2:
                address = args[0];
                file = args[1];
                break;
        }
        if (address.empty() || file.empty()) {
            ShowUsage();
        }
        success = RunClient(address, port, file);
    }

    if (!success) {
        exit(1);
    }

    return 0;
}

bool RunServer(std::string port) {
    auto server_socket = Socket::Listen("", port.c_str());
    if (!server_socket) {
        printf("Failed to listen on port %s\n", port.c_str());
        return false;
    }
    printf("Listening on port %s...\n", port.c_str());
    while (auto conn = server_socket->Accept()) {
        std::thread([=] {
            DEBUG("Client connected...");
            Stream stream{conn.get()};

            {
                ConnectionRequest req;
                stream >> req;
                if (!stream.error.empty()) {
                    DEBUG("%s", stream.error.c_str());
                    return;
                }
                ConnectionResponse resp;
                if (req.protocol_version != kProtocolVersion) {
                    DEBUG("Protocol version mismatch");
                    resp.error = "Protocol version mismatch";
                    stream << resp;
                    return;
                }
                stream << resp;
            }
            DEBUG("Connection established");
            {
                CompileRequest req;
                stream >> req;
                if (!stream.error.empty()) {
                    DEBUG("%s\n", stream.error.c_str());
                    return;
                }
#ifdef TINT_ENABLE_MSL_COMPILATION_USING_METAL_API
                if (req.language == SourceLanguage::MSL) {
                    auto result = CompileMslUsingMetalAPI(req.source);
                    CompileResponse resp;
                    if (!result.success) {
                        resp.error = result.output;
                    }
                    stream << resp;
                    return;
                }
#endif
                CompileResponse resp;
                resp.error = "server cannot compile this type of shader";
                stream << resp;
            }
        }).detach();
    }
    return true;
}

bool RunClient(std::string address, std::string port, std::string file) {
    // Read the file
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        printf("Couldn't open '%s'\n", file.c_str());
        return false;
    }
    std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    constexpr const int timeout_ms = 10000;
    DEBUG("Connecting to %s:%s...", address.c_str(), port.c_str());
    auto conn = Socket::Connect(address.c_str(), port.c_str(), timeout_ms);
    if (!conn) {
        printf("Connection failed\n");
        return false;
    }

    Stream stream{conn.get()};

    DEBUG("Sending connection request...");
    auto conn_resp = Send(stream, ConnectionRequest{kProtocolVersion});
    if (!stream.error.empty()) {
        printf("%s\n", stream.error.c_str());
        return false;
    }
    if (!conn_resp.error.empty()) {
        printf("%s\n", conn_resp.error.c_str());
        return false;
    }
    DEBUG("Connection established. Requesting compile...");
    auto comp_resp = Send(stream, CompileRequest{SourceLanguage::MSL, source});
    if (!stream.error.empty()) {
        printf("%s\n", stream.error.c_str());
        return false;
    }
    if (!comp_resp.error.empty()) {
        printf("%s\n", comp_resp.error.c_str());
        return false;
    }
    DEBUG("Compilation successful");
    return true;
}
