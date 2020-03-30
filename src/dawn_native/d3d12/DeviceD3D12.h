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

#ifndef DAWNNATIVE_D3D12_DEVICED3D12_H_
#define DAWNNATIVE_D3D12_DEVICED3D12_H_

#include "dawn_native/dawn_platform.h"

#include "common/SerialQueue.h"
#include "dawn_native/Device.h"
#include "dawn_native/d3d12/CommandRecordingContext.h"
#include "dawn_native/d3d12/D3D12Info.h"
#include "dawn_native/d3d12/Forward.h"
#include "dawn_native/d3d12/ResourceHeapAllocationD3D12.h"

#include <memory>

namespace dawn_native { namespace d3d12 {

    class CommandAllocatorManager;
    class DescriptorHeapAllocator;
    class ShaderVisibleDescriptorAllocator;
    class MapRequestTracker;
    class PlatformFunctions;
    class ResourceAllocatorManager;
    class ResidencyManager;

#define ASSERT_SUCCESS(hr)            \
    {                                 \
        HRESULT succeeded = hr;       \
        ASSERT(SUCCEEDED(succeeded)); \
    }

    // Definition of backend types
    class Device : public DeviceBase {
      public:
        Device(Adapter* adapter, const DeviceDescriptor* descriptor);
        ~Device();

        MaybeError Initialize();

        CommandBufferBase* CreateCommandBuffer(CommandEncoder* encoder,
                                               const CommandBufferDescriptor* descriptor) override;

        Serial GetCompletedCommandSerial() const final override;
        Serial GetLastSubmittedCommandSerial() const final override;
        MaybeError TickImpl() override;

        ComPtr<ID3D12Device> GetD3D12Device() const;
        ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
        ID3D12SharingContract* GetSharingContract() const;

        ComPtr<ID3D12CommandSignature> GetDispatchIndirectSignature() const;
        ComPtr<ID3D12CommandSignature> GetDrawIndirectSignature() const;
        ComPtr<ID3D12CommandSignature> GetDrawIndexedIndirectSignature() const;

        DescriptorHeapAllocator* GetDescriptorHeapAllocator() const;
        MapRequestTracker* GetMapRequestTracker() const;
        CommandAllocatorManager* GetCommandAllocatorManager() const;
        ResidencyManager* GetResidencyManager() const;

        const PlatformFunctions* GetFunctions() const;
        ComPtr<IDXGIFactory4> GetFactory() const;

        ResultOrError<CommandRecordingContext*> GetPendingCommandContext();
        Serial GetPendingCommandSerial() const override;

        const D3D12DeviceInfo& GetDeviceInfo() const;

        MaybeError NextSerial();
        MaybeError WaitForSerial(Serial serial);

        void ReferenceUntilUnused(ComPtr<IUnknown> object);

        MaybeError ExecutePendingCommandContext();

        ResultOrError<std::unique_ptr<StagingBufferBase>> CreateStagingBuffer(size_t size) override;
        MaybeError CopyFromStagingToBuffer(StagingBufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) override;

        ResultOrError<ResourceHeapAllocation> AllocateMemory(
            D3D12_HEAP_TYPE heapType,
            const D3D12_RESOURCE_DESC& resourceDescriptor,
            D3D12_RESOURCE_STATES initialUsage);

        void DeallocateMemory(ResourceHeapAllocation& allocation);

        ShaderVisibleDescriptorAllocator* GetShaderVisibleDescriptorAllocator() const;

        TextureBase* WrapSharedHandle(const ExternalImageDescriptor* descriptor,
                                      HANDLE sharedHandle,
                                      uint64_t acquireMutexKey,
                                      bool isSwapChainTexture);
        ResultOrError<ComPtr<IDXGIKeyedMutex>> CreateKeyedMutexForTexture(
            ID3D12Resource* d3d12Resource);
        void ReleaseKeyedMutexForTexture(ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex);

        void InitTogglesFromDriver();

      private:
        ResultOrError<BindGroupBase*> CreateBindGroupImpl(
            const BindGroupDescriptor* descriptor) override;
        ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const BindGroupLayoutDescriptor* descriptor) override;
        ResultOrError<BufferBase*> CreateBufferImpl(const BufferDescriptor* descriptor) override;
        ResultOrError<ComputePipelineBase*> CreateComputePipelineImpl(
            const ComputePipelineDescriptor* descriptor) override;
        ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const PipelineLayoutDescriptor* descriptor) override;
        ResultOrError<QueueBase*> CreateQueueImpl() override;
        ResultOrError<RenderPipelineBase*> CreateRenderPipelineImpl(
            const RenderPipelineDescriptor* descriptor) override;
        ResultOrError<SamplerBase*> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
        ResultOrError<ShaderModuleBase*> CreateShaderModuleImpl(
            const ShaderModuleDescriptor* descriptor) override;
        ResultOrError<SwapChainBase*> CreateSwapChainImpl(
            const SwapChainDescriptor* descriptor) override;
        ResultOrError<NewSwapChainBase*> CreateSwapChainImpl(
            Surface* surface,
            NewSwapChainBase* previousSwapChain,
            const SwapChainDescriptor* descriptor) override;
        ResultOrError<TextureBase*> CreateTextureImpl(const TextureDescriptor* descriptor) override;
        ResultOrError<TextureViewBase*> CreateTextureViewImpl(
            TextureBase* texture,
            const TextureViewDescriptor* descriptor) override;

        void Destroy() override;
        MaybeError WaitForIdleForDestruction() override;

        Serial mCompletedSerial = 0;
        Serial mLastSubmittedSerial = 0;
        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent = nullptr;

        ComPtr<ID3D12Device> mD3d12Device;  // Device is owned by adapter and will not be outlived.
        ComPtr<ID3D12CommandQueue> mCommandQueue;
        ComPtr<ID3D12SharingContract> mD3d12SharingContract;

        // 11on12 device and device context corresponding to mCommandQueue
        ComPtr<ID3D11On12Device> mD3d11On12Device;
        ComPtr<ID3D11DeviceContext2> mD3d11On12DeviceContext;

        ComPtr<ID3D12CommandSignature> mDispatchIndirectSignature;
        ComPtr<ID3D12CommandSignature> mDrawIndirectSignature;
        ComPtr<ID3D12CommandSignature> mDrawIndexedIndirectSignature;

        CommandRecordingContext mPendingCommands;

        SerialQueue<ComPtr<IUnknown>> mUsedComObjectRefs;

        std::unique_ptr<CommandAllocatorManager> mCommandAllocatorManager;
        std::unique_ptr<DescriptorHeapAllocator> mDescriptorHeapAllocator;
        std::unique_ptr<MapRequestTracker> mMapRequestTracker;
        std::unique_ptr<ResourceAllocatorManager> mResourceAllocatorManager;
        std::unique_ptr<ResidencyManager> mResidencyManager;
        std::unique_ptr<ShaderVisibleDescriptorAllocator> mShaderVisibleDescriptorAllocator;
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_DEVICED3D12_H_
