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

#include "dawn_native/null/DeviceNull.h"

#include "dawn_native/BackendConnection.h"
#include "dawn_native/Commands.h"
#include "dawn_native/DynamicUploader.h"

#include <spirv-cross/spirv_cross.hpp>

namespace dawn_native { namespace null {

    // Implementation of pre-Device objects: the null adapter, null backend connection and Connect()

    class Adapter : public AdapterBase {
      public:
        Adapter(InstanceBase* instance) : AdapterBase(instance, BackendType::Null) {
            mPCIInfo.name = "Null backend";
        }
        virtual ~Adapter() = default;

      private:
        ResultOrError<DeviceBase*> CreateDeviceImpl() override {
            return {new Device(this)};
        }
    };

    class Backend : public BackendConnection {
      public:
        Backend(InstanceBase* instance) : BackendConnection(instance, BackendType::Null) {
        }

        std::vector<std::unique_ptr<AdapterBase>> DiscoverDefaultAdapters() override {
            // There is always a single Null adapter because it is purely CPU based and doesn't
            // depend on the system.
            std::vector<std::unique_ptr<AdapterBase>> adapters;
            adapters.push_back(std::make_unique<Adapter>(GetInstance()));
            return adapters;
        }
    };

    BackendConnection* Connect(InstanceBase* instance) {
        return new Backend(instance);
    }

    // Device

    Device::Device(Adapter* adapter) : DeviceBase(adapter) {
    }

    Device::~Device() {
        mDynamicUploader = nullptr;
    }

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return new BindGroup(this, descriptor);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return new Buffer(this, descriptor);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandEncoderBase* encoder) {
        return new CommandBuffer(this, encoder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
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
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return new RenderPipeline(this, descriptor);
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
    ResultOrError<SwapChainBase*> Device::CreateSwapChainImpl(
        const SwapChainDescriptor* descriptor) {
        return new SwapChain(this, descriptor);
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return new Texture(this, descriptor);
    }
    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture, descriptor);
    }

    ResultOrError<std::unique_ptr<StagingBufferBase>> Device::CreateStagingBuffer(size_t size) {
        std::unique_ptr<StagingBufferBase> stagingBuffer =
            std::make_unique<StagingBuffer>(size, this);
        return std::move(stagingBuffer);
    }

    MaybeError Device::CopyFromStagingToBuffer(StagingBufferBase* source,
                                               uint32_t sourceOffset,
                                               BufferBase* destination,
                                               uint32_t destinationOffset,
                                               uint32_t size) {
        return DAWN_UNIMPLEMENTED_ERROR("Device unable to copy from staging buffer.");
    }

    Serial Device::GetCompletedCommandSerial() const {
        return mCompletedSerial;
    }

    Serial Device::GetLastSubmittedCommandSerial() const {
        return mLastSubmittedSerial;
    }

    Serial Device::GetPendingCommandSerial() const {
        return mLastSubmittedSerial + 1;
    }

    void Device::TickImpl() {
        SubmitPendingOperations();
    }

    void Device::AddPendingOperation(std::unique_ptr<PendingOperation> operation) {
        mPendingOperations.emplace_back(std::move(operation));
    }
    void Device::SubmitPendingOperations() {
        for (auto& operation : mPendingOperations) {
            operation->Execute();
        }
        mPendingOperations.clear();

        mCompletedSerial = mLastSubmittedSerial;
        mLastSubmittedSerial++;
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
            CallMapWriteCallback(serial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, ptr, GetSize());
        } else {
            CallMapReadCallback(serial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, ptr, GetSize());
        }
    }

    MaybeError Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) {
        ASSERT(start + count <= GetSize());
        ASSERT(mBackingData);
        memcpy(mBackingData.get() + start, data, count);
        return {};
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial) {
        MapAsyncImplCommon(serial, false);
    }

    void Buffer::MapWriteAsyncImpl(uint32_t serial) {
        MapAsyncImplCommon(serial, true);
    }

    void Buffer::MapAsyncImplCommon(uint32_t serial, bool isWrite) {
        ASSERT(mBackingData);

        auto operation = new BufferMapReadOperation;
        operation->buffer = this;
        operation->ptr = mBackingData.get();
        operation->serial = serial;
        operation->isWrite = isWrite;

        ToBackend(GetDevice())->AddPendingOperation(std::unique_ptr<PendingOperation>(operation));
    }

    void Buffer::UnmapImpl() {
    }

    // CommandBuffer

    CommandBuffer::CommandBuffer(Device* device, CommandEncoderBase* encoder)
        : CommandBufferBase(device, encoder), mCommands(encoder->AcquireCommands()) {
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
        ToBackend(GetDevice())->SubmitPendingOperations();
    }

    // SwapChain

    SwapChain::SwapChain(Device* device, const SwapChainDescriptor* descriptor)
        : SwapChainBase(device, descriptor) {
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

    // NativeSwapChainImpl

    void NativeSwapChainImpl::Init(WSIContext* context) {
    }

    dawnSwapChainError NativeSwapChainImpl::Configure(dawnTextureFormat format,
                                                      dawnTextureUsageBit,
                                                      uint32_t width,
                                                      uint32_t height) {
        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    dawnSwapChainError NativeSwapChainImpl::GetNextTexture(dawnSwapChainNextTexture* nextTexture) {
        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    dawnSwapChainError NativeSwapChainImpl::Present() {
        return DAWN_SWAP_CHAIN_NO_ERROR;
    }

    dawn::TextureFormat NativeSwapChainImpl::GetPreferredFormat() const {
        return dawn::TextureFormat::R8G8B8A8Unorm;
    }

    // StagingBuffer

    StagingBuffer::StagingBuffer(size_t size, Device* device) : StagingBufferBase(size) {
    }

    MaybeError StagingBuffer::Initialize() {
        mBuffer = std::make_unique<uint8_t[]>(GetSize());
        mMappedPointer = mBuffer.get();
        return {};
    }

}}  // namespace dawn_native::null
