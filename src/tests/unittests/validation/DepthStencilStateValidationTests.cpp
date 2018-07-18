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

#include "tests/unittests/validation/ValidationTest.h"

class DepthStencilStateValidationTest : public ValidationTest {
};

// Test cases where creation should succeed
TEST_F(DepthStencilStateValidationTest, CreationSuccess) {
    // Success for setting all properties
    {
        dawn::DepthStencilState ds = AssertWillBeSuccess(device.CreateDepthStencilStateBuilder())
            .SetDepthCompareFunction(dawn::CompareFunction::Less)
            .SetDepthWriteEnabled(true)
            .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Greater,
                dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
            .SetStencilMask(0x0, 0x1)
            .GetResult();
    }

    // Success for empty builder
    {
        dawn::DepthStencilState ds = AssertWillBeSuccess(device.CreateDepthStencilStateBuilder())
            .GetResult();
    }

    // Test success when setting stencil function on separate faces
    {
        dawn::DepthStencilState ds = AssertWillBeSuccess(device.CreateDepthStencilStateBuilder())
            .SetStencilFunction(dawn::Face::Front, dawn::CompareFunction::Less,
                dawn::StencilOperation::Replace, dawn::StencilOperation::Replace, dawn::StencilOperation::Replace)
            .SetStencilFunction(dawn::Face::Back, dawn::CompareFunction::Greater,
                dawn::StencilOperation::Replace, dawn::StencilOperation::Replace, dawn::StencilOperation::Replace)
            .GetResult();
    }
}

// Test creation failure when specifying properties multiple times
TEST_F(DepthStencilStateValidationTest, CreationDuplicates) {
    // Test failure when specifying depth write enabled multiple times
    {
        dawn::DepthStencilState ds = AssertWillBeError(device.CreateDepthStencilStateBuilder())
            .SetDepthWriteEnabled(true)
            .SetDepthWriteEnabled(false)
            .GetResult();
    }

    // Test failure when specifying depth compare function multiple times
    {
        dawn::DepthStencilState ds = AssertWillBeError(device.CreateDepthStencilStateBuilder())
            .SetDepthCompareFunction(dawn::CompareFunction::Less)
            .SetDepthCompareFunction(dawn::CompareFunction::Greater)
            .GetResult();
    }

    // Test failure when setting stencil mask  multiple times
    {
        dawn::DepthStencilState ds = AssertWillBeError(device.CreateDepthStencilStateBuilder())
            .SetStencilMask(0x00, 0x00)
            .SetStencilMask(0xff, 0xff)
            .GetResult();
    }

    // Test failure when directly setting stencil function on a face multiple times
    {
        dawn::DepthStencilState ds = AssertWillBeError(device.CreateDepthStencilStateBuilder())
            .SetStencilFunction(dawn::Face::Back, dawn::CompareFunction::Less,
                dawn::StencilOperation::Replace, dawn::StencilOperation::Replace, dawn::StencilOperation::Replace)
            .SetStencilFunction(dawn::Face::Back, dawn::CompareFunction::Greater,
                dawn::StencilOperation::Replace, dawn::StencilOperation::Replace, dawn::StencilOperation::Replace)
            .GetResult();
    }

    // Test failure when indirectly setting stencil function on a face multiple times
    {
        dawn::DepthStencilState ds = AssertWillBeError(device.CreateDepthStencilStateBuilder())
            .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Less,
                dawn::StencilOperation::Replace, dawn::StencilOperation::Replace, dawn::StencilOperation::Replace)
            .SetStencilFunction(dawn::Face::Back, dawn::CompareFunction::Greater,
                dawn::StencilOperation::Replace, dawn::StencilOperation::Replace, dawn::StencilOperation::Replace)
            .GetResult();
    }
}
