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

#include "DepthStencilState.h"

#include "Device.h"

namespace backend {

    // DepthStencilStateBase

    DepthStencilStateBase::DepthStencilStateBase(DepthStencilStateBuilder* builder)
        : depthInfo(builder->depthInfo), stencilInfo(builder->stencilInfo) {
    }

    bool DepthStencilStateBase::DepthTestEnabled() const {
        return depthInfo.compareFunction != nxt::CompareFunction::Always;
    }

    bool DepthStencilStateBase::StencilTestEnabled() const {
        return (stencilInfo.back.compareFunction != nxt::CompareFunction::Always ||
        stencilInfo.back.stencilFail != nxt::StencilOperation::Keep ||
        stencilInfo.back.depthFail != nxt::StencilOperation::Keep ||
        stencilInfo.back.depthStencilPass != nxt::StencilOperation::Keep ||
        stencilInfo.front.compareFunction != nxt::CompareFunction::Always ||
        stencilInfo.front.stencilFail != nxt::StencilOperation::Keep ||
        stencilInfo.front.depthFail != nxt::StencilOperation::Keep ||
        stencilInfo.front.depthStencilPass != nxt::StencilOperation::Keep);
    }

    const DepthStencilStateBase::DepthInfo& DepthStencilStateBase::GetDepth() const {
        return depthInfo;
    }

    const DepthStencilStateBase::StencilInfo& DepthStencilStateBase::GetStencil() const {
        return stencilInfo;
    }

    // DepthStencilStateBuilder

    enum DepthStencilStateSetProperties {
        DEPTH_STENCIL_STATE_PROPERTY_DEPTH_COMPARE_FUNCTION = 0x1,
        DEPTH_STENCIL_STATE_PROPERTY_DEPTH_WRITE_ENABLED = 0x2,
        DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_FUNCTION = 0x4,
        DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_MASK = 0x8,
        DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_FUNCTION = 0x10,
        DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_MASK = 0x20,
    };

    DepthStencilStateBuilder::DepthStencilStateBuilder(DeviceBase* device) : Builder(device) {
    }

    DepthStencilStateBase* DepthStencilStateBuilder::GetResultImpl() {
        return device->CreateDepthStencilState(this);
    }

    void DepthStencilStateBuilder::SetDepthCompareFunction(nxt::CompareFunction depthCompareFunction) {
        if ((propertiesSet & DEPTH_STENCIL_STATE_PROPERTY_DEPTH_COMPARE_FUNCTION) != 0) {
            HandleError("Depth compare property set multiple times");
            return;
        }

        propertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_DEPTH_COMPARE_FUNCTION;

        depthInfo.compareFunction = depthCompareFunction;
    }

    void DepthStencilStateBuilder::SetDepthWriteEnabled(bool enabled) {
        if ((propertiesSet & DEPTH_STENCIL_STATE_PROPERTY_DEPTH_WRITE_ENABLED) != 0) {
            HandleError("Depth write enabled property set multiple times");
            return;
        }

        propertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_DEPTH_WRITE_ENABLED;

        depthInfo.depthWriteEnabled = enabled;
    }

    void DepthStencilStateBuilder::SetStencilFunction(nxt::Face face, nxt::CompareFunction stencilCompareFunction,
            nxt::StencilOperation stencilFail, nxt::StencilOperation depthFail, nxt::StencilOperation depthStencilPass) {\
        if (face == nxt::Face::None) {
            HandleError("Can't set stencil function of None face");
            return;
        }

        if (face & nxt::Face::Back) {
            if ((propertiesSet & DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_FUNCTION) != 0) {
                HandleError("Stencil back function property set multiple times");
                return;
            }

            propertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_FUNCTION;

            stencilInfo.back.compareFunction = stencilCompareFunction;
            stencilInfo.back.stencilFail = stencilFail;
            stencilInfo.back.depthFail = depthFail;
            stencilInfo.back.depthStencilPass = depthStencilPass;
        }
        if (face & nxt::Face::Front) {
            if ((propertiesSet & DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_FUNCTION) != 0) {
                HandleError("Stencil front function property set multiple times");
                return;
            }

            propertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_FUNCTION;

            stencilInfo.front.compareFunction = stencilCompareFunction;
            stencilInfo.front.stencilFail = stencilFail;
            stencilInfo.front.depthFail = depthFail;
            stencilInfo.front.depthStencilPass = depthStencilPass;
        }
    }

    void DepthStencilStateBuilder::SetStencilMask(nxt::Face face, uint32_t mask) {
        if (face == nxt::Face::None) {
            HandleError("Can't set stencil mask of None face");
            return;
        }

        if (face & nxt::Face::Back) {
            if ((propertiesSet & DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_MASK) != 0) {
                HandleError("Stencil back mask property set multiple times");
                return;
            }

            propertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_MASK;

            stencilInfo.back.mask = mask;
        }
        if (face & nxt::Face::Front) {
            if ((propertiesSet & DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_MASK) != 0) {
                HandleError("Stencil front mask property set multiple times");
                return;
            }

            propertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_MASK;

            stencilInfo.front.mask = mask;
        }
    }

}
