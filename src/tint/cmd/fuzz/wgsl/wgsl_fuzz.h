// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_CMD_FUZZ_WGSL_WGSL_FUZZ_H_
#define SRC_TINT_CMD_FUZZ_WGSL_WGSL_FUZZ_H_

#include <string>

#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/macros/static_init.h"

namespace tint::fuzz::wgsl {

/// ProgramFuzzer describes a fuzzer function that takes a WGSL program as input
struct ProgramFuzzer {
    /// The function signature
    using Fn = void(const Program&);

    /// Name of the fuzzer function
    std::string_view name;
    /// The fuzzer function pointer
    Fn* fn = nullptr;
};

/// Runs all the registered WGSL fuzzers with the supplied WGSL
/// @param wgsl the input WGSL
void Run(std::string_view wgsl);

/// Registers the fuzzer function with the WGSL fuzzer executable.
/// @param fuzzer the fuzzer
void Register(const ProgramFuzzer& fuzzer);

/// TINT_WGSL_PROGRAM_FUZZER registers the fuzzer function to run as part of `tint_wgsl_fuzzer`
#define TINT_WGSL_PROGRAM_FUZZER(FUNCTION) \
    TINT_STATIC_INIT(::tint::fuzz::wgsl::Register({#FUNCTION, FUNCTION}))

}  // namespace tint::fuzz::wgsl

#endif  // SRC_TINT_CMD_FUZZ_WGSL_WGSL_FUZZ_H_
