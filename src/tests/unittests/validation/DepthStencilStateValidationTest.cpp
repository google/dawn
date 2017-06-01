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

#include "ValidationTest.h"

class DepthStencilStateValidationTest : public ValidationTest {
};

TEST_F(DepthStencilStateValidationTest, Creation) {
    // Success
    nxt::DepthStencilState buf0 = AssertWillBeSuccess(device.CreateDepthStencilStateBuilder())
        .SetDepthCompareFunction(nxt::CompareFunction::Less)
        .SetDepthWriteEnabled(true)
        .SetStencilFunction(nxt::Face::Both, nxt::CompareFunction::Greater,
            nxt::StencilOperation::Keep, nxt::StencilOperation::Keep, nxt::StencilOperation::Replace)
        .SetStencilMask(nxt::Face::Both, 0x1)
        .GetResult();

    // Success for empty builder
    nxt::DepthStencilState buf1 = AssertWillBeSuccess(device.CreateDepthStencilStateBuilder())
        .GetResult();

    // Test failure when specifying properties multiple times
    nxt::DepthStencilState buf2 = AssertWillBeError(device.CreateDepthStencilStateBuilder())
        .SetDepthWriteEnabled(true)
        .SetDepthWriteEnabled(false)
        .GetResult();

    nxt::DepthStencilState buf3 = AssertWillBeError(device.CreateDepthStencilStateBuilder())
        .SetDepthCompareFunction(nxt::CompareFunction::Less)
        .SetDepthCompareFunction(nxt::CompareFunction::Greater)
        .GetResult();

    // Test success when setting properties on separate faces
    nxt::DepthStencilState buf4 = AssertWillBeSuccess(device.CreateDepthStencilStateBuilder())
        .SetStencilMask(nxt::Face::Front, 0x00)
        .SetStencilMask(nxt::Face::Back, 0xff)
        .GetResult();

    nxt::DepthStencilState buf5 = AssertWillBeSuccess(device.CreateDepthStencilStateBuilder())
        .SetStencilFunction(nxt::Face::Front, nxt::CompareFunction::Less,
            nxt::StencilOperation::Replace, nxt::StencilOperation::Replace, nxt::StencilOperation::Replace)
        .SetStencilFunction(nxt::Face::Back, nxt::CompareFunction::Greater,
            nxt::StencilOperation::Replace, nxt::StencilOperation::Replace, nxt::StencilOperation::Replace)
        .GetResult();

    // Test failure when setting properties on a face multiple times
    nxt::DepthStencilState buf6 = AssertWillBeError(device.CreateDepthStencilStateBuilder())
        .SetStencilMask(nxt::Face::Back, 0x00)
        .SetStencilMask(nxt::Face::Back, 0xff)
        .GetResult();

    nxt::DepthStencilState buf7 = AssertWillBeError(device.CreateDepthStencilStateBuilder())
        .SetStencilFunction(nxt::Face::Back, nxt::CompareFunction::Less,
            nxt::StencilOperation::Replace, nxt::StencilOperation::Replace, nxt::StencilOperation::Replace)
        .SetStencilFunction(nxt::Face::Back, nxt::CompareFunction::Greater,
            nxt::StencilOperation::Replace, nxt::StencilOperation::Replace, nxt::StencilOperation::Replace)
        .GetResult();

    nxt::DepthStencilState buf8 = AssertWillBeError(device.CreateDepthStencilStateBuilder())
        .SetStencilMask(nxt::Face::Both, 0x00)
        .SetStencilMask(nxt::Face::Back, 0xff)
        .GetResult();

    nxt::DepthStencilState buf9 = AssertWillBeError(device.CreateDepthStencilStateBuilder())
        .SetStencilFunction(nxt::Face::Both, nxt::CompareFunction::Less,
            nxt::StencilOperation::Replace, nxt::StencilOperation::Replace, nxt::StencilOperation::Replace)
        .SetStencilFunction(nxt::Face::Back, nxt::CompareFunction::Greater,
            nxt::StencilOperation::Replace, nxt::StencilOperation::Replace, nxt::StencilOperation::Replace)
        .GetResult();
}
