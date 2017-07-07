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

#include "tests/unittests/validation/ValidationTest.h"

#include <gmock/gmock.h>

using namespace testing;

class UsageValidationTest : public ValidationTest {
    protected:
        nxt::Queue queue;

    private:
        void SetUp() override {
            ValidationTest::SetUp();
            queue = device.CreateQueueBuilder().GetResult();
        }
};

// Test that command buffer submit changes buffer usage
TEST_F(UsageValidationTest, UsageAfterCommandBuffer) {
    // TODO(kainino@chromium.org): This needs to be tested on every backend.
    // Should we make an end2end test that tests this as well?

    nxt::Buffer buf = device.CreateBufferBuilder()
        .SetSize(4)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::Vertex)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();

    uint32_t foo = 0;
    buf.SetSubData(0, 1, &foo);

    buf.TransitionUsage(nxt::BufferUsageBit::Vertex);
    ASSERT_DEVICE_ERROR(buf.SetSubData(0, 1, &foo));

    nxt::CommandBuffer cmdbuf = device.CreateCommandBufferBuilder()
        .TransitionBufferUsage(buf, nxt::BufferUsageBit::TransferDst)
        .GetResult();
    queue.Submit(1, &cmdbuf);
    // buf should be in TransferDst usage

    buf.SetSubData(0, 1, &foo);
}
