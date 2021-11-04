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

#include "dawn_native/metal/UtilsMetal.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/Pipeline.h"
#include "dawn_native/ShaderModule.h"

#include "common/Assert.h"

namespace dawn_native { namespace metal {

    MTLCompareFunction ToMetalCompareFunction(wgpu::CompareFunction compareFunction) {
        switch (compareFunction) {
            case wgpu::CompareFunction::Never:
                return MTLCompareFunctionNever;
            case wgpu::CompareFunction::Less:
                return MTLCompareFunctionLess;
            case wgpu::CompareFunction::LessEqual:
                return MTLCompareFunctionLessEqual;
            case wgpu::CompareFunction::Greater:
                return MTLCompareFunctionGreater;
            case wgpu::CompareFunction::GreaterEqual:
                return MTLCompareFunctionGreaterEqual;
            case wgpu::CompareFunction::NotEqual:
                return MTLCompareFunctionNotEqual;
            case wgpu::CompareFunction::Equal:
                return MTLCompareFunctionEqual;
            case wgpu::CompareFunction::Always:
                return MTLCompareFunctionAlways;

            case wgpu::CompareFunction::Undefined:
                UNREACHABLE();
        }
    }

    TextureBufferCopySplit ComputeTextureBufferCopySplit(const Texture* texture,
                                                         uint32_t mipLevel,
                                                         Origin3D origin,
                                                         Extent3D copyExtent,
                                                         uint64_t bufferSize,
                                                         uint64_t bufferOffset,
                                                         uint32_t bytesPerRow,
                                                         uint32_t rowsPerImage,
                                                         Aspect aspect) {
        TextureBufferCopySplit copy;
        const Format textureFormat = texture->GetFormat();
        const TexelBlockInfo& blockInfo = textureFormat.GetAspectInfo(aspect).block;

        // When copying textures from/to an unpacked buffer, the Metal validation layer doesn't
        // compute the correct range when checking if the buffer is big enough to contain the
        // data for the whole copy. Instead of looking at the position of the last texel in the
        // buffer, it computes the volume of the 3D box with bytesPerRow * (rowsPerImage /
        // format.blockHeight) * copySize.depthOrArrayLayers. For example considering the pixel
        // buffer below where in memory, each row data (D) of the texture is followed by some
        // padding data (P):
        //     |DDDDDDD|PP|
        //     |DDDDDDD|PP|
        //     |DDDDDDD|PP|
        //     |DDDDDDD|PP|
        //     |DDDDDDA|PP|
        // The last pixel read will be A, but the driver will think it is the whole last padding
        // row, causing it to generate an error when the pixel buffer is just big enough.

        // We work around this limitation by detecting when Metal would complain and copy the
        // last image and row separately using tight sourceBytesPerRow or sourceBytesPerImage.
        uint32_t bytesPerImage = bytesPerRow * rowsPerImage;

        // Metal validation layer requires that if the texture's pixel format is a compressed
        // format, the sourceSize must be a multiple of the pixel format's block size or be
        // clamped to the edge of the texture if the block extends outside the bounds of a
        // texture.
        const Extent3D clampedCopyExtent =
            texture->ClampToMipLevelVirtualSize(mipLevel, origin, copyExtent);

        ASSERT(texture->GetDimension() != wgpu::TextureDimension::e1D);

        // Check whether buffer size is big enough.
        bool needWorkaround =
            bufferSize - bufferOffset < bytesPerImage * copyExtent.depthOrArrayLayers;
        if (!needWorkaround) {
            copy.count = 1;
            copy.copies[0].bufferOffset = bufferOffset;
            copy.copies[0].bytesPerRow = bytesPerRow;
            copy.copies[0].bytesPerImage = bytesPerImage;
            copy.copies[0].textureOrigin = origin;
            copy.copies[0].copyExtent = {clampedCopyExtent.width, clampedCopyExtent.height,
                                         copyExtent.depthOrArrayLayers};
            return copy;
        }

        uint64_t currentOffset = bufferOffset;

        // Doing all the copy except the last image.
        if (copyExtent.depthOrArrayLayers > 1) {
            copy.copies[copy.count].bufferOffset = currentOffset;
            copy.copies[copy.count].bytesPerRow = bytesPerRow;
            copy.copies[copy.count].bytesPerImage = bytesPerImage;
            copy.copies[copy.count].textureOrigin = origin;
            copy.copies[copy.count].copyExtent = {clampedCopyExtent.width, clampedCopyExtent.height,
                                                  copyExtent.depthOrArrayLayers - 1};

            ++copy.count;

            // Update offset to copy to the last image.
            currentOffset += (copyExtent.depthOrArrayLayers - 1) * bytesPerImage;
        }

        // Doing all the copy in last image except the last row.
        uint32_t copyBlockRowCount = copyExtent.height / blockInfo.height;
        if (copyBlockRowCount > 1) {
            copy.copies[copy.count].bufferOffset = currentOffset;
            copy.copies[copy.count].bytesPerRow = bytesPerRow;
            copy.copies[copy.count].bytesPerImage = bytesPerRow * (copyBlockRowCount - 1);
            copy.copies[copy.count].textureOrigin = {origin.x, origin.y,
                                                     origin.z + copyExtent.depthOrArrayLayers - 1};

            ASSERT(copyExtent.height - blockInfo.height <
                   texture->GetMipLevelVirtualSize(mipLevel).height);
            copy.copies[copy.count].copyExtent = {clampedCopyExtent.width,
                                                  copyExtent.height - blockInfo.height, 1};

            ++copy.count;

            // Update offset to copy to the last row.
            currentOffset += (copyBlockRowCount - 1) * bytesPerRow;
        }

        // Doing the last row copy with the exact number of bytes in last row.
        // Workaround this issue in a way just like the copy to a 1D texture.
        uint32_t lastRowDataSize = (copyExtent.width / blockInfo.width) * blockInfo.byteSize;
        uint32_t lastRowCopyExtentHeight =
            blockInfo.height + clampedCopyExtent.height - copyExtent.height;
        ASSERT(lastRowCopyExtentHeight <= blockInfo.height);

        copy.copies[copy.count].bufferOffset = currentOffset;
        copy.copies[copy.count].bytesPerRow = lastRowDataSize;
        copy.copies[copy.count].bytesPerImage = lastRowDataSize;
        copy.copies[copy.count].textureOrigin = {origin.x,
                                                 origin.y + copyExtent.height - blockInfo.height,
                                                 origin.z + copyExtent.depthOrArrayLayers - 1};
        copy.copies[copy.count].copyExtent = {clampedCopyExtent.width, lastRowCopyExtentHeight, 1};
        ++copy.count;

        return copy;
    }

