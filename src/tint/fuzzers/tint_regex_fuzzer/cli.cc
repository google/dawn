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

#include "src/tint/fuzzers/tint_regex_fuzzer/cli.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace tint::fuzzers::regex_fuzzer {
namespace {

const char* const kHelpMessage = R"(
This is a fuzzer for the Tint compiler that works by mutating a WGSL shader.

Below is a list of all supported parameters for this fuzzer. You may want to
run it with -help=1 to check out libfuzzer parameters.

  -tint_fuzzing_target=
                       Specifies the shading language to target during fuzzing.
                       This must be one or a combination of `wgsl`, `spv`, `hlsl`,
                       `msl` (without `) separated by commas. By default it's
                       `wgsl,msl,hlsl,spv`.

  -tint_help
                       Show this message. Note that there is also a -help=1
                       parameter that will display libfuzzer's help message.
)";

bool HasPrefix(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

[[noreturn]] void InvalidParam(const char* param) {
    std::cout << "Invalid value for " << param << std::endl;
    std::cout << kHelpMessage << std::endl;
    exit(1);
}

bool ParseFuzzingTarget(const char* value, FuzzingTarget* out) {
    if (!strcmp(value, "wgsl")) {
        *out = FuzzingTarget::kWgsl;
    } else if (!strcmp(value, "spv")) {
        *out = FuzzingTarget::kSpv;
    } else if (!strcmp(value, "msl")) {
        *out = FuzzingTarget::kMsl;
    } else if (!strcmp(value, "hlsl")) {
        *out = FuzzingTarget::kHlsl;
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
        auto param = argv[i];
        auto recognized_parameter = true;

        if (HasPrefix(param, "-tint_fuzzing_target=")) {
            auto result = FuzzingTarget::kNone;

            std::stringstream ss(param + sizeof("-tint_fuzzing_target=") - 1);
            for (std::string value; std::getline(ss, value, ',');) {
                auto tmp = FuzzingTarget::kNone;
                if (!ParseFuzzingTarget(value.c_str(), &tmp)) {
                    InvalidParam(param);
                }
                result = result | tmp;
            }

            if (result == FuzzingTarget::kNone) {
                InvalidParam(param);
            }

            cli_params.fuzzing_target = result;
        } else if (!strcmp(param, "-tint_help")) {
            help = true;
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

}  // namespace tint::fuzzers::regex_fuzzer
