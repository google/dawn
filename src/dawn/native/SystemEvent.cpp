// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/native/SystemEvent.h"
#include <limits>
#include "dawn/common/Assert.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include <windows.h>
#elif DAWN_PLATFORM_IS(POSIX)
#include <sys/poll.h>
#include <unistd.h>
#endif

#include <tuple>
#include <utility>
#include <vector>

#include "dawn/native/EventManager.h"

namespace dawn::native {

namespace {

template <typename T, T Infinity>
T ToMillisecondsGeneric(Nanoseconds timeout) {
    uint64_t ns = uint64_t{timeout};
    uint64_t ms = 0;
    if (ns > 0) {
        ms = (ns - 1) / 1'000'000 + 1;
        if (ms > std::numeric_limits<T>::max()) {
            return Infinity;  // Round long timeout up to infinity
        }
    }
    return static_cast<T>(ms);
}

#if DAWN_PLATFORM_IS(WINDOWS)
// #define ToMilliseconds ToMillisecondsGeneric<DWORD, INFINITE>
#elif DAWN_PLATFORM_IS(POSIX)
#define ToMilliseconds ToMillisecondsGeneric<int, -1>
#endif

#if DAWN_PLATFORM_IS(WINDOWS)
HANDLE AsHANDLE(const SystemEventPrimitive& p) {
    return reinterpret_cast<HANDLE>(p.value);
}
#elif DAWN_PLATFORM_IS(POSIX)
int AsFD(const SystemEventPrimitive& p) {
    DAWN_ASSERT(p.value <= std::numeric_limits<int>::max());
    return p.value;
}
#endif

}  // namespace

// SystemEventPrimitive

SystemEventPrimitive::SystemEventPrimitive(void* win32Handle)
    : value(reinterpret_cast<uintptr_t>(win32Handle)) {
#if DAWN_PLATFORM_IS(WINDOWS)
    static_assert(std::is_same_v<void*, HANDLE>);
    DAWN_ASSERT(win32Handle != nullptr);
#else
    DAWN_ASSERT(false);  // Wrong platform.
#endif
}

SystemEventPrimitive::SystemEventPrimitive(int posixFd) : value(posixFd) {
#if DAWN_PLATFORM_IS(POSIX)
    static_assert(sizeof(uintptr_t) >= sizeof(int));
    DAWN_ASSERT(posixFd > 0);
#else
    DAWN_ASSERT(false);  // Wrong platform.
#endif
}

SystemEventPrimitive::~SystemEventPrimitive() {
    if (IsValid()) {
        Close();
    }
}

SystemEventPrimitive::SystemEventPrimitive(SystemEventPrimitive&& rhs) {
    *this = std::move(rhs);
}

SystemEventPrimitive& SystemEventPrimitive::operator=(SystemEventPrimitive&& rhs) {
    if (this != &rhs) {
        if (IsValid()) {
            Close();
        }
        std::swap(value, rhs.value);
    }
    return *this;
}

bool SystemEventPrimitive::IsValid() const {
    return value != kInvalid;
}

void SystemEventPrimitive::Close() {
    DAWN_ASSERT(IsValid());

#if DAWN_PLATFORM_IS(WINDOWS)
    CloseHandle(AsHANDLE(*this));
#elif DAWN_PLATFORM_IS(POSIX)
    close(AsFD(*this));
#else
    DAWN_CHECK(false);  // Not implemented.
#endif

    value = kInvalid;
}

// SystemEventReceiver

SystemEventReceiver SystemEventReceiver::CreateAlreadySignaled() {
    SystemEventPipeSender sender;
    SystemEventReceiver receiver;
    std::tie(sender, receiver) = CreateSystemEventPipe();
    std::move(sender).Signal();
    return receiver;
}

// SystemEventPipeSender

SystemEventPipeSender::~SystemEventPipeSender() {
    // Make sure it's been Signaled (or is empty) before being dropped.
    // Dropping this would "leak" the receiver (it'll never get signalled).
    DAWN_ASSERT(!mPrimitive.IsValid());
}

void SystemEventPipeSender::Signal() && {
    DAWN_ASSERT(mPrimitive.IsValid());
#if DAWN_PLATFORM_IS(WINDOWS)
    // This is not needed on Windows yet. It's implementable using SetEvent().
    DAWN_UNREACHABLE();
#elif DAWN_PLATFORM_IS(POSIX)
    // Send one byte to signal the receiver
    char zero[1] = {0};
    int status = write(AsFD(mPrimitive), zero, 1);
    DAWN_CHECK(status >= 0);
#else
    // Not implemented for this platform.
    DAWN_CHECK(false);
#endif

    mPrimitive.Close();
}

// standalone functions

bool WaitAnySystemEvent(size_t count, TrackedFutureWaitInfo* futures, Nanoseconds timeout) {
#if DAWN_PLATFORM_IS(WINDOWS)
    // TODO(crbug.com/dawn/2054): Implement this.
    DAWN_CHECK(false);
#elif DAWN_PLATFORM_IS(POSIX)
    std::vector<pollfd> pollfds(count);
    for (size_t i = 0; i < count; ++i) {
        int fd = AsFD(futures[i].event->GetReceiver().mPrimitive);
        pollfds[i] = pollfd{fd, POLLIN, 0};
    }

    int status = poll(pollfds.data(), pollfds.size(), ToMilliseconds(timeout));

    DAWN_CHECK(status >= 0);
    if (status == 0) {
        return false;
    }

    for (size_t i = 0; i < count; ++i) {
        int revents = pollfds[i].revents;
        static constexpr int kAllowedEvents = POLLIN | POLLHUP;
        DAWN_CHECK((revents & kAllowedEvents) == revents);
    }

    for (size_t i = 0; i < count; ++i) {
        bool ready = (pollfds[i].revents & POLLIN) != 0;
        futures[i].ready = ready;
    }

    return true;
#else
    DAWN_CHECK(false);  // Not implemented.
#endif
}

std::pair<SystemEventPipeSender, SystemEventReceiver> CreateSystemEventPipe() {
#if DAWN_PLATFORM_IS(WINDOWS)
    // This is not needed on Windows yet. It's implementable using CreateEvent().
    DAWN_UNREACHABLE();
#elif DAWN_PLATFORM_IS(POSIX)
    int pipeFds[2];
    int status = pipe(pipeFds);
    DAWN_CHECK(status >= 0);

    SystemEventReceiver receiver;
    receiver.mPrimitive = SystemEventPrimitive{pipeFds[0]};

    SystemEventPipeSender sender;
    sender.mPrimitive = SystemEventPrimitive{pipeFds[1]};

    return std::make_pair(std::move(sender), std::move(receiver));
#else
    // Not implemented for this platform.
    DAWN_CHECK(false);
#endif
}

}  // namespace dawn::native