    void EnsureDestinationTextureInitialized(CommandRecordingContext* commandContext,
                                             Texture* texture,
                                             const TextureCopy& dst,
                                             const Extent3D& size) {
        ASSERT(texture == dst.texture.Get());
        SubresourceRange range = GetSubresourcesAffectedByCopy(dst, size);
        if (IsCompleteSubresourceCopiedTo(dst.texture.Get(), size, dst.mipLevel)) {
            texture->SetIsSubresourceContentInitialized(true, range);
        } else {
            texture->EnsureSubresourceContentInitialized(commandContext, range);
        }
    }

    MTLBlitOption ComputeMTLBlitOption(const Format& format, Aspect aspect) {
        ASSERT(HasOneBit(aspect));
        ASSERT(format.aspects & aspect);

        if (IsSubset(Aspect::Depth | Aspect::Stencil, format.aspects)) {
            // We only provide a blit option if the format has both depth and stencil.
            // It is invalid to provide a blit option otherwise.
            switch (aspect) {
                case Aspect::Depth:
                    return MTLBlitOptionDepthFromDepthStencil;
                case Aspect::Stencil:
                    return MTLBlitOptionStencilFromDepthStencil;
                default:
                    UNREACHABLE();
            }
        }
        return MTLBlitOptionNone;
    }

