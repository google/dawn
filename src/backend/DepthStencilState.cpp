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

#include "backend/DepthStencilState.h"

#include "backend/Device.h"

namespace backend {

    // DepthStencilStateBase

    DepthStencilStateBase::DepthStencilStateBase(DepthStencilStateBuilder* builder)
        : mDepthInfo(builder->mDepthInfo), mStencilInfo(builder->mStencilInfo) {
    }

    bool DepthStencilStateBase::StencilTestEnabled() const {
        return mStencilInfo.back.compareFunction != nxt::CompareFunction::Always ||
            mStencilInfo.back.stencilFail != nxt::StencilOperation::Keep ||
            mStencilInfo.back.depthFail != nxt::StencilOperation::Keep ||
            mStencilInfo.back.depthStencilPass != nxt::StencilOperation::Keep ||
            mStencilInfo.front.compareFunction != nxt::CompareFunction::Always ||
            mStencilInfo.front.stencilFail != nxt::StencilOperation::Keep ||
            mStencilInfo.front.depthFail != nxt::StencilOperation::Keep ||
            mStencilInfo.front.depthStencilPass != nxt::StencilOperation::Keep;
    }

    const DepthStencilStateBase::DepthInfo& DepthStencilStateBase::GetDepth() const {
        return mDepthInfo;
    }

    const DepthStencilStateBase::StencilInfo& DepthStencilStateBase::GetStencil() const {
        return mStencilInfo;
    }

    // DepthStencilStateBuilder

    enum DepthStencilStateSetProperties {
        DEPTH_STENCIL_STATE_PROPERTY_DEPTH_COMPARE_FUNCTION = 0x1,
        DEPTH_STENCIL_STATE_PROPERTY_DEPTH_WRITE_ENABLED = 0x2,
        DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_FUNCTION = 0x4,
        DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_FUNCTION = 0x08,
        DEPTH_STENCIL_STATE_PROPERTY_STENCIL_MASK = 0x10,
    };

    DepthStencilStateBuilder::DepthStencilStateBuilder(DeviceBase* device) : Builder(device) {
    }

    DepthStencilStateBase* DepthStencilStateBuilder::GetResultImpl() {
        return mDevice->CreateDepthStencilState(this);
    }

    void DepthStencilStateBuilder::SetDepthCompareFunction(nxt::CompareFunction depthCompareFunction) {
        if ((mPropertiesSet & DEPTH_STENCIL_STATE_PROPERTY_DEPTH_COMPARE_FUNCTION) != 0) {
            HandleError("Depth compare property set multiple times");
            return;
        }

        mPropertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_DEPTH_COMPARE_FUNCTION;

        mDepthInfo.compareFunction = depthCompareFunction;
    }

    void DepthStencilStateBuilder::SetDepthWriteEnabled(bool enabled) {
        if ((mPropertiesSet & DEPTH_STENCIL_STATE_PROPERTY_DEPTH_WRITE_ENABLED) != 0) {
            HandleError("Depth write enabled property set multiple times");
            return;
        }

        mPropertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_DEPTH_WRITE_ENABLED;

        mDepthInfo.depthWriteEnabled = enabled;
    }

    void DepthStencilStateBuilder::SetStencilFunction(nxt::Face face, nxt::CompareFunction stencilCompareFunction,
            nxt::StencilOperation stencilFail, nxt::StencilOperation depthFail, nxt::StencilOperation depthStencilPass) {\
        if (face == nxt::Face::None) {
            HandleError("Can't set stencil function of None face");
            return;
        }

        if (face & nxt::Face::Back) {
            if ((mPropertiesSet & DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_FUNCTION) != 0) {
                HandleError("Stencil back function property set multiple times");
                return;
            }

            mPropertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_STENCIL_BACK_FUNCTION;

            mStencilInfo.back.compareFunction = stencilCompareFunction;
            mStencilInfo.back.stencilFail = stencilFail;
            mStencilInfo.back.depthFail = depthFail;
            mStencilInfo.back.depthStencilPass = depthStencilPass;
        }
        if (face & nxt::Face::Front) {
            if ((mPropertiesSet & DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_FUNCTION) != 0) {
                HandleError("Stencil front function property set multiple times");
                return;
            }

            mPropertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_STENCIL_FRONT_FUNCTION;

            mStencilInfo.front.compareFunction = stencilCompareFunction;
            mStencilInfo.front.stencilFail = stencilFail;
            mStencilInfo.front.depthFail = depthFail;
            mStencilInfo.front.depthStencilPass = depthStencilPass;
        }
    }

    void DepthStencilStateBuilder::SetStencilMask(uint32_t readMask, uint32_t writeMask) {
         if ((mPropertiesSet & DEPTH_STENCIL_STATE_PROPERTY_STENCIL_MASK) != 0) {
            HandleError("Stencilmask property set multiple times");
            return;
        }

        mPropertiesSet |= DEPTH_STENCIL_STATE_PROPERTY_STENCIL_MASK;
        mStencilInfo.readMask = readMask;
        mStencilInfo.writeMask = writeMask;
    }

}
