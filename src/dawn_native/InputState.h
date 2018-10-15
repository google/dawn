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

#ifndef DAWNNATIVE_INPUTSTATE_H_
#define DAWNNATIVE_INPUTSTATE_H_

#include "common/Constants.h"
#include "dawn_native/Builder.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    size_t IndexFormatSize(dawn::IndexFormat format);
    uint32_t VertexFormatNumComponents(dawn::VertexFormat format);
    size_t VertexFormatComponentSize(dawn::VertexFormat format);
    size_t VertexFormatSize(dawn::VertexFormat format);

    class InputStateBase : public ObjectBase {
      public:
        InputStateBase(InputStateBuilder* builder);

        struct AttributeInfo {
            uint32_t bindingSlot;
            dawn::VertexFormat format;
            uint32_t offset;
        };

        struct InputInfo {
            uint32_t stride;
            dawn::InputStepMode stepMode;
        };

        const std::bitset<kMaxVertexAttributes>& GetAttributesSetMask() const;
        const AttributeInfo& GetAttribute(uint32_t location) const;
        const std::bitset<kMaxVertexInputs>& GetInputsSetMask() const;
        const InputInfo& GetInput(uint32_t slot) const;

      private:
        std::bitset<kMaxVertexAttributes> mAttributesSetMask;
        std::array<AttributeInfo, kMaxVertexAttributes> mAttributeInfos;
        std::bitset<kMaxVertexInputs> mInputsSetMask;
        std::array<InputInfo, kMaxVertexInputs> mInputInfos;
    };

    class InputStateBuilder : public Builder<InputStateBase> {
      public:
        InputStateBuilder(DeviceBase* device);

        // Dawn API
        void SetAttribute(uint32_t shaderLocation,
                          uint32_t bindingSlot,
                          dawn::VertexFormat format,
                          uint32_t offset);
        void SetInput(uint32_t bindingSlot, uint32_t stride, dawn::InputStepMode stepMode);

      private:
        friend class InputStateBase;

        InputStateBase* GetResultImpl() override;

        std::bitset<kMaxVertexAttributes> mAttributesSetMask;
        std::array<InputStateBase::AttributeInfo, kMaxVertexAttributes> mAttributeInfos;
        std::bitset<kMaxVertexInputs> mInputsSetMask;
        std::array<InputStateBase::InputInfo, kMaxVertexInputs> mInputInfos;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_INPUTSTATE_H_
