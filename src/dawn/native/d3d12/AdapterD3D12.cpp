// Copyright 2019 The Dawn Authors
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

#include "dawn/native/d3d12/AdapterD3D12.h"

#include <sstream>

#include "dawn/common/Constants.h"
#include "dawn/common/WindowsUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PlatformFunctions.h"

namespace dawn::native::d3d12 {

Adapter::Adapter(Backend* backend, ComPtr<IDXGIAdapter3> hardwareAdapter)
    : AdapterBase(backend->GetInstance(), wgpu::BackendType::D3D12),
      mHardwareAdapter(hardwareAdapter),
      mBackend(backend) {}

Adapter::~Adapter() {
    CleanUpDebugLayerFilters();
}

bool Adapter::SupportsExternalImages() const {
    // Via dawn::native::d3d12::ExternalImageDXGI::Create
    return true;
}

const D3D12DeviceInfo& Adapter::GetDeviceInfo() const {
    return mDeviceInfo;
}

IDXGIAdapter3* Adapter::GetHardwareAdapter() const {
    return mHardwareAdapter.Get();
}

Backend* Adapter::GetBackend() const {
    return mBackend;
}

ComPtr<ID3D12Device> Adapter::GetDevice() const {
    return mD3d12Device;
}

const gpu_info::D3DDriverVersion& Adapter::GetDriverVersion() const {
    return mDriverVersion;
}

MaybeError Adapter::InitializeImpl() {
    // D3D12 cannot check for feature support without a device.
    // Create the device to populate the adapter properties then reuse it when needed for actual
    // rendering.
    const PlatformFunctions* functions = GetBackend()->GetFunctions();
    if (FAILED(functions->d3d12CreateDevice(GetHardwareAdapter(), D3D_FEATURE_LEVEL_11_0,
                                            _uuidof(ID3D12Device), &mD3d12Device))) {
        return DAWN_INTERNAL_ERROR("D3D12CreateDevice failed");
    }

    DAWN_TRY(InitializeDebugLayerFilters());

    DXGI_ADAPTER_DESC1 adapterDesc;
    mHardwareAdapter->GetDesc1(&adapterDesc);

    mDeviceId = adapterDesc.DeviceId;
    mVendorId = adapterDesc.VendorId;
    mName = WCharToUTF8(adapterDesc.Description);

    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this));

    if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        mAdapterType = wgpu::AdapterType::CPU;
    } else {
        mAdapterType =
            (mDeviceInfo.isUMA) ? wgpu::AdapterType::IntegratedGPU : wgpu::AdapterType::DiscreteGPU;
    }

    // Convert the adapter's D3D12 driver version to a readable string like "24.21.13.9793".
    LARGE_INTEGER umdVersion;
    if (mHardwareAdapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion) !=
        DXGI_ERROR_UNSUPPORTED) {
        uint64_t encodedVersion = umdVersion.QuadPart;

        std::ostringstream o;
        o << "D3D12 driver version ";
        for (size_t i = 0; i < mDriverVersion.size(); ++i) {
            mDriverVersion[i] = (encodedVersion >> (48 - 16 * i)) & 0xFFFF;
            o << mDriverVersion[i] << ".";
        }
        mDriverDescription = o.str();
    }

    return {};
}

bool Adapter::AreTimestampQueriesSupported() const {
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;
    HRESULT hr = mD3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3d12CommandQueue));
    if (FAILED(hr)) {
        return false;
    }

    // GetTimestampFrequency returns an error HRESULT when there are bugs in Windows container
    // and vGPU implementations.
    uint64_t timeStampFrequency;
    hr = d3d12CommandQueue->GetTimestampFrequency(&timeStampFrequency);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

MaybeError Adapter::InitializeSupportedFeaturesImpl() {
    if (AreTimestampQueriesSupported()) {
        mSupportedFeatures.EnableFeature(Feature::TimestampQuery);
    }
    mSupportedFeatures.EnableFeature(Feature::TextureCompressionBC);
    mSupportedFeatures.EnableFeature(Feature::PipelineStatisticsQuery);
    mSupportedFeatures.EnableFeature(Feature::MultiPlanarFormats);
    mSupportedFeatures.EnableFeature(Feature::Depth24UnormStencil8);
    mSupportedFeatures.EnableFeature(Feature::Depth32FloatStencil8);

    return {};
}

