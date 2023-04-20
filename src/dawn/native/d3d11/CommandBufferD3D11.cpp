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
namespace {

class BindGroupTracker : public BindGroupTrackerBase<false, uint64_t> {
  public:
    MaybeError Apply(CommandRecordingContext* commandContext) {
        BeforeApply();
        for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
            DAWN_TRY(
                ApplyBindGroup(commandContext, index, mBindGroups[index], mDynamicOffsets[index]));
        }
        AfterApply();
        return {};
    }

    void AfterDispatch(CommandRecordingContext* commandContext) {
        // Clear the UAVs after the dispatch, otherwise the buffer cannot be used as input in vertex
        // or pixel stage.
        for (UINT uav : mUnorderedAccessViews) {
            ID3D11UnorderedAccessView* nullUAV = nullptr;
            commandContext->GetD3D11DeviceContext1()->CSSetUnorderedAccessViews(uav, 1, &nullUAV,
                                                                                nullptr);
        }
        mUnorderedAccessViews.clear();
    }

  private:
    MaybeError ApplyBindGroup(CommandRecordingContext* commandContext,
                              BindGroupIndex index,
                              BindGroupBase* group,
                              const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets) {
        const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];
        for (BindingIndex bindingIndex{0}; bindingIndex < group->GetLayout()->GetBindingCount();
             ++bindingIndex) {
            const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
            const uint32_t bindingSlot = indices[bindingIndex];

            switch (bindingInfo.bindingType) {
                case BindingInfoType::Buffer: {
                    BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                    ID3D11Buffer* d3d11Buffer = ToBackend(binding.buffer)->GetD3D11Buffer();
                    auto offset = binding.offset;
                    if (bindingInfo.buffer.hasDynamicOffset) {
                        // Dynamic buffers are packed at the front of BindingIndices.
                        offset += dynamicOffsets[bindingIndex];
                    }

                    auto* deviceContext = commandContext->GetD3D11DeviceContext1();

                    switch (bindingInfo.buffer.type) {
                        case wgpu::BufferBindingType::Uniform: {
                            // https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
                            // Offset and size are measured in shader constants, which are 16 bytes
                            // (4*32-bit components). And the offsets and counts must be multiples
                            // of 16.
                            ASSERT(IsAligned(offset, 256));
                            UINT firstConstant = static_cast<UINT>(offset / 16);
                            UINT size = static_cast<UINT>(binding.size / 16);
                            UINT numConstants = Align(size, 16);

                            if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                                deviceContext->VSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                     &firstConstant, &numConstants);
                            }
                            if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                                deviceContext->PSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                     &firstConstant, &numConstants);
                            }
                            if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                                deviceContext->CSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                     &firstConstant, &numConstants);
                            }
                            break;
                        }
                        case wgpu::BufferBindingType::Storage: {
                            ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV, ToBackend(binding.buffer)
                                                          ->CreateD3D11UnorderedAccessView1(
                                                              0, binding.buffer->GetSize()));
                            UINT firstElement = offset / 4;
                            if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                                deviceContext->CSSetUnorderedAccessViews(
                                    bindingSlot, 1, d3d11UAV.GetAddressOf(), &firstElement);
                                // Record the bounded UAVs so that we can clear them after the
                                // dispatch.
                                mUnorderedAccessViews.emplace_back(bindingSlot);
                            } else {
                                return DAWN_INTERNAL_ERROR(
                                    "Storage buffers are only supported in compute shaders");
                            }
                            break;
                        }
                        case wgpu::BufferBindingType::ReadOnlyStorage: {
                            ComPtr<ID3D11ShaderResourceView> d3d11SRV;
                            DAWN_TRY_ASSIGN(d3d11SRV, ToBackend(binding.buffer)
                                                          ->CreateD3D11ShaderResourceView(
                                                              binding.offset, binding.size));
                            if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                                deviceContext->VSSetShaderResources(bindingSlot, 1,
                                                                    d3d11SRV.GetAddressOf());
                            }
                            if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                                deviceContext->PSSetShaderResources(bindingSlot, 1,
                                                                    d3d11SRV.GetAddressOf());
                            }
                            if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                                deviceContext->CSSetShaderResources(bindingSlot, 1,
                                                                    d3d11SRV.GetAddressOf());
                            }
                            break;
                        }
                        case wgpu::BufferBindingType::Undefined:
                            UNREACHABLE();
                    }
                    break;
                }

                case BindingInfoType::Sampler: {
                    Sampler* sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                    ID3D11SamplerState* d3d11SamplerState = sampler->GetD3D11SamplerState();
                    if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                        commandContext->GetD3D11DeviceContext1()->VSSetSamplers(bindingSlot, 1,
                                                                                &d3d11SamplerState);
                    }
                    if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                        commandContext->GetD3D11DeviceContext1()->PSSetSamplers(bindingSlot, 1,
                                                                                &d3d11SamplerState);
                    }
                    if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                        commandContext->GetD3D11DeviceContext1()->CSSetSamplers(bindingSlot, 1,
                                                                                &d3d11SamplerState);
                    }
                    break;
                }

                case BindingInfoType::Texture: {
                    TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                    ComPtr<ID3D11ShaderResourceView> srv;
                    DAWN_TRY_ASSIGN(srv, view->CreateD3D11ShaderResourceView());
                    if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                        commandContext->GetD3D11DeviceContext1()->VSSetShaderResources(
                            bindingSlot, 1, srv.GetAddressOf());
                    }
                    if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                        commandContext->GetD3D11DeviceContext1()->PSSetShaderResources(
                            bindingSlot, 1, srv.GetAddressOf());
                    }
                    if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                        commandContext->GetD3D11DeviceContext1()->CSSetShaderResources(
                            bindingSlot, 1, srv.GetAddressOf());
                    }
                    break;
                }

                case BindingInfoType::StorageTexture: {
                    return DAWN_UNIMPLEMENTED_ERROR("Storage textures are not supported");
                }

                case BindingInfoType::ExternalTexture: {
                    return DAWN_UNIMPLEMENTED_ERROR("External textures are not supported");
                    break;
                }
            }
        }
        return {};
    }

    std::vector<UINT> mUnorderedAccessViews;
};

}  // namespace

