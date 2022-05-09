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

#ifndef SRC_DAWN_NATIVE_CALLBACKTASKMANAGER_H_
#define SRC_DAWN_NATIVE_CALLBACKTASKMANAGER_H_

#include <memory>
#include <mutex>
#include <vector>

namespace dawn::native {

struct CallbackTask {
  public:
    virtual ~CallbackTask() = default;
    virtual void Finish() = 0;
    virtual void HandleShutDown() = 0;
    virtual void HandleDeviceLoss() = 0;
};

class CallbackTaskManager {
  public:
    CallbackTaskManager();
    ~CallbackTaskManager();

    void AddCallbackTask(std::unique_ptr<CallbackTask> callbackTask);
    bool IsEmpty();
    std::vector<std::unique_ptr<CallbackTask>> AcquireCallbackTasks();

  private:
    std::mutex mCallbackTaskQueueMutex;
    std::vector<std::unique_ptr<CallbackTask>> mCallbackTaskQueue;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CALLBACKTASKMANAGER_H_
