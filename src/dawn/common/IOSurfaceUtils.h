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

#ifndef SRC_DAWN_COMMON_IOSURFACEUTILS_H_
#define SRC_DAWN_COMMON_IOSURFACEUTILS_H_

#include <IOSurface/IOSurfaceRef.h>

#include "dawn/webgpu_cpp.h"

namespace dawn {

// TODO(dawn:2134) Implement a function to create a single plane IOSurface.

// Create a multiplanar IOSurface. Note single plane format won't be supported.
IOSurfaceRef CreateMultiPlanarIOSurface(wgpu::TextureFormat format,
                                        uint32_t width,
                                        uint32_t height);
}  // namespace dawn

#endif  // SRC_DAWN_COMMON_IOSURFACEUTILS_H_
