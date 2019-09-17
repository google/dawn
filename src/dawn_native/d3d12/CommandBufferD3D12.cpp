// Copyright 2017 The Dawn Authors
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

#include "dawn_native/d3d12/CommandBufferD3D12.h"

#include "common/Assert.h"
#include "dawn_native/BindGroupTracker.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/d3d12/BindGroupD3D12.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/ComputePipelineD3D12.h"
#include "dawn_native/d3d12/DescriptorHeapAllocator.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/RenderPipelineD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"
#include "dawn_native/d3d12/SamplerD3D12.h"
#include "dawn_native/d3d12/TextureCopySplitter.h"
#include "dawn_native/d3d12/TextureD3D12.h"
#include "dawn_native/d3d12/UtilsD3D12.h"

#include <deque>

namespace dawn_native { namespace d3d12 {

    namespace {

        DXGI_FORMAT DXGIIndexFormat(dawn::IndexFormat format) {
            switch (format) {
                case dawn::IndexFormat::Uint16:
                    return DXGI_FORMAT_R16_UINT;
                case dawn::IndexFormat::Uint32:
                    return DXGI_FORMAT_R32_UINT;
                default:
                    UNREACHABLE();
            }
        }

        bool CanUseCopyResource(const uint32_t sourceNumMipLevels,
                                const Extent3D& srcSize,
                                const Extent3D& dstSize,
                                const Extent3D& copySize) {
            if (sourceNumMipLevels == 1 && srcSize.width == dstSize.width &&
                srcSize.height == dstSize.height && srcSize.depth == dstSize.depth &&
                srcSize.width == copySize.width && srcSize.height == copySize.height &&
                srcSize.depth == copySize.depth) {
                return true;
            }

            return false;
        }

        struct OMSetRenderTargetArgs {
            unsigned int numRTVs = 0;
            std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxColorAttachments> RTVs = {};
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
        };

    }  // anonymous namespace

    class BindGroupStateTracker : public BindGroupTrackerBase<BindGroup*, false> {
      public:
        BindGroupStateTracker(Device* device) : BindGroupTrackerBase(), mDevice(device) {
        }

        void SetInComputePass(bool inCompute_) {
            mInCompute = inCompute_;
        }

        void AllocateDescriptorHeaps(Device* device) {
            // This function should only be called once.
            ASSERT(mCbvSrvUavGPUDescriptorHeap.Get() == nullptr &&
                   mSamplerGPUDescriptorHeap.Get() == nullptr);

            DescriptorHeapAllocator* descriptorHeapAllocator = device->GetDescriptorHeapAllocator();

            if (mCbvSrvUavDescriptorHeapSize > 0) {
                mCbvSrvUavGPUDescriptorHeap = descriptorHeapAllocator->AllocateGPUHeap(
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, mCbvSrvUavDescriptorHeapSize);
            }

            if (mSamplerDescriptorHeapSize > 0) {
                mSamplerGPUDescriptorHeap = descriptorHeapAllocator->AllocateGPUHeap(
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, mSamplerDescriptorHeapSize);
            }

            uint32_t cbvSrvUavDescriptorIndex = 0;
            uint32_t samplerDescriptorIndex = 0;
            for (BindGroup* group : mBindGroupsToAllocate) {
                ASSERT(group);
                ASSERT(cbvSrvUavDescriptorIndex +
                           ToBackend(group->GetLayout())->GetCbvUavSrvDescriptorCount() <=
                       mCbvSrvUavDescriptorHeapSize);
                ASSERT(samplerDescriptorIndex +
                           ToBackend(group->GetLayout())->GetSamplerDescriptorCount() <=
                       mSamplerDescriptorHeapSize);
                group->AllocateDescriptors(mCbvSrvUavGPUDescriptorHeap, &cbvSrvUavDescriptorIndex,
                                           mSamplerGPUDescriptorHeap, &samplerDescriptorIndex);
            }

            ASSERT(cbvSrvUavDescriptorIndex == mCbvSrvUavDescriptorHeapSize);
            ASSERT(samplerDescriptorIndex == mSamplerDescriptorHeapSize);
        }

        // This function must only be called before calling AllocateDescriptorHeaps().
        void TrackSetBindGroup(BindGroup* group, uint32_t index, uint32_t indexInSubmit) {
            if (mBindGroups[index] != group) {
                mBindGroups[index] = group;
                if (!group->TestAndSetCounted(mDevice->GetPendingCommandSerial(), indexInSubmit)) {
                    const BindGroupLayout* layout = ToBackend(group->GetLayout());

                    mCbvSrvUavDescriptorHeapSize += layout->GetCbvUavSrvDescriptorCount();
                    mSamplerDescriptorHeapSize += layout->GetSamplerDescriptorCount();
                    mBindGroupsToAllocate.push_back(group);
                }
            }
        }

        void Apply(ID3D12GraphicsCommandList* commandList) {
            for (uint32_t index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
                ApplyBindGroup(commandList, ToBackend(mPipelineLayout), index, mBindGroups[index],
                               mDynamicOffsetCounts[index], mDynamicOffsets[index].data());
            }
            DidApply();
        }

        void Reset() {
            for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
                mBindGroups[i] = nullptr;
            }
        }

        void SetID3D12DescriptorHeaps(ComPtr<ID3D12GraphicsCommandList> commandList) {
            ASSERT(commandList != nullptr);
            ID3D12DescriptorHeap* descriptorHeaps[2] = {mCbvSrvUavGPUDescriptorHeap.Get(),
                                                        mSamplerGPUDescriptorHeap.Get()};
            if (descriptorHeaps[0] && descriptorHeaps[1]) {
                commandList->SetDescriptorHeaps(2, descriptorHeaps);
            } else if (descriptorHeaps[0]) {
                commandList->SetDescriptorHeaps(1, descriptorHeaps);
            } else if (descriptorHeaps[1]) {
                commandList->SetDescriptorHeaps(1, &descriptorHeaps[1]);
            }
        }

