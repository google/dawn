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

#ifndef SRC_TINT_FUZZERS_TINT_REGEX_FUZZER_OVERRIDE_CLI_PARAMS_H_
#define SRC_TINT_FUZZERS_TINT_REGEX_FUZZER_OVERRIDE_CLI_PARAMS_H_

#include "src/tint/fuzzers/tint_regex_fuzzer/cli.h"

namespace tint::fuzzers::regex_fuzzer {

/// @brief Allows CLI parameters to be overridden.
///
/// This function allows fuzz targets to override particular CLI parameters,
/// for example forcing a particular back-end to be targeted.
///
/// @param cli_params - the parsed CLI parameters to be updated.
void OverrideCliParams(CliParams& cli_params);

}  // namespace tint::fuzzers::regex_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_REGEX_FUZZER_OVERRIDE_CLI_PARAMS_H_
