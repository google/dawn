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

#include "src/tint/program_id.h"

#include <atomic>

namespace tint {

namespace {

std::atomic<uint32_t> next_program_id{1};

}  // namespace

ProgramID::ProgramID() = default;

ProgramID::ProgramID(uint32_t id) : val(id) {}

ProgramID ProgramID::New() {
    return ProgramID(next_program_id++);
}

namespace detail {

/// AssertProgramIDsEqual is called by TINT_ASSERT_PROGRAM_IDS_EQUAL() and
/// TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID() to assert that the ProgramIDs
/// `a` and `b` are equal.
void AssertProgramIDsEqual(ProgramID a,
                           ProgramID b,
                           bool if_valid,
                           diag::System system,
                           const char* msg,
                           const char* file,
                           size_t line) {
    if (a == b) {
        return;  // matched
    }
    if (if_valid && (!a || !b)) {
        return;  //  a or b were not valid
    }
    diag::List diagnostics;
    tint::InternalCompilerError(file, line, system, diagnostics) << msg;
}

}  // namespace detail
}  // namespace tint
