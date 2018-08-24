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

#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"

namespace dawn_native { namespace metal {

    ComputePipeline::ComputePipeline(ComputePipelineBuilder* builder)
        : ComputePipelineBase(builder) {
        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();

        const auto& module = ToBackend(builder->GetStageInfo(dawn::ShaderStage::Compute).module);
        const auto& entryPoint = builder->GetStageInfo(dawn::ShaderStage::Compute).entryPoint;

        auto compilationData = module->GetFunction(entryPoint.c_str(), dawn::ShaderStage::Compute,
                                                   ToBackend(GetLayout()));

        NSError* error = nil;
        mMtlComputePipelineState =
            [mtlDevice newComputePipelineStateWithFunction:compilationData.function error:&error];
        if (error != nil) {
            NSLog(@" error => %@", error);
            builder->HandleError("Error creating pipeline state");
            return;
        }

        // Copy over the local workgroup size as it is passed to dispatch explicitly in Metal
        mLocalWorkgroupSize = compilationData.localWorkgroupSize;
    }

    ComputePipeline::~ComputePipeline() {
        [mMtlComputePipelineState release];
    }

    void ComputePipeline::Encode(id<MTLComputeCommandEncoder> encoder) {
        [encoder setComputePipelineState:mMtlComputePipelineState];
    }

    MTLSize ComputePipeline::GetLocalWorkGroupSize() const {
        return mLocalWorkgroupSize;
    }

}}  // namespace dawn_native::metal
