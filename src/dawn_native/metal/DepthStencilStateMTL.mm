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

#include "dawn_native/metal/DepthStencilStateMTL.h"

#include "dawn_native/metal/DeviceMTL.h"

namespace dawn_native { namespace metal {

    namespace {
        MTLCompareFunction MetalDepthStencilCompareFunction(dawn::CompareFunction compareFunction) {
            switch (compareFunction) {
                case dawn::CompareFunction::Never:
                    return MTLCompareFunctionNever;
                case dawn::CompareFunction::Less:
                    return MTLCompareFunctionLess;
                case dawn::CompareFunction::LessEqual:
                    return MTLCompareFunctionLessEqual;
                case dawn::CompareFunction::Greater:
                    return MTLCompareFunctionGreater;
                case dawn::CompareFunction::GreaterEqual:
                    return MTLCompareFunctionGreaterEqual;
                case dawn::CompareFunction::NotEqual:
                    return MTLCompareFunctionNotEqual;
                case dawn::CompareFunction::Equal:
                    return MTLCompareFunctionEqual;
                case dawn::CompareFunction::Always:
                    return MTLCompareFunctionAlways;
            }
        }

        MTLStencilOperation MetalStencilOperation(dawn::StencilOperation stencilOperation) {
            switch (stencilOperation) {
                case dawn::StencilOperation::Keep:
                    return MTLStencilOperationKeep;
                case dawn::StencilOperation::Zero:
                    return MTLStencilOperationZero;
                case dawn::StencilOperation::Replace:
                    return MTLStencilOperationReplace;
                case dawn::StencilOperation::Invert:
                    return MTLStencilOperationInvert;
                case dawn::StencilOperation::IncrementClamp:
                    return MTLStencilOperationIncrementClamp;
                case dawn::StencilOperation::DecrementClamp:
                    return MTLStencilOperationDecrementClamp;
                case dawn::StencilOperation::IncrementWrap:
                    return MTLStencilOperationIncrementWrap;
                case dawn::StencilOperation::DecrementWrap:
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
                MetalDepthStencilCompareFunction(stencil.back.compare);
            backFaceStencil.stencilFailureOperation =
                MetalStencilOperation(stencil.back.stencilFailOp);
            backFaceStencil.depthFailureOperation = MetalStencilOperation(stencil.back.depthFailOp);
            backFaceStencil.depthStencilPassOperation = MetalStencilOperation(stencil.back.passOp);
            backFaceStencil.readMask = stencil.readMask;
            backFaceStencil.writeMask = stencil.writeMask;

            frontFaceStencil.stencilCompareFunction =
                MetalDepthStencilCompareFunction(stencil.front.compare);
            frontFaceStencil.stencilFailureOperation =
                MetalStencilOperation(stencil.front.stencilFailOp);
            frontFaceStencil.depthFailureOperation =
                MetalStencilOperation(stencil.front.depthFailOp);
            frontFaceStencil.depthStencilPassOperation =
                MetalStencilOperation(stencil.front.passOp);
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

}}  // namespace dawn_native::metal
