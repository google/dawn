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

#include "dawn_native/metal/ComputePipelineMTL.h"

#include "dawn_native/CreatePipelineAsyncTask.h"
#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"
#include "dawn_native/metal/UtilsMetal.h"

namespace dawn_native { namespace metal {

    // static
    Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
        Device* device,
        const ComputePipelineDescriptor* descriptor) {
        return AcquireRef(new ComputePipeline(device, descriptor));
    }

    MaybeError ComputePipeline::Initialize() {
        auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();

        const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);
        ShaderModule::MetalFunctionData computeData;

        DAWN_TRY(CreateMTLFunction(computeStage, SingleShaderStage::Compute, ToBackend(GetLayout()),
                                   &computeData));

        NSError* error = nullptr;
        mMtlComputePipelineState.Acquire([mtlDevice
            newComputePipelineStateWithFunction:computeData.function.Get()
                                          error:&error]);
        if (error != nullptr) {
            return DAWN_INTERNAL_ERROR("Error creating pipeline state" +
                                       std::string([error.localizedDescription UTF8String]));
        }
        ASSERT(mMtlComputePipelineState != nil);

        // Copy over the local workgroup size as it is passed to dispatch explicitly in Metal
        Origin3D localSize = GetStage(SingleShaderStage::Compute).metadata->localWorkgroupSize;
        mLocalWorkgroupSize = MTLSizeMake(localSize.x, localSize.y, localSize.z);

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
            [encoder setThreadgroupMemoryLength:mWorkgroupAllocations[i] atIndex:i];
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
        std::unique_ptr<CreateComputePipelineAsyncTask> asyncTask =
            std::make_unique<CreateComputePipelineAsyncTask>(std::move(computePipeline), callback,
                                                             userdata);
        CreateComputePipelineAsyncTask::RunAsync(std::move(asyncTask));
    }

}}  // namespace dawn_native::metal
