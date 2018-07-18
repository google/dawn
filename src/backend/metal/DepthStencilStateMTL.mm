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

#include "backend/metal/DepthStencilStateMTL.h"

#include "backend/metal/DeviceMTL.h"

namespace backend { namespace metal {

    namespace {
        MTLCompareFunction MetalDepthStencilCompareFunction(nxt::CompareFunction compareFunction) {
            switch (compareFunction) {
                case nxt::CompareFunction::Never:
                    return MTLCompareFunctionNever;
                case nxt::CompareFunction::Less:
                    return MTLCompareFunctionLess;
                case nxt::CompareFunction::LessEqual:
                    return MTLCompareFunctionLessEqual;
                case nxt::CompareFunction::Greater:
                    return MTLCompareFunctionGreater;
                case nxt::CompareFunction::GreaterEqual:
                    return MTLCompareFunctionGreaterEqual;
                case nxt::CompareFunction::NotEqual:
                    return MTLCompareFunctionNotEqual;
                case nxt::CompareFunction::Equal:
                    return MTLCompareFunctionEqual;
                case nxt::CompareFunction::Always:
                    return MTLCompareFunctionAlways;
            }
        }

        MTLStencilOperation MetalStencilOperation(nxt::StencilOperation stencilOperation) {
            switch (stencilOperation) {
                case nxt::StencilOperation::Keep:
                    return MTLStencilOperationKeep;
                case nxt::StencilOperation::Zero:
                    return MTLStencilOperationZero;
                case nxt::StencilOperation::Replace:
                    return MTLStencilOperationReplace;
                case nxt::StencilOperation::Invert:
                    return MTLStencilOperationInvert;
                case nxt::StencilOperation::IncrementClamp:
                    return MTLStencilOperationIncrementClamp;
                case nxt::StencilOperation::DecrementClamp:
                    return MTLStencilOperationDecrementClamp;
                case nxt::StencilOperation::IncrementWrap:
                    return MTLStencilOperationIncrementWrap;
                case nxt::StencilOperation::DecrementWrap:
                    return MTLStencilOperationDecrementWrap;
            }
        }
    }

    DepthStencilState::DepthStencilState(DepthStencilStateBuilder* builder)
        : DepthStencilStateBase(builder) {
        MTLDepthStencilDescriptor* mtlDepthStencilDescriptor = [MTLDepthStencilDescriptor new];

        auto& depth = GetDepth();
        mtlDepthStencilDescriptor.depthCompareFunction =
            MetalDepthStencilCompareFunction(depth.compareFunction);
        mtlDepthStencilDescriptor.depthWriteEnabled = depth.depthWriteEnabled;

        auto& stencil = GetStencil();
        if (StencilTestEnabled()) {
            MTLStencilDescriptor* backFaceStencil = [MTLStencilDescriptor new];
            MTLStencilDescriptor* frontFaceStencil = [MTLStencilDescriptor new];

            backFaceStencil.stencilCompareFunction =
                MetalDepthStencilCompareFunction(stencil.back.compareFunction);
            backFaceStencil.stencilFailureOperation =
                MetalStencilOperation(stencil.back.stencilFail);
            backFaceStencil.depthFailureOperation = MetalStencilOperation(stencil.back.depthFail);
            backFaceStencil.depthStencilPassOperation =
                MetalStencilOperation(stencil.back.depthStencilPass);
            backFaceStencil.readMask = stencil.readMask;
            backFaceStencil.writeMask = stencil.writeMask;

            frontFaceStencil.stencilCompareFunction =
                MetalDepthStencilCompareFunction(stencil.front.compareFunction);
            frontFaceStencil.stencilFailureOperation =
                MetalStencilOperation(stencil.front.stencilFail);
            frontFaceStencil.depthFailureOperation = MetalStencilOperation(stencil.front.depthFail);
            frontFaceStencil.depthStencilPassOperation =
                MetalStencilOperation(stencil.front.depthStencilPass);
            frontFaceStencil.readMask = stencil.readMask;
            frontFaceStencil.writeMask = stencil.writeMask;

            mtlDepthStencilDescriptor.backFaceStencil = backFaceStencil;
            mtlDepthStencilDescriptor.frontFaceStencil = frontFaceStencil;
            [backFaceStencil release];
            [frontFaceStencil release];
        }

        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();
        mMtlDepthStencilState =
            [mtlDevice newDepthStencilStateWithDescriptor:mtlDepthStencilDescriptor];
        [mtlDepthStencilDescriptor release];
    }

    DepthStencilState::~DepthStencilState() {
        [mMtlDepthStencilState release];
        mMtlDepthStencilState = nil;
    }

    id<MTLDepthStencilState> DepthStencilState::GetMTLDepthStencilState() {
        return mMtlDepthStencilState;
    }

}}  // namespace backend::metal
