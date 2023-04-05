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

#include "dawn/common/Mutex.h"

namespace dawn {
Mutex::~Mutex() = default;

void Mutex::Lock() {
#if defined(DAWN_ENABLE_ASSERTS)
    auto currentThread = std::this_thread::get_id();
    ASSERT(mOwner.load(std::memory_order_acquire) != currentThread);
#endif  // DAWN_ENABLE_ASSERTS

    mNativeMutex.lock();

#if defined(DAWN_ENABLE_ASSERTS)
    mOwner.store(currentThread, std::memory_order_release);
#endif  // DAWN_ENABLE_ASSERTS
}
void Mutex::Unlock() {
#if defined(DAWN_ENABLE_ASSERTS)
    ASSERT(IsLockedByCurrentThread());
    mOwner.store(std::thread::id(), std::memory_order_release);
#endif  // DAWN_ENABLE_ASSERTS

    mNativeMutex.unlock();
}

bool Mutex::IsLockedByCurrentThread() {
#if defined(DAWN_ENABLE_ASSERTS)
    return mOwner.load(std::memory_order_acquire) == std::this_thread::get_id();
#else
    // This is not supported.
    abort();
#endif
}
}  // namespace dawn
