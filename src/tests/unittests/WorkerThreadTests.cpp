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
// WorkerThreadTests:
//     Simple tests for the worker thread class.

#include <gtest/gtest.h>

#include <list>
#include <memory>
#include <mutex>
#include <queue>

#include "common/NonCopyable.h"
#include "dawn_platform/DawnPlatform.h"

namespace {

    struct SimpleTaskResult {
        uint32_t id;
        bool isDone = false;
    };

    // A thread-safe queue that stores the task results.
    class ConcurrentTaskResultQueue : public NonCopyable {
      public:
        void TaskCompleted(const SimpleTaskResult& result) {
            ASSERT_TRUE(result.isDone);

            std::lock_guard<std::mutex> lock(mMutex);
            mTaskResultQueue.push(result);
        }

        std::vector<SimpleTaskResult> GetAndPopCompletedTasks() {
            std::lock_guard<std::mutex> lock(mMutex);

            std::vector<SimpleTaskResult> results;
            while (!mTaskResultQueue.empty()) {
                results.push_back(mTaskResultQueue.front());
                mTaskResultQueue.pop();
            }
            return results;
        }

      private:
        std::mutex mMutex;
        std::queue<SimpleTaskResult> mTaskResultQueue;
    };

    // A simple task that will be executed asynchronously with pool->PostWorkerTask().
    class SimpleTask : public NonCopyable {
      public:
        SimpleTask(uint32_t id, ConcurrentTaskResultQueue* resultQueue)
            : mId(id), mResultQueue(resultQueue) {
        }

      private:
        friend class Tracker;

        static void DoTaskOnWorkerTaskPool(void* task) {
            SimpleTask* simpleTaskPtr = static_cast<SimpleTask*>(task);
            simpleTaskPtr->doTask();
        }

        void doTask() {
            SimpleTaskResult result;
            result.id = mId;
            result.isDone = true;
            mResultQueue->TaskCompleted(result);
        }

        uint32_t mId;
        ConcurrentTaskResultQueue* mResultQueue;
    };

    // A simple implementation of task tracker which is only called in main thread and not
    // thread-safe.
    class Tracker : public NonCopyable {
      public:
        explicit Tracker(dawn_platform::WorkerTaskPool* pool) : mPool(pool) {
        }

        void StartNewTask(uint32_t taskId) {
            mTasksInFlight.emplace_back(this, mPool, taskId);
        }

        uint64_t GetTasksInFlightCount() {
            return mTasksInFlight.size();
        }

        void WaitAll() {
            for (auto iter = mTasksInFlight.begin(); iter != mTasksInFlight.end(); ++iter) {
                iter->waitableEvent->Wait();
            }
        }

        // In Tick() we clean up all the completed tasks and consume all the available results.
        void Tick() {
            auto iter = mTasksInFlight.begin();
            while (iter != mTasksInFlight.end()) {
                if (iter->waitableEvent->IsComplete()) {
                    iter = mTasksInFlight.erase(iter);
                } else {
                    ++iter;
                }
            }

            const std::vector<SimpleTaskResult>& results =
                mCompletedTaskResultQueue.GetAndPopCompletedTasks();
            for (const SimpleTaskResult& result : results) {
                EXPECT_TRUE(result.isDone);
            }
        }

      private:
        SimpleTask* CreateSimpleTask(uint32_t taskId) {
            return new SimpleTask(taskId, &mCompletedTaskResultQueue);
        }

        struct WaitableTask {
            WaitableTask(Tracker* tracker, dawn_platform::WorkerTaskPool* pool, uint32_t taskId) {
                task.reset(tracker->CreateSimpleTask(taskId));
                waitableEvent =
                    pool->PostWorkerTask(SimpleTask::DoTaskOnWorkerTaskPool, task.get());
            }

            std::unique_ptr<SimpleTask> task;
            std::unique_ptr<dawn_platform::WaitableEvent> waitableEvent;
        };

        dawn_platform::WorkerTaskPool* mPool;

        std::list<WaitableTask> mTasksInFlight;
        ConcurrentTaskResultQueue mCompletedTaskResultQueue;
    };

}  // anonymous namespace

class WorkerThreadTest : public testing::Test {};

// Emulate the basic usage of worker thread pool in CreateReady*Pipeline().
TEST_F(WorkerThreadTest, Basic) {
    dawn_platform::Platform platform;
    std::unique_ptr<dawn_platform::WorkerTaskPool> pool = platform.CreateWorkerTaskPool();
    Tracker tracker(pool.get());

    constexpr uint32_t kTaskCount = 4;
    for (uint32_t i = 0; i < kTaskCount; ++i) {
        tracker.StartNewTask(i);
    }
    EXPECT_EQ(kTaskCount, tracker.GetTasksInFlightCount());

    // Wait for the completion of all the tasks.
    tracker.WaitAll();

    tracker.Tick();
    EXPECT_EQ(0u, tracker.GetTasksInFlightCount());
}
