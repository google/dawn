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

#ifndef SRC_DAWN_NATIVE_ASYNCTASK_H_
#define SRC_DAWN_NATIVE_ASYNCTASK_H_

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "dawn/common/RefCounted.h"

namespace dawn::platform {
class WaitableEvent;
class WorkerTaskPool;
}  // namespace dawn::platform

namespace dawn::native {

// TODO(crbug.com/dawn/826): we'll add additional things to AsyncTask in the future, like
// Cancel() and RunNow(). Cancelling helps avoid running the task's body when we are just
// shutting down the device. RunNow() could be used for more advanced scenarios, for example
// always doing ShaderModule initial compilation asynchronously, but being able to steal the
// task if we need it for synchronous pipeline compilation.
using AsyncTask = std::function<void()>;

class AsyncTaskManager {
  public:
    explicit AsyncTaskManager(dawn::platform::WorkerTaskPool* workerTaskPool);

    void PostTask(AsyncTask asyncTask);
    void WaitAllPendingTasks();
    bool HasPendingTasks();

  private:
    class WaitableTask : public RefCounted {
      public:
        WaitableTask();
        ~WaitableTask() override;

        AsyncTask asyncTask;
        AsyncTaskManager* taskManager;
        std::unique_ptr<dawn::platform::WaitableEvent> waitableEvent;
    };

    static void DoWaitableTask(void* task);
    void HandleTaskCompletion(WaitableTask* task);

    std::mutex mPendingTasksMutex;
    std::unordered_map<WaitableTask*, Ref<WaitableTask>> mPendingTasks;
    dawn::platform::WorkerTaskPool* mWorkerTaskPool;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ASYNCTASK_H_
