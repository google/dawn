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

#include "dawn/platform/WorkerThread.h"

#include <condition_variable>
#include <functional>
#include <thread>

#include "dawn/common/Assert.h"

namespace {

class AsyncWaitableEventImpl {
  public:
    AsyncWaitableEventImpl() : mIsComplete(false) {}

    void Wait() {
        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [this] { return mIsComplete; });
    }

    bool IsComplete() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mIsComplete;
    }

    void MarkAsComplete() {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mIsComplete = true;
        }
        mCondition.notify_all();
    }

  private:
    std::mutex mMutex;
    std::condition_variable mCondition;
    bool mIsComplete;
};

class AsyncWaitableEvent final : public dawn::platform::WaitableEvent {
  public:
    AsyncWaitableEvent() : mWaitableEventImpl(std::make_shared<AsyncWaitableEventImpl>()) {}

    void Wait() override { mWaitableEventImpl->Wait(); }

    bool IsComplete() override { return mWaitableEventImpl->IsComplete(); }

    std::shared_ptr<AsyncWaitableEventImpl> GetWaitableEventImpl() const {
        return mWaitableEventImpl;
    }

  private:
    std::shared_ptr<AsyncWaitableEventImpl> mWaitableEventImpl;
};

}  // anonymous namespace

namespace dawn::platform {

std::unique_ptr<dawn::platform::WaitableEvent> AsyncWorkerThreadPool::PostWorkerTask(
    dawn::platform::PostWorkerTaskCallback callback,
    void* userdata) {
    std::unique_ptr<AsyncWaitableEvent> waitableEvent = std::make_unique<AsyncWaitableEvent>();

    std::function<void()> doTask = [callback, userdata,
                                    waitableEventImpl = waitableEvent->GetWaitableEventImpl()]() {
        callback(userdata);
        waitableEventImpl->MarkAsComplete();
    };

    std::thread thread(doTask);
    thread.detach();

    return waitableEvent;
}

}  // namespace dawn::platform
