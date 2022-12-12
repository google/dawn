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

#ifndef SRC_DAWN_NATIVE_COMMANDVALIDATION_H_
#define SRC_DAWN_NATIVE_COMMANDVALIDATION_H_

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/StackContainer.h"
#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/Features.h"
#include "dawn/native/Texture.h"
#include "dawn/native/UsageValidationMode.h"

namespace dawn::native {

class QuerySetBase;
struct SyncScopeResourceUsage;
struct TexelBlockInfo;

MaybeError ValidateSyncScopeResourceUsage(const SyncScopeResourceUsage& usage);

MaybeError ValidateTimestampQuery(const DeviceBase* device,
                                  const QuerySetBase* querySet,
                                  uint32_t queryIndex,
                                  Feature requiredFeature = Feature::TimestampQuery);

MaybeError ValidateWriteBuffer(const DeviceBase* device,
                               const BufferBase* buffer,
                               uint64_t bufferOffset,
                               uint64_t size);

template <typename A, typename B>
DAWN_FORCE_INLINE uint64_t Safe32x32(A a, B b) {
    static_assert(std::is_same<A, uint32_t>::value, "'a' must be uint32_t");
    static_assert(std::is_same<B, uint32_t>::value, "'b' must be uint32_t");
    return uint64_t(a) * uint64_t(b);
}

ResultOrError<uint64_t> ComputeRequiredBytesInCopy(const TexelBlockInfo& blockInfo,
                                                   const Extent3D& copySize,
                                                   uint32_t bytesPerRow,
                                                   uint32_t rowsPerImage);

void ApplyDefaultTextureDataLayoutOptions(TextureDataLayout* layout,
                                          const TexelBlockInfo& blockInfo,
                                          const Extent3D& copyExtent);
MaybeError ValidateLinearTextureData(const TextureDataLayout& layout,
                                     uint64_t byteSize,
                                     const TexelBlockInfo& blockInfo,
                                     const Extent3D& copyExtent);
MaybeError ValidateTextureCopyRange(DeviceBase const* device,
                                    const ImageCopyTexture& imageCopyTexture,
                                    const Extent3D& copySize);
ResultOrError<Aspect> SingleAspectUsedByImageCopyTexture(const ImageCopyTexture& view);
MaybeError ValidateLinearToDepthStencilCopyRestrictions(const ImageCopyTexture& dst);

MaybeError ValidateImageCopyBuffer(DeviceBase const* device,
                                   const ImageCopyBuffer& imageCopyBuffer);
MaybeError ValidateImageCopyTexture(DeviceBase const* device,
                                    const ImageCopyTexture& imageCopyTexture,
                                    const Extent3D& copySize);

MaybeError ValidateCopySizeFitsInBuffer(const Ref<BufferBase>& buffer,
                                        uint64_t offset,
                                        uint64_t size);

bool IsRangeOverlapped(uint32_t startA, uint32_t startB, uint32_t length);

MaybeError ValidateTextureToTextureCopyCommonRestrictions(const ImageCopyTexture& src,
                                                          const ImageCopyTexture& dst,
                                                          const Extent3D& copySize);
MaybeError ValidateTextureToTextureCopyRestrictions(const ImageCopyTexture& src,
                                                    const ImageCopyTexture& dst,
                                                    const Extent3D& copySize);

MaybeError ValidateCanUseAs(const TextureBase* texture,
                            wgpu::TextureUsage usage,
                            UsageValidationMode mode);
MaybeError ValidateCanUseAs(const BufferBase* buffer, wgpu::BufferUsage usage);

using ColorAttachmentFormats = StackVector<const Format*, kMaxColorAttachments>;
MaybeError ValidateColorAttachmentBytesPerSample(DeviceBase* device,
                                                 const ColorAttachmentFormats& formats);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDVALIDATION_H_
