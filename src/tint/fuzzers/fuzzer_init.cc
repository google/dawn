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

#include <cstddef>

#include "src/tint/fuzzers/cli.h"
#include "src/tint/fuzzers/fuzzer_init.h"
#include "testing/libfuzzer/libfuzzer_exports.h"

namespace tint::fuzzers {

namespace {
CliParams cli_params;
}

const CliParams& GetCliParams() {
    return cli_params;
}

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    cli_params = ParseCliParams(argc, *argv);
    return 0;
}

}  // namespace tint::fuzzers
