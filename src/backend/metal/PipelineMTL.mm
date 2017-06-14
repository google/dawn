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

#include "PipelineMTL.h"

#include "DepthStencilStateMTL.h"
#include "InputStateMTL.h"
#include "MetalBackend.h"
#include "PipelineLayoutMTL.h"
#include "ShaderModuleMTL.h"

namespace backend {
namespace metal {

    Pipeline::Pipeline(PipelineBuilder* builder)
        : PipelineBase(builder) {

        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();

        if (IsCompute()) {
            const auto& module = ToBackend(builder->GetStageInfo(nxt::ShaderStage::Compute).module);
            const auto& entryPoint = builder->GetStageInfo(nxt::ShaderStage::Compute).entryPoint;

            id<MTLFunction> function = module->GetFunction(entryPoint.c_str());

            NSError *error = nil;
            mtlComputePipelineState = [mtlDevice
                newComputePipelineStateWithFunction:function error:&error];
            if (error != nil) {
                NSLog(@" error => %@", error);
                builder->HandleError("Error creating pipeline state");
                return;
            }

            // Copy over the local workgroup size as it is passed to dispatch explicitly in Metal
            localWorkgroupSize = module->GetLocalWorkGroupSize(entryPoint);

        } else {
            MTLRenderPipelineDescriptor* descriptor = [MTLRenderPipelineDescriptor new];

            for (auto stage : IterateStages(GetStageMask())) {
                const auto& module = ToBackend(builder->GetStageInfo(stage).module);

                const auto& entryPoint = builder->GetStageInfo(stage).entryPoint;
                id<MTLFunction> function = module->GetFunction(entryPoint.c_str());

                switch (stage) {
                    case nxt::ShaderStage::Vertex:
                        descriptor.vertexFunction = function;
                        break;
                    case nxt::ShaderStage::Fragment:
                        descriptor.fragmentFunction = function;
                        break;
                    case nxt::ShaderStage::Compute:
                        ASSERT(false);
                        break;
                }
            }

            descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            descriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

            InputState* inputState = ToBackend(GetInputState());
            descriptor.vertexDescriptor = inputState->GetMTLVertexDescriptor();

            // TODO(kainino@chromium.org): push constants, textures, samplers

            NSError *error = nil;
            mtlRenderPipelineState = [mtlDevice
                newRenderPipelineStateWithDescriptor:descriptor error:&error];
            if (error != nil) {
                NSLog(@" error => %@", error);
                builder->HandleError("Error creating pipeline state");
                return;
            }

            [descriptor release];
        }
    }

    Pipeline::~Pipeline() {
        [mtlRenderPipelineState release];
        [mtlComputePipelineState release];
    }

    void Pipeline::Encode(id<MTLRenderCommandEncoder> encoder) {
        ASSERT(!IsCompute());
        [encoder setRenderPipelineState:mtlRenderPipelineState];
    }

    void Pipeline::Encode(id<MTLComputeCommandEncoder> encoder) {
        ASSERT(IsCompute());
        [encoder setComputePipelineState:mtlComputePipelineState];
    }

    MTLSize Pipeline::GetLocalWorkGroupSize() const {
        return localWorkgroupSize;
    }

}
}
