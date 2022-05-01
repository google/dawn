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

#ifndef SRC_TINT_FUZZERS_CLI_H_
#define SRC_TINT_FUZZERS_CLI_H_

#include <cstdint>

namespace tint::fuzzers {

/// CLI parameters accepted by the fuzzer. Type -tint_help in the CLI to see the
/// help message
struct CliParams {
    /// Log contents of input shader
    bool dump_input = false;
    /// Throw error if shader becomes invalid during run
    bool enforce_validity = false;
};

/// @brief Parses CLI parameters.
///
/// This function will exit the process with non-zero return code if some
/// parameters are invalid. This function will remove recognized parameters from
/// `argv` and adjust `argc` accordingly.
///
/// @param argc - the total number of parameters.
/// @param argv - array of all CLI parameters.
/// @return parsed parameters.
CliParams ParseCliParams(int* argc, char** argv);

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_CLI_H_
