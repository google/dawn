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

#include "dawn/native/d3d/Fence.h"

#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/D3DError.h"

namespace dawn::native::d3d {

Fence::Fence(UINT64 fenceValue, HANDLE sharedHandle)
    : mFenceValue(fenceValue), mSharedHandle(sharedHandle) {}

Fence::~Fence() {
    if (mSharedHandle != nullptr) {
        ::CloseHandle(mSharedHandle);
    }
}

UINT64 Fence::GetFenceValue() const {
    return mFenceValue;
}

}  // namespace dawn::native::d3d
