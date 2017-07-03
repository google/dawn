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

#include "InputStateD3D12.h"

namespace backend {
namespace d3d12 {

    static DXGI_FORMAT VertexFormatType(nxt::VertexFormat format) {
        switch (format) {
            case nxt::VertexFormat::FloatR32G32B32A32:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case nxt::VertexFormat::FloatR32G32B32:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case nxt::VertexFormat::FloatR32G32:
                return DXGI_FORMAT_R32G32_FLOAT;
            case nxt::VertexFormat::FloatR32:
                return DXGI_FORMAT_R32_FLOAT;
        }
    }

    static D3D12_INPUT_CLASSIFICATION InputStepModeFunction(nxt::InputStepMode mode) {
        switch (mode) {
            case nxt::InputStepMode::Vertex:
                return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            case nxt::InputStepMode::Instance:
                return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        }
    }

    InputState::InputState(Device* device, InputStateBuilder* builder)
        : InputStateBase(builder), device(device) {

        const auto& attributesSetMask = GetAttributesSetMask();

        size_t count = 0;
        for (size_t i = 0; i < attributesSetMask.size(); ++i) {
            if (!attributesSetMask[i]) {
                continue;
            }

            D3D12_INPUT_ELEMENT_DESC& inputElementDescriptor = inputElementDescriptors[count++];

            const AttributeInfo& attribute = GetAttribute(i);

            // If the HLSL semantic is TEXCOORDN the SemanticName should be "TEXCOORD" and the SemanticIndex N
            inputElementDescriptor.SemanticName = "TEXCOORD";
            inputElementDescriptor.SemanticIndex = i;
            inputElementDescriptor.Format = VertexFormatType(attribute.format);
            inputElementDescriptor.InputSlot = attribute.bindingSlot;

            const InputInfo& input = GetInput(attribute.bindingSlot);

            inputElementDescriptor.AlignedByteOffset = attribute.offset;
            inputElementDescriptor.InputSlotClass = InputStepModeFunction(input.stepMode);
            if (inputElementDescriptor.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) {
                inputElementDescriptor.InstanceDataStepRate = 0;
            } else {
                inputElementDescriptor.InstanceDataStepRate = 1;
            }
        }

        inputLayoutDescriptor.pInputElementDescs = inputElementDescriptors;
        inputLayoutDescriptor.NumElements = count;

    }

    const D3D12_INPUT_LAYOUT_DESC& InputState::GetD3D12InputLayoutDescriptor() const {
        return inputLayoutDescriptor;
    }

}
}
