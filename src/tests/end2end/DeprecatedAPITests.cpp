// Copyright 2020 The Dawn Authors
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

// This file contains test for deprecated parts of Dawn's API while following WebGPU's evolution.
// It contains test for the "old" behavior that will be deleted once users are migrated, tests that
// a deprecation warning is emitted when the "old" behavior is used, and tests that an error is
// emitted when both the old and the new behavior are used (when applicable).

#include "tests/DawnTest.h"

class DeprecationTests : public DawnTest {};

#define EXPECT_DEPRECATION_WARNING(statement)                                    \
    do {                                                                         \
        if (UsesWire()) {                                                        \
            statement;                                                           \
        } else {                                                                 \
            size_t warningsBefore =                                              \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            statement;                                                           \
            size_t warningsAfter =                                               \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            EXPECT_EQ(warningsAfter, warningsBefore + 1);                        \
        }                                                                        \
    } while (0)

// Tests for Device::CreateQueue -> Device::GetDefaultQueue.

// Test that using CreateQueue produces a deprecation warning
TEST_P(DeprecationTests, CreateQueueIsDeprecated) {
    EXPECT_DEPRECATION_WARNING(device.CreateQueue());
}

// Test that queues created from CreateQueue can be used for things
TEST_P(DeprecationTests, CreateQueueReturnsFunctionalQueue) {
    wgpu::Queue q;
    EXPECT_DEPRECATION_WARNING(q = device.CreateQueue());

    q.Submit(0, nullptr);
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
