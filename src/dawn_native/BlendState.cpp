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

#include "dawn_native/BlendState.h"

#include "dawn_native/Device.h"

namespace dawn_native {

    // BlendStateBase

    BlendStateBase::BlendStateBase(BlendStateBuilder* builder)
        : ObjectBase(builder->GetDevice()), mBlendInfo(builder->mBlendInfo) {
    }

    const BlendStateBase::BlendInfo& BlendStateBase::GetBlendInfo() const {
        return mBlendInfo;
    }

    // BlendStateBuilder

    enum BlendStateSetProperties {
        BLEND_STATE_PROPERTY_BLEND_ENABLED = 0x1,
        BLEND_STATE_PROPERTY_ALPHA_BLEND = 0x2,
        BLEND_STATE_PROPERTY_COLOR_BLEND = 0x4,
        BLEND_STATE_PROPERTY_COLOR_WRITE_MASK = 0x08,
    };

    BlendStateBuilder::BlendStateBuilder(DeviceBase* device) : Builder(device) {
    }

    BlendStateBase* BlendStateBuilder::GetResultImpl() {
        return GetDevice()->CreateBlendState(this);
    }

    void BlendStateBuilder::SetBlendEnabled(bool blendEnabled) {
        if ((mPropertiesSet & BLEND_STATE_PROPERTY_BLEND_ENABLED) != 0) {
            HandleError("Blend enabled property set multiple times");
            return;
        }

        mPropertiesSet |= BLEND_STATE_PROPERTY_BLEND_ENABLED;

        mBlendInfo.blendEnabled = blendEnabled;
    }

    void BlendStateBuilder::SetAlphaBlend(dawn::BlendOperation blendOperation,
                                          dawn::BlendFactor srcFactor,
                                          dawn::BlendFactor dstFactor) {
        if ((mPropertiesSet & BLEND_STATE_PROPERTY_ALPHA_BLEND) != 0) {
            HandleError("Alpha blend property set multiple times");
            return;
        }

        mPropertiesSet |= BLEND_STATE_PROPERTY_ALPHA_BLEND;

        mBlendInfo.alphaBlend = {blendOperation, srcFactor, dstFactor};
    }

    void BlendStateBuilder::SetColorBlend(dawn::BlendOperation blendOperation,
                                          dawn::BlendFactor srcFactor,
                                          dawn::BlendFactor dstFactor) {
        if ((mPropertiesSet & BLEND_STATE_PROPERTY_COLOR_BLEND) != 0) {
            HandleError("Color blend property set multiple times");
            return;
        }

        mPropertiesSet |= BLEND_STATE_PROPERTY_COLOR_BLEND;

        mBlendInfo.colorBlend = {blendOperation, srcFactor, dstFactor};
    }

    void BlendStateBuilder::SetColorWriteMask(dawn::ColorWriteMask colorWriteMask) {
        if ((mPropertiesSet & BLEND_STATE_PROPERTY_COLOR_WRITE_MASK) != 0) {
            HandleError("Color write mask property set multiple times");
            return;
        }

        mPropertiesSet |= BLEND_STATE_PROPERTY_COLOR_WRITE_MASK;

        mBlendInfo.colorWriteMask = colorWriteMask;
    }
}  // namespace dawn_native
