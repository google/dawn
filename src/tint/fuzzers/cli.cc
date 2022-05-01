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
