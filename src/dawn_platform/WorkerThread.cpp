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

#include "dawn_platform/WorkerThread.h"

#include <future>

#include "common/Assert.h"

namespace {

    class AsyncWaitableEvent final : public dawn_platform::WaitableEvent {
      public:
        explicit AsyncWaitableEvent(std::function<void()> func) {
            mFuture = std::async(std::launch::async, func);
        }
        void Wait() override {
            ASSERT(mFuture.valid());
            mFuture.wait();
        }
        bool IsComplete() override {
            ASSERT(mFuture.valid());
            return mFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }

      private:
        // It is safe not to call Wait() in the destructor of AsyncWaitableEvent because since
        // C++14 the destructor of std::future will always be blocked until its state becomes
        // std::future_status::ready when it was created by a call of std::async and it is the
        // last reference to the shared state.
        // See https://en.cppreference.com/w/cpp/thread/future/~future for more details.
        std::future<void> mFuture;
    };

}  // anonymous namespace

std::unique_ptr<dawn_platform::WaitableEvent> AsyncWorkerThreadPool::PostWorkerTask(
    dawn_platform::PostWorkerTaskCallback callback,
    void* userdata) {
    std::function<void()> doTask = [callback, userdata]() { callback(userdata); };
    return std::make_unique<AsyncWaitableEvent>(doTask);
}