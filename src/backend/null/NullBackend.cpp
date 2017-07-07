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

namespace backend {
namespace null {

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
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    FramebufferBase* Device::CreateFramebuffer(FramebufferBuilder* builder) {
        return new Framebuffer(builder);
    }
    PipelineBase* Device::CreatePipeline(PipelineBuilder* builder) {
        return new Pipeline(builder);
    }
    PipelineLayoutBase* Device::CreatePipelineLayout(PipelineLayoutBuilder* builder) {
        return new PipelineLayout(builder);
    }
    QueueBase* Device::CreateQueue(QueueBuilder* builder) {
        return new Queue(builder);
    }
    RenderPassBase* Device::CreateRenderPass(RenderPassBuilder* builder) {
        return new RenderPass(builder);
    }
    SamplerBase* Device::CreateSampler(SamplerBuilder* builder) {
        return new Sampler(builder);
    }
    ShaderModuleBase* Device::CreateShaderModule(ShaderModuleBuilder* builder) {
        auto module = new ShaderModule(builder);

        spirv_cross::Compiler compiler(builder->AcquireSpirv());
        module->ExtractSpirvInfo(compiler);

        return module;
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
        pendingOperations.emplace_back(std::move(operation));
    }
    std::vector<std::unique_ptr<PendingOperation>> Device::AcquirePendingOperations() {
        return std::move(pendingOperations);
    }

    // Buffer

    struct BufferMapReadOperation : PendingOperation {
        virtual void Execute() {
            buffer->MapReadOperationCompleted(serial, ptr);
        }

        Ref<Buffer> buffer;
        const void* ptr;
        uint32_t serial;
    };

    Buffer::Buffer(BufferBuilder* builder)
        : BufferBase(builder) {
        if (GetAllowedUsage() & (nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::MapWrite)) {
            backingData = std::unique_ptr<char[]>(new char[GetSize()]);
        }
    }

    Buffer::~Buffer() {
    }

    void Buffer::MapReadOperationCompleted(uint32_t serial, const void* ptr) {
        CallMapReadCallback(serial, NXT_BUFFER_MAP_READ_STATUS_SUCCESS, ptr);
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
        ASSERT(start + count <= GetSize());
        ASSERT(backingData);
        memcpy(backingData.get() + start, data, count);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        ASSERT(start + count <= GetSize());
        ASSERT(backingData);

        auto operation = new BufferMapReadOperation;
        operation->buffer = this;
        operation->ptr = backingData.get() + start;
        operation->serial = serial;

        ToBackend(GetDevice())->AddPendingOperation(std::unique_ptr<PendingOperation>(operation));
    }

    void Buffer::UnmapImpl() {
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit, nxt::BufferUsageBit) {
    }

    // CommandBuffer

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), commands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&commands);
    }

    void CommandBuffer::Execute() {
        Command type;
        while (commands.NextCommandId(&type)) {
            switch (type) {
                case Command::TransitionBufferUsage:
                    {
                        TransitionBufferUsageCmd* cmd = commands.NextCommand<TransitionBufferUsageCmd>();
                        cmd->buffer->UpdateUsageInternal(cmd->usage);
                    }
                    break;
                case Command::TransitionTextureUsage:
                    {
                        TransitionTextureUsageCmd* cmd = commands.NextCommand<TransitionTextureUsageCmd>();
                        cmd->texture->UpdateUsageInternal(cmd->usage);
                    }
                    break;
                default:
                    SkipCommand(&commands, type);
                    break;
            }
        }
    }

    // Queue

    Queue::Queue(QueueBuilder* builder)
        : QueueBase(builder) {
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

    Texture::Texture(TextureBuilder* builder)
        : TextureBase(builder) {
    }

    Texture::~Texture() {
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit, nxt::TextureUsageBit) {
    }

}
}
