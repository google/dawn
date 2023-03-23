// Copyright 2023 The Dawn Authors
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

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <utility>

#include "dawn/native/AsyncTask.h"
#include "mocks/DawnMockTest.h"

namespace dawn::native {
namespace {
using ::testing::Test;

class DeviceAsyncTaskTests : public DawnMockTest {};

// Test that a long async task's execution won't extend to after the device is dropped.
// Device dropping should wait for that task to finish.
TEST_F(DeviceAsyncTaskTests, LongAsyncTaskFinishesBeforeDeviceIsDropped) {
    std::atomic_bool done(false);

    // Simulate that an async task would take a long time to finish.
    dawn::native::AsyncTask asyncTask([&done] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        done = true;
    });

    mDeviceMock->GetAsyncTaskManager()->PostTask(std::move(asyncTask));
    device = nullptr;
    // Dropping the device should force the async task to finish.
    EXPECT_TRUE(done.load());
}

}  // namespace
}  // namespace dawn::native
