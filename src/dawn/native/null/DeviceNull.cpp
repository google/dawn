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

#include "dawn/native/null/DeviceNull.h"

#include <limits>
#include <utility>

#include "dawn/native/BackendConnection.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Surface.h"
#include "dawn/native/TintUtils.h"

#include "tint/tint.h"

namespace dawn::native::null {

// Implementation of pre-Device objects: the null adapter, null backend connection and Connect()

Adapter::Adapter(InstanceBase* instance) : AdapterBase(instance, wgpu::BackendType::Null) {
    mVendorId = 0;
    mDeviceId = 0;
    mName = "Null backend";
    mAdapterType = wgpu::AdapterType::CPU;
    MaybeError err = Initialize();
    ASSERT(err.IsSuccess());
}

Adapter::~Adapter() = default;

bool Adapter::SupportsExternalImages() const {
    return false;
}

// Used for the tests that intend to use an adapter without all features enabled.
void Adapter::SetSupportedFeatures(const std::vector<wgpu::FeatureName>& requiredFeatures) {
    mSupportedFeatures = {};
    for (wgpu::FeatureName f : requiredFeatures) {
        mSupportedFeatures.EnableFeature(f);
    }
}

MaybeError Adapter::InitializeImpl() {
    return {};
}

void Adapter::InitializeSupportedFeaturesImpl() {
    // Enable all features by default for the convenience of tests.
    mSupportedFeatures.featuresBitSet.set();
}

MaybeError Adapter::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    GetDefaultLimits(&limits->v1);
    return {};
}

void Adapter::SetupBackendDeviceToggles(TogglesState* deviceToggles) const {}

ResultOrError<Ref<DeviceBase>> Adapter::CreateDeviceImpl(const DeviceDescriptor* descriptor,
                                                         const TogglesState& deviceToggles) {
    return Device::Create(this, descriptor, deviceToggles);
}

MaybeError Adapter::ValidateFeatureSupportedWithDeviceTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& deviceToggles) {
    return {};
}

class Backend : public BackendConnection {
  public:
    explicit Backend(InstanceBase* instance)
        : BackendConnection(instance, wgpu::BackendType::Null) {}

    std::vector<Ref<AdapterBase>> DiscoverDefaultAdapters() override {
        // There is always a single Null adapter because it is purely CPU based and doesn't
        // depend on the system.
        std::vector<Ref<AdapterBase>> adapters;
        Ref<Adapter> adapter = AcquireRef(new Adapter(GetInstance()));
        adapters.push_back(std::move(adapter));
        return adapters;
    }
};

BackendConnection* Connect(InstanceBase* instance) {
    return new Backend(instance);
}

struct CopyFromStagingToBufferOperation : PendingOperation {
    void Execute() override {
        destination->CopyFromStaging(staging, sourceOffset, destinationOffset, size);
    }

    BufferBase* staging;
    Ref<Buffer> destination;
    uint64_t sourceOffset;
    uint64_t destinationOffset;
    uint64_t size;
};

// Device

// static
ResultOrError<Ref<Device>> Device::Create(Adapter* adapter,
                                          const DeviceDescriptor* descriptor,
                                          const TogglesState& deviceToggles) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor, deviceToggles));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

Device::~Device() {
    Destroy();
}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    return DeviceBase::Initialize(AcquireRef(new Queue(this, &descriptor->defaultQueue)));
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return AcquireRef(new BindGroup(this, descriptor));
}
ResultOrError<Ref<BindGroupLayoutBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    return AcquireRef(new BindGroupLayout(this, descriptor, pipelineCompatibilityToken));
}
ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
    DAWN_TRY(IncrementMemoryUsage(descriptor->size));
    return AcquireRef(new Buffer(this, descriptor));
}
ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}
Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const ComputePipelineDescriptor* descriptor) {
    return AcquireRef(new ComputePipeline(this, descriptor));
}
ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const PipelineLayoutDescriptor* descriptor) {
    return AcquireRef(new PipelineLayout(this, descriptor));
}
ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return AcquireRef(new QuerySet(this, descriptor));
}
Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const RenderPipelineDescriptor* descriptor) {
    return AcquireRef(new RenderPipeline(this, descriptor));
}
ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return AcquireRef(new Sampler(this, descriptor));
}
ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(this, descriptor));
    DAWN_TRY(module->Initialize(parseResult, compilationMessages));
    return module;
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    const SwapChainDescriptor* descriptor) {
    return AcquireRef(new OldSwapChain(this, descriptor));
}
ResultOrError<Ref<NewSwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    NewSwapChainBase* previousSwapChain,
    const SwapChainDescriptor* descriptor) {
    return SwapChain::Create(this, surface, previousSwapChain, descriptor);
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
    return AcquireRef(new Texture(this, descriptor, TextureBase::TextureState::OwnedInternal));
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    return AcquireRef(new TextureView(texture, descriptor));
}

