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

#include "dawn_native/d3d12/InputStateD3D12.h"

#include "common/BitSetIterator.h"

namespace dawn_native { namespace d3d12 {

    static DXGI_FORMAT VertexFormatType(dawn::VertexFormat format) {
        switch (format) {
            case dawn::VertexFormat::UChar2:
                return DXGI_FORMAT_R8G8_UINT;
            case dawn::VertexFormat::UChar4:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            case dawn::VertexFormat::Char2:
                return DXGI_FORMAT_R8G8_SINT;
            case dawn::VertexFormat::Char4:
                return DXGI_FORMAT_R8G8B8A8_SINT;
            case dawn::VertexFormat::UChar2Norm:
                return DXGI_FORMAT_R8G8_UNORM;
            case dawn::VertexFormat::UChar4Norm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case dawn::VertexFormat::Char2Norm:
                return DXGI_FORMAT_R8G8_SNORM;
            case dawn::VertexFormat::Char4Norm:
                return DXGI_FORMAT_R8G8B8A8_SNORM;
            case dawn::VertexFormat::UShort2:
                return DXGI_FORMAT_R16G16_UINT;
            case dawn::VertexFormat::UShort4:
                return DXGI_FORMAT_R16G16B16A16_UINT;
            case dawn::VertexFormat::Short2:
                return DXGI_FORMAT_R16G16_SINT;
            case dawn::VertexFormat::Short4:
                return DXGI_FORMAT_R16G16B16A16_SINT;
            case dawn::VertexFormat::UShort2Norm:
                return DXGI_FORMAT_R16G16_UNORM;
            case dawn::VertexFormat::UShort4Norm:
                return DXGI_FORMAT_R16G16B16A16_UNORM;
            case dawn::VertexFormat::Short2Norm:
                return DXGI_FORMAT_R16G16_SNORM;
            case dawn::VertexFormat::Short4Norm:
                return DXGI_FORMAT_R16G16B16A16_SNORM;
            case dawn::VertexFormat::Half2:
                return DXGI_FORMAT_R16G16_FLOAT;
            case dawn::VertexFormat::Half4:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case dawn::VertexFormat::Float:
                return DXGI_FORMAT_R32_FLOAT;
            case dawn::VertexFormat::Float2:
                return DXGI_FORMAT_R32G32_FLOAT;
            case dawn::VertexFormat::Float3:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case dawn::VertexFormat::Float4:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case dawn::VertexFormat::UInt:
                return DXGI_FORMAT_R32_UINT;
            case dawn::VertexFormat::UInt2:
                return DXGI_FORMAT_R32G32_UINT;
            case dawn::VertexFormat::UInt3:
                return DXGI_FORMAT_R32G32B32_UINT;
            case dawn::VertexFormat::UInt4:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            case dawn::VertexFormat::Int:
                return DXGI_FORMAT_R32_SINT;
            case dawn::VertexFormat::Int2:
                return DXGI_FORMAT_R32G32_SINT;
            case dawn::VertexFormat::Int3:
                return DXGI_FORMAT_R32G32B32_SINT;
            case dawn::VertexFormat::Int4:
                return DXGI_FORMAT_R32G32B32A32_SINT;
            default:
                UNREACHABLE();
        }
    }

    static D3D12_INPUT_CLASSIFICATION InputStepModeFunction(dawn::InputStepMode mode) {
        switch (mode) {
            case dawn::InputStepMode::Vertex:
                return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            case dawn::InputStepMode::Instance:
                return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            default:
                UNREACHABLE();
        }
    }

    InputState::InputState(InputStateBuilder* builder) : InputStateBase(builder) {
        const auto& attributesSetMask = GetAttributesSetMask();

        unsigned int count = 0;
        for (auto i : IterateBitSet(attributesSetMask)) {
            if (!attributesSetMask[i]) {
                continue;
            }

            D3D12_INPUT_ELEMENT_DESC& inputElementDescriptor = mInputElementDescriptors[count++];

            const VertexAttributeDescriptor& attribute = GetAttribute(i);

            // If the HLSL semantic is TEXCOORDN the SemanticName should be "TEXCOORD" and the
            // SemanticIndex N
            inputElementDescriptor.SemanticName = "TEXCOORD";
            inputElementDescriptor.SemanticIndex = static_cast<uint32_t>(i);
            inputElementDescriptor.Format = VertexFormatType(attribute.format);
            inputElementDescriptor.InputSlot = attribute.inputSlot;

            const VertexInputDescriptor& input = GetInput(attribute.inputSlot);

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

}}  // namespace dawn_native::d3d12
