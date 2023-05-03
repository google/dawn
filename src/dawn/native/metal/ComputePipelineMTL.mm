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

#include "dawn/native/metal/ComputePipelineMTL.h"

#include "dawn/common/Math.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/CreatePipelineAsyncTask.h"
#include "dawn/native/Instance.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/ShaderModuleMTL.h"
#include "dawn/native/metal/UtilsMetal.h"

namespace dawn::native::metal {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const ComputePipelineDescriptor* descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::ComputePipeline(DeviceBase* dev, const ComputePipelineDescriptor* desc)
    : ComputePipelineBase(dev, desc) {}

ComputePipeline::~ComputePipeline() = default;

MaybeError ComputePipeline::Initialize() {
    auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();

    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);
    ShaderModule::MetalFunctionData computeData;

    DAWN_TRY(ToBackend(computeStage.module.Get())
                 ->CreateFunction(SingleShaderStage::Compute, computeStage, ToBackend(GetLayout()),
                                  &computeData));

    NSError* error = nullptr;
    NSRef<NSString> label = MakeDebugName(GetDevice(), "Dawn_ComputePipeline", GetLabel());

    NSRef<MTLComputePipelineDescriptor> descriptorRef =
        AcquireNSRef([MTLComputePipelineDescriptor new]);
    MTLComputePipelineDescriptor* descriptor = descriptorRef.Get();
    descriptor.computeFunction = computeData.function.Get();
    descriptor.label = label.Get();

    mMtlComputePipelineState.Acquire([mtlDevice
        newComputePipelineStateWithDescriptor:descriptor
                                      options:MTLPipelineOptionNone
                                   reflection:nil
                                        error:&error]);
    if (error != nullptr) {
        return DAWN_INTERNAL_ERROR("Error creating pipeline state " +
                                   std::string([error.localizedDescription UTF8String]));
    }
    ASSERT(mMtlComputePipelineState != nil);

    // Copy over the local workgroup size as it is passed to dispatch explicitly in Metal
    mLocalWorkgroupSize = computeData.localWorkgroupSize;

    mRequiresStorageBufferLength = computeData.needsStorageBufferLength;
    mWorkgroupAllocations = std::move(computeData.workgroupAllocations);
    return {};
}

void ComputePipeline::Encode(id<MTLComputeCommandEncoder> encoder) {
    [encoder setComputePipelineState:mMtlComputePipelineState.Get()];
    for (size_t i = 0; i < mWorkgroupAllocations.size(); ++i) {
        if (mWorkgroupAllocations[i] == 0) {
            continue;
        }
        // Size must be a multiple of 16 bytes.
        uint32_t rounded = Align<uint32_t>(mWorkgroupAllocations[i], 16);
        [encoder setThreadgroupMemoryLength:rounded atIndex:i];
    }
}

MTLSize ComputePipeline::GetLocalWorkGroupSize() const {
    return mLocalWorkgroupSize;
}

bool ComputePipeline::RequiresStorageBufferLength() const {
    return mRequiresStorageBufferLength;
}

void ComputePipeline::InitializeAsync(Ref<ComputePipelineBase> computePipeline,
                                      WGPUCreateComputePipelineAsyncCallback callback,
                                      void* userdata) {
    PhysicalDeviceBase* physicalDevice = computePipeline->GetDevice()->GetPhysicalDevice();
    std::unique_ptr<CreateComputePipelineAsyncTask> asyncTask =
        std::make_unique<CreateComputePipelineAsyncTask>(std::move(computePipeline), callback,
                                                         userdata);
    // Workaround a crash where the validation layers on AMD crash with partition alloc.
    // See crbug.com/dawn/1200.
    if (physicalDevice->GetInstance()->IsBackendValidationEnabled() &&
        gpu_info::IsAMD(physicalDevice->GetVendorId())) {
        asyncTask->Run();
        return;
    }
    CreateComputePipelineAsyncTask::RunAsync(std::move(asyncTask));
}

}  // namespace dawn::native::metal