void Device::DestroyImpl() {
    ASSERT(GetState() == State::Disconnected);

    // Clear pending operations before checking mMemoryUsage because some operations keep a
    // reference to Buffers.
    mPendingOperations.clear();
    ASSERT(mMemoryUsage == 0);
}

MaybeError Device::WaitForIdleForDestruction() {
    mPendingOperations.clear();
    return {};
}

bool Device::HasPendingCommands() const {
    return false;
}

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    if (IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        destination->SetIsDataInitialized();
    }

    auto operation = std::make_unique<CopyFromStagingToBufferOperation>();
    operation->staging = source;
    operation->destination = ToBackend(destination);
    operation->sourceOffset = sourceOffset;
    operation->destinationOffset = destinationOffset;
    operation->size = size;

    AddPendingOperation(std::move(operation));

    return {};
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TextureDataLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    return {};
}

MaybeError Device::IncrementMemoryUsage(uint64_t bytes) {
    static_assert(kMaxMemoryUsage <= std::numeric_limits<size_t>::max());
    if (bytes > kMaxMemoryUsage || mMemoryUsage > kMaxMemoryUsage - bytes) {
        return DAWN_OUT_OF_MEMORY_ERROR("Out of memory.");
    }
    mMemoryUsage += bytes;
    return {};
}

void Device::DecrementMemoryUsage(uint64_t bytes) {
    ASSERT(mMemoryUsage >= bytes);
    mMemoryUsage -= bytes;
}

MaybeError Device::TickImpl() {
    return SubmitPendingOperations();
}

ResultOrError<ExecutionSerial> Device::CheckAndUpdateCompletedSerials() {
    return GetLastSubmittedCommandSerial();
}

void Device::AddPendingOperation(std::unique_ptr<PendingOperation> operation) {
    mPendingOperations.emplace_back(std::move(operation));
}

MaybeError Device::SubmitPendingOperations() {
    for (auto& operation : mPendingOperations) {
        operation->Execute();
    }
    mPendingOperations.clear();

    DAWN_TRY(CheckPassedSerials());
    IncrementLastSubmittedCommandSerial();

    return {};
}

// BindGroupDataHolder

BindGroupDataHolder::BindGroupDataHolder(size_t size)
    : mBindingDataAllocation(malloc(size))  // malloc is guaranteed to return a
                                            // pointer aligned enough for the allocation
{}

BindGroupDataHolder::~BindGroupDataHolder() {
    free(mBindingDataAllocation);
}

// BindGroup

BindGroup::BindGroup(DeviceBase* device, const BindGroupDescriptor* descriptor)
    : BindGroupDataHolder(descriptor->layout->GetBindingDataSize()),
      BindGroupBase(device, descriptor, mBindingDataAllocation) {}

// BindGroupLayout

BindGroupLayout::BindGroupLayout(DeviceBase* device,
                                 const BindGroupLayoutDescriptor* descriptor,
                                 PipelineCompatibilityToken pipelineCompatibilityToken)
    : BindGroupLayoutBase(device, descriptor, pipelineCompatibilityToken) {}

// Buffer

Buffer::Buffer(Device* device, const BufferDescriptor* descriptor)
    : BufferBase(device, descriptor) {
    mBackingData = std::unique_ptr<uint8_t[]>(new uint8_t[GetSize()]);
    mAllocatedSize = GetSize();
}

bool Buffer::IsCPUWritableAtCreation() const {
    // Only return true for mappable buffers so we can test cases that need / don't need a
    // staging buffer.
    return (GetUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) != 0;
}

MaybeError Buffer::MapAtCreationImpl() {
    return {};
}

void Buffer::CopyFromStaging(BufferBase* staging,
                             uint64_t sourceOffset,
                             uint64_t destinationOffset,
                             uint64_t size) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(staging->GetMappedPointer());
    memcpy(mBackingData.get() + destinationOffset, ptr + sourceOffset, size);
}

void Buffer::DoWriteBuffer(uint64_t bufferOffset, const void* data, size_t size) {
    ASSERT(bufferOffset + size <= GetSize());
    ASSERT(mBackingData);
    memcpy(mBackingData.get() + bufferOffset, data, size);
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    return {};
}

void* Buffer::GetMappedPointer() {
    return mBackingData.get();
}

void Buffer::UnmapImpl() {}

void Buffer::DestroyImpl() {
    BufferBase::DestroyImpl();
    ToBackend(GetDevice())->DecrementMemoryUsage(GetSize());
}

// CommandBuffer

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {}

// QuerySet

QuerySet::QuerySet(Device* device, const QuerySetDescriptor* descriptor)
    : QuerySetBase(device, descriptor) {}

// Queue

Queue::Queue(Device* device, const QueueDescriptor* descriptor) : QueueBase(device, descriptor) {}

Queue::~Queue() {}

MaybeError Queue::SubmitImpl(uint32_t, CommandBufferBase* const*) {
    Device* device = ToBackend(GetDevice());

    // The Vulkan, D3D12 and Metal implementation all tick the device here,
    // for testing purposes we should also tick in the null implementation.
    DAWN_TRY(device->Tick());

    return device->SubmitPendingOperations();
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    ToBackend(buffer)->DoWriteBuffer(bufferOffset, data, size);
    return {};
}

