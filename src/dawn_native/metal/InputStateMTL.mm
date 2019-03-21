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

#include "dawn_native/metal/InputStateMTL.h"

#include "common/BitSetIterator.h"

namespace dawn_native { namespace metal {

    namespace {
        MTLVertexFormat VertexFormatType(dawn::VertexFormat format) {
            switch (format) {
                case dawn::VertexFormat::UChar2:
                    return MTLVertexFormatUChar2;
                case dawn::VertexFormat::UChar4:
                    return MTLVertexFormatUChar4;
                case dawn::VertexFormat::Char2:
                    return MTLVertexFormatChar2;
                case dawn::VertexFormat::Char4:
                    return MTLVertexFormatChar4;
                case dawn::VertexFormat::UChar2Norm:
                    return MTLVertexFormatUChar2Normalized;
                case dawn::VertexFormat::UChar4Norm:
                    return MTLVertexFormatUChar4Normalized;
                case dawn::VertexFormat::Char2Norm:
                    return MTLVertexFormatChar2Normalized;
                case dawn::VertexFormat::Char4Norm:
                    return MTLVertexFormatChar4Normalized;
                case dawn::VertexFormat::UShort2:
                    return MTLVertexFormatUShort2;
                case dawn::VertexFormat::UShort4:
                    return MTLVertexFormatUShort4;
                case dawn::VertexFormat::Short2:
                    return MTLVertexFormatShort2;
                case dawn::VertexFormat::Short4:
                    return MTLVertexFormatShort4;
                case dawn::VertexFormat::UShort2Norm:
                    return MTLVertexFormatUShort2Normalized;
                case dawn::VertexFormat::UShort4Norm:
                    return MTLVertexFormatUShort4Normalized;
                case dawn::VertexFormat::Short2Norm:
                    return MTLVertexFormatShort2Normalized;
                case dawn::VertexFormat::Short4Norm:
                    return MTLVertexFormatShort4Normalized;
                case dawn::VertexFormat::Half2:
                    return MTLVertexFormatHalf2;
                case dawn::VertexFormat::Half4:
                    return MTLVertexFormatHalf4;
                case dawn::VertexFormat::Float:
                    return MTLVertexFormatFloat;
                case dawn::VertexFormat::Float2:
                    return MTLVertexFormatFloat2;
                case dawn::VertexFormat::Float3:
                    return MTLVertexFormatFloat3;
                case dawn::VertexFormat::Float4:
                    return MTLVertexFormatFloat4;
                case dawn::VertexFormat::UInt:
                    return MTLVertexFormatUInt;
                case dawn::VertexFormat::UInt2:
                    return MTLVertexFormatUInt2;
                case dawn::VertexFormat::UInt3:
                    return MTLVertexFormatUInt3;
                case dawn::VertexFormat::UInt4:
                    return MTLVertexFormatUInt4;
                case dawn::VertexFormat::Int:
                    return MTLVertexFormatInt;
                case dawn::VertexFormat::Int2:
                    return MTLVertexFormatInt2;
                case dawn::VertexFormat::Int3:
                    return MTLVertexFormatInt3;
                case dawn::VertexFormat::Int4:
                    return MTLVertexFormatInt4;
            }
        }

        MTLVertexStepFunction InputStepModeFunction(dawn::InputStepMode mode) {
            switch (mode) {
                case dawn::InputStepMode::Vertex:
                    return MTLVertexStepFunctionPerVertex;
                case dawn::InputStepMode::Instance:
                    return MTLVertexStepFunctionPerInstance;
            }
        }
    }

    InputState::InputState(InputStateBuilder* builder) : InputStateBase(builder) {
        mMtlVertexDescriptor = [MTLVertexDescriptor new];

        const auto& attributesSetMask = GetAttributesSetMask();
        for (uint32_t i = 0; i < attributesSetMask.size(); ++i) {
            if (!attributesSetMask[i]) {
                continue;
            }
            const VertexAttributeDescriptor& info = GetAttribute(i);

            auto attribDesc = [MTLVertexAttributeDescriptor new];
            attribDesc.format = VertexFormatType(info.format);
            attribDesc.offset = info.offset;
            attribDesc.bufferIndex = kMaxBindingsPerGroup + info.inputSlot;
            mMtlVertexDescriptor.attributes[i] = attribDesc;
            [attribDesc release];
        }

        for (uint32_t i : IterateBitSet(GetInputsSetMask())) {
            const VertexInputDescriptor& info = GetInput(i);

            auto layoutDesc = [MTLVertexBufferLayoutDescriptor new];
            if (info.stride == 0) {
                // For MTLVertexStepFunctionConstant, the stepRate must be 0,
                // but the stride must NOT be 0, so we made up it with
                // max(attrib.offset + sizeof(attrib) for each attrib)
                uint32_t max_stride = 0;
                for (uint32_t attribIndex : IterateBitSet(attributesSetMask)) {
                    const VertexAttributeDescriptor& attrib = GetAttribute(attribIndex);
                    // Only use the attributes that use the current input
                    if (attrib.inputSlot != info.inputSlot) {
                        continue;
                    }
                    max_stride = std::max(
                        max_stride,
                        static_cast<uint32_t>(VertexFormatSize(attrib.format)) + attrib.offset);
                }

                layoutDesc.stepFunction = MTLVertexStepFunctionConstant;
                layoutDesc.stepRate = 0;
                // Metal requires the stride must be a multiple of 4 bytes, align it with next
                // multiple of 4 if it's not.
                layoutDesc.stride = Align(max_stride, 4);
            } else {
                layoutDesc.stepFunction = InputStepModeFunction(info.stepMode);
                layoutDesc.stepRate = 1;
                layoutDesc.stride = info.stride;
            }
            // TODO(cwallez@chromium.org): make the offset depend on the pipeline layout
            mMtlVertexDescriptor.layouts[kMaxBindingsPerGroup + i] = layoutDesc;
            [layoutDesc release];
        }
    }

    InputState::~InputState() {
        [mMtlVertexDescriptor release];
        mMtlVertexDescriptor = nil;
    }

    MTLVertexDescriptor* InputState::GetMTLVertexDescriptor() {
        return mMtlVertexDescriptor;
    }

}}  // namespace dawn_native::metal
