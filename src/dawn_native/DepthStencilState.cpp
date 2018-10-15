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

#include "dawn_native/DepthStencilState.h"

#include "dawn_native/Device.h"

namespace dawn_native {

    // DepthStencilStateBase

    DepthStencilStateBase::DepthStencilStateBase(DepthStencilStateBuilder* builder)
        : ObjectBase(builder->GetDevice()),
          mDepthInfo(builder->mDepthInfo),
          mStencilInfo(builder->mStencilInfo) {
    }

    bool DepthStencilStateBase::StencilTestEnabled() const {
        return mStencilInfo.back.compareFunction != dawn::CompareFunction::Always ||
               mStencilInfo.back.stencilFail != dawn::StencilOperation::Keep ||
               mStencilInfo.back.depthFail != dawn::StencilOperation::Keep ||
               mStencilInfo.back.depthStencilPass != dawn::StencilOperation::Keep ||
               mStencilInfo.front.compareFunction != dawn::CompareFunction::Always ||
               mStencilInfo.front.stencilFail != dawn::StencilOperation::Keep ||
               mStencilInfo.front.depthFail != dawn::StencilOperation::Keep ||
               mStencilInfo.front.depthStencilPass != dawn::StencilOperation::Keep;
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
        return GetDevice()->CreateDepthStencilState(this);
    }

    void DepthStencilStateBuilder::SetDepthCompareFunction(
        dawn::CompareFunction depthCompareFunction) {
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

    void DepthStencilStateBuilder::SetStencilFunction(dawn::Face face,
                                                      dawn::CompareFunction stencilCompareFunction,
                                                      dawn::StencilOperation stencilFail,
                                                      dawn::StencilOperation depthFail,
                                                      dawn::StencilOperation depthStencilPass) {
        if (face == dawn::Face::None) {
            HandleError("Can't set stencil function of None face");
            return;
        }

        if (face & dawn::Face::Back) {
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
        if (face & dawn::Face::Front) {
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

}  // namespace dawn_native
