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

//
// AsyncTaskTests:
//     Simple tests for dawn::native::AsyncTask and dawn::native::AsnycTaskManager.

#include <memory>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

#include "dawn/common/NonCopyable.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/platform/DawnPlatform.h"
#include "gtest/gtest.h"

namespace {

struct SimpleTaskResult {
    uint32_t id;
};

// A thread-safe queue that stores the task results.
class ConcurrentTaskResultQueue : public NonCopyable {
  public:
    void AddResult(std::unique_ptr<SimpleTaskResult> result) {
        std::lock_guard<std::mutex> lock(mMutex);
        mTaskResults.push_back(std::move(result));
    }

    std::vector<std::unique_ptr<SimpleTaskResult>> GetAllResults() {
        std::vector<std::unique_ptr<SimpleTaskResult>> outputResults;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            outputResults.swap(mTaskResults);
        }
        return outputResults;
    }

  private:
    std::mutex mMutex;
    std::vector<std::unique_ptr<SimpleTaskResult>> mTaskResults;
};

void DoTask(ConcurrentTaskResultQueue* resultQueue, uint32_t id) {
    std::unique_ptr<SimpleTaskResult> result = std::make_unique<SimpleTaskResult>();
    result->id = id;
    resultQueue->AddResult(std::move(result));
}

}  // anonymous namespace

class AsyncTaskTest : public testing::Test {};

// Emulate the basic usage of worker thread pool in Create*PipelineAsync().
TEST_F(AsyncTaskTest, Basic) {
    dawn::platform::Platform platform;
    std::unique_ptr<dawn::platform::WorkerTaskPool> pool = platform.CreateWorkerTaskPool();

    dawn::native::AsyncTaskManager taskManager(pool.get());
    ConcurrentTaskResultQueue taskResultQueue;

    constexpr size_t kTaskCount = 4u;
    std::set<uint32_t> idset;
    for (uint32_t i = 0; i < kTaskCount; ++i) {
        dawn::native::AsyncTask asyncTask([&taskResultQueue, i] { DoTask(&taskResultQueue, i); });
        taskManager.PostTask(std::move(asyncTask));
        idset.insert(i);
    }

    taskManager.WaitAllPendingTasks();

    std::vector<std::unique_ptr<SimpleTaskResult>> results = taskResultQueue.GetAllResults();
    ASSERT_EQ(kTaskCount, results.size());
    for (std::unique_ptr<SimpleTaskResult>& result : results) {
        idset.erase(result->id);
    }
    ASSERT_TRUE(idset.empty());
}
