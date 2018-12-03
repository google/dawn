// Copyright 2018 The Dawn Authors
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

#include "dawn_native/Fence.h"

#include "common/Assert.h"
#include "dawn_native/Device.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <cstdio>
#include <utility>

namespace dawn_native {

    MaybeError ValidateFenceDescriptor(DeviceBase*, const FenceDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        return {};
    }

    // Fence

    FenceBase::FenceBase(DeviceBase* device, const FenceDescriptor* descriptor)
        : ObjectBase(device),
          mSignalValue(descriptor->initialValue),
          mCompletedValue(descriptor->initialValue) {
    }

    FenceBase::~FenceBase() {
        for (auto& request : mRequests.IterateAll()) {
            request.completionCallback(DAWN_FENCE_COMPLETION_STATUS_UNKNOWN, request.userdata);
        }
        mRequests.Clear();
    }

    uint64_t FenceBase::GetCompletedValue() const {
        return mCompletedValue;
    }

    void FenceBase::OnCompletion(uint64_t value,
                                 dawn::FenceOnCompletionCallback callback,
                                 dawn::CallbackUserdata userdata) {
        if (GetDevice()->ConsumedError(ValidateOnCompletion(value))) {
            callback(DAWN_FENCE_COMPLETION_STATUS_ERROR, userdata);
            return;
        }

        if (value <= mCompletedValue) {
            callback(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata);
            return;
        }

        OnCompletionData request;
        request.completionCallback = callback;
        request.userdata = userdata;
        mRequests.Enqueue(std::move(request), value);
    }

    uint64_t FenceBase::GetSignaledValue() const {
        return mSignalValue;
    }

    void FenceBase::SetSignaledValue(uint64_t signalValue) {
        ASSERT(signalValue > mSignalValue);
        mSignalValue = signalValue;
    }

    void FenceBase::SetCompletedValue(uint64_t completedValue) {
        ASSERT(completedValue <= mSignalValue);
        ASSERT(completedValue > mCompletedValue);
        mCompletedValue = completedValue;

        for (auto& request : mRequests.IterateUpTo(mCompletedValue)) {
            request.completionCallback(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, request.userdata);
        }
        mRequests.ClearUpTo(mCompletedValue);
    }

    MaybeError FenceBase::ValidateOnCompletion(uint64_t value) const {
        if (value > mSignalValue) {
            return DAWN_VALIDATION_ERROR("Value greater than fence signaled value");
        }
        return {};
    }

}  // namespace dawn_native
