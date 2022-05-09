// Copyright 2022 The Dawn Authors
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

#include "dawn/native/AsyncTask.h"

#include <utility>

#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

AsyncTaskManager::AsyncTaskManager(dawn::platform::WorkerTaskPool* workerTaskPool)
    : mWorkerTaskPool(workerTaskPool) {}

void AsyncTaskManager::PostTask(AsyncTask asyncTask) {
    // If these allocations becomes expensive, we can slab-allocate tasks.
    Ref<WaitableTask> waitableTask = AcquireRef(new WaitableTask());
    waitableTask->taskManager = this;
    waitableTask->asyncTask = std::move(asyncTask);

    {
        // We insert new waitableTask objects into mPendingTasks in main thread (PostTask()),
        // and we may remove waitableTask objects from mPendingTasks in either main thread
        // (WaitAllPendingTasks()) or sub-thread (TaskCompleted), so mPendingTasks should be
        // protected by a mutex.
        std::lock_guard<std::mutex> lock(mPendingTasksMutex);
        mPendingTasks.emplace(waitableTask.Get(), waitableTask);
    }

    // Ref the task since it is accessed inside the worker function.
    // The worker function will acquire and release the task upon completion.
    waitableTask->Reference();
    waitableTask->waitableEvent =
        mWorkerTaskPool->PostWorkerTask(DoWaitableTask, waitableTask.Get());
}

void AsyncTaskManager::HandleTaskCompletion(WaitableTask* task) {
    std::lock_guard<std::mutex> lock(mPendingTasksMutex);
    auto iter = mPendingTasks.find(task);
    if (iter != mPendingTasks.end()) {
        mPendingTasks.erase(iter);
    }
}

void AsyncTaskManager::WaitAllPendingTasks() {
    std::unordered_map<WaitableTask*, Ref<WaitableTask>> allPendingTasks;

    {
        std::lock_guard<std::mutex> lock(mPendingTasksMutex);
        allPendingTasks.swap(mPendingTasks);
    }

    for (auto& [_, task] : allPendingTasks) {
        task->waitableEvent->Wait();
    }
}

bool AsyncTaskManager::HasPendingTasks() {
    std::lock_guard<std::mutex> lock(mPendingTasksMutex);
    return !mPendingTasks.empty();
}

void AsyncTaskManager::DoWaitableTask(void* task) {
    Ref<WaitableTask> waitableTask = AcquireRef(static_cast<WaitableTask*>(task));
    waitableTask->asyncTask();
    waitableTask->taskManager->HandleTaskCompletion(waitableTask.Get());
}

AsyncTaskManager::WaitableTask::WaitableTask() = default;

AsyncTaskManager::WaitableTask::~WaitableTask() = default;

}  // namespace dawn::native
