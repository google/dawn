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

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator.h"

namespace tint::fuzzers::spvtools_fuzzer {

// We need to define destructor here so that vtable is produced in this
// translation unit (see -Wweak-vtables clang flag).
Mutator::~Mutator() = default;

Mutator::Result::Result(Status status, bool is_changed) : status_(status), is_changed_(is_changed) {
    assert((is_changed || status == Status::kStuck || status == Status::kLimitReached) &&
           "Returning invalid result state");
}

}  // namespace tint::fuzzers::spvtools_fuzzer
