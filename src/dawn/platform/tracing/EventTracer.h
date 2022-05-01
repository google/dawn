// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_PLATFORM_TRACING_EVENTTRACER_H_
#define SRC_DAWN_PLATFORM_TRACING_EVENTTRACER_H_

#include <cstdint>

#include "dawn/platform/dawn_platform_export.h"

namespace dawn::platform {

class Platform;
enum class TraceCategory;

namespace tracing {

using TraceEventHandle = uint64_t;

DAWN_PLATFORM_EXPORT const unsigned char* GetTraceCategoryEnabledFlag(Platform* platform,
                                                                      TraceCategory category);

// TODO(enga): Simplify this API.
DAWN_PLATFORM_EXPORT TraceEventHandle AddTraceEvent(Platform* platform,
                                                    char phase,
                                                    const unsigned char* categoryGroupEnabled,
                                                    const char* name,
                                                    uint64_t id,
                                                    int numArgs,
                                                    const char** argNames,
                                                    const unsigned char* argTypes,
                                                    const uint64_t* argValues,
                                                    unsigned char flags);

}  // namespace tracing
}  // namespace dawn::platform

#endif  // SRC_DAWN_PLATFORM_TRACING_EVENTTRACER_H_
