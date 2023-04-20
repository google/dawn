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

#include "dawn/native/d3d11/AdapterD3D11.h"

#include <string>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"

namespace dawn::native::d3d11 {

Adapter::Adapter(Backend* backend,
                 ComPtr<IDXGIAdapter3> hardwareAdapter,
                 const TogglesState& adapterToggles)
    : Base(backend, std::move(hardwareAdapter), wgpu::BackendType::D3D11, adapterToggles) {}

Adapter::~Adapter() = default;

bool Adapter::SupportsExternalImages() const {
    // TODO(dawn:1724): Implement external images on D3D11.
    return false;
}

const DeviceInfo& Adapter::GetDeviceInfo() const {
    return mDeviceInfo;
}

ResultOrError<ComPtr<ID3D11Device>> Adapter::CreateD3D11Device() {
    ComPtr<ID3D11Device> device = std::move(mD3d11Device);
    if (!device) {
        const PlatformFunctions* functions = static_cast<Backend*>(GetBackend())->GetFunctions();
        const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

        UINT flags = 0;
        if (GetInstance()->IsBackendValidationEnabled()) {
            flags |= D3D11_CREATE_DEVICE_DEBUG;
        }

        DAWN_TRY(CheckHRESULT(functions->d3d11CreateDevice(
                                  GetHardwareAdapter(), D3D_DRIVER_TYPE_UNKNOWN,
                                  /*Software=*/nullptr, flags, featureLevels,
                                  std::size(featureLevels), D3D11_SDK_VERSION, &device,
                                  /*pFeatureLevel=*/nullptr, /*[out] ppImmediateContext=*/nullptr),
                              "D3D11CreateDevice failed"));
    }
    return device;
}

MaybeError Adapter::InitializeImpl() {
    DAWN_TRY(Base::InitializeImpl());
    // D3D11 cannot check for feature support without a device.
    // Create the device to populate the adapter properties then reuse it when needed for actual
    // rendering.
    DAWN_TRY_ASSIGN(mD3d11Device, CreateD3D11Device());

    mFeatureLevel = mD3d11Device->GetFeatureLevel();
    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(mD3d11Device));

    // Base::InitializeImpl() cannot distinguish between discrete and integrated GPUs, so we need to
    // overwrite it.
    if (mAdapterType == wgpu::AdapterType::DiscreteGPU && mDeviceInfo.isUMA) {
        mAdapterType = wgpu::AdapterType::IntegratedGPU;
    }

    return {};
}

void Adapter::InitializeSupportedFeaturesImpl() {
    EnableFeature(Feature::TextureCompressionBC);
    EnableFeature(Feature::SurfaceCapabilities);
}

MaybeError Adapter::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    GetDefaultLimits(&limits->v1);

    // // https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-feature-levels

    // Limits that are the same across D3D feature levels
    limits->v1.maxTextureDimension1D = D3D11_REQ_TEXTURE1D_U_DIMENSION;
    limits->v1.maxTextureDimension2D = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    limits->v1.maxTextureDimension3D = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
    limits->v1.maxTextureArrayLayers = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
    // Slot values can be 0-15, inclusive:
    // https://docs.microsoft.com/en-ca/windows/win32/api/d3d11/ns-d3d11-d3d11_input_element_desc
    limits->v1.maxVertexBuffers = 16;
    limits->v1.maxVertexAttributes = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;

    uint32_t maxUAVsAllStages = mFeatureLevel == D3D_FEATURE_LEVEL_11_1
                                    ? D3D11_1_UAV_SLOT_COUNT
                                    : D3D11_PS_CS_UAV_REGISTER_COUNT;
    ASSERT(maxUAVsAllStages / 4 > limits->v1.maxStorageTexturesPerShaderStage);
    ASSERT(maxUAVsAllStages / 4 > limits->v1.maxStorageBuffersPerShaderStage);
    uint32_t maxUAVsPerStage = maxUAVsAllStages / 2;

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;
    limits->v1.maxUniformBuffersPerShaderStage =
        D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots;

    // Allocate half of the UAVs to storage buffers, and half to storage textures.
    limits->v1.maxStorageTexturesPerShaderStage = maxUAVsPerStage / 2;
    limits->v1.maxStorageBuffersPerShaderStage = maxUAVsPerStage - maxUAVsPerStage / 2;
    limits->v1.maxSampledTexturesPerShaderStage = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    limits->v1.maxSamplersPerShaderStage = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    limits->v1.maxColorAttachments = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

    // TODO(dawn:1721): support dynamic uniform buffers and storage buffers?
    limits->v1.maxDynamicUniformBuffersPerPipelineLayout = 0;
    limits->v1.maxDynamicStorageBuffersPerPipelineLayout = 0;

    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-attributes-numthreads
    limits->v1.maxComputeWorkgroupSizeX = D3D11_CS_THREAD_GROUP_MAX_X;
    limits->v1.maxComputeWorkgroupSizeY = D3D11_CS_THREAD_GROUP_MAX_Y;
    limits->v1.maxComputeWorkgroupSizeZ = D3D11_CS_THREAD_GROUP_MAX_Z;
    limits->v1.maxComputeInvocationsPerWorkgroup = D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;

    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-dispatch
    limits->v1.maxComputeWorkgroupsPerDimension = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-compute-shaders
    // Thread Group Shared Memory is limited to 16Kb on downlevel hardware. This is less than
    // the 32Kb that is available to Direct3D 11 hardware. D3D12 is also 32kb.
    limits->v1.maxComputeWorkgroupStorageSize = 32768;

    // Max number of "constants" where each constant is a 16-byte float4
    limits->v1.maxUniformBufferBindingSize = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
    // D3D11 has no documented limit on the size of a storage buffer binding.
    limits->v1.maxStorageBufferBindingSize = 4294967295;
    // D3D11 has no documented limit on the buffer size.
    limits->v1.maxBufferSize = kAssumedMaxBufferSize;

    return {};
}

MaybeError Adapter::ValidateFeatureSupportedWithTogglesImpl(wgpu::FeatureName feature,
                                                            const TogglesState& toggles) const {
    return {};
}

void Adapter::SetupBackendDeviceToggles(TogglesState* deviceToggles) const {}

ResultOrError<Ref<DeviceBase>> Adapter::CreateDeviceImpl(const DeviceDescriptor* descriptor,
                                                         const TogglesState& deviceToggles) {
    return Device::Create(this, descriptor, deviceToggles);
}

// Resets the backend device and creates a new one. If any D3D11 objects belonging to the
// current ID3D11Device have not been destroyed, a non-zero value will be returned upon Reset()
// and the subequent call to CreateDevice will return a handle the existing device instead of
// creating a new one.
MaybeError Adapter::ResetInternalDeviceForTestingImpl() {
    [[maybe_unused]] auto refCount = mD3d11Device.Reset();
    ASSERT(refCount == 0);
    DAWN_TRY(Initialize());

    return {};
}

}  // namespace dawn::native::d3d11
