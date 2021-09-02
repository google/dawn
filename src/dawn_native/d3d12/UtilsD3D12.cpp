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

#include "dawn_native/d3d12/UtilsD3D12.h"

#include "common/Assert.h"
#include "dawn_native/Format.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/CommandRecordingContext.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"

#include <stringapiset.h>

namespace dawn_native { namespace d3d12 {

    ResultOrError<std::wstring> ConvertStringToWstring(const char* str) {
        size_t len = strlen(str);
        if (len == 0) {
            return std::wstring();
        }
        int numChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, len, nullptr, 0);
        if (numChars == 0) {
            return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
        }
        std::wstring result;
        result.resize(numChars);
        int numConvertedChars =
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, len, &result[0], numChars);
        if (numConvertedChars != numChars) {
            return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
        }
        return std::move(result);
    }

    D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(wgpu::CompareFunction func) {
        switch (func) {
            case wgpu::CompareFunction::Never:
                return D3D12_COMPARISON_FUNC_NEVER;
            case wgpu::CompareFunction::Less:
                return D3D12_COMPARISON_FUNC_LESS;
            case wgpu::CompareFunction::LessEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case wgpu::CompareFunction::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;
            case wgpu::CompareFunction::GreaterEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case wgpu::CompareFunction::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;
            case wgpu::CompareFunction::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case wgpu::CompareFunction::Always:
                return D3D12_COMPARISON_FUNC_ALWAYS;

            case wgpu::CompareFunction::Undefined:
                UNREACHABLE();
        }
    }

    D3D12_TEXTURE_COPY_LOCATION ComputeTextureCopyLocationForTexture(const Texture* texture,
                                                                     uint32_t level,
                                                                     uint32_t layer,
                                                                     Aspect aspect) {
        D3D12_TEXTURE_COPY_LOCATION copyLocation;
        copyLocation.pResource = texture->GetD3D12Resource();
        copyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        copyLocation.SubresourceIndex = texture->GetSubresourceIndex(level, layer, aspect);

        return copyLocation;
    }

    D3D12_TEXTURE_COPY_LOCATION ComputeBufferLocationForCopyTextureRegion(
        const Texture* texture,
        ID3D12Resource* bufferResource,
        const Extent3D& bufferSize,
        const uint64_t offset,
        const uint32_t rowPitch,
        Aspect aspect) {
        D3D12_TEXTURE_COPY_LOCATION bufferLocation;
        bufferLocation.pResource = bufferResource;
        bufferLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        bufferLocation.PlacedFootprint.Offset = offset;
        bufferLocation.PlacedFootprint.Footprint.Format =
            texture->GetD3D12CopyableSubresourceFormat(aspect);
        bufferLocation.PlacedFootprint.Footprint.Width = bufferSize.width;
        bufferLocation.PlacedFootprint.Footprint.Height = bufferSize.height;
        bufferLocation.PlacedFootprint.Footprint.Depth = bufferSize.depthOrArrayLayers;
        bufferLocation.PlacedFootprint.Footprint.RowPitch = rowPitch;
        return bufferLocation;
    }

    D3D12_BOX ComputeD3D12BoxFromOffsetAndSize(const Origin3D& offset, const Extent3D& copySize) {
        D3D12_BOX sourceRegion;
        sourceRegion.left = offset.x;
        sourceRegion.top = offset.y;
        sourceRegion.front = offset.z;
        sourceRegion.right = offset.x + copySize.width;
        sourceRegion.bottom = offset.y + copySize.height;
        sourceRegion.back = offset.z + copySize.depthOrArrayLayers;
        return sourceRegion;
    }

    bool IsTypeless(DXGI_FORMAT format) {
        // List generated from <dxgiformat.h>
        switch (format) {
            case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            case DXGI_FORMAT_R32G32B32_TYPELESS:
            case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            case DXGI_FORMAT_R32G32_TYPELESS:
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R16G16_TYPELESS:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_R8G8_TYPELESS:
            case DXGI_FORMAT_R16_TYPELESS:
            case DXGI_FORMAT_R8_TYPELESS:
            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC7_TYPELESS:
                return true;
            default:
                return false;
        }
    }

    void RecordCopyBufferToTextureFromTextureCopySplit(ID3D12GraphicsCommandList* commandList,
                                                       const TextureCopySubresource& baseCopySplit,
                                                       ID3D12Resource* bufferResource,
                                                       uint64_t baseOffset,
                                                       uint64_t bufferBytesPerRow,
                                                       Texture* texture,
                                                       uint32_t textureMiplevel,
                                                       uint32_t textureLayer,
                                                       Aspect aspect) {
        ASSERT(HasOneBit(aspect));
        const D3D12_TEXTURE_COPY_LOCATION textureLocation =
            ComputeTextureCopyLocationForTexture(texture, textureMiplevel, textureLayer, aspect);

        for (uint32_t i = 0; i < baseCopySplit.count; ++i) {
            const TextureCopySubresource::CopyInfo& info = baseCopySplit.copies[i];

            // TODO(jiawei.shao@intel.com): pre-compute bufferLocation and sourceRegion as
            // members in TextureCopySubresource::CopyInfo.
            const uint64_t offsetBytes = info.alignedOffset + baseOffset;
            const D3D12_TEXTURE_COPY_LOCATION bufferLocation =
                ComputeBufferLocationForCopyTextureRegion(texture, bufferResource, info.bufferSize,
                                                          offsetBytes, bufferBytesPerRow, aspect);
            const D3D12_BOX sourceRegion =
                ComputeD3D12BoxFromOffsetAndSize(info.bufferOffset, info.copySize);

            commandList->CopyTextureRegion(&textureLocation, info.textureOffset.x,
                                           info.textureOffset.y, info.textureOffset.z,
                                           &bufferLocation, &sourceRegion);
        }
    }

    void CopyBufferTo2DTextureWithCopySplit(CommandRecordingContext* commandContext,
                                            const TextureCopy& textureCopy,
                                            ID3D12Resource* bufferResource,
                                            const uint64_t offset,
                                            const uint32_t bytesPerRow,
                                            const uint32_t rowsPerImage,
                                            const Extent3D& copySize,
                                            Texture* texture,
                                            Aspect aspect) {
        ASSERT(HasOneBit(aspect));
        // See comments in Compute2DTextureCopySplits() for more details.
        const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(aspect).block;
        const TextureCopySplits copySplits = Compute2DTextureCopySplits(
            textureCopy.origin, copySize, blockInfo, offset, bytesPerRow, rowsPerImage);

        const uint64_t bytesPerLayer = bytesPerRow * rowsPerImage;

        // copySplits.copySubresources[1] is always calculated for the second copy layer with
        // extra "bytesPerLayer" copy offset compared with the first copy layer. So
        // here we use an array bufferOffsetsForNextLayer to record the extra offsets
        // for each copy layer: bufferOffsetsForNextLayer[0] is the extra offset for
        // the next copy layer that uses copySplits.copySubresources[0], and
        // bufferOffsetsForNextLayer[1] is the extra offset for the next copy layer
        // that uses copySplits.copySubresources[1].
        std::array<uint64_t, TextureCopySplits::kMaxTextureCopySubresources>
            bufferOffsetsForNextLayer = {{0u, 0u}};

        for (uint32_t copyLayer = 0; copyLayer < copySize.depthOrArrayLayers; ++copyLayer) {
            const uint32_t splitIndex = copyLayer % copySplits.copySubresources.size();

            const TextureCopySubresource& copySplitPerLayerBase =
                copySplits.copySubresources[splitIndex];
            const uint64_t bufferOffsetForNextLayer = bufferOffsetsForNextLayer[splitIndex];
            const uint32_t copyTextureLayer = copyLayer + textureCopy.origin.z;

            RecordCopyBufferToTextureFromTextureCopySplit(
                commandContext->GetCommandList(), copySplitPerLayerBase, bufferResource,
                bufferOffsetForNextLayer, bytesPerRow, texture, textureCopy.mipLevel,
                copyTextureLayer, aspect);

            bufferOffsetsForNextLayer[splitIndex] +=
                bytesPerLayer * copySplits.copySubresources.size();
        }
    }

    void CopyBufferTo3DTexture(CommandRecordingContext* commandContext,
                               const TextureCopy& textureCopy,
                               ID3D12Resource* bufferResource,
                               const uint64_t offset,
                               const uint32_t bytesPerRow,
                               const uint32_t rowsPerImage,
                               const Extent3D& copySize,
                               Texture* texture,
                               Aspect aspect) {
        ASSERT(HasOneBit(aspect));
        // See comments in Compute3DTextureCopySplits() for more details.
        const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(aspect).block;
        const TextureCopySubresource copyRegions = Compute3DTextureCopySplits(
            textureCopy.origin, copySize, blockInfo, offset, bytesPerRow, rowsPerImage);

        RecordCopyBufferToTextureFromTextureCopySplit(commandContext->GetCommandList(), copyRegions,
                                                      bufferResource, 0, bytesPerRow, texture,
                                                      textureCopy.mipLevel, 0, aspect);
    }

    void RecordCopyBufferToTexture(CommandRecordingContext* commandContext,
                                   const TextureCopy& textureCopy,
                                   ID3D12Resource* bufferResource,
                                   const uint64_t offset,
                                   const uint32_t bytesPerRow,
                                   const uint32_t rowsPerImage,
                                   const Extent3D& copySize,
                                   Texture* texture,
                                   Aspect aspect) {
        // Record the CopyTextureRegion commands for 3D textures. Multiple depths of 3D
        // textures can be copied in one shot and copySplits are not needed.
        if (texture->GetDimension() == wgpu::TextureDimension::e3D) {
            CopyBufferTo3DTexture(commandContext, textureCopy, bufferResource, offset, bytesPerRow,
                                  rowsPerImage, copySize, texture, aspect);
        } else {
            // Compute the copySplits and record the CopyTextureRegion commands for 2D
            // textures.
            CopyBufferTo2DTextureWithCopySplit(commandContext, textureCopy, bufferResource, offset,
                                               bytesPerRow, rowsPerImage, copySize, texture,
                                               aspect);
        }
    }

    void RecordCopyTextureToBufferFromTextureCopySplit(ID3D12GraphicsCommandList* commandList,
                                                       const TextureCopySubresource& baseCopySplit,
                                                       Buffer* buffer,
                                                       uint64_t baseOffset,
                                                       uint64_t bufferBytesPerRow,
                                                       Texture* texture,
                                                       uint32_t textureMiplevel,
                                                       uint32_t textureLayer,
                                                       Aspect aspect) {
        const D3D12_TEXTURE_COPY_LOCATION textureLocation =
            ComputeTextureCopyLocationForTexture(texture, textureMiplevel, textureLayer, aspect);

        for (uint32_t i = 0; i < baseCopySplit.count; ++i) {
            const TextureCopySubresource::CopyInfo& info = baseCopySplit.copies[i];

            // TODO(jiawei.shao@intel.com): pre-compute bufferLocation and sourceRegion as
            // members in TextureCopySubresource::CopyInfo.
            const uint64_t offsetBytes = info.alignedOffset + baseOffset;
            const D3D12_TEXTURE_COPY_LOCATION bufferLocation =
                ComputeBufferLocationForCopyTextureRegion(texture, buffer->GetD3D12Resource(),
                                                          info.bufferSize, offsetBytes,
                                                          bufferBytesPerRow, aspect);
            const D3D12_BOX sourceRegion =
                ComputeD3D12BoxFromOffsetAndSize(info.textureOffset, info.copySize);

            commandList->CopyTextureRegion(&bufferLocation, info.bufferOffset.x,
                                           info.bufferOffset.y, info.bufferOffset.z,
                                           &textureLocation, &sourceRegion);
        }
    }

    void Copy2DTextureToBufferWithCopySplit(ID3D12GraphicsCommandList* commandList,
                                            const TextureCopy& textureCopy,
                                            const BufferCopy& bufferCopy,
                                            Texture* texture,
                                            Buffer* buffer,
                                            const Extent3D& copySize) {
        ASSERT(HasOneBit(textureCopy.aspect));
        const TexelBlockInfo& blockInfo =
            texture->GetFormat().GetAspectInfo(textureCopy.aspect).block;

        // See comments around Compute2DTextureCopySplits() for more details.
        const TextureCopySplits copySplits =
            Compute2DTextureCopySplits(textureCopy.origin, copySize, blockInfo, bufferCopy.offset,
                                       bufferCopy.bytesPerRow, bufferCopy.rowsPerImage);

        const uint64_t bytesPerLayer = bufferCopy.bytesPerRow * bufferCopy.rowsPerImage;

        // copySplits.copySubresources[1] is always calculated for the second copy layer with
        // extra "bytesPerLayer" copy offset compared with the first copy layer. So
        // here we use an array bufferOffsetsForNextLayer to record the extra offsets
        // for each copy layer: bufferOffsetsForNextLayer[0] is the extra offset for
        // the next copy layer that uses copySplits.copySubresources[0], and
        // bufferOffsetsForNextLayer[1] is the extra offset for the next copy layer
        // that uses copySplits.copySubresources[1].
        std::array<uint64_t, TextureCopySplits::kMaxTextureCopySubresources>
            bufferOffsetsForNextLayer = {{0u, 0u}};
        for (uint32_t copyLayer = 0; copyLayer < copySize.depthOrArrayLayers; ++copyLayer) {
            const uint32_t splitIndex = copyLayer % copySplits.copySubresources.size();

            const TextureCopySubresource& copySplitPerLayerBase =
                copySplits.copySubresources[splitIndex];
            const uint64_t bufferOffsetForNextLayer = bufferOffsetsForNextLayer[splitIndex];
            const uint32_t copyTextureLayer = copyLayer + textureCopy.origin.z;

            RecordCopyTextureToBufferFromTextureCopySplit(
                commandList, copySplitPerLayerBase, buffer, bufferOffsetForNextLayer,
                bufferCopy.bytesPerRow, texture, textureCopy.mipLevel, copyTextureLayer,
                textureCopy.aspect);

            bufferOffsetsForNextLayer[splitIndex] +=
                bytesPerLayer * copySplits.copySubresources.size();
        }
    }

    void Copy3DTextureToBuffer(ID3D12GraphicsCommandList* commandList,
                               const TextureCopy& textureCopy,
                               const BufferCopy& bufferCopy,
                               Texture* texture,
                               Buffer* buffer,
                               const Extent3D& copySize) {
        ASSERT(HasOneBit(textureCopy.aspect));
        const TexelBlockInfo& blockInfo =
            texture->GetFormat().GetAspectInfo(textureCopy.aspect).block;

        // See comments around Compute3DTextureCopySplits() for more details.
        const TextureCopySubresource copyRegions =
            Compute3DTextureCopySplits(textureCopy.origin, copySize, blockInfo, bufferCopy.offset,
                                       bufferCopy.bytesPerRow, bufferCopy.rowsPerImage);

        RecordCopyTextureToBufferFromTextureCopySplit(commandList, copyRegions, buffer, 0,
                                                      bufferCopy.bytesPerRow, texture,
                                                      textureCopy.mipLevel, 0, textureCopy.aspect);
    }

    void RecordCopyTextureToBuffer(ID3D12GraphicsCommandList* commandList,
                                   const TextureCopy& textureCopy,
                                   const BufferCopy& bufferCopy,
                                   Texture* texture,
                                   Buffer* buffer,
                                   const Extent3D& copySize) {
        if (texture->GetDimension() == wgpu::TextureDimension::e3D) {
            Copy3DTextureToBuffer(commandList, textureCopy, bufferCopy, texture, buffer, copySize);
        } else {
            Copy2DTextureToBufferWithCopySplit(commandList, textureCopy, bufferCopy, texture,
                                               buffer, copySize);
        }
    }

    void SetDebugName(Device* device, ID3D12Object* object, const char* prefix, std::string label) {
        if (!object) {
            return;
        }

        if (label.empty() || !device->IsToggleEnabled(Toggle::UseUserDefinedLabelsInBackend)) {
            object->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(prefix), prefix);
            return;
        }

        std::string objectName = prefix;
        objectName += "_";
        objectName += label;
        object->SetPrivateData(WKPDID_D3DDebugObjectName, objectName.length(), objectName.c_str());
    }

}}  // namespace dawn_native::d3d12
