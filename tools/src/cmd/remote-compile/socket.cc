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

#include "tools/src/cmd/remote-compile/socket.h"

#include "tools/src/cmd/remote-compile/rwmutex.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <atomic>
namespace {
std::atomic<int> wsa_init_count = {0};
}  // anonymous namespace
#else
#include <fcntl.h>
namespace {
using SOCKET = int;
}  // anonymous namespace
#endif

namespace {
constexpr SOCKET InvalidSocket = static_cast<SOCKET>(-1);
void Init() {
#if defined(_WIN32)
    if (wsa_init_count++ == 0) {
        WSADATA winsock_data;
        (void)WSAStartup(MAKEWORD(2, 2), &winsock_data);
    }
#endif
}

void Term() {
#if defined(_WIN32)
    if (--wsa_init_count == 0) {
        WSACleanup();
    }
#endif
}

bool SetBlocking(SOCKET s, bool blocking) {
#if defined(_WIN32)
    u_long mode = blocking ? 0 : 1;
    return ioctlsocket(s, FIONBIO, &mode) == NO_ERROR;
#else
    auto arg = fcntl(s, F_GETFL, nullptr);
    if (arg < 0) {
        return false;
    }
    arg = blocking ? (arg & ~O_NONBLOCK) : (arg | O_NONBLOCK);
    return fcntl(s, F_SETFL, arg) >= 0;
#endif
}

bool Errored(SOCKET s) {
    if (s == InvalidSocket) {
        return true;
    }
    char error = 0;
    socklen_t len = sizeof(error);
    getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len);
    return error != 0;
}

class Impl : public Socket {
  public:
    static std::shared_ptr<Impl> create(const char* address, const char* port) {
        Init();

        addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        addrinfo* info = nullptr;
        auto err = getaddrinfo(address, port, &hints, &info);
#if !defined(_WIN32)
        if (err) {
            printf("getaddrinfo(%s, %s) error: %s\n", address, port, gai_strerror(err));
        }
#endif

        if (info) {
            auto socket = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
            auto out = std::make_shared<Impl>(info, socket);
            out->SetOptions();
            return out;
        }

        freeaddrinfo(info);
        Term();
        return nullptr;
    }

    explicit Impl(SOCKET socket) : info(nullptr), s(socket) {}
    Impl(addrinfo* info, SOCKET socket) : info(info), s(socket) {}

    ~Impl() {
        freeaddrinfo(info);
        Close();
        Term();
    }

    template <typename FUNCTION>
    void Lock(FUNCTION&& f) {
        RLock l(mutex);
        f(s, info);
    }

    void SetOptions() {
        RLock l(mutex);
        if (s == InvalidSocket) {
            return;
        }

        int enable = 1;

#if !defined(_WIN32)
        // Prevent sockets lingering after process termination, causing
        // reconnection issues on the same port.
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&enable), sizeof(enable));

        struct {
            int l_onoff;  /* linger active */
            int l_linger; /* how many seconds to linger for */
        } linger = {false, 0};
        setsockopt(s, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof(linger));
#endif  // !defined(_WIN32)

        // Enable TCP_NODELAY.
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&enable), sizeof(enable));
    }

    bool IsOpen() override {
        {
            RLock l(mutex);
            if ((s != InvalidSocket) && !Errored(s)) {
                return true;
            }
        }
        WLock lock(mutex);
        s = InvalidSocket;
        return false;
    }

    void Close() override {
        {
            RLock l(mutex);
            if (s != InvalidSocket) {
#if defined(_WIN32)
                closesocket(s);
#else
                ::shutdown(s, SHUT_RDWR);
#endif
            }
        }

        WLock l(mutex);
        if (s != InvalidSocket) {
#if !defined(_WIN32)
            ::close(s);
#endif
            s = InvalidSocket;
        }
    }

    size_t Read(void* buffer, size_t bytes) override {
        {
            RLock lock(mutex);
            if (s == InvalidSocket) {
                return 0;
            }
            size_t len = recv(s, reinterpret_cast<char*>(buffer), static_cast<int>(bytes), 0);
            if (len > 0) {
                return len;
            }
        }
        // Socket closed or errored
        WLock lock(mutex);
        s = InvalidSocket;
        return 0;
    }

    bool Write(const void* buffer, size_t bytes) override {
        RLock lock(mutex);
        if (s == InvalidSocket) {
            return false;
        }
        if (bytes == 0) {
            return true;
        }
        return ::send(s, reinterpret_cast<const char*>(buffer), static_cast<int>(bytes), 0) > 0;
    }

    std::shared_ptr<Socket> Accept() override {
        std::shared_ptr<Impl> out;
        Lock([&](SOCKET socket, const addrinfo*) {
            if (socket != InvalidSocket) {
                Init();
                if (auto s = ::accept(socket, 0, 0); s >= 0) {
                    out = std::make_shared<Impl>(s);
                    out->SetOptions();
                }
            }
        });
        return out;
    }

  private:
    addrinfo* const info;
    SOCKET s = InvalidSocket;
    RWMutex mutex;
};

}  // anonymous namespace

std::shared_ptr<Socket> Socket::Listen(const char* address, const char* port) {
    auto impl = Impl::create(address, port);
    if (!impl) {
        return nullptr;
    }
    impl->Lock([&](SOCKET socket, const addrinfo* info) {
        if (bind(socket, info->ai_addr, static_cast<int>(info->ai_addrlen)) != 0) {
            impl.reset();
            return;
        }

        if (listen(socket, 0) != 0) {
            impl.reset();
            return;
        }
    });
    return impl;
}

std::shared_ptr<Socket> Socket::Connect(const char* address,
                                        const char* port,
                                        uint32_t timeout_ms) {
    auto impl = Impl::create(address, port);
    if (!impl) {
        return nullptr;
    }

    std::shared_ptr<Socket> out;
    impl->Lock([&](SOCKET socket, const addrinfo* info) {
        if (socket == InvalidSocket) {
            return;
        }

        if (timeout_ms == 0) {
            if (::connect(socket, info->ai_addr, static_cast<int>(info->ai_addrlen)) == 0) {
                out = impl;
            }
            return;
        }

        if (!SetBlocking(socket, false)) {
            return;
        }

        auto res = ::connect(socket, info->ai_addr, static_cast<int>(info->ai_addrlen));
        if (res == 0) {
            if (SetBlocking(socket, true)) {
                out = impl;
            }
        } else {
            const auto timeout_us = timeout_ms * 1000;

            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(socket, &fdset);

            timeval tv;
            tv.tv_sec = timeout_us / 1000000;
            tv.tv_usec = timeout_us - static_cast<uint32_t>(tv.tv_sec * 1000000);
            res = select(static_cast<int>(socket + 1), nullptr, &fdset, nullptr, &tv);
            if (res > 0 && !Errored(socket) && SetBlocking(socket, true)) {
                out = impl;
            }
        }
    });

    if (!out) {
        return nullptr;
    }

    return out->IsOpen() ? out : nullptr;
}
