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

#include "backend/metal/BlendStateMTL.h"
#include "backend/metal/DepthStencilStateMTL.h"
#include "backend/metal/InputStateMTL.h"
#include "backend/metal/MetalBackend.h"
#include "backend/metal/PipelineLayoutMTL.h"
#include "backend/metal/ShaderModuleMTL.h"
#include "backend/metal/TextureMTL.h"

namespace backend { namespace metal {

    namespace {
        MTLPrimitiveType MTLPrimitiveTopology(nxt::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case nxt::PrimitiveTopology::PointList:
                    return MTLPrimitiveTypePoint;
                case nxt::PrimitiveTopology::LineList:
                    return MTLPrimitiveTypeLine;
                case nxt::PrimitiveTopology::LineStrip:
                    return MTLPrimitiveTypeLineStrip;
                case nxt::PrimitiveTopology::TriangleList:
                    return MTLPrimitiveTypeTriangle;
                case nxt::PrimitiveTopology::TriangleStrip:
                    return MTLPrimitiveTypeTriangleStrip;
            }
        }

        MTLPrimitiveTopologyClass MTLInputPrimitiveTopology(
            nxt::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case nxt::PrimitiveTopology::PointList:
                    return MTLPrimitiveTopologyClassPoint;
                case nxt::PrimitiveTopology::LineList:
                case nxt::PrimitiveTopology::LineStrip:
                    return MTLPrimitiveTopologyClassLine;
                case nxt::PrimitiveTopology::TriangleList:
                case nxt::PrimitiveTopology::TriangleStrip:
                    return MTLPrimitiveTopologyClassTriangle;
            }
        }

        MTLIndexType MTLIndexFormat(nxt::IndexFormat format) {
            switch (format) {
                case nxt::IndexFormat::Uint16:
                    return MTLIndexTypeUInt16;
                case nxt::IndexFormat::Uint32:
                    return MTLIndexTypeUInt32;
            }
        }
    }

    RenderPipeline::RenderPipeline(RenderPipelineBuilder* builder)
        : RenderPipelineBase(builder),
          mMtlIndexType(MTLIndexFormat(GetIndexFormat())),
          mMtlPrimitiveTopology(MTLPrimitiveTopology(GetPrimitiveTopology())) {
        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();

        MTLRenderPipelineDescriptor* descriptor = [MTLRenderPipelineDescriptor new];

        for (auto stage : IterateStages(GetStageMask())) {
            const auto& module = ToBackend(builder->GetStageInfo(stage).module);

            const auto& entryPoint = builder->GetStageInfo(stage).entryPoint;
            ShaderModule::MetalFunctionData data =
                module->GetFunction(entryPoint.c_str(), ToBackend(GetLayout()));
            id<MTLFunction> function = data.function;

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

        if (HasDepthStencilAttachment()) {
            // TODO(kainino@chromium.org): Handle depth-only and stencil-only formats.
            nxt::TextureFormat depthStencilFormat = GetDepthStencilFormat();
            descriptor.depthAttachmentPixelFormat = MetalPixelFormat(depthStencilFormat);
            descriptor.stencilAttachmentPixelFormat = MetalPixelFormat(depthStencilFormat);
        }

        for (uint32_t i : IterateBitSet(GetColorAttachmentsMask())) {
            descriptor.colorAttachments[i].pixelFormat =
                MetalPixelFormat(GetColorAttachmentFormat(i));
            ToBackend(GetBlendState(i))->ApplyBlendState(descriptor.colorAttachments[i]);
        }

        descriptor.inputPrimitiveTopology = MTLInputPrimitiveTopology(GetPrimitiveTopology());

        InputState* inputState = ToBackend(GetInputState());
        descriptor.vertexDescriptor = inputState->GetMTLVertexDescriptor();

        // TODO(kainino@chromium.org): push constants, textures, samplers

        NSError* error = nil;
        mMtlRenderPipelineState =
            [mtlDevice newRenderPipelineStateWithDescriptor:descriptor error:&error];
        if (error != nil) {
            NSLog(@" error => %@", error);
            builder->HandleError("Error creating pipeline state");
            [descriptor release];
            return;
        }

        [descriptor release];
    }

    RenderPipeline::~RenderPipeline() {
        [mMtlRenderPipelineState release];
    }

    MTLIndexType RenderPipeline::GetMTLIndexType() const {
        return mMtlIndexType;
    }

    MTLPrimitiveType RenderPipeline::GetMTLPrimitiveTopology() const {
        return mMtlPrimitiveTopology;
    }

    void RenderPipeline::Encode(id<MTLRenderCommandEncoder> encoder) {
        [encoder setRenderPipelineState:mMtlRenderPipelineState];
    }

}}  // namespace backend::metal
