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

#ifndef SRC_DAWN_NATIVE_SYSTEMEVENT_H_
#define SRC_DAWN_NATIVE_SYSTEMEVENT_H_

#include <utility>

#include "dawn/common/NonCopyable.h"
#include "dawn/common/Platform.h"
#include "dawn/native/IntegerTypes.h"

namespace dawn::native {

struct TrackedFutureWaitInfo;
class SystemEventPipeSender;

// Either a Win32 HANDLE or a POSIX fd (int) depending on OS, represented as a uintptr_t with
// necessary conversions.
class SystemEventPrimitive : NonCopyable {
  public:
    SystemEventPrimitive() = default;
    // void* is the typedef of HANDLE in Win32.
    explicit SystemEventPrimitive(void* win32Handle);
    explicit SystemEventPrimitive(int posixFd);
    ~SystemEventPrimitive();

    SystemEventPrimitive(SystemEventPrimitive&&);
    SystemEventPrimitive& operator=(SystemEventPrimitive&&);

    bool IsValid() const;
    void Close();

    static constexpr uintptr_t kInvalid = 0;
    // The underlying primitive, either a Win32 HANDLE (void*) or a POSIX fd (int), cast to
    // uintptr_t. We treat 0 as the "invalid" value, even for POSIX.
    uintptr_t value = kInvalid;
};

// SystemEventReceiver holds an OS event primitive (Win32 Event Object or POSIX file descriptor (fd)
// that will be signalled by some other thing: either an OS integration like SetEventOnCompletion(),
// or our own code like SystemEventPipeSender.
//
// SystemEventReceiver is one-time-use (to make it easier to use correctly) - once it's been
// signalled, it won't ever get reset (become unsignalled). Instead, if we want to reuse underlying
// OS objects, it should be reset and recycled *after* the SystemEventReceiver and
// SystemEventPipeSender have been destroyed.
class SystemEventReceiver final : NonCopyable {
  public:
    static SystemEventReceiver CreateAlreadySignaled();

    SystemEventReceiver() = default;
    SystemEventReceiver(SystemEventReceiver&&) = default;
    SystemEventReceiver& operator=(SystemEventReceiver&&) = default;

  private:
    friend bool WaitAnySystemEvent(size_t, TrackedFutureWaitInfo*, Nanoseconds);
    friend std::pair<SystemEventPipeSender, SystemEventReceiver> CreateSystemEventPipe();
    SystemEventPrimitive mPrimitive;
};

// See CreateSystemEventPipe.
class SystemEventPipeSender final : NonCopyable {
  public:
    SystemEventPipeSender() = default;
    SystemEventPipeSender(SystemEventPipeSender&&) = default;
    SystemEventPipeSender& operator=(SystemEventPipeSender&&) = default;
    ~SystemEventPipeSender();

    void Signal() &&;

  private:
    friend std::pair<SystemEventPipeSender, SystemEventReceiver> CreateSystemEventPipe();
    SystemEventPrimitive mPrimitive;
};

// Implementation of WaitAny when backed by SystemEventReceiver.
// Returns true if some future is now ready, false if not (it timed out).
[[nodiscard]] bool WaitAnySystemEvent(size_t count,
                                      TrackedFutureWaitInfo* futures,
                                      Nanoseconds timeout);

// CreateSystemEventPipe provides an SystemEventReceiver that can be signalled by Dawn code. This is
// useful for queue completions on Metal (where Metal signals us by calling a callback) and for
// async pipeline creations that happen in a worker-thread task.
//
// We use OS events even for these because, unlike C++/pthreads primitives (mutexes, atomics,
// condvars, etc.), it's possible to wait-any on them (wait for any of a list of events to fire).
// Other use-cases in Dawn that don't require wait-any should generally use C++ primitives, for
// example for signalling the completion of other types of worker-thread work that don't need to
// signal a WGPUFuture.
//
// SystemEventReceiver is one-time-use (see its docs), so there's no way to reset an
// SystemEventPipeSender.
//
// - On Windows, SystemEventReceiver is a Win32 Event Object, so we can create one with
//   CreateEvent() and signal it with SetEvent().
// - On POSIX, SystemEventReceiver is a file descriptor (fd), so we can create one with pipe(), and
//   signal it by write()ing into the pipe (to make it become readable, though we won't read() it).
std::pair<SystemEventPipeSender, SystemEventReceiver> CreateSystemEventPipe();

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SYSTEMEVENT_H_
