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
            depthInfo(builder->depthInfo), backStencilInfo(builder->backStencilInfo),
            frontStencilInfo(builder->frontStencilInfo) {
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

    const DepthStencilStateBase::StencilInfo& DepthStencilStateBase::GetBackStencil() const {
        return backStencilInfo;
    }

    const DepthStencilStateBase::StencilInfo& DepthStencilStateBase::GetFrontStencil() const {
        return frontStencilInfo;
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
            backStencilInfo.stencilFail = stencilFail;
            backStencilInfo.depthFail = stencilFail;
            backStencilInfo.stencilPass = stencilPass;
        }
        if (face & nxt::Face::Front) {
            frontStencilInfo.stencilFail = stencilFail;
            frontStencilInfo.depthFail = stencilFail;
            frontStencilInfo.stencilPass = stencilPass;
        }
    }

    void DepthStencilStateBuilder::SetStencilCompareFunction(nxt::Face face, nxt::CompareFunction stencilCompareFunction) {
        if (face & nxt::Face::Back) {
            backStencilInfo.compareFunction = stencilCompareFunction;
        }
        if (face & nxt::Face::Front) {
            frontStencilInfo.compareFunction = stencilCompareFunction;
        }
    }

    void DepthStencilStateBuilder::SetStencilMask(nxt::Face face, uint32_t readMask, uint32_t writeMask) {
        if (face & nxt::Face::Back) {
            backStencilInfo.readMask = readMask;
            backStencilInfo.writeMask = writeMask;
        }
        if (face & nxt::Face::Front) {
            frontStencilInfo.readMask = readMask;
            frontStencilInfo.writeMask = writeMask;
        }
    }

}