      private:
        void ApplyBindGroup(ID3D12GraphicsCommandList* commandList,
                            PipelineLayout* pipelineLayout,
                            uint32_t index,
                            BindGroup* group,
                            uint32_t dynamicOffsetCount,
                            uint64_t* dynamicOffsets) {
            // Usually, the application won't set the same offsets many times,
            // so always try to apply dynamic offsets even if the offsets stay the same
            if (dynamicOffsetCount) {
                // Update dynamic offsets
                const BindGroupLayout::LayoutBindingInfo& layout =
                    group->GetLayout()->GetBindingInfo();
                uint32_t currentDynamicBufferIndex = 0;

                for (uint32_t bindingIndex : IterateBitSet(layout.dynamic)) {
                    ASSERT(dynamicOffsetCount > 0);
                    uint32_t parameterIndex =
                        pipelineLayout->GetDynamicRootParameterIndex(index, bindingIndex);
                    BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);

                    // Calculate buffer locations that root descriptors links to. The location
                    // is (base buffer location + initial offset + dynamic offset)
                    uint64_t dynamicOffset = dynamicOffsets[currentDynamicBufferIndex];
                    uint64_t offset = binding.offset + dynamicOffset;
                    D3D12_GPU_VIRTUAL_ADDRESS bufferLocation =
                        ToBackend(binding.buffer)->GetVA() + offset;

                    switch (layout.types[bindingIndex]) {
                        case dawn::BindingType::UniformBuffer:
                            if (mInCompute) {
                                commandList->SetComputeRootConstantBufferView(parameterIndex,
                                                                              bufferLocation);
                            } else {
                                commandList->SetGraphicsRootConstantBufferView(parameterIndex,
                                                                               bufferLocation);
                            }
                            break;
                        case dawn::BindingType::StorageBuffer:
                            if (mInCompute) {
                                commandList->SetComputeRootUnorderedAccessView(parameterIndex,
                                                                               bufferLocation);
                            } else {
                                commandList->SetGraphicsRootUnorderedAccessView(parameterIndex,
                                                                                bufferLocation);
                            }
                            break;
                        case dawn::BindingType::SampledTexture:
                        case dawn::BindingType::Sampler:
                        case dawn::BindingType::StorageTexture:
                        case dawn::BindingType::ReadonlyStorageBuffer:
                            UNREACHABLE();
                            break;
                    }

                    ++currentDynamicBufferIndex;
                }
            }

            // It's not necessary to update descriptor tables if only the dynamic offset changed.
            if (!mDirtyBindGroups[index]) {
                return;
            }

            uint32_t cbvUavSrvCount = ToBackend(group->GetLayout())->GetCbvUavSrvDescriptorCount();
            uint32_t samplerCount = ToBackend(group->GetLayout())->GetSamplerDescriptorCount();

            if (cbvUavSrvCount > 0) {
                uint32_t parameterIndex = pipelineLayout->GetCbvUavSrvRootParameterIndex(index);

                if (mInCompute) {
                    commandList->SetComputeRootDescriptorTable(
                        parameterIndex,
                        mCbvSrvUavGPUDescriptorHeap.GetGPUHandle(group->GetCbvUavSrvHeapOffset()));
                } else {
                    commandList->SetGraphicsRootDescriptorTable(
                        parameterIndex,
                        mCbvSrvUavGPUDescriptorHeap.GetGPUHandle(group->GetCbvUavSrvHeapOffset()));
                }
            }

            if (samplerCount > 0) {
                uint32_t parameterIndex = pipelineLayout->GetSamplerRootParameterIndex(index);

                if (mInCompute) {
                    commandList->SetComputeRootDescriptorTable(
                        parameterIndex,
                        mSamplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                } else {
                    commandList->SetGraphicsRootDescriptorTable(
                        parameterIndex,
                        mSamplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                }
            }
        }

        uint32_t mCbvSrvUavDescriptorHeapSize = 0;
        uint32_t mSamplerDescriptorHeapSize = 0;
        std::deque<BindGroup*> mBindGroupsToAllocate = {};
        bool mInCompute = false;

        DescriptorHeapHandle mCbvSrvUavGPUDescriptorHeap = {};
        DescriptorHeapHandle mSamplerGPUDescriptorHeap = {};

        Device* mDevice;
    };

    class RenderPassDescriptorHeapTracker {
      public:
        RenderPassDescriptorHeapTracker(Device* device) : mDevice(device) {
        }

        // This function must only be called before calling AllocateRTVAndDSVHeaps().
        void TrackRenderPass(const BeginRenderPassCmd* renderPass) {
            DAWN_ASSERT(mRTVHeap.Get() == nullptr && mDSVHeap.Get() == nullptr);

            mNumRTVs += static_cast<uint32_t>(
                renderPass->attachmentState->GetColorAttachmentsMask().count());
            if (renderPass->attachmentState->HasDepthStencilAttachment()) {
                ++mNumDSVs;
            }
        }

        void AllocateRTVAndDSVHeaps() {
            // This function should only be called once.
            DAWN_ASSERT(mRTVHeap.Get() == nullptr && mDSVHeap.Get() == nullptr);
            DescriptorHeapAllocator* allocator = mDevice->GetDescriptorHeapAllocator();
            if (mNumRTVs > 0) {
                mRTVHeap = allocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, mNumRTVs);
            }
            if (mNumDSVs > 0) {
                mDSVHeap = allocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, mNumDSVs);
            }
        }