// ComputePipeline
MaybeError ComputePipeline::Initialize() {
    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);

    tint::Program transformedProgram;
    const tint::Program* program;
    tint::transform::Manager transformManager;
    tint::transform::DataMap transformInputs;

    transformManager.Add<tint::transform::Robustness>();

    if (!computeStage.metadata->overrides.empty()) {
        transformManager.Add<tint::transform::SingleEntryPoint>();
        transformInputs.Add<tint::transform::SingleEntryPoint::Config>(
            computeStage.entryPoint.c_str());

        // This needs to run after SingleEntryPoint transform which removes unused overrides for
        // current entry point.
        transformManager.Add<tint::transform::SubstituteOverride>();
        transformInputs.Add<tint::transform::SubstituteOverride::Config>(
            BuildSubstituteOverridesTransformConfig(computeStage));
    }

    DAWN_TRY_ASSIGN(transformedProgram,
                    RunTransforms(&transformManager, computeStage.module->GetTintProgram(),
                                  transformInputs, nullptr, nullptr));

    program = &transformedProgram;

    // Do the workgroup size validation as it is actually backend agnostic.
    const CombinedLimits& limits = GetDevice()->GetLimits();
    Extent3D _;
    DAWN_TRY_ASSIGN(
        _, ValidateComputeStageWorkgroupSize(*program, computeStage.entryPoint.c_str(),
                                             LimitsForCompilationRequest::Create(limits.v1)));

    return {};
}

// RenderPipeline
MaybeError RenderPipeline::Initialize() {
    return {};
}

// SwapChain

// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                NewSwapChainBase* previousSwapChain,
                                                const SwapChainDescriptor* descriptor) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, descriptor));
    DAWN_TRY(swapchain->Initialize(previousSwapChain));
    return swapchain;
}

MaybeError SwapChain::Initialize(NewSwapChainBase* previousSwapChain) {
    if (previousSwapChain != nullptr) {
        // TODO(crbug.com/dawn/269): figure out what should happen when surfaces are used by
        // multiple backends one after the other. It probably needs to block until the backend
        // and GPU are completely finished with the previous swapchain.
        DAWN_INVALID_IF(previousSwapChain->GetBackendType() != wgpu::BackendType::Null,
                        "null::SwapChain cannot switch between APIs");
    }

    return {};
}

SwapChain::~SwapChain() = default;

MaybeError SwapChain::PresentImpl() {
    mTexture->APIDestroy();
    mTexture = nullptr;
    return {};
}

ResultOrError<Ref<TextureViewBase>> SwapChain::GetCurrentTextureViewImpl() {
    TextureDescriptor textureDesc = GetSwapChainBaseTextureDescriptor(this);
    mTexture = AcquireRef(
        new Texture(GetDevice(), &textureDesc, TextureBase::TextureState::OwnedInternal));
    return mTexture->CreateView();
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mTexture != nullptr) {
        mTexture->APIDestroy();
        mTexture = nullptr;
    }
}

// ShaderModule

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    return InitializeBase(parseResult, compilationMessages);
}

// OldSwapChain

OldSwapChain::OldSwapChain(Device* device, const SwapChainDescriptor* descriptor)
    : OldSwapChainBase(device, descriptor) {
    const auto& im = GetImplementation();
    im.Init(im.userData, nullptr);
}

OldSwapChain::~OldSwapChain() {}

TextureBase* OldSwapChain::GetNextTextureImpl(const TextureDescriptor* descriptor) {
    return GetDevice()->APICreateTexture(descriptor);
}

MaybeError OldSwapChain::OnBeforePresent(TextureViewBase*) {
    return {};
}

// NativeSwapChainImpl

void NativeSwapChainImpl::Init(WSIContext* context) {}

DawnSwapChainError NativeSwapChainImpl::Configure(WGPUTextureFormat format,
                                                  WGPUTextureUsage,
                                                  uint32_t width,
                                                  uint32_t height) {
    return DAWN_SWAP_CHAIN_NO_ERROR;
}

DawnSwapChainError NativeSwapChainImpl::GetNextTexture(DawnSwapChainNextTexture* nextTexture) {
    return DAWN_SWAP_CHAIN_NO_ERROR;
}

DawnSwapChainError NativeSwapChainImpl::Present() {
    return DAWN_SWAP_CHAIN_NO_ERROR;
}

wgpu::TextureFormat NativeSwapChainImpl::GetPreferredFormat() const {
    return wgpu::TextureFormat::RGBA8Unorm;
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return 1;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return 1.0f;
}

void Device::ForceEventualFlushOfCommands() {}

Texture::Texture(DeviceBase* device, const TextureDescriptor* descriptor, TextureState state)
    : TextureBase(device, descriptor, state) {}

}  // namespace dawn::native::null
