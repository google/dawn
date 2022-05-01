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

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/cli.h"

#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "source/opt/build_module.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/util.h"

namespace tint::fuzzers::spvtools_fuzzer {
namespace {

const char* const kMutatorParameters = R"(
Mutators' parameters:

  -tint_donors=
                       A path to the text file with a list of paths to the
                       SPIR-V donor files. Check out the doc for the spirv-fuzz
                       to learn more about donor binaries. Donors are not used
                       by default.

  -tint_enable_all_fuzzer_passes=
                       Whether to use all fuzzer passes or a randomly selected subset
                       of them. This must be one of `true` or `false` (without `).
                       By default it's `false`.

  -tint_enable_all_reduce_passes=
                       Whether to use all reduction passes or a randomly selected subset
                       of them. This must be one of `true` or `false` (without `).
                       By default it's `false`.

  -tint_opt_batch_size=
                       The maximum number of spirv-opt optimizations that
                       will be applied in a single mutation session (i.e.
                       a call to LLVMFuzzerCustomMutator). This must fit in
                       uint32_t. By default it's 6.

  -tint_reduction_batch_size=
                       The maximum number of spirv-reduce reductions that
                       will be applied in a single mutation session (i.e.
                       a call to LLVMFuzzerCustomMutator). This must fit in
                       uint32_t. By default it's 3.

  -tint_repeated_pass_strategy=
                       The strategy that will be used to recommend the next fuzzer
                       pass. This must be one of `simple`, `looped` or `random`
                       (without `). By default it's `simple`. Check out the doc for
                       spirv-fuzz to learn more.

  -tint_transformation_batch_size=
                       The maximum number of spirv-fuzz transformations
                       that will be applied during a single mutation
                       session (i.e. a call to LLVMFuzzerCustomMutator).
                       This must fit in uint32_t. By default it's 3.

  -tint_validate_after_each_fuzzer_pass=
                       Whether to validate SPIR-V binary after each fuzzer pass.
                       This must be one of `true` or `false` (without `).
                       By default it's `true`. Switch this to `false` if you experience
                       bad performance.

  -tint_validate_after_each_opt_pass=
                       Whether to validate SPIR-V binary after each optimization pass.
                       This must be one of `true` or `false` (without `).
                       By default it's `true`. Switch this to `false` if you experience
                       bad performance.

  -tint_validate_after_each_reduce_pass=
                       Whether to validate SPIR-V binary after each reduction pass.
                       This must be one of `true` or `false` (without `).
                       By default it's `true`. Switch this to `false` if you experience
                       bad performance.
)";

const char* const kFuzzerHelpMessage = R"(
This fuzzer uses SPIR-V binaries to fuzz the Tint compiler. It uses SPIRV-Tools
to mutate those binaries. The fuzzer works on a corpus of SPIR-V shaders.
For each shader from the corpus it uses one of `spirv-fuzz`, `spirv-reduce` or
`spirv-opt` to mutate it and then runs the shader through the Tint compiler in
two steps:
- Converts the mutated shader to WGSL.
- Converts WGSL to some target language specified in the CLI arguments.

Below is a list of all supported parameters for this fuzzer. You may want to
run it with -help=1 to check out libfuzzer parameters.

Fuzzer parameters:

  -tint_error_dir
                       The directory that will be used to output invalid SPIR-V
                       binaries to. This is especially useful during debugging
                       mutators. The directory must have the following subdirectories:
                       - spv/ - will be used to output errors, produced during
                         the conversion from the SPIR-V to WGSL.
                       - wgsl/ - will be used to output errors, produced during
                         the conversion from the WGSL to `--fuzzing_target`.
                       - mutator/ - will be used to output errors, produced by
                         the mutators.
                       By default invalid files are not printed out.

