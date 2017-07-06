// Copyright 2017 The NXT Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "nxt/nxt.h"

namespace backend {
    namespace null {
        void Init(nxtProcTable* procs, nxtDevice* device);
    }
}

ValidationTest::ValidationTest() {
    nxtProcTable procs;
    nxtDevice cDevice;
    backend::null::Init(&procs, &cDevice);

    nxtSetProcs(&procs);
    device = nxt::Device::Acquire(cDevice);

    device.SetErrorCallback(ValidationTest::OnDeviceError, static_cast<nxtCallbackUserdata>(reinterpret_cast<uintptr_t>(this)));
}

ValidationTest::~ValidationTest() {
    // We need to destroy NXT objects before setting the procs to null otherwise the nxt*Release
    // will call a nullptr
    device = nxt::Device();
    nxtSetProcs(nullptr);
}

void ValidationTest::TearDown() {
    ASSERT_FALSE(expectError);

    for (auto& expectation : expectations) {
        std::string name = expectation.debugName;
        if (name.empty()) {
            name = "<no debug name set>";
        }

        ASSERT_TRUE(expectation.gotStatus) << "Didn't get a status for " << name;

        ASSERT_NE(NXT_BUILDER_ERROR_STATUS_UNKNOWN, expectation.gotStatus) << "Got unknown status for " << name;

        bool wasSuccess = expectation.status == NXT_BUILDER_ERROR_STATUS_SUCCESS;
        ASSERT_EQ(expectation.expectSuccess, wasSuccess)
            << "Got wrong status value for " << name
            << ", status was " << expectation.status << " with \"" << expectation.statusMessage << "\"";
    }
}

void ValidationTest::StartExpectDeviceError() {
    expectError = true;
    error = false;
}
bool ValidationTest::EndExpectDeviceError() {
    expectError = false;
    return error;
}

void ValidationTest::OnDeviceError(const char* message, nxtCallbackUserdata userdata) {
    // Skip this one specific error that is raised when a builder is used after it got an error
    // this is important because we don't want to wrap all creation tests in ASSERT_DEVICE_ERROR.
    // Yes the error message is misleading.
    if (std::string(message) == "Builder cannot be used after GetResult") {
        return;
    }

    auto self = reinterpret_cast<ValidationTest*>(static_cast<uintptr_t>(userdata));
    ASSERT_TRUE(self->expectError) << "Got unexpected device error: " << message;
    ASSERT_FALSE(self->error) << "Got two errors in expect block";
    self->error = true;
}

void ValidationTest::OnBuilderErrorStatus(nxtBuilderErrorStatus status, const char* message, nxt::CallbackUserdata userdata1, nxt::CallbackUserdata userdata2) {
    auto* self = reinterpret_cast<ValidationTest*>(static_cast<uintptr_t>(userdata1));
    size_t index = userdata2;

    ASSERT_LT(index, self->expectations.size());

    auto& expectation = self->expectations[index];
    ASSERT_FALSE(expectation.gotStatus);
    expectation.gotStatus = true;
    expectation.status = status;
    expectation.statusMessage = message;
}
