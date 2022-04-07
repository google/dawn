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

#ifndef SRC_TINT_FUZZERS_FUZZER_INIT_H_
#define SRC_TINT_FUZZERS_FUZZER_INIT_H_

#include "src/tint/fuzzers/cli.h"

namespace tint::fuzzers {

/// Returns the common CliParams parsed and populated by LLVMFuzzerInitialize()
const CliParams& GetCliParams();

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_FUZZER_INIT_H_
