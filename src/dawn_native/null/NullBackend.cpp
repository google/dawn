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

#include "dawn_native/null/NullBackend.h"

#include "dawn_native/Commands.h"
#include "dawn_native/NullBackend.h"

#include <spirv-cross/spirv_cross.hpp>

namespace dawn_native { namespace null {

    dawnDevice CreateDevice() {
        return reinterpret_cast<dawnDevice>(new Device);
    }

    // Device

    Device::Device() {
        InitFakePCIInfo();
    }

    Device::~Device() {
    }

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(builder);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return new Buffer(this, descriptor);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return new PipelineLayout(this, descriptor);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    RenderPassDescriptorBase* Device::CreateRenderPassDescriptor(
        RenderPassDescriptorBuilder* builder) {
        return new RenderPassDescriptor(builder);
    }
    RenderPipelineBase* Device::CreateRenderPipeline(RenderPipelineBuilder* builder) {
        return new RenderPipeline(builder);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor) {
        auto module = new ShaderModule(this, descriptor);

        spirv_cross::Compiler compiler(descriptor->code, descriptor->codeSize);
        module->ExtractSpirvInfo(compiler);

        return module;
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return new Texture(this, descriptor);
    }
    TextureViewBase* Device::CreateDefaultTextureView(TextureBase* texture) {
        return new TextureView(texture);
    }
    // TODO(jiawei.shao@intel.com): implement creating texture view by TextureViewDescriptor
    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture);
    }

    void Device::InitFakePCIInfo() {
        mPCIInfo.name = "Null backend";
    }

    const dawn_native::PCIInfo& Device::GetPCIInfo() const {
        return mPCIInfo;
    }

    void Device::TickImpl() {
    }

    void Device::AddPendingOperation(std::unique_ptr<PendingOperation> operation) {
        mPendingOperations.emplace_back(std::move(operation));
    }
    std::vector<std::unique_ptr<PendingOperation>> Device::AcquirePendingOperations() {
        return std::move(mPendingOperations);
    }

    // Buffer

    struct BufferMapReadOperation : PendingOperation {
        virtual void Execute() {
            buffer->MapReadOperationCompleted(serial, ptr, isWrite);
        }

        Ref<Buffer> buffer;
        void* ptr;
        uint32_t serial;
        bool isWrite;
    };

    Buffer::Buffer(Device* device, const BufferDescriptor* descriptor)
        : BufferBase(device, descriptor) {
        if (GetUsage() & (dawn::BufferUsageBit::TransferDst | dawn::BufferUsageBit::MapRead |
                          dawn::BufferUsageBit::MapWrite)) {
            mBackingData = std::unique_ptr<char[]>(new char[GetSize()]);
        }
    }

    Buffer::~Buffer() {
    }

    void Buffer::MapReadOperationCompleted(uint32_t serial, void* ptr, bool isWrite) {
        if (isWrite) {
            CallMapWriteCallback(serial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, ptr);
        } else {
            CallMapReadCallback(serial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, ptr);
        }
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) {
        ASSERT(start + count <= GetSize());
        ASSERT(mBackingData);
        memcpy(mBackingData.get() + start, data, count);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        MapAsyncImplCommon(serial, start, count, false);
    }

    void Buffer::MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        MapAsyncImplCommon(serial, start, count, true);
    }

    void Buffer::MapAsyncImplCommon(uint32_t serial, uint32_t start, uint32_t count, bool isWrite) {
        ASSERT(start + count <= GetSize());
        ASSERT(mBackingData);

        auto operation = new BufferMapReadOperation;
        operation->buffer = this;
        operation->ptr = mBackingData.get() + start;
        operation->serial = serial;
        operation->isWrite = isWrite;

        ToBackend(GetDevice())->AddPendingOperation(std::unique_ptr<PendingOperation>(operation));
    }

    void Buffer::UnmapImpl() {
    }

    // CommandBuffer

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    // Queue

    Queue::Queue(Device* device) : QueueBase(device) {
    }

    Queue::~Queue() {
    }

    void Queue::SubmitImpl(uint32_t, CommandBufferBase* const*) {
        auto operations = ToBackend(GetDevice())->AcquirePendingOperations();

        for (auto& operation : operations) {
            operation->Execute();
        }

        operations.clear();
    }

    // SwapChain

    SwapChain::SwapChain(SwapChainBuilder* builder) : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        im.Init(im.userData, nullptr);
    }

    SwapChain::~SwapChain() {
    }

    TextureBase* SwapChain::GetNextTextureImpl(const TextureDescriptor* descriptor) {
        return GetDevice()->CreateTexture(descriptor);
    }

    void SwapChain::OnBeforePresent(TextureBase*) {
    }

}}  // namespace dawn_native::null
