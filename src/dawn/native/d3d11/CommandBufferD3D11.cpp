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

#include "dawn/native/d3d11/CommandBufferD3D11.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "dawn/common/WindowsUtils.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupTracker.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/VertexFormat.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/ComputePipelineD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/Forward.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/RenderPipelineD3D11.h"
#include "dawn/native/d3d11/SamplerD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {

// Create CommandBuffer
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

MaybeError CommandBuffer::Execute() {
    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();

    ID3D11DeviceContext1* d3d11DeviceContext1 = commandContext->GetD3D11DeviceContext1();

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                mCommands.NextCommand<BeginComputePassCmd>();
                return DAWN_UNIMPLEMENTED_ERROR("Compute pass not implemented");
            }

            case Command::BeginRenderPass: {
                [[maybe_unused]] auto* cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                return DAWN_UNIMPLEMENTED_ERROR("Render pass not implemented");
            }

            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                if (copy->size == 0) {
                    // Skip no-op copies.
                    break;
                }

                Buffer* source = ToBackend(copy->source.Get());
                Buffer* destination = ToBackend(copy->destination.Get());

                // Buffer::Copy() will ensure the source and destination buffers are initialized.
                DAWN_TRY(Buffer::Copy(commandContext, source, copy->sourceOffset, copy->size,
                                      destination, copy->destinationOffset));
                break;
            }

            case Command::CopyBufferToTexture: {
                CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }

                Buffer* buffer = ToBackend(copy->source.buffer.Get());
                Buffer::ScopedMap scopedMap;
                DAWN_TRY_ASSIGN(scopedMap, Buffer::ScopedMap::Create(buffer));
                DAWN_TRY(buffer->EnsureDataInitialized(commandContext));

                if (!scopedMap.GetMappedData()) {
                    // TODO(dawn:1768): implement CopyBufferToTexture with non-mappable buffers.
                    return DAWN_UNIMPLEMENTED_ERROR(
                        "CopyBufferToTexture isn't implemented with non-mappable buffers");
                }

                Texture* texture = ToBackend(copy->destination.texture.Get());
                SubresourceRange subresources =
                    GetSubresourcesAffectedByCopy(copy->destination, copy->copySize);
                const uint8_t* data = scopedMap.GetMappedData() + copy->source.offset;

                DAWN_TRY(texture->Write(commandContext, subresources, copy->destination.origin,
                                        copy->copySize, data, copy->source.bytesPerRow,
                                        copy->source.rowsPerImage));
                break;
            }

            case Command::CopyTextureToBuffer: {
                CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }

                auto& src = copy->source;
                auto& dst = copy->destination;

                SubresourceRange subresources = GetSubresourcesAffectedByCopy(src, copy->copySize);
                DAWN_TRY(ToBackend(src.texture)
                             ->EnsureSubresourceContentInitialized(commandContext, subresources));

                // Create a staging texture.
                D3D11_TEXTURE2D_DESC stagingTextureDesc;
                stagingTextureDesc.Width = copy->copySize.width;
                stagingTextureDesc.Height = copy->copySize.height;
                stagingTextureDesc.MipLevels = 1;
                stagingTextureDesc.ArraySize = copy->copySize.depthOrArrayLayers;
                stagingTextureDesc.Format = ToBackend(src.texture)->GetD3D11Format();
                stagingTextureDesc.SampleDesc.Count = 1;
                stagingTextureDesc.SampleDesc.Quality = 0;
                stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
                stagingTextureDesc.BindFlags = 0;
                stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                stagingTextureDesc.MiscFlags = 0;

                ComPtr<ID3D11Texture2D> stagingTexture;
                DAWN_TRY(CheckHRESULT(commandContext->GetD3D11Device()->CreateTexture2D(
                                          &stagingTextureDesc, nullptr, &stagingTexture),
                                      "D3D11 create staging texture"));

                uint32_t subresource =
                    src.texture->GetSubresourceIndex(src.mipLevel, src.origin.z, src.aspect);

                if (src.texture->GetDimension() != wgpu::TextureDimension::e2D) {
                    return DAWN_UNIMPLEMENTED_ERROR(
                        "CopyTextureToBuffer is not implemented for non-2D textures");
                } else {
                    for (uint32_t z = 0; z < copy->copySize.depthOrArrayLayers; ++z) {
                        // Copy the texture to the staging texture.
                        if (src.texture->GetFormat().HasDepthOrStencil()) {
                            d3d11DeviceContext1->CopySubresourceRegion(
                                stagingTexture.Get(), z, 0, 0, 0,
                                ToBackend(src.texture)->GetD3D11Resource(), subresource, nullptr);
                        } else {
                            D3D11_BOX srcBox;
                            srcBox.left = src.origin.x;
                            srcBox.right = src.origin.x + copy->copySize.width;
                            srcBox.top = src.origin.y;
                            srcBox.bottom = src.origin.y + copy->copySize.height;
                            srcBox.front = 0;
                            srcBox.back = 1;

                            d3d11DeviceContext1->CopySubresourceRegion(
                                stagingTexture.Get(), z, 0, 0, 0,
                                ToBackend(src.texture)->GetD3D11Resource(), subresource, &srcBox);
                        }
                    }
                }

                for (uint32_t z = 0; z < copy->copySize.depthOrArrayLayers; ++z) {
                    // Copy the staging texture to the buffer.
                    // The Map() will block until the GPU is done with the texture.
                    // TODO(dawn:1705): avoid blocking the CPU.
                    D3D11_MAPPED_SUBRESOURCE mappedResource;
                    DAWN_TRY(
                        CheckHRESULT(d3d11DeviceContext1->Map(stagingTexture.Get(), z,
                                                              D3D11_MAP_READ, 0, &mappedResource),
                                     "D3D11 map staging texture"));

                    Buffer* buffer = ToBackend(dst.buffer.Get());

                    Buffer::ScopedMap scopedMap;
                    DAWN_TRY_ASSIGN(scopedMap, Buffer::ScopedMap::Create(buffer));
                    DAWN_TRY(buffer->EnsureDataInitializedAsDestination(
                        commandContext, dst.offset, dst.bytesPerRow * copy->copySize.height));
                    uint8_t* pSrcData = reinterpret_cast<uint8_t*>(mappedResource.pData);

                    const TexelBlockInfo& blockInfo =
                        ToBackend(src.texture)->GetFormat().GetAspectInfo(src.aspect).block;
                    uint32_t memcpySize = blockInfo.byteSize * copy->copySize.width;
                    uint8_t* pDstData = scopedMap.GetMappedData() + dst.offset +
                                        dst.bytesPerRow * dst.rowsPerImage * z;
                    for (uint32_t y = 0; y < copy->copySize.height; ++y) {
                        memcpy(pDstData, pSrcData, memcpySize);
                        pDstData += dst.bytesPerRow;
                        pSrcData += mappedResource.RowPitch;
                    }
                    d3d11DeviceContext1->Unmap(stagingTexture.Get(), z);
                }

                break;
            }

            case Command::CopyTextureToTexture: {
                CopyTextureToTextureCmd* copy = mCommands.NextCommand<CopyTextureToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }

                DAWN_TRY(Texture::Copy(commandContext, copy));
                break;
            }

            case Command::ClearBuffer: {
                ClearBufferCmd* cmd = mCommands.NextCommand<ClearBufferCmd>();
                if (cmd->size == 0) {
                    // Skip no-op fills.
                    break;
                }
                Buffer* buffer = ToBackend(cmd->buffer.Get());
                DAWN_TRY(buffer->Clear(commandContext, 0, cmd->offset, cmd->size));
                break;
            }

            case Command::ResolveQuerySet: {
                return DAWN_UNIMPLEMENTED_ERROR("ResolveQuerySet unimplemented");
            }

            case Command::WriteTimestamp: {
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");
            }

            case Command::WriteBuffer: {
                WriteBufferCmd* cmd = mCommands.NextCommand<WriteBufferCmd>();
                if (cmd->size == 0) {
                    // Skip no-op writes.
                    continue;
                }

                Buffer* dstBuffer = ToBackend(cmd->buffer.Get());
                uint8_t* data = mCommands.NextData<uint8_t>(cmd->size);
                DAWN_TRY(dstBuffer->Write(commandContext, cmd->offset, data, cmd->size));

                break;
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                return DAWN_UNIMPLEMENTED_ERROR("Debug markers unimplemented");
            }

            default:
                return DAWN_FORMAT_INTERNAL_ERROR("Unknown command type: %d", type);
        }
    }

    return {};
}

}  // namespace dawn::native::d3d11
