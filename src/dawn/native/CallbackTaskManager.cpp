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

#include "dawn/native/CallbackTaskManager.h"

#include <utility>

#include "dawn/common/Assert.h"

namespace dawn::native {

namespace {
struct GenericFunctionTask : CallbackTask {
  public:
    explicit GenericFunctionTask(std::function<void()> func) : mFunction(std::move(func)) {}

  private:
    void FinishImpl() override { mFunction(); }
    void HandleShutDownImpl() override { mFunction(); }
    void HandleDeviceLossImpl() override { mFunction(); }

    std::function<void()> mFunction;
};
}  // namespace

void CallbackTask::Execute() {
    switch (mState) {
        case State::HandleDeviceLoss:
            HandleDeviceLossImpl();
            break;
        case State::HandleShutDown:
            HandleShutDownImpl();
            break;
        default:
            FinishImpl();
    }
}

void CallbackTask::OnShutDown() {
    // Only first state change will have effects in final Execute().
    if (mState != State::Normal) {
        return;
    }
    mState = State::HandleShutDown;
}

void CallbackTask::OnDeviceLoss() {
    if (mState != State::Normal) {
        return;
    }
    mState = State::HandleDeviceLoss;
}

CallbackTaskManager::CallbackTaskManager() = default;

CallbackTaskManager::~CallbackTaskManager() = default;

bool CallbackTaskManager::IsEmpty() {
    std::lock_guard<std::mutex> lock(mCallbackTaskQueueMutex);
    return mCallbackTaskQueue.empty();
}

void CallbackTaskManager::AddCallbackTask(std::unique_ptr<CallbackTask> callbackTask) {
    std::lock_guard<std::mutex> lock(mCallbackTaskQueueMutex);
    mCallbackTaskQueue.push_back(std::move(callbackTask));
}

void CallbackTaskManager::AddCallbackTask(std::function<void()> callback) {
    AddCallbackTask(std::make_unique<GenericFunctionTask>(std::move(callback)));
}

void CallbackTaskManager::HandleDeviceLoss() {
    std::lock_guard<std::mutex> lock(mCallbackTaskQueueMutex);
    for (auto& task : mCallbackTaskQueue) {
        task->OnDeviceLoss();
    }
}

void CallbackTaskManager::HandleShutDown() {
    std::lock_guard<std::mutex> lock(mCallbackTaskQueueMutex);
    for (auto& task : mCallbackTaskQueue) {
        task->OnShutDown();
    }
}

void CallbackTaskManager::Flush() {
    std::unique_lock<std::mutex> lock(mCallbackTaskQueueMutex);
    if (mCallbackTaskQueue.empty()) {
        return;
    }

    // If a user calls Queue::Submit inside the callback, then the device will be ticked,
    // which in turns ticks the tracker, causing reentrance and dead lock here. To prevent
    // such reentrant call, we remove all the callback tasks from mCallbackTaskManager,
    // update mCallbackTaskManager, then call all the callbacks.
    std::vector<std::unique_ptr<CallbackTask>> allTasks;
    allTasks.swap(mCallbackTaskQueue);
    lock.unlock();

    for (std::unique_ptr<CallbackTask>& callbackTask : allTasks) {
        callbackTask->Execute();
    }
}

}  // namespace dawn::native
