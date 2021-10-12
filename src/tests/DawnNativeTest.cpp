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

#include <gtest/gtest.h>

#include "absl/strings/str_cat.h"
#include "dawn_native/ErrorData.h"

namespace dawn_native {

    void AddFatalDawnFailure(const char* expression, const ErrorData* error) {
        const auto& backtrace = error->GetBacktrace();
        GTEST_MESSAGE_AT_(
            backtrace.at(0).file, backtrace.at(0).line,
            absl::StrCat(expression, " returned error: ", error->GetMessage()).c_str(),
            ::testing::TestPartResult::kFatalFailure);
    }

}  // namespace dawn_native
