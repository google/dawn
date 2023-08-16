// Copyright 2021 The Tint Authors.
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

#include "gmock/gmock.h"

#include "src/tint/api/tint.h"
#include "src/tint/utils/ice/ice.h"

namespace {

void TintInternalCompilerErrorReporter(const tint::InternalCompilerError& err) {
    FAIL() << err.Error();
}

}  // namespace

// Entry point for tint unit tests
int main(int argc, char** argv) {
    testing::InitGoogleMock(&argc, argv);

    tint::Initialize();

    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

    auto res = RUN_ALL_TESTS();

    tint::Shutdown();

    return res;
}