    MaybeError CreateMTLFunction(const ProgrammableStage& programmableStage,
                                 SingleShaderStage singleShaderStage,
                                 PipelineLayout* pipelineLayout,
                                 ShaderModule::MetalFunctionData* functionData,
                                 uint32_t sampleMask,
                                 const RenderPipeline* renderPipeline) {
        ShaderModule* shaderModule = ToBackend(programmableStage.module.Get());
        const char* shaderEntryPoint = programmableStage.entryPoint.c_str();
        const auto& entryPointMetadata = programmableStage.module->GetEntryPoint(shaderEntryPoint);
        if (entryPointMetadata.overridableConstants.size() == 0) {
            DAWN_TRY(shaderModule->CreateFunction(shaderEntryPoint, singleShaderStage,
                                                  pipelineLayout, functionData, nil, sampleMask,
                                                  renderPipeline));
            return {};
        }

        if (@available(macOS 10.12, *)) {
            // MTLFunctionConstantValues can only be created within the if available branch
            NSRef<MTLFunctionConstantValues> constantValues =
                AcquireNSRef([MTLFunctionConstantValues new]);

            std::unordered_set<std::string> overriddenConstants;

            auto switchType = [&](EntryPointMetadata::OverridableConstant::Type dawnType,
                                  MTLDataType* type, OverridableConstantScalar* entry,
                                  double value = 0) {
                switch (dawnType) {
                    case EntryPointMetadata::OverridableConstant::Type::Boolean:
                        *type = MTLDataTypeBool;
                        if (entry) {
                            entry->b = static_cast<int32_t>(value);
                        }
                        break;
                    case EntryPointMetadata::OverridableConstant::Type::Float32:
                        *type = MTLDataTypeFloat;
                        if (entry) {
                            entry->f32 = static_cast<float>(value);
                        }
                        break;
                    case EntryPointMetadata::OverridableConstant::Type::Int32:
                        *type = MTLDataTypeInt;
                        if (entry) {
                            entry->i32 = static_cast<int32_t>(value);
                        }
                        break;
                    case EntryPointMetadata::OverridableConstant::Type::Uint32:
                        *type = MTLDataTypeUInt;
                        if (entry) {
                            entry->u32 = static_cast<uint32_t>(value);
                        }
                        break;
                    default:
                        UNREACHABLE();
                }
            };

            for (const auto& pipelineConstant : programmableStage.constants) {
                const std::string& name = pipelineConstant.first;
                double value = pipelineConstant.second;

                overriddenConstants.insert(name);

                // This is already validated so `name` must exist
                const auto& moduleConstant = entryPointMetadata.overridableConstants.at(name);

                MTLDataType type;
                OverridableConstantScalar entry{};

                switchType(moduleConstant.type, &type, &entry, value);

                [constantValues.Get() setConstantValue:&entry type:type atIndex:moduleConstant.id];
            }

            // Set shader initialized default values because MSL function_constant
            // has no default value
            for (const std::string& name : entryPointMetadata.initializedOverridableConstants) {
                if (overriddenConstants.count(name) != 0) {
                    // This constant already has overridden value
                    continue;
                }

                // Must exist because it is validated
                const auto& moduleConstant = entryPointMetadata.overridableConstants.at(name);
                ASSERT(moduleConstant.isInitialized);
                MTLDataType type;

                switchType(moduleConstant.type, &type, nullptr);

                [constantValues.Get() setConstantValue:&moduleConstant.defaultValue
                                                  type:type
                                               atIndex:moduleConstant.id];
            }

            DAWN_TRY(shaderModule->CreateFunction(
                shaderEntryPoint, singleShaderStage, pipelineLayout, functionData,
                constantValues.Get(), sampleMask, renderPipeline));
        } else {
            UNREACHABLE();
        }
        return {};
    }

}}  // namespace dawn_native::metal
