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

#ifndef FUZZERS_FUZZER_INIT_H_
#define FUZZERS_FUZZER_INIT_H_

#include "fuzzers/cli.h"

namespace tint {
namespace fuzzers {

/// Returns the common CliParams parsed and populated by LLVMFuzzerInitialize()
const CliParams& GetCliParams();

}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_FUZZER_INIT_H_
