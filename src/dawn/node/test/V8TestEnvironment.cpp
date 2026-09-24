// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/node/test/V8TestEnvironment.h"

#include <memory>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#include "libplatform/libplatform.h"
#pragma clang diagnostic pop

namespace dawn::node::test {

namespace {

std::unique_ptr<v8::Platform> g_platform;

// Brings V8 up before the first test and takes it down after the last.
class V8Environment : public ::testing::Environment {
  public:
    ~V8Environment() override = default;

    void SetUp() override {
        v8::V8::SetFlagsFromString("--expose_gc");
        g_platform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(g_platform.get());
        v8::V8::Initialize();
    }

    void TearDown() override {
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
        g_platform.reset();
    }
};

testing::Environment* const v8_env = testing::AddGlobalTestEnvironment(new V8Environment);

}  // namespace

v8::Platform* V8Platform() {
    return g_platform.get();
}

void V8IsolateTest::SetUp() {
    allocator_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
    create_params_.array_buffer_allocator = allocator_.get();
    isolate_ = v8::Isolate::New(create_params_);
    isolate_->SetMicrotasksPolicy(microtasks_policy_);
    isolate_->Enter();

    // Every local handle a test creates lands in this scope, which outlives the test body.
    handle_scope_.emplace(isolate_);
    v8::Context::New(isolate_)->Enter();
}

void V8IsolateTest::TearDown() {
    context()->Exit();
    handle_scope_.reset();
    isolate_->Exit();
    isolate_->Dispose();
    allocator_.reset();
}

}  // namespace dawn::node::test