MaybeError Adapter::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    D3D12_FEATURE_DATA_D3D12_OPTIONS featureData = {};

    DAWN_TRY(CheckHRESULT(mD3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,
                                                            &featureData, sizeof(featureData)),
                          "CheckFeatureSupport D3D12_FEATURE_D3D12_OPTIONS"));

    // Check if the device is at least D3D_FEATURE_LEVEL_11_1 or D3D_FEATURE_LEVEL_11_0
    const D3D_FEATURE_LEVEL levelsToQuery[]{D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

    D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels;
    featureLevels.NumFeatureLevels = sizeof(levelsToQuery) / sizeof(D3D_FEATURE_LEVEL);
    featureLevels.pFeatureLevelsRequested = levelsToQuery;
    DAWN_TRY(CheckHRESULT(mD3d12Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS,
                                                            &featureLevels, sizeof(featureLevels)),
                          "CheckFeatureSupport D3D12_FEATURE_FEATURE_LEVELS"));

    if (featureLevels.MaxSupportedFeatureLevel == D3D_FEATURE_LEVEL_11_0 &&
        featureData.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_2) {
        return DAWN_VALIDATION_ERROR(
            "At least Resource Binding Tier 2 is required for D3D12 Feature Level 11.0 "
            "devices.");
    }

    GetDefaultLimits(&limits->v1);

    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-feature-levels

    // Limits that are the same across D3D feature levels
    limits->v1.maxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
    limits->v1.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    limits->v1.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
    limits->v1.maxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
    // Slot values can be 0-15, inclusive:
    // https://docs.microsoft.com/en-ca/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
    limits->v1.maxVertexBuffers = 16;
    limits->v1.maxVertexAttributes = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;

    // Note: WebGPU requires FL11.1+
    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
    // Resource Binding Tier:   1      2      3

    // Max(CBV+UAV+SRV)         1M    1M    1M+
    // Max CBV per stage        14    14   full
    // Max SRV per stage       128  full   full
    // Max UAV in all stages    64    64   full
    // Max Samplers per stage   16  2048   2048

    // https://docs.microsoft.com/en-us/windows-hardware/test/hlk/testref/efad06e8-51d1-40ce-ad5c-573a134b4bb6
    // "full" means the full heap can be used. This is tested
    // to work for 1 million descriptors, and 1.1M for tier 3.
    uint32_t maxCBVsPerStage;
    uint32_t maxSRVsPerStage;
    uint32_t maxUAVsAllStages;
    uint32_t maxSamplersPerStage;
    switch (featureData.ResourceBindingTier) {
        case D3D12_RESOURCE_BINDING_TIER_1:
            maxCBVsPerStage = 14;
            maxSRVsPerStage = 128;
            maxUAVsAllStages = 64;
            maxSamplersPerStage = 16;
            break;
        case D3D12_RESOURCE_BINDING_TIER_2:
            maxCBVsPerStage = 14;
            maxSRVsPerStage = 1'000'000;
            maxUAVsAllStages = 64;
            maxSamplersPerStage = 2048;
            break;
        case D3D12_RESOURCE_BINDING_TIER_3:
        default:
            maxCBVsPerStage = 1'100'000;
            maxSRVsPerStage = 1'100'000;
            maxUAVsAllStages = 1'100'000;
            maxSamplersPerStage = 2048;
            break;
    }

    ASSERT(maxUAVsAllStages / 4 > limits->v1.maxStorageTexturesPerShaderStage);
    ASSERT(maxUAVsAllStages / 4 > limits->v1.maxStorageBuffersPerShaderStage);
    uint32_t maxUAVsPerStage = maxUAVsAllStages / 2;

    limits->v1.maxUniformBuffersPerShaderStage = maxCBVsPerStage;
    // Allocate half of the UAVs to storage buffers, and half to storage textures.
    limits->v1.maxStorageTexturesPerShaderStage = maxUAVsPerStage / 2;
    limits->v1.maxStorageBuffersPerShaderStage = maxUAVsPerStage - maxUAVsPerStage / 2;
    limits->v1.maxSampledTexturesPerShaderStage = maxSRVsPerStage;
    limits->v1.maxSamplersPerShaderStage = maxSamplersPerStage;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits
    // In DWORDS. Descriptor tables cost 1, Root constants cost 1, Root descriptors cost 2.
    static constexpr uint32_t kMaxRootSignatureSize = 64u;
    // Dawn maps WebGPU's binding model by:
    //  - (maxBindGroups)
    //    CBVs/UAVs/SRVs for bind group are a root descriptor table
    //  - (maxBindGroups)
    //    Samplers for each bind group are a root descriptor table
    //  - (2 * maxDynamicBuffers)
    //    Each dynamic buffer is a root descriptor
    //  RESERVED:
    //  - 3 = max of:
    //    - 2 root constants for the baseVertex/baseInstance constants.
    //    - 3 root constants for num workgroups X, Y, Z
    //  - 4 root constants (kMaxDynamicStorageBuffersPerPipelineLayout) for dynamic storage
    //  buffer lengths.
    static constexpr uint32_t kReservedSlots = 7;

    // Available slots after base limits considered.
    uint32_t availableRootSignatureSlots =
        kMaxRootSignatureSize - kReservedSlots -
        2 * (limits->v1.maxBindGroups + limits->v1.maxDynamicUniformBuffersPerPipelineLayout +
             limits->v1.maxDynamicStorageBuffersPerPipelineLayout);

    // Because we need either:
    //  - 1 cbv/uav/srv table + 1 sampler table
    //  - 2 slots for a root descriptor
    uint32_t availableDynamicBufferOrBindGroup = availableRootSignatureSlots / 2;

    // We can either have a bind group, a dyn uniform buffer or a dyn storage buffer.
    // Distribute evenly.
    limits->v1.maxBindGroups += availableDynamicBufferOrBindGroup / 3;
    limits->v1.maxDynamicUniformBuffersPerPipelineLayout += availableDynamicBufferOrBindGroup / 3;
    limits->v1.maxDynamicStorageBuffersPerPipelineLayout +=
        (availableDynamicBufferOrBindGroup - 2 * (availableDynamicBufferOrBindGroup / 3));

    ASSERT(2 * (limits->v1.maxBindGroups + limits->v1.maxDynamicUniformBuffersPerPipelineLayout +
                limits->v1.maxDynamicStorageBuffersPerPipelineLayout) <=
           kMaxRootSignatureSize - kReservedSlots);

    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-attributes-numthreads
    limits->v1.maxComputeWorkgroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X;
    limits->v1.maxComputeWorkgroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y;
    limits->v1.maxComputeWorkgroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z;
    limits->v1.maxComputeInvocationsPerWorkgroup = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;

    // https://docs.maxComputeWorkgroupSizeXmicrosoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_dispatch_arguments
    limits->v1.maxComputeWorkgroupsPerDimension = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-compute-shaders
    // Thread Group Shared Memory is limited to 16Kb on downlevel hardware. This is less than
    // the 32Kb that is available to Direct3D 11 hardware. D3D12 is also 32kb.
    limits->v1.maxComputeWorkgroupStorageSize = 32768;

    // Max number of "constants" where each constant is a 16-byte float4
    limits->v1.maxUniformBufferBindingSize = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
    // D3D12 has no documented limit on the size of a storage buffer binding.
    limits->v1.maxStorageBufferBindingSize = 4294967295;

    // TODO(crbug.com/dawn/685):
    // LIMITS NOT SET:
    // - maxInterStageShaderComponents
    // - maxVertexBufferArrayStride

    return {};
}