        // TODO(jiawei.shao@intel.com): use hash map <RenderPass, OMSetRenderTargetArgs> as
        // cache to avoid redundant RTV and DSV memory allocations.
        OMSetRenderTargetArgs GetSubpassOMSetRenderTargetArgs(BeginRenderPassCmd* renderPass) {
            OMSetRenderTargetArgs args = {};

            unsigned int rtvIndex = 0;
            uint32_t rtvCount = static_cast<uint32_t>(
                renderPass->attachmentState->GetColorAttachmentsMask().count());
            DAWN_ASSERT(mAllocatedRTVs + rtvCount <= mNumRTVs);
            for (uint32_t i :
                 IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                TextureView* view = ToBackend(renderPass->colorAttachments[i].view).Get();
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRTVHeap.GetCPUHandle(mAllocatedRTVs);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = view->GetRTVDescriptor();
                mDevice->GetD3D12Device()->CreateRenderTargetView(
                    ToBackend(view->GetTexture())->GetD3D12Resource(), &rtvDesc, rtvHandle);
                args.RTVs[i] = rtvHandle;

                ++rtvIndex;
                ++mAllocatedRTVs;
            }
            args.numRTVs = rtvIndex;

            if (renderPass->attachmentState->HasDepthStencilAttachment()) {
                DAWN_ASSERT(mAllocatedDSVs < mNumDSVs);
                TextureView* view = ToBackend(renderPass->depthStencilAttachment.view).Get();
                D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDSVHeap.GetCPUHandle(mAllocatedDSVs);
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = view->GetDSVDescriptor();
                mDevice->GetD3D12Device()->CreateDepthStencilView(
                    ToBackend(view->GetTexture())->GetD3D12Resource(), &dsvDesc, dsvHandle);
                args.dsv = dsvHandle;

                ++mAllocatedDSVs;
            }

            return args;
        }

        bool IsHeapAllocationCompleted() const {
            return mNumRTVs == mAllocatedRTVs && mNumDSVs == mAllocatedDSVs;
        }

      private:
        Device* mDevice;
        DescriptorHeapHandle mRTVHeap = {};
        DescriptorHeapHandle mDSVHeap = {};
        uint32_t mNumRTVs = 0;
        uint32_t mNumDSVs = 0;

