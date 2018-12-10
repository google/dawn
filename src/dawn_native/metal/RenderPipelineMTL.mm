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

#include "dawn_native/metal/RenderPipelineMTL.h"

#include "dawn_native/metal/BlendStateMTL.h"
#include "dawn_native/metal/DepthStencilStateMTL.h"
#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/InputStateMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"
#include "dawn_native/metal/TextureMTL.h"

namespace dawn_native { namespace metal {

    namespace {
        MTLPrimitiveType MTLPrimitiveTopology(dawn::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case dawn::PrimitiveTopology::PointList:
                    return MTLPrimitiveTypePoint;
                case dawn::PrimitiveTopology::LineList:
                    return MTLPrimitiveTypeLine;
                case dawn::PrimitiveTopology::LineStrip:
                    return MTLPrimitiveTypeLineStrip;
                case dawn::PrimitiveTopology::TriangleList:
                    return MTLPrimitiveTypeTriangle;
                case dawn::PrimitiveTopology::TriangleStrip:
                    return MTLPrimitiveTypeTriangleStrip;
            }
        }

        MTLPrimitiveTopologyClass MTLInputPrimitiveTopology(
            dawn::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case dawn::PrimitiveTopology::PointList:
                    return MTLPrimitiveTopologyClassPoint;
                case dawn::PrimitiveTopology::LineList:
                case dawn::PrimitiveTopology::LineStrip:
                    return MTLPrimitiveTopologyClassLine;
                case dawn::PrimitiveTopology::TriangleList:
                case dawn::PrimitiveTopology::TriangleStrip:
                    return MTLPrimitiveTopologyClassTriangle;
            }
        }

        MTLIndexType MTLIndexFormat(dawn::IndexFormat format) {
            switch (format) {
                case dawn::IndexFormat::Uint16:
                    return MTLIndexTypeUInt16;
                case dawn::IndexFormat::Uint32:
                    return MTLIndexTypeUInt32;
            }
        }
    }

    RenderPipeline::RenderPipeline(Device* device, const RenderPipelineDescriptor* descriptor)
        : RenderPipelineBase(device, descriptor),
          mMtlIndexType(MTLIndexFormat(GetIndexFormat())),
          mMtlPrimitiveTopology(MTLPrimitiveTopology(GetPrimitiveTopology())) {
        auto mtlDevice = device->GetMTLDevice();

        MTLRenderPipelineDescriptor* descriptorMTL = [MTLRenderPipelineDescriptor new];

        const ShaderModule* vertexModule = ToBackend(descriptor->vertexStage->module);
        const char* vertexEntryPoint = descriptor->vertexStage->entryPoint;
        ShaderModule::MetalFunctionData vertexData = vertexModule->GetFunction(
            vertexEntryPoint, dawn::ShaderStage::Vertex, ToBackend(GetLayout()));
        descriptorMTL.vertexFunction = vertexData.function;

        const ShaderModule* fragmentModule = ToBackend(descriptor->fragmentStage->module);
        const char* fragmentEntryPoint = descriptor->fragmentStage->entryPoint;
        ShaderModule::MetalFunctionData fragmentData = fragmentModule->GetFunction(
            fragmentEntryPoint, dawn::ShaderStage::Fragment, ToBackend(GetLayout()));
        descriptorMTL.fragmentFunction = fragmentData.function;

        if (HasDepthStencilAttachment()) {
            // TODO(kainino@chromium.org): Handle depth-only and stencil-only formats.
            dawn::TextureFormat depthStencilFormat = GetDepthStencilFormat();
            descriptorMTL.depthAttachmentPixelFormat = MetalPixelFormat(depthStencilFormat);
            descriptorMTL.stencilAttachmentPixelFormat = MetalPixelFormat(depthStencilFormat);
        }

        for (uint32_t i : IterateBitSet(GetColorAttachmentsMask())) {
            descriptorMTL.colorAttachments[i].pixelFormat =
                MetalPixelFormat(GetColorAttachmentFormat(i));
            ToBackend(GetBlendState(i))->ApplyBlendState(descriptorMTL.colorAttachments[i]);
        }

        descriptorMTL.inputPrimitiveTopology = MTLInputPrimitiveTopology(GetPrimitiveTopology());

        InputState* inputState = ToBackend(GetInputState());
        descriptorMTL.vertexDescriptor = inputState->GetMTLVertexDescriptor();

        // TODO(kainino@chromium.org): push constants, textures, samplers

        NSError* error = nil;
        mMtlRenderPipelineState = [mtlDevice newRenderPipelineStateWithDescriptor:descriptorMTL
                                                                            error:&error];
        if (error != nil) {
            NSLog(@" error => %@", error);
            device->HandleError("Error creating rendering pipeline state");
            [descriptorMTL release];
            return;
        }

        [descriptorMTL release];
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

}}  // namespace dawn_native::metal
