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
                                                                     uint32_t slice,
                                                                     Aspect aspect) {
        D3D12_TEXTURE_COPY_LOCATION copyLocation;
        copyLocation.pResource = texture->GetD3D12Resource();
        copyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        copyLocation.SubresourceIndex = texture->GetSubresourceIndex(level, slice, aspect);

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
        bufferLocation.PlacedFootprint.Footprint.Depth = bufferSize.depth;
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
        sourceRegion.back = offset.z + copySize.depth;
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
                                                       const Texture2DCopySplit& baseCopySplit,
                                                       ID3D12Resource* bufferResource,
                                                       uint64_t baseOffsetBytes,
                                                       uint64_t bufferBytesPerRow,
                                                       Texture* texture,
                                                       uint32_t textureMiplevel,
                                                       uint32_t textureSlice,
                                                       Aspect aspect) {
        ASSERT(HasOneBit(aspect));
        const D3D12_TEXTURE_COPY_LOCATION textureLocation =
            ComputeTextureCopyLocationForTexture(texture, textureMiplevel, textureSlice, aspect);

        const uint64_t offsetBytes = baseCopySplit.offset + baseOffsetBytes;

        for (uint32_t i = 0; i < baseCopySplit.count; ++i) {
            const Texture2DCopySplit::CopyInfo& info = baseCopySplit.copies[i];

            // TODO(jiawei.shao@intel.com): pre-compute bufferLocation and sourceRegion as
            // members in Texture2DCopySplit::CopyInfo.
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

    void CopyBufferToTextureWithCopySplit(CommandRecordingContext* commandContext,
                                          const TextureCopy& textureCopy,
                                          const Extent3D& copySize,
                                          Texture* texture,
                                          ID3D12Resource* bufferResource,
                                          const uint64_t offsetBytes,
                                          const uint32_t bytesPerRow,
                                          const uint32_t rowsPerImage,
                                          Aspect aspect) {
        ASSERT(HasOneBit(aspect));
        // See comments in ComputeTextureCopySplits() for more details.
        const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(aspect).block;
        const TextureCopySplits copySplits = ComputeTextureCopySplits(
            textureCopy.origin, copySize, blockInfo, offsetBytes, bytesPerRow, rowsPerImage);

        const uint64_t bytesPerSlice = bytesPerRow * rowsPerImage;

        // copySplits.copies2D[1] is always calculated for the second copy slice with
        // extra "bytesPerSlice" copy offset compared with the first copy slice. So
        // here we use an array bufferOffsetsForNextSlice to record the extra offsets
        // for each copy slice: bufferOffsetsForNextSlice[0] is the extra offset for
        // the next copy slice that uses copySplits.copies2D[0], and
        // bufferOffsetsForNextSlice[1] is the extra offset for the next copy slice
        // that uses copySplits.copies2D[1].
        std::array<uint64_t, TextureCopySplits::kMaxTextureCopySplits> bufferOffsetsForNextSlice = {
            {0u, 0u}};

        for (uint32_t copySlice = 0; copySlice < copySize.depth; ++copySlice) {
            const uint32_t splitIndex = copySlice % copySplits.copies2D.size();

            const Texture2DCopySplit& copySplitPerLayerBase = copySplits.copies2D[splitIndex];
            const uint64_t bufferOffsetForNextSlice = bufferOffsetsForNextSlice[splitIndex];
            const uint32_t copyTextureLayer = copySlice + textureCopy.origin.z;

            RecordCopyBufferToTextureFromTextureCopySplit(
                commandContext->GetCommandList(), copySplitPerLayerBase, bufferResource,
                bufferOffsetForNextSlice, bytesPerRow, texture, textureCopy.mipLevel,
                copyTextureLayer, aspect);

            bufferOffsetsForNextSlice[splitIndex] += bytesPerSlice * copySplits.copies2D.size();
        }
    }

}}  // namespace dawn_native::d3d12
