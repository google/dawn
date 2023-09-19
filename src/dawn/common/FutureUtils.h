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

#ifndef SRC_DAWN_COMMON_FUTUREUTILS_H_
#define SRC_DAWN_COMMON_FUTUREUTILS_H_

#include <cstddef>
#include <cstdint>

#include "dawn/webgpu.h"

namespace dawn {

using FutureID = uint64_t;
constexpr FutureID kNullFutureID = 0;

constexpr size_t kTimedWaitAnyMaxCountDefault = 64;

enum class EventCompletionType {
    // The event is completing because it became ready.
    Ready,
    // The event is completing because the instance is shutting down.
    Shutdown,
};

// Flattened version of the wgpu::CallbackMode flags.
// TODO(crbug.com/dawn/2052) Remove when API changes to use an enum instead of flags.
enum class [[nodiscard]] CallbackMode {
    Spontaneous,
    Future,
    FutureOrSpontaneous,
    ProcessEvents,
    ProcessEventsOrSpontaneous,
};
CallbackMode ValidateAndFlattenCallbackMode(WGPUCallbackModeFlags mode);

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_FUTUREUTILS_H_
