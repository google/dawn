// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>

#include "absl/types/span.h"  // TODO(343500108): Use std::span when we have C++20.
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/SupportedFeatures.h"
#include "src/dawn/wire/server/ObjectStorage.h"
#include "src/dawn/wire/server/Server.h"

namespace dawn::wire::server {

WireResult Server::DoInstanceRequestAdapter(Known<WGPUInstance> instance,
                                            Future future,
                                            ObjectHandle adapterHandle,
                                            const RequestAdapterOptions* options) {
    Reserved<WGPUAdapter> adapter;
    WIRE_TRY(Allocate(&adapter, adapterHandle, AllocationState::Reserved));

    auto userdata = MakeUserdata<RequestAdapterUserdata>();
    userdata->instanceId = instance.id;
    userdata->future = ToAPI(future);
    userdata->adapter = adapter.AsHandle();

    mProcs->instanceRequestAdapter(
        instance->handle, ToAPI(options),
        MakeCallbackInfo<WGPURequestAdapterCallbackInfo, &Server::OnRequestAdapterCallback,
                         WGPUCallbackMode_AllowSpontaneous>(userdata.release()));
    return WireResult::Success;
}

void Server::OnRequestAdapterCallback(RequestAdapterUserdata* data,
                                      WGPURequestAdapterStatus status,
                                      WGPUAdapter adapter,
                                      WGPUStringView message) {
    ReturnInstanceRequestAdapterCallbackCmd cmd = {};
    cmd.instanceId = data->instanceId;
    cmd.future = FromAPI(data->future);
    cmd.status = FromAPI(status);
    cmd.message = FromAPI(message);

    if (status != WGPURequestAdapterStatus_Success) {
        DAWN_ASSERT(adapter == nullptr);
        SerializeCommand(cmd);
        return;
    }

    // Assign the handle and allocated status if the adapter is created successfully.
    if (FillReservation(data->adapter, adapter) == WireResult::FatalError) {
        cmd.status = wgpu::RequestAdapterStatus::CallbackCancelled;
        cmd.message = FromAPI(ToOutputStringView("Destroyed before request was fulfilled."));
        SerializeCommand(cmd);
        return;
    }

    // Query and report the adapter supported features.
    FreeMembers<SupportedFeatures> supportedFeatures(mProcs);
    mProcs->adapterGetFeatures(adapter, ToAPI(&supportedFeatures));
    cmd.features = supportedFeatures.features;

    // Query and report the adapter info.
    FreeMembers<AdapterInfo> info(mProcs);
    ChainedStructOut** propertiesChain = &info.nextInChain;

    // Query AdapterPropertiesMemoryHeaps if the feature is supported.
    FreeMembers<AdapterPropertiesMemoryHeaps> memoryHeapProperties(mProcs);
    if (mProcs->adapterHasFeature(adapter, WGPUFeatureName_AdapterPropertiesMemoryHeaps)) {
        *propertiesChain = &memoryHeapProperties;
        propertiesChain = &(*propertiesChain)->nextInChain;
    }

    // Query AdapterPropertiesD3D if the feature is supported.
    AdapterPropertiesD3D d3dProperties;
    if (mProcs->adapterHasFeature(adapter, WGPUFeatureName_AdapterPropertiesD3D)) {
        *propertiesChain = &d3dProperties;
        propertiesChain = &(*propertiesChain)->nextInChain;
    }

    // Query AdapterPropertiesVk if the feature is supported.
    AdapterPropertiesVk vkProperties;
    if (mProcs->adapterHasFeature(adapter, WGPUFeatureName_AdapterPropertiesVk)) {
        *propertiesChain = &vkProperties;
        propertiesChain = &(*propertiesChain)->nextInChain;
    }

    // Query AdapterPropertiesSubgroupMatrixConfigs if the feature is supported.
    FreeMembers<AdapterPropertiesSubgroupMatrixConfigs> subgroupMatrixConfigs(mProcs);
    if (mProcs->adapterHasFeature(adapter, WGPUFeatureName_ChromiumExperimentalSubgroupMatrix)) {
        *propertiesChain = &subgroupMatrixConfigs;
        propertiesChain = &(*propertiesChain)->nextInChain;
    }

    DawnAdapterPropertiesPowerPreference powerProperties;
    *propertiesChain = &powerProperties;
    propertiesChain = &(*propertiesChain)->nextInChain;

    mProcs->adapterGetInfo(adapter, ToAPI(&info));
    cmd.info = &info;

    // Query and report the adapter limits, including all known extension limits.
    // TODO(crbug.com/421950205): Use dawn::utils::ComboLimits here.
    WGPULimits limits = {};
    // Chained CompatibilityModeLimits.
    WGPUCompatibilityModeLimits compatLimits = WGPU_COMPATIBILITY_MODE_LIMITS_INIT;
    compatLimits.chain.sType = WGPUSType_CompatibilityModeLimits;
    limits.nextInChain = &compatLimits.chain;
    // Chained DawnTexelCopyBufferRowAlignmentLimits.
    WGPUDawnTexelCopyBufferRowAlignmentLimits texelCopyBufferRowAlignmentLimits =
        WGPU_DAWN_TEXEL_COPY_BUFFER_ROW_ALIGNMENT_LIMITS_INIT;
    compatLimits.chain.next = &texelCopyBufferRowAlignmentLimits.chain;

    mProcs->adapterGetLimits(adapter, &limits);
    cmd.limits = FromAPI(&limits);

    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
