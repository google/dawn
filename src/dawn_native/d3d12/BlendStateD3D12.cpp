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

#include "dawn_native/d3d12/BlendStateD3D12.h"

#include "common/Assert.h"
#include "dawn_native/d3d12/DeviceD3D12.h"

namespace dawn_native { namespace d3d12 {

    namespace {
        D3D12_BLEND D3D12Blend(dawn::BlendFactor factor) {
            switch (factor) {
                case dawn::BlendFactor::Zero:
                    return D3D12_BLEND_ZERO;
                case dawn::BlendFactor::One:
                    return D3D12_BLEND_ONE;
                case dawn::BlendFactor::SrcColor:
                    return D3D12_BLEND_SRC_COLOR;
                case dawn::BlendFactor::OneMinusSrcColor:
                    return D3D12_BLEND_INV_SRC_COLOR;
                case dawn::BlendFactor::SrcAlpha:
                    return D3D12_BLEND_SRC_ALPHA;
                case dawn::BlendFactor::OneMinusSrcAlpha:
                    return D3D12_BLEND_INV_SRC_ALPHA;
                case dawn::BlendFactor::DstColor:
                    return D3D12_BLEND_DEST_COLOR;
                case dawn::BlendFactor::OneMinusDstColor:
                    return D3D12_BLEND_INV_DEST_COLOR;
                case dawn::BlendFactor::DstAlpha:
                    return D3D12_BLEND_DEST_ALPHA;
                case dawn::BlendFactor::OneMinusDstAlpha:
                    return D3D12_BLEND_INV_DEST_ALPHA;
                case dawn::BlendFactor::SrcAlphaSaturated:
                    return D3D12_BLEND_SRC_ALPHA_SAT;
                case dawn::BlendFactor::BlendColor:
                    return D3D12_BLEND_BLEND_FACTOR;
                case dawn::BlendFactor::OneMinusBlendColor:
                    return D3D12_BLEND_INV_BLEND_FACTOR;
                default:
                    UNREACHABLE();
            }
        }

        D3D12_BLEND_OP D3D12BlendOperation(dawn::BlendOperation operation) {
            switch (operation) {
                case dawn::BlendOperation::Add:
                    return D3D12_BLEND_OP_ADD;
                case dawn::BlendOperation::Subtract:
                    return D3D12_BLEND_OP_SUBTRACT;
                case dawn::BlendOperation::ReverseSubtract:
                    return D3D12_BLEND_OP_REV_SUBTRACT;
                case dawn::BlendOperation::Min:
                    return D3D12_BLEND_OP_MIN;
                case dawn::BlendOperation::Max:
                    return D3D12_BLEND_OP_MAX;
                default:
                    UNREACHABLE();
            }
        }

        uint8_t D3D12RenderTargetWriteMask(dawn::ColorWriteMask colorWriteMask) {
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(dawn::ColorWriteMask::Red) ==
                              D3D12_COLOR_WRITE_ENABLE_RED,
                          "ColorWriteMask values must match");
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(dawn::ColorWriteMask::Green) ==
                              D3D12_COLOR_WRITE_ENABLE_GREEN,
                          "ColorWriteMask values must match");
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(dawn::ColorWriteMask::Blue) ==
                              D3D12_COLOR_WRITE_ENABLE_BLUE,
                          "ColorWriteMask values must match");
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(dawn::ColorWriteMask::Alpha) ==
                              D3D12_COLOR_WRITE_ENABLE_ALPHA,
                          "ColorWriteMask values must match");
            return static_cast<uint8_t>(colorWriteMask);
        }
    }  // namespace

    BlendState::BlendState(BlendStateBuilder* builder) : BlendStateBase(builder) {
        auto& info = GetBlendInfo();
        mBlendDesc.BlendEnable = info.blendEnabled;
        mBlendDesc.SrcBlend = D3D12Blend(info.colorBlend.srcFactor);
        mBlendDesc.DestBlend = D3D12Blend(info.colorBlend.dstFactor);
        mBlendDesc.BlendOp = D3D12BlendOperation(info.colorBlend.operation);
        mBlendDesc.SrcBlendAlpha = D3D12Blend(info.alphaBlend.srcFactor);
        mBlendDesc.DestBlendAlpha = D3D12Blend(info.alphaBlend.dstFactor);
        mBlendDesc.BlendOpAlpha = D3D12BlendOperation(info.alphaBlend.operation);
        mBlendDesc.RenderTargetWriteMask = D3D12RenderTargetWriteMask(info.colorWriteMask);
        mBlendDesc.LogicOpEnable = false;
        mBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    }

    const D3D12_RENDER_TARGET_BLEND_DESC& BlendState::GetD3D12BlendDesc() const {
        return mBlendDesc;
    }

}}  // namespace dawn_native::d3d12
