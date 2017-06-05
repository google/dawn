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

#include "PipelineLayoutD3D12.h"

#include "D3D12Backend.h"

using Microsoft::WRL::ComPtr;

namespace backend {
namespace d3d12 {

    PipelineLayout::PipelineLayout(Device* device, PipelineLayoutBuilder* builder)
        : PipelineLayoutBase(builder), device(device) {

        // Create an empty root signature.
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor;
        rootSignatureDescriptor.NumParameters = 0;
        rootSignatureDescriptor.pParameters = nullptr;
        rootSignatureDescriptor.NumStaticSamplers = 0;
        rootSignatureDescriptor.pStaticSamplers = nullptr;
        rootSignatureDescriptor.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ASSERT_SUCCESS(D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ASSERT_SUCCESS(device->GetD3D12Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
    }

    ComPtr<ID3D12RootSignature> PipelineLayout::GetRootSignature() {
        return rootSignature;
    }

}
}
