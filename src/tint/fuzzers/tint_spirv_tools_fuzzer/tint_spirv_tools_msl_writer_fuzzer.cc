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

#include <cassert>

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/cli.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/override_cli_params.h"

namespace tint::fuzzers::spvtools_fuzzer {

void OverrideCliParams(FuzzerCliParams& cli_params) {
    assert(cli_params.fuzzing_target == FuzzingTarget::kAll &&
           "The fuzzing target should not have been set by a CLI parameter: it "
           "should have its default value.");
    cli_params.fuzzing_target = FuzzingTarget::kMsl;
}

}  // namespace tint::fuzzers::spvtools_fuzzer
