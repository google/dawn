// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_WAITANYSYSTEMEVENT_H_
#define SRC_DAWN_NATIVE_WAITANYSYSTEMEVENT_H_

#include <limits>
#include <utility>

#include "src/utils/platform.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "src/utils/windows_with_undefs.h"
#endif

#include "src/dawn/native/SystemEvent.h"
#include "src/utils/span.h"

namespace dawn::native {

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
#define ToMilliseconds ToMillisecondsGeneric<DWORD, INFINITE>
#elif DAWN_PLATFORM_IS(POSIX)
#define ToMilliseconds ToMillisecondsGeneric<int, -1>
#endif

// WaitAnySystemEvent on an iterator range converts those iterators to the platform-specific wait
// handles, and then waits on them.
[[nodiscard]] bool WaitAnySystemEvent(Span<std::pair<const SystemEventReceiver&, bool*>> events,
                                      Nanoseconds timeout);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_WAITANYSYSTEMEVENT_H_
