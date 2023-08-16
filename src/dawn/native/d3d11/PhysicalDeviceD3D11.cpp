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

#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"

#include <string>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"

namespace dawn::native::d3d11 {
namespace {

MaybeError InitializeDebugLayerFilters(ComPtr<ID3D11Device> d3d11Device) {
    ComPtr<ID3D11InfoQueue> infoQueue;
    DAWN_TRY(CheckHRESULT(d3d11Device.As(&infoQueue),
                          "D3D11 querying device for ID3D11InfoQueue interface"));

    static D3D11_MESSAGE_ID kDenyIds[] = {
        // D3D11 Debug layer warns no RTV set, however it is allowed.
        D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
        // D3D11 Debug layer warns SetPrivateData() with same name more than once.
        D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
    };

    // Filter out info/message and only create errors from warnings or worse.
    static D3D11_MESSAGE_SEVERITY kDenySeverities[] = {
        D3D11_MESSAGE_SEVERITY_INFO,
        D3D11_MESSAGE_SEVERITY_MESSAGE,
    };

    static D3D11_INFO_QUEUE_FILTER filter = {
        {},  // AllowList
        {
            0,                           // NumCategories
            nullptr,                     // pCategoryList
            std::size(kDenySeverities),  // NumSeverities
            kDenySeverities,             // pSeverityList
            std::size(kDenyIds),         // NumIDs
            kDenyIds,                    // pIDList
        },                               // DenyList
    };

    return CheckHRESULT(infoQueue->PushStorageFilter(&filter),
                        "D3D11 InfoQueue pushing storage filter");
}

}  // namespace

PhysicalDevice::PhysicalDevice(Backend* backend, ComPtr<IDXGIAdapter3> hardwareAdapter)
    : Base(backend, std::move(hardwareAdapter), wgpu::BackendType::D3D11) {}

PhysicalDevice::~PhysicalDevice() = default;

bool PhysicalDevice::SupportsExternalImages() const {
    return true;
}

bool PhysicalDevice::SupportsFeatureLevel(FeatureLevel featureLevel) const {
    // TODO(dawn:1820): compare D3D11 feature levels with Dawn feature levels.
    switch (featureLevel) {
        case FeatureLevel::Core: {
            return mFeatureLevel >= D3D_FEATURE_LEVEL_11_1;
        }
        case FeatureLevel::Compatibility: {
            return true;
        }
    }
}

const DeviceInfo& PhysicalDevice::GetDeviceInfo() const {
    return mDeviceInfo;
}

ResultOrError<ComPtr<ID3D11Device>> PhysicalDevice::CreateD3D11Device() {
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

        if (GetInstance()->IsBackendValidationEnabled()) {
            DAWN_TRY(InitializeDebugLayerFilters(device));
        }
    }
    return device;
}

MaybeError PhysicalDevice::InitializeImpl() {
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

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    EnableFeature(Feature::Depth32FloatStencil8);
    EnableFeature(Feature::DepthClipControl);
    EnableFeature(Feature::TextureCompressionBC);
    EnableFeature(Feature::SurfaceCapabilities);
    EnableFeature(Feature::D3D11MultithreadProtected);
    EnableFeature(Feature::MSAARenderToSingleSampled);
    EnableFeature(Feature::DualSourceBlending);

    // To import multi planar textures, we need to at least tier 2 support.
    if (mDeviceInfo.supportsSharedResourceCapabilityTier2) {
        EnableFeature(Feature::MultiPlanarFormats);
    }
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
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
    mUAVSlotCount = maxUAVsAllStages;
    uint32_t maxUAVsPerStage = maxUAVsAllStages / 2;

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;
    limits->v1.maxUniformBuffersPerShaderStage =
        D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots;

    // Allocate half of the UAVs to storage buffers, and half to storage textures.
    limits->v1.maxStorageTexturesPerShaderStage = maxUAVsPerStage / 2;
    limits->v1.maxStorageBuffersPerShaderStage = maxUAVsPerStage / 2;
    limits->v1.maxSampledTexturesPerShaderStage = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    limits->v1.maxSamplersPerShaderStage = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    limits->v1.maxColorAttachments = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

    limits->v1.maxDynamicUniformBuffersPerPipelineLayout =
        limits->v1.maxUniformBuffersPerShaderStage;
    limits->v1.maxDynamicStorageBuffersPerPipelineLayout =
        limits->v1.maxStorageBuffersPerShaderStage;

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

    if (gpu_info::IsQualcomm(GetVendorId())) {
        // limit of number of texels in a buffer == (1 << 27)
        // D3D11_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP
        // This limit doesn't apply to a raw buffer, but only applies to
        // typed, or structured buffer. so this could be a QC driver bug.
        limits->v1.maxStorageBufferBindingSize = uint64_t(1)
                                                 << D3D11_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP;
    } else {
        limits->v1.maxStorageBufferBindingSize = kAssumedMaxBufferSize;
    }

    // D3D11 has no documented limit on the buffer size.
    limits->v1.maxBufferSize = kAssumedMaxBufferSize;

    return {};
}

MaybeError PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    return {};
}

void PhysicalDevice::SetupBackendAdapterToggles(TogglesState* adpterToggles) const {
    // D3D11 must use FXC, not DXC.
    adpterToggles->ForceSet(Toggle::UseDXC, false);
}

void PhysicalDevice::SetupBackendDeviceToggles(TogglesState* deviceToggles) const {
    // D3D11 can only clear RTV with float values.
    deviceToggles->Default(Toggle::ApplyClearBigIntegerColorValueWithDraw, true);
    deviceToggles->Default(Toggle::UseBlitForBufferToStencilTextureCopy, true);
}

ResultOrError<Ref<DeviceBase>> PhysicalDevice::CreateDeviceImpl(AdapterBase* adapter,
                                                                const DeviceDescriptor* descriptor,
                                                                const TogglesState& deviceToggles) {
    return Device::Create(adapter, descriptor, deviceToggles);
}

// Resets the backend device and creates a new one. If any D3D11 objects belonging to the
// current ID3D11Device have not been destroyed, a non-zero value will be returned upon Reset()
// and the subequent call to CreateDevice will return a handle the existing device instead of
// creating a new one.
MaybeError PhysicalDevice::ResetInternalDeviceForTestingImpl() {
    [[maybe_unused]] auto refCount = mD3d11Device.Reset();
    ASSERT(refCount == 0);
    DAWN_TRY(Initialize());

    return {};
}

}  // namespace dawn::native::d3d11