        uint32_t mAllocatedRTVs = 0;
        uint32_t mAllocatedDSVs = 0;
    };

    namespace {

        class VertexBufferTracker {
          public:
            void OnSetVertexBuffers(uint32_t startSlot,
                                    uint32_t count,
                                    Ref<BufferBase>* buffers,
                                    uint64_t* offsets) {
                mStartSlot = std::min(mStartSlot, startSlot);
                mEndSlot = std::max(mEndSlot, startSlot + count);

                for (uint32_t i = 0; i < count; ++i) {
                    Buffer* buffer = ToBackend(buffers[i].Get());
                    auto* d3d12BufferView = &mD3D12BufferViews[startSlot + i];
                    d3d12BufferView->BufferLocation = buffer->GetVA() + offsets[i];
                    d3d12BufferView->SizeInBytes = buffer->GetSize() - offsets[i];
                    // The bufferView stride is set based on the input state before a draw.
                }
            }

            void Apply(ID3D12GraphicsCommandList* commandList,
                       const RenderPipeline* renderPipeline) {
                ASSERT(renderPipeline != nullptr);

                std::bitset<kMaxVertexBuffers> inputsMask = renderPipeline->GetInputsSetMask();

                uint32_t startSlot = mStartSlot;
                uint32_t endSlot = mEndSlot;

                // If the input state has changed, we need to update the StrideInBytes
                // for the D3D12 buffer views. We also need to extend the dirty range to
                // touch all these slots because the stride may have changed.
                if (mLastAppliedRenderPipeline != renderPipeline) {
                    mLastAppliedRenderPipeline = renderPipeline;

                    for (uint32_t slot : IterateBitSet(inputsMask)) {
                        startSlot = std::min(startSlot, slot);
                        endSlot = std::max(endSlot, slot + 1);
                        mD3D12BufferViews[slot].StrideInBytes =
                            renderPipeline->GetInput(slot).stride;
                    }
                }

                if (endSlot <= startSlot) {
                    return;
                }

                // mD3D12BufferViews is kept up to date with the most recent data passed
                // to SetVertexBuffers. This makes it correct to only track the start
                // and end of the dirty range. When Apply is called,
                // we will at worst set non-dirty vertex buffers in duplicate.
                uint32_t count = endSlot - startSlot;
                commandList->IASetVertexBuffers(startSlot, count, &mD3D12BufferViews[startSlot]);

                mStartSlot = kMaxVertexBuffers;
                mEndSlot = 0;
            }

          private:
            // startSlot and endSlot indicate the range of dirty vertex buffers.
            // If there are multiple calls to SetVertexBuffers, the start and end
            // represent the union of the dirty ranges (the union may have non-dirty
            // data in the middle of the range).
            const RenderPipeline* mLastAppliedRenderPipeline = nullptr;
            uint32_t mStartSlot = kMaxVertexBuffers;
            uint32_t mEndSlot = 0;
            std::array<D3D12_VERTEX_BUFFER_VIEW, kMaxVertexBuffers> mD3D12BufferViews = {};
        };

        class IndexBufferTracker {
          public:
            void OnSetIndexBuffer(Buffer* buffer, uint64_t offset) {
                mD3D12BufferView.BufferLocation = buffer->GetVA() + offset;
                mD3D12BufferView.SizeInBytes = buffer->GetSize() - offset;

                // We don't need to dirty the state unless BufferLocation or SizeInBytes
                // change, but most of the time this will always be the case.
                mLastAppliedIndexFormat = DXGI_FORMAT_UNKNOWN;
            }

            void OnSetPipeline(const RenderPipelineBase* pipeline) {
                mD3D12BufferView.Format =
                    DXGIIndexFormat(pipeline->GetVertexInputDescriptor()->indexFormat);
            }

            void Apply(ID3D12GraphicsCommandList* commandList) {
                if (mD3D12BufferView.Format == mLastAppliedIndexFormat) {
                    return;
                }

                commandList->IASetIndexBuffer(&mD3D12BufferView);
                mLastAppliedIndexFormat = mD3D12BufferView.Format;
            }

          private:
            DXGI_FORMAT mLastAppliedIndexFormat = DXGI_FORMAT_UNKNOWN;
            D3D12_INDEX_BUFFER_VIEW mD3D12BufferView = {};
        };

        void AllocateAndSetDescriptorHeaps(Device* device,
                                           BindGroupStateTracker* bindingTracker,
                                           RenderPassDescriptorHeapTracker* renderPassTracker,
                                           CommandIterator* commands,
                                           uint32_t indexInSubmit) {
            {
                Command type;

                auto HandleCommand = [&](CommandIterator* commands, Command type) {
                    switch (type) {
                        case Command::SetBindGroup: {
                            SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                            BindGroup* group = ToBackend(cmd->group.Get());
                            if (cmd->dynamicOffsetCount) {
                                commands->NextData<uint64_t>(cmd->dynamicOffsetCount);
                            }
                            bindingTracker->TrackSetBindGroup(group, cmd->index, indexInSubmit);
                        } break;
                        case Command::BeginRenderPass: {
                            BeginRenderPassCmd* cmd = commands->NextCommand<BeginRenderPassCmd>();
                            renderPassTracker->TrackRenderPass(cmd);
                        } break;
                        default:
                            SkipCommand(commands, type);
                    }
                };

                while (commands->NextCommandId(&type)) {
                    switch (type) {
                        case Command::ExecuteBundles: {
                            ExecuteBundlesCmd* cmd = commands->NextCommand<ExecuteBundlesCmd>();
                            auto bundles = commands->NextData<Ref<RenderBundleBase>>(cmd->count);

                            for (uint32_t i = 0; i < cmd->count; ++i) {
                                CommandIterator* commands = bundles[i]->GetCommands();
                                commands->Reset();
                                while (commands->NextCommandId(&type)) {
                                    HandleCommand(commands, type);
                                }
                            }
                        } break;
                        default:
                            HandleCommand(commands, type);
                            break;
                    }
                }

                commands->Reset();
            }

            renderPassTracker->AllocateRTVAndDSVHeaps();
            bindingTracker->AllocateDescriptorHeaps(device);
        }

        void ResolveMultisampledRenderPass(ComPtr<ID3D12GraphicsCommandList> commandList,
                                           BeginRenderPassCmd* renderPass) {
            ASSERT(renderPass != nullptr);

            for (uint32_t i :
                 IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                TextureViewBase* resolveTarget =
                    renderPass->colorAttachments[i].resolveTarget.Get();
                if (resolveTarget == nullptr) {
                    continue;
                }

                Texture* colorTexture =
                    ToBackend(renderPass->colorAttachments[i].view->GetTexture());
                Texture* resolveTexture = ToBackend(resolveTarget->GetTexture());

                // Transition the usages of the color attachment and resolve target.
                colorTexture->TransitionUsageNow(commandList, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
                resolveTexture->TransitionUsageNow(commandList, D3D12_RESOURCE_STATE_RESOLVE_DEST);

                // Do MSAA resolve with ResolveSubResource().
                ID3D12Resource* colorTextureHandle = colorTexture->GetD3D12Resource();
                ID3D12Resource* resolveTextureHandle = resolveTexture->GetD3D12Resource();
                const uint32_t resolveTextureSubresourceIndex = resolveTexture->GetSubresourceIndex(
                    resolveTarget->GetBaseMipLevel(), resolveTarget->GetBaseArrayLayer());
                constexpr uint32_t kColorTextureSubresourceIndex = 0;
                commandList->ResolveSubresource(
                    resolveTextureHandle, resolveTextureSubresourceIndex, colorTextureHandle,
                    kColorTextureSubresourceIndex, colorTexture->GetD3D12Format());
            }
        }

    }  // anonymous namespace

    CommandBuffer::CommandBuffer(CommandEncoderBase* encoder,
                                 const CommandBufferDescriptor* descriptor)
        : CommandBufferBase(encoder, descriptor), mCommands(encoder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::RecordCommands(ComPtr<ID3D12GraphicsCommandList> commandList,
                                       uint32_t indexInSubmit) {
        Device* device = ToBackend(GetDevice());
        BindGroupStateTracker bindingTracker(device);
        RenderPassDescriptorHeapTracker renderPassTracker(device);

        // Precompute the allocation of bindgroups in descriptor heaps
        // TODO(cwallez@chromium.org): Iterating over all the commands here is inefficient. We
        // should have a system where commands and descriptors are recorded in parallel then the
        // heaps set using a small CommandList inserted just before the main CommandList.
        {
            AllocateAndSetDescriptorHeaps(device, &bindingTracker, &renderPassTracker, &mCommands,
                                          indexInSubmit);
            bindingTracker.Reset();
            bindingTracker.SetID3D12DescriptorHeaps(commandList);
        }

        // Records the necessary barriers for the resource usage pre-computed by the frontend
        auto TransitionForPass = [](ComPtr<ID3D12GraphicsCommandList> commandList,
                                    const PassResourceUsage& usages) {
            std::vector<D3D12_RESOURCE_BARRIER> barriers;

            for (size_t i = 0; i < usages.buffers.size(); ++i) {
                D3D12_RESOURCE_BARRIER barrier;
                if (ToBackend(usages.buffers[i])
                        ->TransitionUsageAndGetResourceBarrier(&barrier, usages.bufferUsages[i])) {
                    barriers.push_back(barrier);
                }
            }

            for (size_t i = 0; i < usages.textures.size(); ++i) {
                Texture* texture = ToBackend(usages.textures[i]);
                // Clear textures that are not output attachments. Output attachments will be
                // cleared during record render pass if the texture subresource has not been
                // initialized before the render pass.
                if (!(usages.textureUsages[i] & dawn::TextureUsage::OutputAttachment)) {
                    texture->EnsureSubresourceContentInitialized(
                        commandList, 0, texture->GetNumMipLevels(), 0, texture->GetArrayLayers());
                }
            }

            for (size_t i = 0; i < usages.textures.size(); ++i) {
                D3D12_RESOURCE_BARRIER barrier;
                if (ToBackend(usages.textures[i])
                        ->TransitionUsageAndGetResourceBarrier(&barrier, usages.textureUsages[i])) {
                    barriers.push_back(barrier);
                }
            }

            if (barriers.size()) {
                commandList->ResourceBarrier(barriers.size(), barriers.data());
            }
        };

        const std::vector<PassResourceUsage>& passResourceUsages = GetResourceUsages().perPass;
        uint32_t nextPassNumber = 0;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();

                    TransitionForPass(commandList, passResourceUsages[nextPassNumber]);
                    bindingTracker.SetInComputePass(true);
                    RecordComputePass(commandList, &bindingTracker);

                    nextPassNumber++;
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* beginRenderPassCmd =
                        mCommands.NextCommand<BeginRenderPassCmd>();

                    TransitionForPass(commandList, passResourceUsages[nextPassNumber]);
                    bindingTracker.SetInComputePass(false);
                    RecordRenderPass(commandList, &bindingTracker, &renderPassTracker,
                                     beginRenderPassCmd);

                    nextPassNumber++;
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                    Buffer* srcBuffer = ToBackend(copy->source.Get());
                    Buffer* dstBuffer = ToBackend(copy->destination.Get());

                    srcBuffer->TransitionUsageNow(commandList, dawn::BufferUsage::CopySrc);
                    dstBuffer->TransitionUsageNow(commandList, dawn::BufferUsage::CopyDst);

                    commandList->CopyBufferRegion(
                        dstBuffer->GetD3D12Resource().Get(), copy->destinationOffset,
                        srcBuffer->GetD3D12Resource().Get(), copy->sourceOffset, copy->size);
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                    Buffer* buffer = ToBackend(copy->source.buffer.Get());
                    Texture* texture = ToBackend(copy->destination.texture.Get());

                    if (IsCompleteSubresourceCopiedTo(texture, copy->copySize,
                                                      copy->destination.mipLevel)) {
                        texture->SetIsSubresourceContentInitialized(
                            copy->destination.mipLevel, 1, copy->destination.arrayLayer, 1);
                    } else {
                        texture->EnsureSubresourceContentInitialized(
                            commandList, copy->destination.mipLevel, 1,
                            copy->destination.arrayLayer, 1);
                    }

                    buffer->TransitionUsageNow(commandList, dawn::BufferUsage::CopySrc);
                    texture->TransitionUsageNow(commandList, dawn::TextureUsage::CopyDst);

                    auto copySplit = ComputeTextureCopySplit(
                        copy->destination.origin, copy->copySize, texture->GetFormat(),
                        copy->source.offset, copy->source.rowPitch, copy->source.imageHeight);

                    D3D12_TEXTURE_COPY_LOCATION textureLocation =
                        ComputeTextureCopyLocationForTexture(texture, copy->destination.mipLevel,
                                                             copy->destination.arrayLayer);

                    for (uint32_t i = 0; i < copySplit.count; ++i) {
                        TextureCopySplit::CopyInfo& info = copySplit.copies[i];

                        D3D12_TEXTURE_COPY_LOCATION bufferLocation =
                            ComputeBufferLocationForCopyTextureRegion(
                                texture, buffer->GetD3D12Resource().Get(), info.bufferSize,
                                copySplit.offset, copy->source.rowPitch);
                        D3D12_BOX sourceRegion =
                            ComputeD3D12BoxFromOffsetAndSize(info.bufferOffset, info.copySize);

                        commandList->CopyTextureRegion(&textureLocation, info.textureOffset.x,
                                                       info.textureOffset.y, info.textureOffset.z,
                                                       &bufferLocation, &sourceRegion);
                    }
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    Texture* texture = ToBackend(copy->source.texture.Get());
                    Buffer* buffer = ToBackend(copy->destination.buffer.Get());

                    texture->EnsureSubresourceContentInitialized(commandList, copy->source.mipLevel,
                                                                 1, copy->source.arrayLayer, 1);

                    texture->TransitionUsageNow(commandList, dawn::TextureUsage::CopySrc);
                    buffer->TransitionUsageNow(commandList, dawn::BufferUsage::CopyDst);

                    TextureCopySplit copySplit = ComputeTextureCopySplit(
                        copy->source.origin, copy->copySize, texture->GetFormat(),
                        copy->destination.offset, copy->destination.rowPitch,
                        copy->destination.imageHeight);

                    D3D12_TEXTURE_COPY_LOCATION textureLocation =
                        ComputeTextureCopyLocationForTexture(texture, copy->source.mipLevel,
                                                             copy->source.arrayLayer);

                    for (uint32_t i = 0; i < copySplit.count; ++i) {
                        TextureCopySplit::CopyInfo& info = copySplit.copies[i];

                        D3D12_TEXTURE_COPY_LOCATION bufferLocation =
                            ComputeBufferLocationForCopyTextureRegion(
                                texture, buffer->GetD3D12Resource().Get(), info.bufferSize,
                                copySplit.offset, copy->destination.rowPitch);

                        D3D12_BOX sourceRegion =
                            ComputeD3D12BoxFromOffsetAndSize(info.textureOffset, info.copySize);

                        commandList->CopyTextureRegion(&bufferLocation, info.bufferOffset.x,
                                                       info.bufferOffset.y, info.bufferOffset.z,
                                                       &textureLocation, &sourceRegion);
                    }
                } break;

                case Command::CopyTextureToTexture: {
                    CopyTextureToTextureCmd* copy =
                        mCommands.NextCommand<CopyTextureToTextureCmd>();

                    Texture* source = ToBackend(copy->source.texture.Get());
                    Texture* destination = ToBackend(copy->destination.texture.Get());

                    source->EnsureSubresourceContentInitialized(commandList, copy->source.mipLevel,
                                                                1, copy->source.arrayLayer, 1);
                    if (IsCompleteSubresourceCopiedTo(destination, copy->copySize,
                                                      copy->destination.mipLevel)) {
                        destination->SetIsSubresourceContentInitialized(
                            copy->destination.mipLevel, 1, copy->destination.arrayLayer, 1);
                    } else {
                        destination->EnsureSubresourceContentInitialized(
                            commandList, copy->destination.mipLevel, 1,
                            copy->destination.arrayLayer, 1);
                    }
                    source->TransitionUsageNow(commandList, dawn::TextureUsage::CopySrc);
                    destination->TransitionUsageNow(commandList, dawn::TextureUsage::CopyDst);

                    if (CanUseCopyResource(source->GetNumMipLevels(), source->GetSize(),
                                           destination->GetSize(), copy->copySize)) {
                        commandList->CopyResource(destination->GetD3D12Resource(),
                                                  source->GetD3D12Resource());
                    } else {
                        D3D12_TEXTURE_COPY_LOCATION srcLocation =
                            ComputeTextureCopyLocationForTexture(source, copy->source.mipLevel,
                                                                 copy->source.arrayLayer);

                        D3D12_TEXTURE_COPY_LOCATION dstLocation =
                            ComputeTextureCopyLocationForTexture(destination,
                                                                 copy->destination.mipLevel,
                                                                 copy->destination.arrayLayer);

                        D3D12_BOX sourceRegion =
                            ComputeD3D12BoxFromOffsetAndSize(copy->source.origin, copy->copySize);

                        commandList->CopyTextureRegion(
                            &dstLocation, copy->destination.origin.x, copy->destination.origin.y,
                            copy->destination.origin.z, &srcLocation, &sourceRegion);
                    }
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        DAWN_ASSERT(renderPassTracker.IsHeapAllocationCompleted());
    }

    void CommandBuffer::RecordComputePass(ComPtr<ID3D12GraphicsCommandList> commandList,
                                          BindGroupStateTracker* bindingTracker) {
        PipelineLayout* lastLayout = nullptr;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();

                    bindingTracker->Apply(commandList.Get());
                    commandList->Dispatch(dispatch->x, dispatch->y, dispatch->z);
                } break;

                case Command::DispatchIndirect: {
                    DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();

                    bindingTracker->Apply(commandList.Get());
                    Buffer* buffer = ToBackend(dispatch->indirectBuffer.Get());
                    ComPtr<ID3D12CommandSignature> signature =
                        ToBackend(GetDevice())->GetDispatchIndirectSignature();
                    commandList->ExecuteIndirect(signature.Get(), 1,
                                                 buffer->GetD3D12Resource().Get(),
                                                 dispatch->indirectOffset, nullptr, 0);
                } break;

                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    return;
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    ComputePipeline* pipeline = ToBackend(cmd->pipeline).Get();
                    PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                    commandList->SetComputeRootSignature(layout->GetRootSignature().Get());
                    commandList->SetPipelineState(pipeline->GetPipelineState().Get());

                    bindingTracker->OnSetPipeline(pipeline);

                    lastLayout = layout;
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    BindGroup* group = ToBackend(cmd->group.Get());
                    uint64_t* dynamicOffsets = nullptr;

                    if (cmd->dynamicOffsetCount > 0) {
                        dynamicOffsets = mCommands.NextData<uint64_t>(cmd->dynamicOffsetCount);
                    }

                    bindingTracker->OnSetBindGroup(cmd->index, group, cmd->dynamicOffsetCount,
                                                   dynamicOffsets);
                } break;

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                    const char* label = mCommands.NextData<char>(cmd->length + 1);

                    if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                        // PIX color is 1 byte per channel in ARGB format
                        constexpr uint64_t kPIXBlackColor = 0xff000000;
                        ToBackend(GetDevice())
                            ->GetFunctions()
                            ->pixSetMarkerOnCommandList(commandList.Get(), kPIXBlackColor, label);
                    }
                } break;

                case Command::PopDebugGroup: {
                    mCommands.NextCommand<PopDebugGroupCmd>();

                    if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                        ToBackend(GetDevice())
                            ->GetFunctions()
                            ->pixEndEventOnCommandList(commandList.Get());
                    }
                } break;

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                    const char* label = mCommands.NextData<char>(cmd->length + 1);

                    if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                        // PIX color is 1 byte per channel in ARGB format
                        constexpr uint64_t kPIXBlackColor = 0xff000000;
                        ToBackend(GetDevice())
                            ->GetFunctions()
                            ->pixBeginEventOnCommandList(commandList.Get(), kPIXBlackColor, label);
                    }
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

    void CommandBuffer::RecordRenderPass(ComPtr<ID3D12GraphicsCommandList> commandList,
                                         BindGroupStateTracker* bindingTracker,
                                         RenderPassDescriptorHeapTracker* renderPassTracker,
                                         BeginRenderPassCmd* renderPass) {
        OMSetRenderTargetArgs args = renderPassTracker->GetSubpassOMSetRenderTargetArgs(renderPass);

        // Clear framebuffer attachments as needed and transition to render target
        {
            for (uint32_t i :
                 IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                auto& attachmentInfo = renderPass->colorAttachments[i];
                TextureView* view = ToBackend(attachmentInfo.view.Get());

                // Load op - color
                ASSERT(view->GetLevelCount() == 1);
                ASSERT(view->GetLayerCount() == 1);
                if (attachmentInfo.loadOp == dawn::LoadOp::Clear ||
                    (attachmentInfo.loadOp == dawn::LoadOp::Load &&
                     !view->GetTexture()->IsSubresourceContentInitialized(
                         view->GetBaseMipLevel(), 1, view->GetBaseArrayLayer(), 1))) {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = args.RTVs[i];
                    commandList->ClearRenderTargetView(handle, &attachmentInfo.clearColor.r, 0,
                                                       nullptr);
                }

                TextureView* resolveView = ToBackend(attachmentInfo.resolveTarget.Get());
                if (resolveView != nullptr) {
                    // We need to set the resolve target to initialized so that it does not get
                    // cleared later in the pipeline. The texture will be resolved from the source
                    // color attachment, which will be correctly initialized.
                    ToBackend(resolveView->GetTexture())
                        ->SetIsSubresourceContentInitialized(
                            resolveView->GetBaseMipLevel(), resolveView->GetLevelCount(),
                            resolveView->GetBaseArrayLayer(), resolveView->GetLayerCount());
                }

                switch (attachmentInfo.storeOp) {
                    case dawn::StoreOp::Store: {
                        view->GetTexture()->SetIsSubresourceContentInitialized(
                            view->GetBaseMipLevel(), 1, view->GetBaseArrayLayer(), 1);
                    } break;

                    default: { UNREACHABLE(); } break;
                }
            }

            if (renderPass->attachmentState->HasDepthStencilAttachment()) {
                auto& attachmentInfo = renderPass->depthStencilAttachment;
                Texture* texture = ToBackend(renderPass->depthStencilAttachment.view->GetTexture());
                TextureView* view = ToBackend(attachmentInfo.view.Get());
                float clearDepth = attachmentInfo.clearDepth;
                // TODO(kainino@chromium.org): investigate: should the Dawn clear
                // stencil type be uint8_t?
                uint8_t clearStencil = static_cast<uint8_t>(attachmentInfo.clearStencil);

                // Load op - depth/stencil
                bool doDepthClear = texture->GetFormat().HasDepth() &&
                                    (attachmentInfo.depthLoadOp == dawn::LoadOp::Clear);
                bool doStencilClear = texture->GetFormat().HasStencil() &&
                                      (attachmentInfo.stencilLoadOp == dawn::LoadOp::Clear);

                D3D12_CLEAR_FLAGS clearFlags = {};
                if (doDepthClear) {
                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                }
                if (doStencilClear) {
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                }
                // If the depth stencil texture has not been initialized, we want to use loadop
                // clear to init the contents to 0's
                if (!texture->IsSubresourceContentInitialized(
                        view->GetBaseMipLevel(), view->GetLevelCount(), view->GetBaseArrayLayer(),
                        view->GetLayerCount())) {
                    if (texture->GetFormat().HasDepth() &&
                        attachmentInfo.depthLoadOp == dawn::LoadOp::Load) {
                        clearDepth = 0.0f;
                        clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                    }
                    if (texture->GetFormat().HasStencil() &&
                        attachmentInfo.stencilLoadOp == dawn::LoadOp::Load) {
                        clearStencil = 0u;
                        clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                    }
                }

                if (clearFlags) {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = args.dsv;
                    commandList->ClearDepthStencilView(handle, clearFlags, clearDepth, clearStencil,
                                                       0, nullptr);
                }

                // TODO(natlee@microsoft.com): Need to fix when storeop discard is added
                if (attachmentInfo.depthStoreOp == dawn::StoreOp::Store &&
                    attachmentInfo.stencilStoreOp == dawn::StoreOp::Store) {
                    texture->SetIsSubresourceContentInitialized(
                        view->GetBaseMipLevel(), view->GetLevelCount(), view->GetBaseArrayLayer(),
                        view->GetLayerCount());
                }
            }
        }

        // Set up render targets
        {
            if (args.dsv.ptr) {
                commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, &args.dsv);
            } else {
                commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, nullptr);
            }
        }

        // Set up default dynamic state
        {
            uint32_t width = renderPass->width;
            uint32_t height = renderPass->height;
            D3D12_VIEWPORT viewport = {
                0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 0.f, 1.f};
            D3D12_RECT scissorRect = {0, 0, static_cast<long>(width), static_cast<long>(height)};
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            static constexpr std::array<float, 4> defaultBlendFactor = {0, 0, 0, 0};
            commandList->OMSetBlendFactor(&defaultBlendFactor[0]);
        }

        RenderPipeline* lastPipeline = nullptr;
        PipelineLayout* lastLayout = nullptr;
        VertexBufferTracker vertexBufferTracker = {};
        IndexBufferTracker indexBufferTracker = {};

        auto EncodeRenderBundleCommand = [&](CommandIterator* iter, Command type) {
            switch (type) {
                case Command::Draw: {
                    DrawCmd* draw = iter->NextCommand<DrawCmd>();

                    bindingTracker->Apply(commandList.Get());
                    vertexBufferTracker.Apply(commandList.Get(), lastPipeline);
                    commandList->DrawInstanced(draw->vertexCount, draw->instanceCount,
                                               draw->firstVertex, draw->firstInstance);
                } break;

                case Command::DrawIndexed: {
                    DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();

                    bindingTracker->Apply(commandList.Get());
                    indexBufferTracker.Apply(commandList.Get());
                    vertexBufferTracker.Apply(commandList.Get(), lastPipeline);
                    commandList->DrawIndexedInstanced(draw->indexCount, draw->instanceCount,
                                                      draw->firstIndex, draw->baseVertex,
                                                      draw->firstInstance);
                } break;

                case Command::DrawIndirect: {
                    DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();

                    bindingTracker->Apply(commandList.Get());
                    vertexBufferTracker.Apply(commandList.Get(), lastPipeline);
                    Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                    ComPtr<ID3D12CommandSignature> signature =
                        ToBackend(GetDevice())->GetDrawIndirectSignature();
                    commandList->ExecuteIndirect(signature.Get(), 1,
                                                 buffer->GetD3D12Resource().Get(),
                                                 draw->indirectOffset, nullptr, 0);
                } break;

                case Command::DrawIndexedIndirect: {
                    DrawIndexedIndirectCmd* draw = iter->NextCommand<DrawIndexedIndirectCmd>();

                    bindingTracker->Apply(commandList.Get());
                    indexBufferTracker.Apply(commandList.Get());
                    vertexBufferTracker.Apply(commandList.Get(), lastPipeline);
                    Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                    ComPtr<ID3D12CommandSignature> signature =
                        ToBackend(GetDevice())->GetDrawIndexedIndirectSignature();
                    commandList->ExecuteIndirect(signature.Get(), 1,
                                                 buffer->GetD3D12Resource().Get(),
                                                 draw->indirectOffset, nullptr, 0);
                } break;

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = iter->NextCommand<InsertDebugMarkerCmd>();
                    const char* label = iter->NextData<char>(cmd->length + 1);

                    if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                        // PIX color is 1 byte per channel in ARGB format
                        constexpr uint64_t kPIXBlackColor = 0xff000000;
                        ToBackend(GetDevice())
                            ->GetFunctions()
                            ->pixSetMarkerOnCommandList(commandList.Get(), kPIXBlackColor, label);
                    }
                } break;

                case Command::PopDebugGroup: {
                    iter->NextCommand<PopDebugGroupCmd>();

                    if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                        ToBackend(GetDevice())
                            ->GetFunctions()
                            ->pixEndEventOnCommandList(commandList.Get());
                    }
                } break;

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = iter->NextCommand<PushDebugGroupCmd>();
                    const char* label = iter->NextData<char>(cmd->length + 1);

                    if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                        // PIX color is 1 byte per channel in ARGB format
                        constexpr uint64_t kPIXBlackColor = 0xff000000;
                        ToBackend(GetDevice())
                            ->GetFunctions()
                            ->pixBeginEventOnCommandList(commandList.Get(), kPIXBlackColor, label);
                    }
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();
                    RenderPipeline* pipeline = ToBackend(cmd->pipeline).Get();
                    PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                    commandList->SetGraphicsRootSignature(layout->GetRootSignature().Get());
                    commandList->SetPipelineState(pipeline->GetPipelineState().Get());
                    commandList->IASetPrimitiveTopology(pipeline->GetD3D12PrimitiveTopology());

                    bindingTracker->OnSetPipeline(pipeline);
                    indexBufferTracker.OnSetPipeline(pipeline);

                    lastPipeline = pipeline;
                    lastLayout = layout;
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();
                    BindGroup* group = ToBackend(cmd->group.Get());
                    uint64_t* dynamicOffsets = nullptr;

                    if (cmd->dynamicOffsetCount > 0) {
                        dynamicOffsets = iter->NextData<uint64_t>(cmd->dynamicOffsetCount);
                    }

                    bindingTracker->OnSetBindGroup(cmd->index, group, cmd->dynamicOffsetCount,
                                                   dynamicOffsets);
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();

                    indexBufferTracker.OnSetIndexBuffer(ToBackend(cmd->buffer.Get()), cmd->offset);
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = iter->NextCommand<SetVertexBuffersCmd>();
                    Ref<BufferBase>* buffers = iter->NextData<Ref<BufferBase>>(cmd->count);
                    uint64_t* offsets = iter->NextData<uint64_t>(cmd->count);

                    vertexBufferTracker.OnSetVertexBuffers(cmd->startSlot, cmd->count, buffers,
                                                           offsets);
                } break;

                default:
                    UNREACHABLE();
                    break;
            }
        };

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();

                    // TODO(brandon1.jones@intel.com): avoid calling this function and enable MSAA
                    // resolve in D3D12 render pass on the platforms that support this feature.
                    if (renderPass->attachmentState->GetSampleCount() > 1) {
                        ResolveMultisampledRenderPass(commandList, renderPass);
                    }
                    return;
                } break;

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();

                    commandList->OMSetStencilRef(cmd->reference);
                } break;

                case Command::SetViewport: {
                    SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();
                    D3D12_VIEWPORT viewport;
                    viewport.TopLeftX = cmd->x;
                    viewport.TopLeftY = cmd->y;
                    viewport.Width = cmd->width;
                    viewport.Height = cmd->height;
                    viewport.MinDepth = cmd->minDepth;
                    viewport.MaxDepth = cmd->maxDepth;

                    commandList->RSSetViewports(1, &viewport);
                } break;

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    D3D12_RECT rect;
                    rect.left = cmd->x;
                    rect.top = cmd->y;
                    rect.right = cmd->x + cmd->width;
                    rect.bottom = cmd->y + cmd->height;

                    commandList->RSSetScissorRects(1, &rect);
                } break;

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();
                    commandList->OMSetBlendFactor(static_cast<const FLOAT*>(&cmd->color.r));
                } break;

                case Command::ExecuteBundles: {
                    ExecuteBundlesCmd* cmd = mCommands.NextCommand<ExecuteBundlesCmd>();
                    auto bundles = mCommands.NextData<Ref<RenderBundleBase>>(cmd->count);

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        CommandIterator* iter = bundles[i]->GetCommands();
                        iter->Reset();
                        while (iter->NextCommandId(&type)) {
                            EncodeRenderBundleCommand(iter, type);
                        }
                    }
                } break;

                default: { EncodeRenderBundleCommand(&mCommands, type); } break;
            }
        }
    }

}}  // namespace dawn_native::d3d12
