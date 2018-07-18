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

#include "backend/d3d12/InputStateD3D12.h"

#include "common/BitSetIterator.h"

namespace backend { namespace d3d12 {

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
            case nxt::VertexFormat::IntR32G32B32A32:
                return DXGI_FORMAT_R32G32B32A32_SINT;
            case nxt::VertexFormat::IntR32G32B32:
                return DXGI_FORMAT_R32G32B32_SINT;
            case nxt::VertexFormat::IntR32G32:
                return DXGI_FORMAT_R32G32_SINT;
            case nxt::VertexFormat::IntR32:
                return DXGI_FORMAT_R32_SINT;
            case nxt::VertexFormat::UshortR16G16B16A16:
                return DXGI_FORMAT_R16G16B16A16_UINT;
            case nxt::VertexFormat::UshortR16G16:
                return DXGI_FORMAT_R16G16_UINT;
            case nxt::VertexFormat::UnormR8G8B8A8:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case nxt::VertexFormat::UnormR8G8:
                return DXGI_FORMAT_R8G8_UNORM;
            default:
                UNREACHABLE();
        }
    }

    static D3D12_INPUT_CLASSIFICATION InputStepModeFunction(nxt::InputStepMode mode) {
        switch (mode) {
            case nxt::InputStepMode::Vertex:
                return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            case nxt::InputStepMode::Instance:
                return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            default:
                UNREACHABLE();
        }
    }

    InputState::InputState(Device* device, InputStateBuilder* builder)
        : InputStateBase(builder), mDevice(device) {
        const auto& attributesSetMask = GetAttributesSetMask();

        unsigned int count = 0;
        for (auto i : IterateBitSet(attributesSetMask)) {
            if (!attributesSetMask[i]) {
                continue;
            }

            D3D12_INPUT_ELEMENT_DESC& inputElementDescriptor = mInputElementDescriptors[count++];

            const AttributeInfo& attribute = GetAttribute(i);

            // If the HLSL semantic is TEXCOORDN the SemanticName should be "TEXCOORD" and the
            // SemanticIndex N
            inputElementDescriptor.SemanticName = "TEXCOORD";
            inputElementDescriptor.SemanticIndex = static_cast<uint32_t>(i);
            inputElementDescriptor.Format = VertexFormatType(attribute.format);
            inputElementDescriptor.InputSlot = attribute.bindingSlot;

            const InputInfo& input = GetInput(attribute.bindingSlot);

            inputElementDescriptor.AlignedByteOffset = attribute.offset;
            inputElementDescriptor.InputSlotClass = InputStepModeFunction(input.stepMode);
            if (inputElementDescriptor.InputSlotClass ==
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) {
                inputElementDescriptor.InstanceDataStepRate = 0;
            } else {
                inputElementDescriptor.InstanceDataStepRate = 1;
            }
        }

        mInputLayoutDescriptor.pInputElementDescs = mInputElementDescriptors;
        mInputLayoutDescriptor.NumElements = count;
    }

    const D3D12_INPUT_LAYOUT_DESC& InputState::GetD3D12InputLayoutDescriptor() const {
        return mInputLayoutDescriptor;
    }

}}  // namespace backend::d3d12
