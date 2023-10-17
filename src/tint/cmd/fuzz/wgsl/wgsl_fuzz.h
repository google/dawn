// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
