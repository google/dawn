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

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "dawn/common/RefCounted.h"
#include "dawn/common/TypeTraits.h"

namespace dawn::native {

struct CallbackTask {
  public:
    virtual ~CallbackTask() = default;

    void Execute();
    void OnShutDown();
    void OnDeviceLoss();

  protected:
    virtual void FinishImpl() = 0;
    virtual void HandleShutDownImpl() = 0;
    virtual void HandleDeviceLossImpl() = 0;

  private:
    enum class State {
        Normal,
        HandleShutDown,
        HandleDeviceLoss,
    };

    State mState = State::Normal;
};

class CallbackTaskManager : public RefCounted {
  public:
    CallbackTaskManager();
    ~CallbackTaskManager() override;

    void AddCallbackTask(std::unique_ptr<CallbackTask> callbackTask);
    void AddCallbackTask(std::function<void()> callback);
    template <typename... Args>
    void AddCallbackTask(void (*callback)(Args... args), Args... args) {
        static_assert((!IsCString<Args>::value && ...), "passing C string argument is not allowed");

        AddCallbackTask([=] { callback(args...); });
    }
    bool IsEmpty();
    void HandleDeviceLoss();
    void HandleShutDown();
    void Flush();

  private:
    std::vector<std::unique_ptr<CallbackTask>> AcquireCallbackTasks();

    std::mutex mCallbackTaskQueueMutex;
    std::vector<std::unique_ptr<CallbackTask>> mCallbackTaskQueue;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CALLBACKTASKMANAGER_H_