// Create CommandBuffer
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

MaybeError CommandBuffer::Execute() {
    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();

    ID3D11DeviceContext1* d3d11DeviceContext1 = commandContext->GetD3D11DeviceContext1();

    auto LazyClearSyncScope = [commandContext](const SyncScopeResourceUsage& scope) -> MaybeError {
        for (size_t i = 0; i < scope.textures.size(); i++) {
            Texture* texture = ToBackend(scope.textures[i]);

            // Clear subresources that are not render attachments. Render attachments will be
            // cleared in RecordBeginRenderPass by setting the loadop to clear when the texture
            // subresource has not been initialized before the render pass.
            DAWN_TRY(scope.textureUsages[i].Iterate([&](const SubresourceRange& range,
                                                        wgpu::TextureUsage usage) -> MaybeError {
                if (usage & ~wgpu::TextureUsage::RenderAttachment) {
                    DAWN_TRY(texture->EnsureSubresourceContentInitialized(commandContext, range));
                }
                return {};
            }));
        }

        for (BufferBase* buffer : scope.buffers) {
            DAWN_TRY(ToBackend(buffer)->EnsureDataInitialized(commandContext));
        }

        return {};
    };

    size_t nextComputePassNumber = 0;

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                mCommands.NextCommand<BeginComputePassCmd>();
                for (const SyncScopeResourceUsage& scope :
                     GetResourceUsages().computePasses[nextComputePassNumber].dispatchUsages) {
                    DAWN_TRY(LazyClearSyncScope(scope));
                }
                DAWN_TRY(ExecuteComputePass(commandContext));

                nextComputePassNumber++;
                break;
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
                HandleDebugCommands(commandContext, type);
                break;
            }

            default:
                return DAWN_FORMAT_INTERNAL_ERROR("Unknown command type: %d", type);
        }
    }

    return {};
}

