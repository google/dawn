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

#ifndef SRC_DAWN_NATIVE_BLITDEPTHSTENCILTOBUFFER_H_
#define SRC_DAWN_NATIVE_BLITDEPTHSTENCILTOBUFFER_H_

#include "dawn/native/Error.h"

namespace dawn::native {

struct TextureCopy;
struct BufferCopy;

// BlitDepthToBuffer works around OpenGL/GLES issues of copying depth textures to a buffer.
// Supported depth texture format: depth16unorm, depth32float
// It dispatches a compute shader textureLoad from the depth texture and writes to the buffer as a
// storage buffer.

MaybeError BlitDepthToBuffer(DeviceBase* device,
                             CommandEncoder* commandEncoder,
                             const TextureCopy& src,
                             const BufferCopy& dst,
                             const Extent3D& copyExtent);

// BlitStencilToBuffer works around OpenGLES issues of copying stencil textures to a buffer.
// Supported stencil texture format: *stencil8
// It dispatches a compute shader textureLoad from the stencil texture and writes to the buffer as a
// storage buffer.

MaybeError BlitStencilToBuffer(DeviceBase* device,
                               CommandEncoder* commandEncoder,
                               const TextureCopy& src,
                               const BufferCopy& dst,
                               const Extent3D& copyExtent);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLITDEPTHSTENCILTOBUFFER_H_
