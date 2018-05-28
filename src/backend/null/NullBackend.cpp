// Copyright 2017 The NXT Authors
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

#include "backend/null/NullBackend.h"

#include "backend/Commands.h"

#include <spirv-cross/spirv_cross.hpp>

namespace backend { namespace null {

    nxtProcTable GetNonValidatingProcs();
    nxtProcTable GetValidatingProcs();

    void Init(nxtProcTable* procs, nxtDevice* device) {
        *procs = GetValidatingProcs();
        *device = reinterpret_cast<nxtDevice>(new Device);
    }

    // Device

    Device::Device() {
    }

    Device::~Device() {
    }

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(builder);
    }
    BindGroupLayoutBase* Device::CreateBindGroupLayout(BindGroupLayoutBuilder* builder) {
        return new BindGroupLayout(builder);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ComputePipelineBase* Device::CreateComputePipeline(ComputePipelineBuilder* builder) {
        return new ComputePipeline(builder);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    PipelineLayoutBase* Device::CreatePipelineLayout(PipelineLayoutBuilder* builder) {
        return new PipelineLayout(builder);
    }
    QueueBase* Device::CreateQueue(QueueBuilder* builder) {
        return new Queue(builder);
    }
    RenderPassDescriptorBase* Device::CreateRenderPassDescriptor(
        RenderPassDescriptorBuilder* builder) {
        return new RenderPassDescriptor(builder);
    }
    RenderPipelineBase* Device::CreateRenderPipeline(RenderPipelineBuilder* builder) {
        return new RenderPipeline(builder);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(
        const nxt::SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ShaderModuleBase* Device::CreateShaderModule(ShaderModuleBuilder* builder) {
        auto module = new ShaderModule(builder);

        spirv_cross::Compiler compiler(builder->AcquireSpirv());
        module->ExtractSpirvInfo(compiler);

        return module;
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    TextureBase* Device::CreateTexture(TextureBuilder* builder) {
        return new Texture(builder);
    }
    TextureViewBase* Device::CreateTextureView(TextureViewBuilder* builder) {
        return new TextureView(builder);
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

    Buffer::Buffer(BufferBuilder* builder) : BufferBase(builder) {
        if (GetAllowedUsage() & (nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::MapRead |
                                 nxt::BufferUsageBit::MapWrite)) {
            mBackingData = std::unique_ptr<char[]>(new char[GetSize()]);
        }
    }

    Buffer::~Buffer() {
    }

    void Buffer::MapReadOperationCompleted(uint32_t serial, void* ptr, bool isWrite) {
        if (isWrite) {
            CallMapWriteCallback(serial, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, ptr);
        } else {
            CallMapReadCallback(serial, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, ptr);
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

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit, nxt::BufferUsageBit) {
    }

    // CommandBuffer

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::Execute() {
        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::TransitionBufferUsage: {
                    TransitionBufferUsageCmd* cmd =
                        mCommands.NextCommand<TransitionBufferUsageCmd>();
                    cmd->buffer->UpdateUsageInternal(cmd->usage);
                } break;
                case Command::TransitionTextureUsage: {
                    TransitionTextureUsageCmd* cmd =
                        mCommands.NextCommand<TransitionTextureUsageCmd>();
                    cmd->texture->UpdateUsageInternal(cmd->usage);
                } break;
                default:
                    SkipCommand(&mCommands, type);
                    break;
            }
        }
    }

    // Queue

    Queue::Queue(QueueBuilder* builder) : QueueBase(builder) {
    }

    Queue::~Queue() {
    }

    void Queue::Submit(uint32_t numCommands, CommandBuffer* const* commands) {
        auto operations = ToBackend(GetDevice())->AcquirePendingOperations();

        for (auto& operation : operations) {
            operation->Execute();
        }

        for (uint32_t i = 0; i < numCommands; ++i) {
            commands[i]->Execute();
        }

        operations.clear();
    }

    // Texture

    Texture::Texture(TextureBuilder* builder) : TextureBase(builder) {
    }

    Texture::~Texture() {
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit, nxt::TextureUsageBit) {
    }

    // SwapChain

    SwapChain::SwapChain(SwapChainBuilder* builder) : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        im.Init(im.userData, nullptr);
    }

    SwapChain::~SwapChain() {
    }

    TextureBase* SwapChain::GetNextTextureImpl(TextureBuilder* builder) {
        return GetDevice()->CreateTexture(builder);
    }
}}  // namespace backend::null
