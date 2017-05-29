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

#include "ValidationTest.h"

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
}

ValidationTest::~ValidationTest() {
    // We need to destroy NXT objects before setting the procs to null otherwise the nxt*Release
    // will call a nullptr
    device = nxt::Device();
    nxtSetProcs(nullptr);
}

void ValidationTest::TearDown() {
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