MaybeError Adapter::InitializeDebugLayerFilters() {
    if (!GetInstance()->IsBackendValidationEnabled()) {
        return {};
    }

    D3D12_MESSAGE_ID denyIds[] = {
        //
        // Permanent IDs: list of warnings that are not applicable
        //

        // Resource sub-allocation partially maps pre-allocated heaps. This means the
        // entire physical addresses space may have no resources or have many resources
        // assigned the same heap.
        D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_HAS_NO_RESOURCE,
        D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS,

        // The debug layer validates pipeline objects when they are created. Dawn validates
        // them when them when they are set. Therefore, since the issue is caught at a later
        // time, we can silence this warnings.
        D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_RENDERTARGETVIEW_NOT_SET,

        // Adding a clear color during resource creation would require heuristics or delayed
        // creation.
        // https://crbug.com/dawn/418
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,

        // Dawn enforces proper Unmaps at a later time.
        // https://crbug.com/dawn/422
        D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_GPU_WRITTEN_READBACK_RESOURCE_MAPPED,

        // WebGPU allows empty scissors without empty viewports.
        D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE,

        //
        // Temporary IDs: list of warnings that should be fixed or promoted
        //

        // Remove after warning have been addressed
        // https://crbug.com/dawn/421
        D3D12_MESSAGE_ID_GPU_BASED_VALIDATION_INCOMPATIBLE_RESOURCE_STATE,

        // For small placed resource alignment, we first request the small alignment, which may
        // get rejected and generate a debug error. Then, we request 0 to get the allowed
        // allowed alignment.
        D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,

        // WebGPU allows OOB vertex buffer access and relies on D3D12's robust buffer access
        // behavior.
        D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_TOO_SMALL,

        // WebGPU allows setVertexBuffer with offset that equals to the whole vertex buffer
        // size.
        // Even this means that no vertex buffer view has been set in D3D12 backend.
        // https://crbug.com/dawn/1255
        D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET,
    };

    // Create a retrieval filter with a deny list to suppress messages.
    // Any messages remaining will be converted to Dawn errors.
    D3D12_INFO_QUEUE_FILTER filter{};
    // Filter out info/message and only create errors from warnings or worse.
    D3D12_MESSAGE_SEVERITY severities[] = {
        D3D12_MESSAGE_SEVERITY_INFO,
        D3D12_MESSAGE_SEVERITY_MESSAGE,
    };
    filter.DenyList.NumSeverities = ARRAYSIZE(severities);
    filter.DenyList.pSeverityList = severities;
    filter.DenyList.NumIDs = ARRAYSIZE(denyIds);
    filter.DenyList.pIDList = denyIds;

    ComPtr<ID3D12InfoQueue> infoQueue;
    DAWN_TRY(CheckHRESULT(mD3d12Device.As(&infoQueue),
                          "D3D12 QueryInterface ID3D12Device to ID3D12InfoQueue"));

    // To avoid flooding the console, a storage-filter is also used to
    // prevent messages from getting logged.
    DAWN_TRY(
        CheckHRESULT(infoQueue->PushStorageFilter(&filter), "ID3D12InfoQueue::PushStorageFilter"));

    DAWN_TRY(CheckHRESULT(infoQueue->PushRetrievalFilter(&filter),
                          "ID3D12InfoQueue::PushRetrievalFilter"));

    return {};
}

void Adapter::CleanUpDebugLayerFilters() {
    if (!GetInstance()->IsBackendValidationEnabled()) {
        return;
    }

    // The device may not exist if this adapter failed to initialize.
    if (mD3d12Device == nullptr) {
        return;
    }

    // If the debug layer is not installed, return immediately to avoid crashing the process.
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (FAILED(mD3d12Device.As(&infoQueue))) {
        return;
    }

    infoQueue->PopRetrievalFilter();
    infoQueue->PopStorageFilter();
}

ResultOrError<Ref<DeviceBase>> Adapter::CreateDeviceImpl(const DeviceDescriptor* descriptor) {
    return Device::Create(this, descriptor);
}

// Resets the backend device and creates a new one. If any D3D12 objects belonging to the
// current ID3D12Device have not been destroyed, a non-zero value will be returned upon Reset()
// and the subequent call to CreateDevice will return a handle the existing device instead of
// creating a new one.
MaybeError Adapter::ResetInternalDeviceForTestingImpl() {
    ASSERT(mD3d12Device.Reset() == 0);
    DAWN_TRY(Initialize());

    return {};
}

}  // namespace dawn::native::d3d12
