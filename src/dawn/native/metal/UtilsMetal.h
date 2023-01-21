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

#ifndef SRC_DAWN_NATIVE_METAL_UTILSMETAL_H_
#define SRC_DAWN_NATIVE_METAL_UTILSMETAL_H_

#include "dawn/common/StackContainer.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/ShaderModuleMTL.h"
#include "dawn/native/metal/TextureMTL.h"

#import <Metal/Metal.h>

namespace dawn::native {
struct BeginRenderPassCmd;
struct ProgrammableStage;
struct EntryPointMetadata;
enum class SingleShaderStage;
}  // namespace dawn::native

namespace dawn::native::metal {

Aspect GetDepthStencilAspects(MTLPixelFormat format);
MTLCompareFunction ToMetalCompareFunction(wgpu::CompareFunction compareFunction);

struct TextureBufferCopySplit {
    // Avoid allocations except in the worse case. Most cases require at most 3 regions.
    static constexpr uint32_t kNumCommonTextureBufferCopyRegions = 3;

    struct CopyInfo {
        CopyInfo(NSUInteger bufferOffset,
                 NSUInteger bytesPerRow,
                 NSUInteger bytesPerImage,
                 Origin3D textureOrigin,
                 Extent3D copyExtent)
            : bufferOffset(bufferOffset),
              bytesPerRow(bytesPerRow),
              bytesPerImage(bytesPerImage),
              textureOrigin(textureOrigin),
              copyExtent(copyExtent) {}

        NSUInteger bufferOffset;
        NSUInteger bytesPerRow;
        NSUInteger bytesPerImage;
        Origin3D textureOrigin;
        Extent3D copyExtent;
    };

    StackVector<CopyInfo, kNumCommonTextureBufferCopyRegions> copies;

    auto begin() const { return copies->begin(); }
    auto end() const { return copies->end(); }
    void push_back(const CopyInfo& copyInfo) { copies->push_back(copyInfo); }
};

TextureBufferCopySplit ComputeTextureBufferCopySplit(const Texture* texture,
                                                     uint32_t mipLevel,
                                                     Origin3D origin,
                                                     Extent3D copyExtent,
                                                     uint64_t bufferSize,
                                                     uint64_t bufferOffset,
                                                     uint32_t bytesPerRow,
                                                     uint32_t rowsPerImage,
                                                     Aspect aspect);

void EnsureDestinationTextureInitialized(CommandRecordingContext* commandContext,
                                         Texture* texture,
                                         const TextureCopy& dst,
                                         const Extent3D& size);

// Allow use MTLStoreActionStoreAndMultismapleResolve because the logic in the backend is
// first to compute what the "best" Metal render pass descriptor is, then fix it up if we
// are not on macOS 10.12 (i.e. the EmulateStoreAndMSAAResolve toggle is on).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability"
constexpr MTLStoreAction kMTLStoreActionStoreAndMultisampleResolve =
    MTLStoreActionStoreAndMultisampleResolve;
#pragma clang diagnostic pop

// Helper functions to encode Metal render passes that take care of multiple workarounds that
// happen at the render pass start and end. Because workarounds wrap the encoding of the render
// pass, the encoding must be entirely done by the `encodeInside` callback.
// At the end of this function, `commandContext` will have no encoder open.
using EncodeInsideRenderPass =
    std::function<MaybeError(id<MTLRenderCommandEncoder>, BeginRenderPassCmd* renderPassCmd)>;
MaybeError EncodeMetalRenderPass(Device* device,
                                 CommandRecordingContext* commandContext,
                                 MTLRenderPassDescriptor* mtlRenderPass,
                                 uint32_t width,
                                 uint32_t height,
                                 EncodeInsideRenderPass encodeInside,
                                 BeginRenderPassCmd* renderPassCmd = nullptr);

MaybeError EncodeEmptyMetalRenderPass(Device* device,
                                      CommandRecordingContext* commandContext,
                                      MTLRenderPassDescriptor* mtlRenderPass,
                                      Extent3D size);

bool SupportCounterSamplingAtCommandBoundary(id<MTLDevice> device)
    API_AVAILABLE(macos(11.0), ios(14.0));
bool SupportCounterSamplingAtStageBoundary(id<MTLDevice> device)
    API_AVAILABLE(macos(11.0), ios(14.0));

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_UTILSMETAL_H_