  -tint_fuzzing_target
                       The type of backend to target during fuzzing. This must
                       be one or a combination of `wgsl`, `spv`, `msl` or `hlsl`
                       (without `) separated by commas. By default it's
                       `wgsl,spv,msl,hlsl`.

  -tint_help
                       Show this message. Note that there is also a -help=1
                       parameter that will display libfuzzer's help message.

  -tint_mutator_cache_size=
                       The maximum size of the cache that stores
                       mutation sessions. This must fit in uint32_t.
                       By default it's 20.

  -tint_mutator_type=
                       Determines types of the mutators to run. This must be one or
                       a combination of `fuzz`, `opt`, `reduce` (without `) separated by
                       comma. If a combination is specified, each element in the
                       combination will have an equal chance of mutating a SPIR-V
                       binary during a mutation session (i.e. if no mutator exists
                       for that binary in the mutator cache). By default, the
                       parameter's value is `fuzz,opt,reduce`.
)";

const char* const kMutatorDebuggerHelpMessage = R"(
This tool is used to debug *mutators*. It uses CLI arguments similar to the
ones used by the fuzzer. To debug some mutator you just need to specify the
mutator type, the seed and the path to the SPIR-V binary that triggered the
error. This tool will run the mutator on the binary until the error is
produced or the mutator returns `kLimitReached`.

Note that this is different from debugging the fuzzer by specifying input
files to test. The difference is that the latter will not execute any
mutator (it will only run the LLVMFuzzerTestOneInput function) whereas this
tool is useful when one of the SPIRV-Tools mutators crashes or produces an
invalid binary in LLVMFuzzerCustomMutator.

