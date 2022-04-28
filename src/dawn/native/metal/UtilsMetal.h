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

#include "dawn/native/dawn_platform.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/ShaderModuleMTL.h"
#include "dawn/native/metal/TextureMTL.h"

#import <Metal/Metal.h>

namespace dawn::native {
    struct ProgrammableStage;
    struct EntryPointMetadata;
    enum class SingleShaderStage;
}

namespace dawn::native::metal {

    MTLCompareFunction ToMetalCompareFunction(wgpu::CompareFunction compareFunction);

    struct TextureBufferCopySplit {
        static constexpr uint32_t kMaxTextureBufferCopyRegions = 3;

        struct CopyInfo {
            NSUInteger bufferOffset;
            NSUInteger bytesPerRow;
            NSUInteger bytesPerImage;
            Origin3D textureOrigin;
            Extent3D copyExtent;
        };

        uint32_t count = 0;
        std::array<CopyInfo, kMaxTextureBufferCopyRegions> copies;

        auto begin() const {
            return copies.begin();
        }

        auto end() const {
            return copies.begin() + count;
        }
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

    MTLBlitOption ComputeMTLBlitOption(const Format& format, Aspect aspect);

    // Helper function to create function with constant values wrapped in
    // if available branch
    MaybeError CreateMTLFunction(const ProgrammableStage& programmableStage,
                                 SingleShaderStage singleShaderStage,
                                 PipelineLayout* pipelineLayout,
                                 ShaderModule::MetalFunctionData* functionData,
                                 uint32_t sampleMask = 0xFFFFFFFF,
                                 const RenderPipeline* renderPipeline = nullptr);

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
    using EncodeInsideRenderPass = std::function<MaybeError(id<MTLRenderCommandEncoder>)>;
    MaybeError EncodeMetalRenderPass(Device* device,
                                     CommandRecordingContext* commandContext,
                                     MTLRenderPassDescriptor* mtlRenderPass,
                                     uint32_t width,
                                     uint32_t height,
                                     EncodeInsideRenderPass encodeInside);

    MaybeError EncodeEmptyMetalRenderPass(Device* device,
                                          CommandRecordingContext* commandContext,
                                          MTLRenderPassDescriptor* mtlRenderPass,
                                          Extent3D size);

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_UTILSMETAL_H_
