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

#include "backend/d3d12/DepthStencilStateD3D12.h"

#include "common/BitSetIterator.h"

namespace backend {
namespace d3d12 {

    static D3D12_STENCIL_OP StencilOp(nxt::StencilOperation op) {
        switch (op) {
            case nxt::StencilOperation::Keep:
                return D3D12_STENCIL_OP_KEEP;
            case nxt::StencilOperation::Zero:
                return D3D12_STENCIL_OP_ZERO;
            case nxt::StencilOperation::Replace:
                return D3D12_STENCIL_OP_REPLACE;
            case nxt::StencilOperation::IncrementClamp:
                return D3D12_STENCIL_OP_INCR_SAT;
            case nxt::StencilOperation::DecrementClamp:
                return D3D12_STENCIL_OP_DECR_SAT;
            case nxt::StencilOperation::Invert:
                return D3D12_STENCIL_OP_INVERT;
            case nxt::StencilOperation::IncrementWrap:
                return D3D12_STENCIL_OP_INCR;
            case nxt::StencilOperation::DecrementWrap:
                return D3D12_STENCIL_OP_DECR;
            default:
                UNREACHABLE();
        }
    }

    static D3D12_COMPARISON_FUNC ComparisonFunc(nxt::CompareFunction func) {
        switch (func)
        {
            case nxt::CompareFunction::Always:
                return D3D12_COMPARISON_FUNC_ALWAYS;
            case nxt::CompareFunction::Equal:
                return D3D12_COMPARISON_FUNC_EQUAL;
            case nxt::CompareFunction::Greater:
                return D3D12_COMPARISON_FUNC_GREATER;
            case nxt::CompareFunction::GreaterEqual:
                return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case nxt::CompareFunction::Less:
                return D3D12_COMPARISON_FUNC_LESS;
            case nxt::CompareFunction::LessEqual:
                return D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case nxt::CompareFunction::Never:
                return D3D12_COMPARISON_FUNC_NEVER;
            case nxt::CompareFunction::NotEqual:
                return D3D12_COMPARISON_FUNC_NOT_EQUAL;
            default:
                UNREACHABLE();
        }
    }

    static D3D12_DEPTH_STENCILOP_DESC StencilOpDesc(backend::DepthStencilStateBase::StencilFaceInfo faceInfo) {
        D3D12_DEPTH_STENCILOP_DESC desc;

        desc.StencilFailOp = StencilOp(faceInfo.stencilFail);
        desc.StencilDepthFailOp = StencilOp(faceInfo.depthFail);
        desc.StencilPassOp = StencilOp(faceInfo.depthStencilPass);
        desc.StencilFunc = ComparisonFunc(faceInfo.compareFunction);

        return desc;
    }

    DepthStencilState::DepthStencilState(Device* device, DepthStencilStateBuilder* builder)
        : DepthStencilStateBase(builder), device(device) {

        // If you have anything other than Never, then enable depth testing
        depthStencilDescriptor.DepthEnable = TRUE;
        depthStencilDescriptor.DepthWriteMask = GetDepth().depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencilDescriptor.DepthFunc = ComparisonFunc(GetDepth().compareFunction);

        depthStencilDescriptor.StencilEnable = StencilTestEnabled() ? TRUE : FALSE;
        depthStencilDescriptor.StencilReadMask = static_cast<UINT8>(GetStencil().readMask);
        depthStencilDescriptor.StencilWriteMask = static_cast<UINT8>(GetStencil().writeMask);

        depthStencilDescriptor.FrontFace = StencilOpDesc(GetStencil().front);
        depthStencilDescriptor.BackFace = StencilOpDesc(GetStencil().back);
	}

    const D3D12_DEPTH_STENCIL_DESC& DepthStencilState::GetD3D12DepthStencilDescriptor() const {
        return depthStencilDescriptor;
    }

}
}
