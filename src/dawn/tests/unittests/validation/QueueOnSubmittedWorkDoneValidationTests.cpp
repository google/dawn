// Copyright 2021 The Dawn Authors
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

#include <memory>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "gmock/gmock.h"

class MockQueueWorkDoneCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUQueueWorkDoneStatus status, void* userdata));
};

static std::unique_ptr<MockQueueWorkDoneCallback> mockQueueWorkDoneCallback;
static void ToMockQueueWorkDone(WGPUQueueWorkDoneStatus status, void* userdata) {
    mockQueueWorkDoneCallback->Call(status, userdata);
}

class QueueOnSubmittedWorkDoneValidationTests : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();
        mockQueueWorkDoneCallback = std::make_unique<MockQueueWorkDoneCallback>();
    }

    void TearDown() override {
        mockQueueWorkDoneCallback = nullptr;
        ValidationTest::TearDown();
    }
};

// Test that OnSubmittedWorkDone can be called as soon as the queue is created.
TEST_F(QueueOnSubmittedWorkDoneValidationTests, CallBeforeSubmits) {
    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, this)).Times(1);
    device.GetQueue().OnSubmittedWorkDone(0u, ToMockQueueWorkDone, this);

    WaitForAllOperations(device);
}
