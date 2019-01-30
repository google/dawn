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

#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/InputStateMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"
#include "dawn_native/metal/TextureMTL.h"
#include "dawn_native/metal/UtilsMetal.h"

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

        MTLBlendFactor MetalBlendFactor(dawn::BlendFactor factor, bool alpha) {
            switch (factor) {
                case dawn::BlendFactor::Zero:
                    return MTLBlendFactorZero;
                case dawn::BlendFactor::One:
                    return MTLBlendFactorOne;
                case dawn::BlendFactor::SrcColor:
                    return MTLBlendFactorSourceColor;
                case dawn::BlendFactor::OneMinusSrcColor:
                    return MTLBlendFactorOneMinusSourceColor;
                case dawn::BlendFactor::SrcAlpha:
                    return MTLBlendFactorSourceAlpha;
                case dawn::BlendFactor::OneMinusSrcAlpha:
                    return MTLBlendFactorOneMinusSourceAlpha;
                case dawn::BlendFactor::DstColor:
                    return MTLBlendFactorDestinationColor;
                case dawn::BlendFactor::OneMinusDstColor:
                    return MTLBlendFactorOneMinusDestinationColor;
                case dawn::BlendFactor::DstAlpha:
                    return MTLBlendFactorDestinationAlpha;
                case dawn::BlendFactor::OneMinusDstAlpha:
                    return MTLBlendFactorOneMinusDestinationAlpha;
                case dawn::BlendFactor::SrcAlphaSaturated:
                    return MTLBlendFactorSourceAlphaSaturated;
                case dawn::BlendFactor::BlendColor:
                    return alpha ? MTLBlendFactorBlendAlpha : MTLBlendFactorBlendColor;
                case dawn::BlendFactor::OneMinusBlendColor:
                    return alpha ? MTLBlendFactorOneMinusBlendAlpha
                                 : MTLBlendFactorOneMinusBlendColor;
            }
        }

        MTLBlendOperation MetalBlendOperation(dawn::BlendOperation operation) {
            switch (operation) {
                case dawn::BlendOperation::Add:
                    return MTLBlendOperationAdd;
                case dawn::BlendOperation::Subtract:
                    return MTLBlendOperationSubtract;
                case dawn::BlendOperation::ReverseSubtract:
                    return MTLBlendOperationReverseSubtract;
                case dawn::BlendOperation::Min:
                    return MTLBlendOperationMin;
                case dawn::BlendOperation::Max:
                    return MTLBlendOperationMax;
            }
        }

        MTLColorWriteMask MetalColorWriteMask(dawn::ColorWriteMask colorWriteMask) {
            MTLColorWriteMask mask = MTLColorWriteMaskNone;

            if (colorWriteMask & dawn::ColorWriteMask::Red) {
                mask |= MTLColorWriteMaskRed;
            }
            if (colorWriteMask & dawn::ColorWriteMask::Green) {
                mask |= MTLColorWriteMaskGreen;
            }
            if (colorWriteMask & dawn::ColorWriteMask::Blue) {
                mask |= MTLColorWriteMaskBlue;
            }
            if (colorWriteMask & dawn::ColorWriteMask::Alpha) {
                mask |= MTLColorWriteMaskAlpha;
            }

            return mask;
        }

        void ComputeBlendDesc(MTLRenderPipelineColorAttachmentDescriptor* attachment,
                              const BlendStateDescriptor* descriptor) {
            attachment.blendingEnabled = descriptor->blendEnabled;
            attachment.sourceRGBBlendFactor =
                MetalBlendFactor(descriptor->colorBlend.srcFactor, false);
            attachment.destinationRGBBlendFactor =
                MetalBlendFactor(descriptor->colorBlend.dstFactor, false);
            attachment.rgbBlendOperation = MetalBlendOperation(descriptor->colorBlend.operation);
            attachment.sourceAlphaBlendFactor =
                MetalBlendFactor(descriptor->alphaBlend.srcFactor, true);
            attachment.destinationAlphaBlendFactor =
                MetalBlendFactor(descriptor->alphaBlend.dstFactor, true);
            attachment.alphaBlendOperation = MetalBlendOperation(descriptor->alphaBlend.operation);
            attachment.writeMask = MetalColorWriteMask(descriptor->colorWriteMask);
        }

        MTLStencilOperation MetalStencilOperation(dawn::StencilOperation stencilOperation) {
            switch (stencilOperation) {
                case dawn::StencilOperation::Keep:
                    return MTLStencilOperationKeep;
                case dawn::StencilOperation::Zero:
                    return MTLStencilOperationZero;
                case dawn::StencilOperation::Replace:
                    return MTLStencilOperationReplace;
                case dawn::StencilOperation::Invert:
                    return MTLStencilOperationInvert;
                case dawn::StencilOperation::IncrementClamp:
                    return MTLStencilOperationIncrementClamp;
                case dawn::StencilOperation::DecrementClamp:
                    return MTLStencilOperationDecrementClamp;
                case dawn::StencilOperation::IncrementWrap:
                    return MTLStencilOperationIncrementWrap;
                case dawn::StencilOperation::DecrementWrap:
                    return MTLStencilOperationDecrementWrap;
            }
        }

        MTLDepthStencilDescriptor* ComputeDepthStencilDesc(
            const DepthStencilStateDescriptor* descriptor) {
            MTLDepthStencilDescriptor* mtlDepthStencilDescriptor =
                [[MTLDepthStencilDescriptor new] autorelease];
            mtlDepthStencilDescriptor.depthCompareFunction =
                ToMetalCompareFunction(descriptor->depthCompare);
            mtlDepthStencilDescriptor.depthWriteEnabled = descriptor->depthWriteEnabled;

            if (StencilTestEnabled(descriptor)) {
                MTLStencilDescriptor* backFaceStencil = [[MTLStencilDescriptor new] autorelease];
                MTLStencilDescriptor* frontFaceStencil = [[MTLStencilDescriptor new] autorelease];

                backFaceStencil.stencilCompareFunction =
                    ToMetalCompareFunction(descriptor->stencilBack.compare);
                backFaceStencil.stencilFailureOperation =
                    MetalStencilOperation(descriptor->stencilBack.failOp);
                backFaceStencil.depthFailureOperation =
                    MetalStencilOperation(descriptor->stencilBack.depthFailOp);
                backFaceStencil.depthStencilPassOperation =
                    MetalStencilOperation(descriptor->stencilBack.passOp);
                backFaceStencil.readMask = descriptor->stencilReadMask;
                backFaceStencil.writeMask = descriptor->stencilWriteMask;

                frontFaceStencil.stencilCompareFunction =
                    ToMetalCompareFunction(descriptor->stencilFront.compare);
                frontFaceStencil.stencilFailureOperation =
                    MetalStencilOperation(descriptor->stencilFront.failOp);
                frontFaceStencil.depthFailureOperation =
                    MetalStencilOperation(descriptor->stencilFront.depthFailOp);
                frontFaceStencil.depthStencilPassOperation =
                    MetalStencilOperation(descriptor->stencilFront.passOp);
                frontFaceStencil.readMask = descriptor->stencilReadMask;
                frontFaceStencil.writeMask = descriptor->stencilWriteMask;

                mtlDepthStencilDescriptor.backFaceStencil = backFaceStencil;
                mtlDepthStencilDescriptor.frontFaceStencil = frontFaceStencil;
            }
            return mtlDepthStencilDescriptor;
        }

    }  // anonymous namespace

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
            const BlendStateDescriptor* descriptor = GetBlendStateDescriptor(i);
            ComputeBlendDesc(descriptorMTL.colorAttachments[i], descriptor);
        }

        descriptorMTL.inputPrimitiveTopology = MTLInputPrimitiveTopology(GetPrimitiveTopology());

        InputState* inputState = ToBackend(GetInputState());
        descriptorMTL.vertexDescriptor = inputState->GetMTLVertexDescriptor();

        // TODO(kainino@chromium.org): push constants, textures, samplers

        {
            NSError* error = nil;
            mMtlRenderPipelineState = [mtlDevice newRenderPipelineStateWithDescriptor:descriptorMTL
                                                                                error:&error];
            [descriptorMTL release];
            if (error != nil) {
                NSLog(@" error => %@", error);
                device->HandleError("Error creating rendering pipeline state");
                return;
            }
        }

        // create depth stencil state and cache it, fetch the cached depth stencil state when we
        // call setDepthStencilState() for a given render pipeline in CommandBuffer, in order to
        // improve performance.
        mMtlDepthStencilState =
            [mtlDevice newDepthStencilStateWithDescriptor:ComputeDepthStencilDesc(
                                                              GetDepthStencilStateDescriptor())];
    }

    RenderPipeline::~RenderPipeline() {
        [mMtlRenderPipelineState release];
        [mMtlDepthStencilState release];
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

    id<MTLDepthStencilState> RenderPipeline::GetMTLDepthStencilState() {
        return mMtlDepthStencilState;
    }

}}  // namespace dawn_native::metal
