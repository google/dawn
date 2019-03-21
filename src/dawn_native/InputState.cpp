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

#include "dawn_native/InputState.h"

#include "common/Assert.h"
#include "dawn_native/Device.h"
#include "dawn_native/ValidationUtils_autogen.h"

namespace dawn_native {

    // InputState helpers

    size_t IndexFormatSize(dawn::IndexFormat format) {
        switch (format) {
            case dawn::IndexFormat::Uint16:
                return sizeof(uint16_t);
            case dawn::IndexFormat::Uint32:
                return sizeof(uint32_t);
            default:
                UNREACHABLE();
        }
    }

    // TODO(shaobo.yan@intel.com): Add end2end test to cover all the formats.
    uint32_t VertexFormatNumComponents(dawn::VertexFormat format) {
        switch (format) {
            case dawn::VertexFormat::UChar4:
            case dawn::VertexFormat::Char4:
            case dawn::VertexFormat::UChar4Norm:
            case dawn::VertexFormat::Char4Norm:
            case dawn::VertexFormat::UShort4:
            case dawn::VertexFormat::Short4:
            case dawn::VertexFormat::UShort4Norm:
            case dawn::VertexFormat::Short4Norm:
            case dawn::VertexFormat::Half4:
            case dawn::VertexFormat::Float4:
            case dawn::VertexFormat::UInt4:
            case dawn::VertexFormat::Int4:
                return 4;
            case dawn::VertexFormat::Float3:
            case dawn::VertexFormat::UInt3:
            case dawn::VertexFormat::Int3:
                return 3;
            case dawn::VertexFormat::UChar2:
            case dawn::VertexFormat::Char2:
            case dawn::VertexFormat::UChar2Norm:
            case dawn::VertexFormat::Char2Norm:
            case dawn::VertexFormat::UShort2:
            case dawn::VertexFormat::Short2:
            case dawn::VertexFormat::UShort2Norm:
            case dawn::VertexFormat::Short2Norm:
            case dawn::VertexFormat::Half2:
            case dawn::VertexFormat::Float2:
            case dawn::VertexFormat::UInt2:
            case dawn::VertexFormat::Int2:
                return 2;
            case dawn::VertexFormat::Float:
            case dawn::VertexFormat::UInt:
            case dawn::VertexFormat::Int:
                return 1;
            default:
                UNREACHABLE();
        }
    }

    size_t VertexFormatComponentSize(dawn::VertexFormat format) {
        switch (format) {
            case dawn::VertexFormat::UChar2:
            case dawn::VertexFormat::UChar4:
            case dawn::VertexFormat::Char2:
            case dawn::VertexFormat::Char4:
            case dawn::VertexFormat::UChar2Norm:
            case dawn::VertexFormat::UChar4Norm:
            case dawn::VertexFormat::Char2Norm:
            case dawn::VertexFormat::Char4Norm:
                return sizeof(char);
            case dawn::VertexFormat::UShort2:
            case dawn::VertexFormat::UShort4:
            case dawn::VertexFormat::UShort2Norm:
            case dawn::VertexFormat::UShort4Norm:
            case dawn::VertexFormat::Short2:
            case dawn::VertexFormat::Short4:
            case dawn::VertexFormat::Short2Norm:
            case dawn::VertexFormat::Short4Norm:
            case dawn::VertexFormat::Half2:
            case dawn::VertexFormat::Half4:
                return sizeof(uint16_t);
            case dawn::VertexFormat::Float:
            case dawn::VertexFormat::Float2:
            case dawn::VertexFormat::Float3:
            case dawn::VertexFormat::Float4:
                return sizeof(float);
            case dawn::VertexFormat::UInt:
            case dawn::VertexFormat::UInt2:
            case dawn::VertexFormat::UInt3:
            case dawn::VertexFormat::UInt4:
            case dawn::VertexFormat::Int:
            case dawn::VertexFormat::Int2:
            case dawn::VertexFormat::Int3:
            case dawn::VertexFormat::Int4:
                return sizeof(int32_t);
            default:
                UNREACHABLE();
        }
    }

    size_t VertexFormatSize(dawn::VertexFormat format) {
        return VertexFormatNumComponents(format) * VertexFormatComponentSize(format);
    }

    // InputStateBase

    InputStateBase::InputStateBase(InputStateBuilder* builder) : ObjectBase(builder->GetDevice()) {
        mAttributesSetMask = builder->mAttributesSetMask;
        mAttributeInfos = builder->mAttributeInfos;
        mInputsSetMask = builder->mInputsSetMask;
        mInputInfos = builder->mInputInfos;
    }

    const std::bitset<kMaxVertexAttributes>& InputStateBase::GetAttributesSetMask() const {
        return mAttributesSetMask;
    }

    const VertexAttributeDescriptor& InputStateBase::GetAttribute(uint32_t location) const {
        ASSERT(mAttributesSetMask[location]);
        return mAttributeInfos[location];
    }

    const std::bitset<kMaxVertexInputs>& InputStateBase::GetInputsSetMask() const {
        return mInputsSetMask;
    }

    const VertexInputDescriptor& InputStateBase::GetInput(uint32_t slot) const {
        ASSERT(mInputsSetMask[slot]);
        return mInputInfos[slot];
    }

    // InputStateBuilder

    InputStateBuilder::InputStateBuilder(DeviceBase* device) : Builder(device) {
    }

    InputStateBase* InputStateBuilder::GetResultImpl() {
        for (uint32_t location = 0; location < kMaxVertexAttributes; ++location) {
            if (mAttributesSetMask[location] &&
                !mInputsSetMask[mAttributeInfos[location].inputSlot]) {
                HandleError("Attribute uses unset input");
                return nullptr;
            }
        }

        return GetDevice()->CreateInputState(this);
    }

    void InputStateBuilder::SetAttribute(const VertexAttributeDescriptor* attribute) {
        if (attribute->shaderLocation >= kMaxVertexAttributes) {
            HandleError("Setting attribute out of bounds");
            return;
        }
        if (attribute->inputSlot >= kMaxVertexInputs) {
            HandleError("Binding slot out of bounds");
            return;
        }
        if (GetDevice()->ConsumedError(ValidateVertexFormat(attribute->format))) {
            return;
        }
        // If attribute->offset is close to 0xFFFFFFFF, the validation below to add
        // attribute->offset and VertexFormatSize(attribute->format) might overflow on a
        // 32bit machine, then it can pass the validation incorrectly. We need to catch it.
        if (attribute->offset >= kMaxVertexAttributeEnd) {
            HandleError("Setting attribute offset out of bounds");
            return;
        }
        if (attribute->offset + VertexFormatSize(attribute->format) > kMaxVertexAttributeEnd) {
            HandleError("Setting attribute offset out of bounds");
            return;
        }
        if (mAttributesSetMask[attribute->shaderLocation]) {
            HandleError("Setting already set attribute");
            return;
        }

        mAttributesSetMask.set(attribute->shaderLocation);
        mAttributeInfos[attribute->shaderLocation] = *attribute;
    }

    void InputStateBuilder::SetInput(const VertexInputDescriptor* input) {
        if (input->inputSlot >= kMaxVertexInputs) {
            HandleError("Setting input out of bounds");
            return;
        }
        if (input->stride > kMaxVertexInputStride) {
            HandleError("Setting input stride out of bounds");
            return;
        }
        if (mInputsSetMask[input->inputSlot]) {
            HandleError("Setting already set input");
            return;
        }

        mInputsSetMask.set(input->inputSlot);
        mInputInfos[input->inputSlot] = *input;
    }

}  // namespace dawn_native
