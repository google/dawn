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

    uint32_t VertexFormatNumComponents(dawn::VertexFormat format) {
        switch (format) {
            case dawn::VertexFormat::FloatR32G32B32A32:
            case dawn::VertexFormat::IntR32G32B32A32:
            case dawn::VertexFormat::UshortR16G16B16A16:
            case dawn::VertexFormat::UnormR8G8B8A8:
                return 4;
            case dawn::VertexFormat::FloatR32G32B32:
            case dawn::VertexFormat::IntR32G32B32:
                return 3;
            case dawn::VertexFormat::FloatR32G32:
            case dawn::VertexFormat::IntR32G32:
            case dawn::VertexFormat::UshortR16G16:
            case dawn::VertexFormat::UnormR8G8:
                return 2;
            case dawn::VertexFormat::FloatR32:
            case dawn::VertexFormat::IntR32:
                return 1;
            default:
                UNREACHABLE();
        }
    }

    size_t VertexFormatComponentSize(dawn::VertexFormat format) {
        switch (format) {
            case dawn::VertexFormat::FloatR32G32B32A32:
            case dawn::VertexFormat::FloatR32G32B32:
            case dawn::VertexFormat::FloatR32G32:
            case dawn::VertexFormat::FloatR32:
                return sizeof(float);
            case dawn::VertexFormat::IntR32G32B32A32:
            case dawn::VertexFormat::IntR32G32B32:
            case dawn::VertexFormat::IntR32G32:
            case dawn::VertexFormat::IntR32:
                return sizeof(int32_t);
            case dawn::VertexFormat::UshortR16G16B16A16:
            case dawn::VertexFormat::UshortR16G16:
                return sizeof(uint16_t);
            case dawn::VertexFormat::UnormR8G8B8A8:
            case dawn::VertexFormat::UnormR8G8:
                return sizeof(uint8_t);
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

    const InputStateBase::AttributeInfo& InputStateBase::GetAttribute(uint32_t location) const {
        ASSERT(mAttributesSetMask[location]);
        return mAttributeInfos[location];
    }

    const std::bitset<kMaxVertexInputs>& InputStateBase::GetInputsSetMask() const {
        return mInputsSetMask;
    }

    const InputStateBase::InputInfo& InputStateBase::GetInput(uint32_t slot) const {
        ASSERT(mInputsSetMask[slot]);
        return mInputInfos[slot];
    }

    // InputStateBuilder

    InputStateBuilder::InputStateBuilder(DeviceBase* device) : Builder(device) {
    }

    InputStateBase* InputStateBuilder::GetResultImpl() {
        for (uint32_t location = 0; location < kMaxVertexAttributes; ++location) {
            if (mAttributesSetMask[location] &&
                !mInputsSetMask[mAttributeInfos[location].bindingSlot]) {
                HandleError("Attribute uses unset input");
                return nullptr;
            }
        }

        return GetDevice()->CreateInputState(this);
    }

    void InputStateBuilder::SetAttribute(uint32_t shaderLocation,
                                         uint32_t bindingSlot,
                                         dawn::VertexFormat format,
                                         uint32_t offset) {
        if (shaderLocation >= kMaxVertexAttributes) {
            HandleError("Setting attribute out of bounds");
            return;
        }
        if (bindingSlot >= kMaxVertexInputs) {
            HandleError("Binding slot out of bounds");
            return;
        }
        if (mAttributesSetMask[shaderLocation]) {
            HandleError("Setting already set attribute");
            return;
        }

        mAttributesSetMask.set(shaderLocation);
        auto& info = mAttributeInfos[shaderLocation];
        info.bindingSlot = bindingSlot;
        info.format = format;
        info.offset = offset;
    }

    void InputStateBuilder::SetInput(uint32_t bindingSlot,
                                     uint32_t stride,
                                     dawn::InputStepMode stepMode) {
        if (bindingSlot >= kMaxVertexInputs) {
            HandleError("Setting input out of bounds");
            return;
        }
        if (mInputsSetMask[bindingSlot]) {
            HandleError("Setting already set input");
            return;
        }

        mInputsSetMask.set(bindingSlot);
        auto& info = mInputInfos[bindingSlot];
        info.stride = stride;
        info.stepMode = stepMode;
    }

}  // namespace dawn_native
