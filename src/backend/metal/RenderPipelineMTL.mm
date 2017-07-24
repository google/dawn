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

#include "backend/metal/RenderPipelineMTL.h"

#include "backend/metal/DepthStencilStateMTL.h"
#include "backend/metal/InputStateMTL.h"
#include "backend/metal/MetalBackend.h"
#include "backend/metal/PipelineLayoutMTL.h"
#include "backend/metal/ShaderModuleMTL.h"

namespace backend {
namespace metal {

    namespace {
        MTLPrimitiveType MTLPrimitiveTopology(nxt::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case nxt::PrimitiveTopology::Point:
                    return MTLPrimitiveTypePoint;
                case nxt::PrimitiveTopology::Line:
                    return MTLPrimitiveTypeLine;
                case nxt::PrimitiveTopology::LineStrip:
                    return MTLPrimitiveTypeLineStrip;
                case nxt::PrimitiveTopology::Triangle:
                    return MTLPrimitiveTypeTriangle;
                case nxt::PrimitiveTopology::TriangleStrip:
                    return MTLPrimitiveTypeTriangleStrip;
            }
        }
    }

    RenderPipeline::RenderPipeline(RenderPipelineBuilder* builder)
        : RenderPipelineBase(builder), mtlPrimitiveTopology(MTLPrimitiveTopology(GetPrimitiveTopology())) {

        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();

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
                    UNREACHABLE();
            }
        }

        // TODO(cwallez@chromium.org): get the attachment formats from the subpass
        descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
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

    RenderPipeline::~RenderPipeline() {
        [mtlRenderPipelineState release];
    }

    MTLPrimitiveType RenderPipeline::GetMTLPrimitiveTopology() const {
        return mtlPrimitiveTopology;
    }

    void RenderPipeline::Encode(id<MTLRenderCommandEncoder> encoder) {
        [encoder setRenderPipelineState:mtlRenderPipelineState];
    }

}
}
