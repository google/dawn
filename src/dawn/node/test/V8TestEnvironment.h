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

#ifndef SRC_DAWN_NODE_TEST_V8TESTENVIRONMENT_H_
#define SRC_DAWN_NODE_TEST_V8TESTENVIRONMENT_H_

#include <gtest/gtest.h>

#include <memory>
#include <optional>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wcast-function-type-strict"
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#pragma clang diagnostic ignored "-Wsuggest-destructor-override"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wunique-object-duplication"
#pragma clang diagnostic ignored "-Wundefined-reinterpret-cast"
#include <v8.h>
#pragma clang diagnostic pop

namespace dawn::node::test {

// The platform the test binary initialized V8 with. Registering a gtest global environment brings
// V8 up once for the whole binary, because InitializePlatform() and Initialize() are process-wide
// and cannot be undone and redone between tests.
//
// A test that drives V8's foreground task queue needs that exact platform - a second platform
// would have its own, permanently empty, queue - and V8 exposes no way to ask for it.
v8::Platform* V8Platform();

// A fixture with an isolate of its own. SetUp() creates and enters one, opens a handle scope that
// outlives the test body, and enters a fresh context; TearDown() undoes all of it in reverse. A
// derived fixture that owns something built on the isolate releases it before delegating here.
class V8IsolateTest : public ::testing::Test {
  protected:
    // Pass kExplicit to keep V8 from draining microtasks on its own, so that a test can pin down
    // when they run.
    explicit V8IsolateTest(v8::MicrotasksPolicy microtasks_policy)
        : microtasks_policy_(microtasks_policy) {}

    void SetUp() override;
    void TearDown() override;

    v8::Local<v8::Context> context() const { return isolate_->GetCurrentContext(); }

    v8::Isolate* isolate_ = nullptr;

  private:
    const v8::MicrotasksPolicy microtasks_policy_;
    std::unique_ptr<v8::ArrayBuffer::Allocator> allocator_;
    v8::Isolate::CreateParams create_params_;
    std::optional<v8::HandleScope> handle_scope_;
};

}  // namespace dawn::node::test

#endif  // SRC_DAWN_NODE_TEST_V8TESTENVIRONMENT_H_
