#include "dawn_native/AsyncTask.h"

#include "dawn_platform/DawnPlatform.h"

namespace dawn_native {

    AsyncTaskManager::AsyncTaskManager(dawn_platform::WorkerTaskPool* workerTaskPool)
        : mWorkerTaskPool(workerTaskPool) {
    }

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

        for (auto& keyValue : allPendingTasks) {
            keyValue.second->waitableEvent->Wait();
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

}  // namespace dawn_native
