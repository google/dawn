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
