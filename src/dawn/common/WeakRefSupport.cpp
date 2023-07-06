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

#include "dawn/common/WeakRefSupport.h"

#include <utility>

namespace dawn::detail {

WeakRefData::WeakRefData(RefCounted* value) : mValue(value) {}

Ref<RefCounted> WeakRefData::TryGetRef() {
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mValue || !mValue->mRefCount.TryIncrement()) {
        return nullptr;
    }
    return AcquireRef(mValue);
}

void WeakRefData::Invalidate() {
    std::lock_guard<std::mutex> lock(mMutex);
    mValue = nullptr;
}

WeakRefSupportBase::WeakRefSupportBase(Ref<detail::WeakRefData> data) : mData(std::move(data)) {}

WeakRefSupportBase::~WeakRefSupportBase() {
    mData->Invalidate();
}

}  // namespace dawn::detail
