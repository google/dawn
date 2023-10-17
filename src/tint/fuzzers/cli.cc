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

#include "src/tint/fuzzers/cli.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace tint::fuzzers {
namespace {

const char* const kHelpMessage = R"(
This is a fuzzer for the Tint compiler that works by mutating the AST.

Below is a list of all supported parameters for this fuzzer. You may want to
run it with -help=1 to check out libfuzzer parameters.

  -tint_dump_input=
                       If `true`, the fuzzer will dump input data to a file with
                       name tint_input_<hash>.spv/wgsl, where the hash is the hash
                       of the input data.

  -tint_help
                       Show this message. Note that there is also a -help=1
                       parameter that will display libfuzzer's help message.

  -tint_enforce_validity=
                       If `true`, the fuzzer will enforce that Tint does not
                       generate invalid shaders. Currently `false` by default
                       since options provided by the fuzzer are not guaranteed
                       to be correct.
                       See https://bugs.chromium.org/p/tint/issues/detail?id=1356
)";

[[noreturn]] void InvalidParam(const std::string& param) {
    std::cout << "Invalid value for " << param << std::endl;
    std::cout << kHelpMessage << std::endl;
    exit(1);
}

bool ParseBool(const std::string& value, bool* out) {
    if (value.compare("true") == 0) {
        *out = true;
    } else if (value.compare("false") == 0) {
        *out = false;
    } else {
        return false;
    }
    return true;
}

}  // namespace

CliParams ParseCliParams(int* argc, char** argv) {
    CliParams cli_params;
    auto help = false;

    for (int i = *argc - 1; i > 0; --i) {
        std::string param(argv[i]);
        auto recognized_parameter = true;

        if (std::string::npos != param.find("-tint_dump_input=")) {
            if (!ParseBool(param.substr(std::string("-tint_dump_input=").length()),
                           &cli_params.dump_input)) {
                InvalidParam(param);
            }
        } else if (std::string::npos != param.find("-tint_help")) {
            help = true;
        } else if (std::string::npos != param.find("-tint_enforce_validity=")) {
            if (!ParseBool(param.substr(std::string("-tint_enforce_validity=").length()),
                           &cli_params.enforce_validity)) {
                InvalidParam(param);
            }
        } else {
            recognized_parameter = false;
        }

        if (recognized_parameter) {
            // Remove the recognized parameter from the list of all parameters by
            // swapping it with the last one. This will suppress warnings in the
            // libFuzzer about unrecognized parameters. By default, libFuzzer thinks
            // that all user-defined parameters start with two dashes. However, we are
            // forced to use a single one to make the fuzzer compatible with the
            // ClusterFuzz.
            std::swap(argv[i], argv[*argc - 1]);
            *argc -= 1;
        }
    }

    if (help) {
        std::cout << kHelpMessage << std::endl;
        exit(0);
    }

    return cli_params;
}

}  // namespace tint::fuzzers
