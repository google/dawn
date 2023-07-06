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

#include "dawn/utils/BinarySemaphore.h"

namespace dawn::utils {

void BinarySemaphore::Release() {
    std::lock_guard<std::mutex> lock(mMutex);
    mSignaled = true;
    mCv.notify_one();
}

void BinarySemaphore::Acquire() {
    std::unique_lock<std::mutex> lock(mMutex);
    while (!mSignaled) {
        mCv.wait(lock);
    }
    mSignaled = false;
}

}  // namespace dawn::utils
