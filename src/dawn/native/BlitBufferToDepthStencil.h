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

#ifndef SRC_DAWN_NATIVE_BLITBUFFERTODEPTHSTENCIL_H_
#define SRC_DAWN_NATIVE_BLITBUFFERTODEPTHSTENCIL_H_

#include "dawn/native/Error.h"

namespace dawn::native {

struct TextureCopy;

// BlitBufferToDepth works around issues where copying from a buffer
// to depth does not work on some drivers.
// Currently, only depth16unorm textures can be CopyDst, so only depth16unorm
// is supported.
// It does the following:
//  - Copies buffer data to an rg8uint texture.
//  - Sets the viewport to the copy rect.
//  - Uploads the copy origin to a uniform buffer.
//  - For each destination layer:
//    - Performs a draw to sample the rg8uint data, computes the
//      floating point depth value, and writes the frag depth.

MaybeError BlitStagingBufferToDepth(DeviceBase* device,
                                    BufferBase* buffer,
                                    const TextureDataLayout& src,
                                    const TextureCopy& dst,
                                    const Extent3D& copyExtent);

MaybeError BlitBufferToDepth(DeviceBase* device,
                             CommandEncoder* commandEncoder,
                             BufferBase* buffer,
                             const TextureDataLayout& src,
                             const TextureCopy& dst,
                             const Extent3D& copyExtent);

// BlitBufferToStencil works around issues where copying from a buffer
// to stencil does not work on some drivers.
// It does the following:
//  - Copies buffer data to an r8uint texture.
//  - Sets the viewport to the copy rect.
//  - Uploads the copy origin to a uniform buffer.
//  - For each destination layer:
//    - Performs a draw to clear stencil to 0.
//    - Performs 8 draws for each bit of stencil to set the respective
//      stencil bit to 1, if the source r8 texture also has that bit set.
//      If the source r8 texture does not, the fragment is discarded.

MaybeError BlitStagingBufferToStencil(DeviceBase* device,
                                      BufferBase* buffer,
                                      const TextureDataLayout& src,
                                      const TextureCopy& dst,
                                      const Extent3D& copyExtent);

MaybeError BlitBufferToStencil(DeviceBase* device,
                               CommandEncoder* commandEncoder,
                               BufferBase* buffer,
                               const TextureDataLayout& src,
                               const TextureCopy& dst,
                               const Extent3D& copyExtent);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLITBUFFERTODEPTHSTENCIL_H_
