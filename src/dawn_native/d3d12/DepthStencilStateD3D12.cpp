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

#include "dawn_native/d3d12/DepthStencilStateD3D12.h"

#include "common/BitSetIterator.h"

namespace dawn_native { namespace d3d12 {

    namespace {
        D3D12_STENCIL_OP StencilOp(dawn::StencilOperation op) {
            switch (op) {
                case dawn::StencilOperation::Keep:
                    return D3D12_STENCIL_OP_KEEP;
                case dawn::StencilOperation::Zero:
                    return D3D12_STENCIL_OP_ZERO;
                case dawn::StencilOperation::Replace:
                    return D3D12_STENCIL_OP_REPLACE;
                case dawn::StencilOperation::IncrementClamp:
                    return D3D12_STENCIL_OP_INCR_SAT;
                case dawn::StencilOperation::DecrementClamp:
                    return D3D12_STENCIL_OP_DECR_SAT;
                case dawn::StencilOperation::Invert:
                    return D3D12_STENCIL_OP_INVERT;
                case dawn::StencilOperation::IncrementWrap:
                    return D3D12_STENCIL_OP_INCR;
                case dawn::StencilOperation::DecrementWrap:
                    return D3D12_STENCIL_OP_DECR;
                default:
                    UNREACHABLE();
            }
        }

        D3D12_COMPARISON_FUNC ComparisonFunc(dawn::CompareFunction func) {
            switch (func) {
                case dawn::CompareFunction::Always:
                    return D3D12_COMPARISON_FUNC_ALWAYS;
                case dawn::CompareFunction::Equal:
                    return D3D12_COMPARISON_FUNC_EQUAL;
                case dawn::CompareFunction::Greater:
                    return D3D12_COMPARISON_FUNC_GREATER;
                case dawn::CompareFunction::GreaterEqual:
                    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                case dawn::CompareFunction::Less:
                    return D3D12_COMPARISON_FUNC_LESS;
                case dawn::CompareFunction::LessEqual:
                    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case dawn::CompareFunction::Never:
                    return D3D12_COMPARISON_FUNC_NEVER;
                case dawn::CompareFunction::NotEqual:
                    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                default:
                    UNREACHABLE();
            }
        }

        D3D12_DEPTH_STENCILOP_DESC StencilOpDesc(const StencilStateFaceDescriptor descriptor) {
            D3D12_DEPTH_STENCILOP_DESC desc;

            desc.StencilFailOp = StencilOp(descriptor.stencilFailOp);
            desc.StencilDepthFailOp = StencilOp(descriptor.depthFailOp);
            desc.StencilPassOp = StencilOp(descriptor.passOp);
            desc.StencilFunc = ComparisonFunc(descriptor.compare);

            return desc;
        }
    }  // anonymous namespace

    DepthStencilState::DepthStencilState(DepthStencilStateBuilder* builder)
        : DepthStencilStateBase(builder) {
        mDepthStencilDescriptor.DepthEnable = TRUE;
        mDepthStencilDescriptor.DepthWriteMask =
            GetDepth().depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        mDepthStencilDescriptor.DepthFunc = ComparisonFunc(GetDepth().compareFunction);

        mDepthStencilDescriptor.StencilEnable = StencilTestEnabled() ? TRUE : FALSE;
        mDepthStencilDescriptor.StencilReadMask = static_cast<UINT8>(GetStencil().readMask);
        mDepthStencilDescriptor.StencilWriteMask = static_cast<UINT8>(GetStencil().writeMask);

        mDepthStencilDescriptor.FrontFace = StencilOpDesc(GetStencil().front);
        mDepthStencilDescriptor.BackFace = StencilOpDesc(GetStencil().back);
    }

    const D3D12_DEPTH_STENCIL_DESC& DepthStencilState::GetD3D12DepthStencilDescriptor() const {
        return mDepthStencilDescriptor;
    }

}}  // namespace dawn_native::d3d12