MaybeError CommandBuffer::ExecuteComputePass(CommandRecordingContext* commandContext) {
    ComputePipeline* lastPipeline = nullptr;
    BindGroupTracker bindGroupTracker = {};

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                mCommands.NextCommand<EndComputePassCmd>();
                return {};
            }

            case Command::Dispatch: {
                DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();

                DAWN_TRY(bindGroupTracker.Apply(commandContext));

                DAWN_TRY(RecordNumWorkgroupsForDispatch(lastPipeline, commandContext, dispatch));
                commandContext->GetD3D11DeviceContext()->Dispatch(dispatch->x, dispatch->y,
                                                                  dispatch->z);
                bindGroupTracker.AfterDispatch(commandContext);

                break;
            }

            case Command::DispatchIndirect: {
                // TODO(1716): figure how to update num workgroups builtins
                DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();

                DAWN_TRY(bindGroupTracker.Apply(commandContext));

                uint64_t indirectBufferOffset = dispatch->indirectOffset;
                Buffer* indirectBuffer = ToBackend(dispatch->indirectBuffer.Get());

                commandContext->GetD3D11DeviceContext()->DispatchIndirect(
                    indirectBuffer->GetD3D11Buffer(), indirectBufferOffset);

                bindGroupTracker.AfterDispatch(commandContext);

                break;
            }

            case Command::SetComputePipeline: {
                SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                lastPipeline = ToBackend(cmd->pipeline).Get();
                lastPipeline->ApplyNow(commandContext);
                bindGroupTracker.OnSetPipeline(lastPipeline);
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();

                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = mCommands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                bindGroupTracker.OnSetBindGroup(cmd->index, cmd->group.Get(),
                                                cmd->dynamicOffsetCount, dynamicOffsets);

                break;
            }

            case Command::WriteTimestamp: {
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                HandleDebugCommands(commandContext, type);
                break;
            }

            default:
                UNREACHABLE();
        }
    }

    // EndComputePass should have been called
    UNREACHABLE();
}

void CommandBuffer::HandleDebugCommands(CommandRecordingContext* commandContext, Command command) {
    switch (command) {
        case Command::InsertDebugMarker: {
            InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
            std::wstring label = UTF8ToWStr(mCommands.NextData<char>(cmd->length + 1));
            commandContext->GetD3DUserDefinedAnnotation()->SetMarker(label.c_str());
            break;
        }

        case Command::PopDebugGroup: {
            std::ignore = mCommands.NextCommand<PopDebugGroupCmd>();
            commandContext->GetD3DUserDefinedAnnotation()->EndEvent();
            break;
        }

        case Command::PushDebugGroup: {
            PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
            std::wstring label = UTF8ToWStr(mCommands.NextData<char>(cmd->length + 1));
            commandContext->GetD3DUserDefinedAnnotation()->BeginEvent(label.c_str());
            break;
        }
        default:
            UNREACHABLE();
    }
}

MaybeError CommandBuffer::RecordNumWorkgroupsForDispatch(ComputePipeline* computePipeline,
                                                         CommandRecordingContext* commandContext,
                                                         DispatchCmd* dispatchCmd) {
    // TODO(dawn:1705): only update the uniform buffer when the value changes.
    static_assert(sizeof(DispatchCmd) == sizeof(uint32_t[3]));
    DAWN_TRY(commandContext->GetUniformBuffer()->Write(commandContext, 0, dispatchCmd,
                                                       sizeof(*dispatchCmd)));

    return {};
}

}  // namespace dawn::native::d3d11
