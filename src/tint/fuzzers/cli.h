// Copyright 2021 The Dawn & Tint Authors
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
