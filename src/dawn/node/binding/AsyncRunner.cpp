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

#include "src/dawn/node/binding/AsyncRunner.h"

#include <cassert>
#include <limits>

namespace wgpu::binding {

AsyncRunner::AsyncRunner(Napi::Env env, wgpu::Device device) : env_(env), device_(device) {}

void AsyncRunner::Begin() {
    assert(count_ != std::numeric_limits<decltype(count_)>::max());
    if (count_++ == 0) {
        QueueTick();
    }
}

void AsyncRunner::End() {
    assert(count_ > 0);
    count_--;
}

void AsyncRunner::QueueTick() {
    // TODO(crbug.com/dawn/1127): We probably want to reduce the frequency at which this gets
    // called.
    if (tick_queued_) {
        return;
    }
    tick_queued_ = true;
    env_.Global()
        .Get("setImmediate")
        .As<Napi::Function>()
        .Call({
            // TODO(crbug.com/dawn/1127): Create once, reuse.
            Napi::Function::New(env_,
                                [this](const Napi::CallbackInfo&) {
                                    tick_queued_ = false;
                                    if (count_ > 0) {
                                        device_.Tick();
                                        QueueTick();
                                    }
                                }),
        });
}

}  // namespace wgpu::binding
