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

#include "dawn/common/FutureUtils.h"

#include "dawn/common/Assert.h"

namespace dawn {

// TODO(crbug.com/dawn/2052) Remove this when we use an enum instead of a bitmask.
CallbackMode ValidateAndFlattenCallbackMode(WGPUCallbackModeFlags mode) {
    switch (mode) {
        case WGPUCallbackMode_Spontaneous:
            return CallbackMode::Spontaneous;
        case WGPUCallbackMode_Future:
            return CallbackMode::Future;
        case WGPUCallbackMode_Future | WGPUCallbackMode_Spontaneous:
            return CallbackMode::FutureOrSpontaneous;
        case WGPUCallbackMode_ProcessEvents:
            return CallbackMode::ProcessEvents;
        case WGPUCallbackMode_ProcessEvents | WGPUCallbackMode_Spontaneous:
            return CallbackMode::ProcessEventsOrSpontaneous;
        default:
            // These cases are undefined behaviors according to the API contract.
            DAWN_ASSERT(false);
            return CallbackMode::Spontaneous;
    }
}

}  // namespace dawn
