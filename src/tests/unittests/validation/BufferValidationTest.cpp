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

class BufferValidationTest : public ValidationTest {
};

TEST_F(BufferValidationTest, Creation) {
    // Success
    nxt::Buffer buf0 = AssertWillBeSuccess(device.CreateBufferBuilder())
        .SetSize(4)
        .SetAllowedUsage(nxt::BufferUsageBit::Uniform)
        .GetResult();

    // Test failure when specifying properties multiple times
    nxt::Buffer buf1 = AssertWillBeError(device.CreateBufferBuilder())
        .SetSize(4)
        .SetSize(3)
        .GetResult();

    // Test failure when properties are missing
    nxt::Buffer buf2 = AssertWillBeError(device.CreateBufferBuilder())
        .SetSize(4)
        .GetResult();
}