Debugger parameters:

  --help
                       Show this message.

  --mutator_type=
                       Determines the type of the mutator to debug. This must be
                       one of `fuzz`, `reduce` or `opt` (without `). This parameter
                       is REQUIRED.

  --original_binary=
                       The path to the SPIR-V binary that the faulty mutator was
                       initialized with. This will be dumped on errors by the fuzzer
                       if `--error_dir` is specified. This parameter is REQUIRED.

  --seed=
                       The seed for the random number generator that was used to
                       initialize the mutator. This value is usually printed to
                       the console when the mutator produces an invalid binary.
                       It is also dumped into the log file if `--error_dir` is
                       specified. This must fit in uint32_t. This parameter is
                       REQUIRED.
)";

void PrintHelpMessage(const char* help_message) {
    std::cout << help_message << std::endl << kMutatorParameters << std::endl;
}

[[noreturn]] void InvalidParameter(const char* help_message, const char* param) {
    std::cout << "Invalid value for " << param << std::endl;
    PrintHelpMessage(help_message);
    exit(1);
}

bool ParseUint32(const char* param, uint32_t* out) {
    uint64_t value = static_cast<uint64_t>(strtoul(param, nullptr, 10));
    if (value > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())) {
        return false;
    }
    *out = static_cast<uint32_t>(value);
    return true;
}

std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier> ParseDonors(const char* file_name) {
    std::ifstream fin(file_name);
    if (!fin) {
        std::cout << "Can't open donors list file: " << file_name << std::endl;
        exit(1);
    }

    std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier> result;
    for (std::string donor_file_name; fin >> donor_file_name;) {
        if (!std::ifstream(donor_file_name)) {
            std::cout << "Can't open donor file: " << donor_file_name << std::endl;
            exit(1);
        }

        result.emplace_back([donor_file_name] {
            std::vector<uint32_t> binary;
            if (!util::ReadBinary(donor_file_name, &binary)) {
                std::cout << "Failed to read donor from: " << donor_file_name << std::endl;
                exit(1);
            }
            return spvtools::BuildModule(kDefaultTargetEnv,
                                         spvtools::fuzz::fuzzerutil::kSilentMessageConsumer,
                                         binary.data(), binary.size());
        });
    }

    return result;
}

bool ParseRepeatedPassStrategy(const char* param, spvtools::fuzz::RepeatedPassStrategy* out) {
    if (!strcmp(param, "simple")) {
        *out = spvtools::fuzz::RepeatedPassStrategy::kSimple;
    } else if (!strcmp(param, "looped")) {
        *out = spvtools::fuzz::RepeatedPassStrategy::kLoopedWithRecommendations;
    } else if (!strcmp(param, "random")) {
        *out = spvtools::fuzz::RepeatedPassStrategy::kRandomWithRecommendations;
    } else {
        return false;
    }
    return true;
}

bool ParseBool(const char* param, bool* out) {
    if (!strcmp(param, "true")) {
        *out = true;
    } else if (!strcmp(param, "false")) {
        *out = false;
    } else {
        return false;
    }
    return true;
}

bool ParseMutatorType(const char* param, MutatorType* out) {
    if (!strcmp(param, "fuzz")) {
        *out = MutatorType::kFuzz;
    } else if (!strcmp(param, "opt")) {
        *out = MutatorType::kOpt;
    } else if (!strcmp(param, "reduce")) {
        *out = MutatorType::kReduce;
    } else {
        return false;
    }
    return true;
}

bool ParseFuzzingTarget(const char* param, FuzzingTarget* out) {
    if (!strcmp(param, "wgsl")) {
        *out = FuzzingTarget::kWgsl;
    } else if (!strcmp(param, "spv")) {
        *out = FuzzingTarget::kSpv;
    } else if (!strcmp(param, "msl")) {
        *out = FuzzingTarget::kMsl;
    } else if (!strcmp(param, "hlsl")) {
        *out = FuzzingTarget::kHlsl;
    } else {
        return false;
    }
    return true;
}

bool HasPrefix(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool ParseMutatorCliParam(const char* param, const char* help_message, MutatorCliParams* out) {
    if (HasPrefix(param, "-tint_transformation_batch_size=")) {
        if (!ParseUint32(param + sizeof("-tint_transformation_batch_size=") - 1,
                         &out->transformation_batch_size)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_reduction_batch_size=")) {
        if (!ParseUint32(param + sizeof("-tint_reduction_batch_size=") - 1,
                         &out->reduction_batch_size)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_opt_batch_size=")) {
        if (!ParseUint32(param + sizeof("-tint_opt_batch_size=") - 1, &out->opt_batch_size)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_donors=")) {
        out->donors = ParseDonors(param + sizeof("-tint_donors=") - 1);
    } else if (HasPrefix(param, "-tint_repeated_pass_strategy=")) {
        if (!ParseRepeatedPassStrategy(param + sizeof("-tint_repeated_pass_strategy=") - 1,
                                       &out->repeated_pass_strategy)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_enable_all_fuzzer_passes=")) {
        if (!ParseBool(param + sizeof("-tint_enable_all_fuzzer_passes=") - 1,
                       &out->enable_all_fuzzer_passes)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_enable_all_reduce_passes=")) {
        if (!ParseBool(param + sizeof("-tint_enable_all_reduce_passes=") - 1,
                       &out->enable_all_reduce_passes)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_validate_after_each_opt_pass=")) {
        if (!ParseBool(param + sizeof("-tint_validate_after_each_opt_pass=") - 1,
                       &out->validate_after_each_opt_pass)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_validate_after_each_fuzzer_pass=")) {
        if (!ParseBool(param + sizeof("-tint_validate_after_each_fuzzer_pass=") - 1,
                       &out->validate_after_each_fuzzer_pass)) {
            InvalidParameter(help_message, param);
        }
    } else if (HasPrefix(param, "-tint_validate_after_each_reduce_pass=")) {
        if (!ParseBool(param + sizeof("-tint_validate_after_each_reduce_pass=") - 1,
                       &out->validate_after_each_reduce_pass)) {
            InvalidParameter(help_message, param);
        }
    } else {
        return false;
    }
    return true;
}

}  // namespace

FuzzerCliParams ParseFuzzerCliParams(int* argc, char** argv) {
    FuzzerCliParams cli_params;
    const auto* help_message = kFuzzerHelpMessage;
    auto help = false;

    for (int i = *argc - 1; i > 0; --i) {
        auto param = argv[i];
        auto recognized_param = true;

        if (HasPrefix(param, "-tint_mutator_cache_size=")) {
            if (!ParseUint32(param + sizeof("-tint_mutator_cache_size=") - 1,
                             &cli_params.mutator_cache_size)) {
                InvalidParameter(help_message, param);
            }
        } else if (HasPrefix(param, "-tint_mutator_type=")) {
            auto result = MutatorType::kNone;

            std::stringstream ss(param + sizeof("-tint_mutator_type=") - 1);
            for (std::string value; std::getline(ss, value, ',');) {
                auto out = MutatorType::kNone;
                if (!ParseMutatorType(value.c_str(), &out)) {
                    InvalidParameter(help_message, param);
                }
                result = result | out;
            }

            if (result == MutatorType::kNone) {
                InvalidParameter(help_message, param);
            }

            cli_params.mutator_type = result;
        } else if (HasPrefix(param, "-tint_fuzzing_target=")) {
            auto result = FuzzingTarget::kNone;

            std::stringstream ss(param + sizeof("-tint_fuzzing_target=") - 1);
            for (std::string value; std::getline(ss, value, ',');) {
                auto tmp = FuzzingTarget::kNone;
                if (!ParseFuzzingTarget(value.c_str(), &tmp)) {
                    InvalidParameter(help_message, param);
                }
                result = result | tmp;
            }

            if (result == FuzzingTarget::kNone) {
                InvalidParameter(help_message, param);
            }

            cli_params.fuzzing_target = result;
        } else if (HasPrefix(param, "-tint_error_dir=")) {
            cli_params.error_dir = param + sizeof("-tint_error_dir=") - 1;
        } else if (!strcmp(param, "-tint_help")) {
            help = true;
        } else {
            recognized_param =
                ParseMutatorCliParam(param, help_message, &cli_params.mutator_params);
        }

        if (recognized_param) {
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
        PrintHelpMessage(help_message);
        exit(0);
    }

    return cli_params;
}

MutatorDebuggerCliParams ParseMutatorDebuggerCliParams(int argc, const char* const* argv) {
    MutatorDebuggerCliParams cli_params;
    bool seed_param_present = false;
    bool original_binary_param_present = false;
    bool mutator_type_param_present = false;
    const auto* help_message = kMutatorDebuggerHelpMessage;
    auto help = false;

    for (int i = 0; i < argc; ++i) {
        auto param = argv[i];
        ParseMutatorCliParam(param, help_message, &cli_params.mutator_params);

        if (HasPrefix(param, "--mutator_type=")) {
            if (!ParseMutatorType(param + sizeof("--mutator_type=") - 1,
                                  &cli_params.mutator_type)) {
                InvalidParameter(help_message, param);
            }
            mutator_type_param_present = true;
        } else if (HasPrefix(param, "--original_binary=")) {
            if (!util::ReadBinary(param + sizeof("--original_binary=") - 1,
                                  &cli_params.original_binary)) {
                InvalidParameter(help_message, param);
            }
            original_binary_param_present = true;
        } else if (HasPrefix(param, "--seed=")) {
            if (!ParseUint32(param + sizeof("--seed=") - 1, &cli_params.seed)) {
                InvalidParameter(help_message, param);
            }
            seed_param_present = true;
        } else if (!strcmp(param, "--help")) {
            help = true;
        }
    }

    if (help) {
        PrintHelpMessage(help_message);
        exit(0);
    }

    std::pair<bool, const char*> required_params[] = {
        {seed_param_present, "--seed"},
        {original_binary_param_present, "--original_binary"},
        {mutator_type_param_present, "--mutator_type"}};

    for (auto required_param : required_params) {
        if (!required_param.first) {
            std::cout << required_param.second << " is missing" << std::endl;
            exit(1);
        }
    }

    return cli_params;
}

}  // namespace tint::fuzzers::spvtools_fuzzer
