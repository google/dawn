// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_
#define SRC_DAWN_NODE_BINDING_ASYNCRUNNER_H_

#include <stdint.h>
#include <memory>
#include <utility>

#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/interop/NodeAPI.h"

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
