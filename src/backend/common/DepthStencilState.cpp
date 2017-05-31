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
        : depthEnabled(builder->depthEnabled), stencilEnabled(builder->stencilEnabled),
            depthInfo(builder->depthInfo),
            stencilInfos { builder->stencilInfos[0], builder->stencilInfos[1] } {
    }

    bool DepthStencilStateBase::DepthIsEnabled() const {
        return depthEnabled;
    }

    bool DepthStencilStateBase::StencilIsEnabled() const {
        return stencilEnabled;
    }

    const DepthStencilStateBase::DepthInfo& DepthStencilStateBase::GetDepth() const {
        return depthInfo;
    }

    const DepthStencilStateBase::StencilInfo& DepthStencilStateBase::GetStencil(nxt::Face face) const {
        switch (face) {
            case nxt::Face::Back:
                return stencilInfos[0];
            case nxt::Face::Front:
                return stencilInfos[1];
            default:
                ASSERT(false);
        }
    }


    // DepthStencilStateBuilder

    DepthStencilStateBuilder::DepthStencilStateBuilder(DeviceBase* device) : Builder(device) {
    }

    DepthStencilStateBase* DepthStencilStateBuilder::GetResultImpl() {
        return device->CreateDepthStencilState(this);
    }

    void DepthStencilStateBuilder::SetDepthEnabled(bool depthEnabled) {
        this->depthEnabled = depthEnabled;
    }

    void DepthStencilStateBuilder::SetDepthCompareFunction(nxt::CompareFunction depthCompareFunction) {
        depthInfo.compareFunction = depthCompareFunction;
    }

    void DepthStencilStateBuilder::SetDepthWrite(nxt::DepthWriteMode depthWriteMode) {
        depthInfo.depthWriteMode = depthWriteMode;
    }

    void DepthStencilStateBuilder::SetStencilEnabled(bool stencilEnabled) {
        this->stencilEnabled = stencilEnabled;
    }

    void DepthStencilStateBuilder::SetStencilOperation(nxt::Face face, nxt::StencilOperation stencilFail,
            nxt::StencilOperation depthFail, nxt::StencilOperation stencilPass) {
        if (face & nxt::Face::Back) {
            auto& stencilInfo = stencilInfos[0];
            stencilInfo.stencilFail = stencilFail;
            stencilInfo.depthFail = stencilFail;
            stencilInfo.stencilPass = stencilPass;
        }
        if (face & nxt::Face::Front) {
            auto& stencilInfo = stencilInfos[1];
            stencilInfo.stencilFail = stencilFail;
            stencilInfo.depthFail = stencilFail;
            stencilInfo.stencilPass = stencilPass;
        }
    }

    void DepthStencilStateBuilder::SetStencilCompareFunction(nxt::Face face, nxt::CompareFunction stencilCompareFunction) {
        if (face & nxt::Face::Back) {
            auto& stencilInfo = stencilInfos[0];
            stencilInfo.compareFunction = stencilCompareFunction;
        }
        if (face & nxt::Face::Front) {
            auto& stencilInfo = stencilInfos[1];
            stencilInfo.compareFunction = stencilCompareFunction;
        }
    }

    void DepthStencilStateBuilder::SetStencilMask(nxt::Face face, uint32_t readMask, uint32_t writeMask) {
        if (face & nxt::Face::Back) {
            auto& stencilInfo = stencilInfos[0];
            stencilInfo.readMask = readMask;
            stencilInfo.writeMask = writeMask;
        }
        if (face & nxt::Face::Front) {
            auto& stencilInfo = stencilInfos[1];
            stencilInfo.readMask = readMask;
            stencilInfo.writeMask = writeMask;
        }
    }

}
