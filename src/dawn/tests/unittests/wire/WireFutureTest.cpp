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

#include "dawn/tests/unittests/wire/WireFutureTest.h"

#include "dawn/common/Assert.h"

namespace dawn::wire {

WGPUCallbackMode ToWGPUCallbackMode(CallbackMode callbackMode) {
    switch (callbackMode) {
        case CallbackMode::WaitAny:
            return WGPUCallbackMode_WaitAnyOnly;
        case CallbackMode::ProcessEvents:
            return WGPUCallbackMode_AllowProcessEvents;
        case CallbackMode::Spontaneous:
            return WGPUCallbackMode_AllowSpontaneous;
        default:
            DAWN_UNREACHABLE();
    }
}

std::ostream& operator<<(std::ostream& os, const WireFutureTestParam& param) {
    switch (param.mCallbackMode) {
        case CallbackMode::Async:
            os << "Async";
            break;
        case CallbackMode::WaitAny:
            os << "WaitOnly";
            break;
        case CallbackMode::ProcessEvents:
            os << "ProcessEvents";
            break;
        case CallbackMode::Spontaneous:
            os << "Spontaneous";
            break;
    }
    return os;
}

}  // namespace dawn::wire
