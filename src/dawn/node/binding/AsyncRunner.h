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

#ifndef SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_
#define SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_

#include <stdint.h>
#include <memory>
#include <utility>

#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/interop/Napi.h"

namespace wgpu::binding {

// AsyncRunner is used to poll a wgpu::Device with calls to Tick() while there are asynchronous
// tasks in flight.
class AsyncRunner {
  public:
    AsyncRunner(Napi::Env env, wgpu::Device device);

    // Begin() should be called when a new asynchronous task is started.
    // If the number of executing asynchronous tasks transitions from 0 to 1, then a function
    // will be scheduled on the main JavaScript thread to call wgpu::Device::Tick() whenever the
    // thread is idle. This will be repeatedly called until the number of executing asynchronous
    // tasks reaches 0 again.
    void Begin();

    // End() should be called once the asynchronous task has finished.
    // Every call to Begin() should eventually result in a call to End().
    void End();

  private:
    void QueueTick();
    Napi::Env env_;
    wgpu::Device const device_;
    uint64_t count_ = 0;
    bool tick_queued_ = false;
};

// AsyncTask is a RAII helper for calling AsyncRunner::Begin() on construction, and
// AsyncRunner::End() on destruction.
class AsyncTask {
  public:
    inline AsyncTask(AsyncTask&&) = default;

    // Constructor.
    // Calls AsyncRunner::Begin()
    explicit inline AsyncTask(std::shared_ptr<AsyncRunner> runner) : runner_(std::move(runner)) {
        runner_->Begin();
    }

    // Destructor.
    // Calls AsyncRunner::End()
    inline ~AsyncTask() { runner_->End(); }

  private:
    AsyncTask(const AsyncTask&) = delete;
    AsyncTask& operator=(const AsyncTask&) = delete;
    std::shared_ptr<AsyncRunner> runner_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_
