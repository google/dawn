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

#include "dawn_native/metal/BlendStateMTL.h"

namespace backend { namespace metal {

    namespace {

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
            return (((colorWriteMask & dawn::ColorWriteMask::Red) != dawn::ColorWriteMask::None
                         ? MTLColorWriteMaskRed
                         : MTLColorWriteMaskNone) |
                    ((colorWriteMask & dawn::ColorWriteMask::Green) != dawn::ColorWriteMask::None
                         ? MTLColorWriteMaskGreen
                         : MTLColorWriteMaskNone) |
                    ((colorWriteMask & dawn::ColorWriteMask::Blue) != dawn::ColorWriteMask::None
                         ? MTLColorWriteMaskBlue
                         : MTLColorWriteMaskNone) |
                    ((colorWriteMask & dawn::ColorWriteMask::Alpha) != dawn::ColorWriteMask::None
                         ? MTLColorWriteMaskAlpha
                         : MTLColorWriteMaskNone));
        }
    }

    BlendState::BlendState(BlendStateBuilder* builder) : BlendStateBase(builder) {
    }

    void BlendState::ApplyBlendState(MTLRenderPipelineColorAttachmentDescriptor* descriptor) const {
        auto& info = GetBlendInfo();
        descriptor.blendingEnabled = info.blendEnabled;
        descriptor.sourceRGBBlendFactor = MetalBlendFactor(info.colorBlend.srcFactor, false);
        descriptor.destinationRGBBlendFactor = MetalBlendFactor(info.colorBlend.dstFactor, false);
        descriptor.rgbBlendOperation = MetalBlendOperation(info.colorBlend.operation);
        descriptor.sourceAlphaBlendFactor = MetalBlendFactor(info.alphaBlend.srcFactor, true);
        descriptor.destinationAlphaBlendFactor = MetalBlendFactor(info.alphaBlend.dstFactor, true);
        descriptor.alphaBlendOperation = MetalBlendOperation(info.alphaBlend.operation);
        descriptor.writeMask = MetalColorWriteMask(info.colorWriteMask);
    }

}}  // namespace backend::metal
