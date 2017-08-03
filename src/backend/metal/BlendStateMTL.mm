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

#include "backend/metal/BlendStateMTL.h"

#include "backend/metal/MetalBackend.h"

namespace backend {
namespace metal {

    namespace {

        MTLBlendFactor MetalBlendFactor(nxt::BlendFactor factor, bool alpha) {
            switch(factor) {
                case nxt::BlendFactor::Zero:
                    return MTLBlendFactorZero;
                case nxt::BlendFactor::One:
                    return MTLBlendFactorOne;
                case nxt::BlendFactor::SrcColor:
                    return MTLBlendFactorSourceColor;
                case nxt::BlendFactor::OneMinusSrcColor:
                    return MTLBlendFactorOneMinusSourceColor;
                case nxt::BlendFactor::SrcAlpha:
                    return MTLBlendFactorSourceAlpha;
                case nxt::BlendFactor::OneMinusSrcAlpha:
                    return MTLBlendFactorOneMinusSourceAlpha;
                case nxt::BlendFactor::DstColor:
                    return MTLBlendFactorDestinationColor;
                case nxt::BlendFactor::OneMinusDstColor:
                    return MTLBlendFactorOneMinusDestinationColor;
                case nxt::BlendFactor::DstAlpha:
                    return MTLBlendFactorDestinationAlpha;
                case nxt::BlendFactor::OneMinusDstAlpha:
                    return MTLBlendFactorOneMinusDestinationAlpha;
                case nxt::BlendFactor::SrcAlphaSaturated:
                    return MTLBlendFactorSourceAlphaSaturated;
                case nxt::BlendFactor::BlendColor:
                    return alpha ? MTLBlendFactorBlendAlpha : MTLBlendFactorBlendColor;
                case nxt::BlendFactor::OneMinusBlendColor:
                    return alpha ? MTLBlendFactorOneMinusBlendAlpha : MTLBlendFactorOneMinusBlendColor;
            }
        }

        MTLBlendOperation MetalBlendOperation(nxt::BlendOperation operation) {
            switch(operation) {
                case nxt::BlendOperation::Add:
                    return MTLBlendOperationAdd;
                case nxt::BlendOperation::Subtract:
                    return MTLBlendOperationSubtract;
                case nxt::BlendOperation::ReverseSubtract:
                    return MTLBlendOperationReverseSubtract;
                case nxt::BlendOperation::Min:
                    return MTLBlendOperationMin;
                case nxt::BlendOperation::Max:
                    return MTLBlendOperationMax;
            }
        }

        MTLColorWriteMask MetalColorWriteMask(nxt::ColorWriteMask colorWriteMask) {
            return (
                ((colorWriteMask & nxt::ColorWriteMask::Red) != nxt::ColorWriteMask::None ? MTLColorWriteMaskRed : MTLColorWriteMaskNone) |
                ((colorWriteMask & nxt::ColorWriteMask::Green) != nxt::ColorWriteMask::None ? MTLColorWriteMaskGreen : MTLColorWriteMaskNone) |
                ((colorWriteMask & nxt::ColorWriteMask::Blue) != nxt::ColorWriteMask::None ? MTLColorWriteMaskBlue : MTLColorWriteMaskNone) |
                ((colorWriteMask & nxt::ColorWriteMask::Alpha) != nxt::ColorWriteMask::None ? MTLColorWriteMaskAlpha : MTLColorWriteMaskNone)
            );
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
}
}
