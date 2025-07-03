// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/DeviceGuard.h"

#include "dawn/common/Assert.h"
#include "dawn/native/Device.h"

namespace dawn::native {

namespace detail {

DeviceGuardBase::DeviceGuardBase(std::optional<class Defer>* defer, RecursiveMutex* mutex)
    : mMutex(mutex) {}

DeviceGuardBase::~DeviceGuardBase() {
    if (mDefer) {
        *mDefer = std::nullopt;
    }
}

}  // namespace detail

DeviceGuard::DeviceGuard(DeviceBase* device,
                         std::optional<class Defer>* defer,
                         RecursiveMutex* mutex)
    : detail::DeviceGuardBase(defer, mutex), GuardBase(device, device->mMutex) {
    DAWN_ASSERT(!mutex || mutex == device->mMutex);

    if (defer && !defer->has_value()) {
        // The first guard created in a thread also creates the defer, i.e. when the optional is
        // nullopt. We also need to set detail::DeviceGuardBase::mDefer here instead of in the base
        // constructor because we only want to mutate the device-owned state when we are holding the
        // lock which only happens after the GuardBase constructor is completed.
        mDefer = defer;
        mDefer->emplace();
    }
}

}  // namespace dawn::native
