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

class BlendStateValidationTest : public ValidationTest {
};

// Test cases where creation should succeed
TEST_F(BlendStateValidationTest, CreationSuccess) {
    // Success for setting all properties
    {
        nxt::BlendState state = AssertWillBeSuccess(device.CreateBlendStateBuilder())
            .SetBlendEnabled(true)
            .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetColorWriteMask(nxt::ColorWriteMask::Red)
            .GetResult();
    }

    // Success for empty builder
    {
        nxt::BlendState state = AssertWillBeSuccess(device.CreateBlendStateBuilder())
            .GetResult();
    }
}

// Test creation failure when specifying properties multiple times
TEST_F(BlendStateValidationTest, CreationDuplicates) {
    // Test failure when specifying blend enabled multiple times
    {
        nxt::BlendState state = AssertWillBeError(device.CreateBlendStateBuilder())
            .SetBlendEnabled(true)
            .SetBlendEnabled(false)
            .GetResult();
    }

    // Test failure when specifying alpha blend multiple times
    {
        nxt::BlendState state = AssertWillBeError(device.CreateBlendStateBuilder())
            .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::Zero, nxt::BlendFactor::Zero)
            .GetResult();
    }

    // Test failure when specifying color blend multiple times
    {
        nxt::BlendState state = AssertWillBeError(device.CreateBlendStateBuilder())
            .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::Zero, nxt::BlendFactor::Zero)
            .GetResult();
    }

    // Test failure when specifying color write mask multiple times
    {
        nxt::BlendState state = AssertWillBeError(device.CreateBlendStateBuilder())
            .SetColorWriteMask(nxt::ColorWriteMask::Red)
            .SetColorWriteMask(nxt::ColorWriteMask::Green)
            .GetResult();
    }

}
