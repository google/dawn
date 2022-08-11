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

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/tint_regex_fuzzer/cli.h"
#include "src/tint/fuzzers/tint_regex_fuzzer/override_cli_params.h"
#include "src/tint/fuzzers/tint_regex_fuzzer/wgsl_mutator.h"
#include "src/tint/fuzzers/transform_builder.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/writer/wgsl/generator.h"
#include "testing/libfuzzer/libfuzzer_exports.h"

namespace tint::fuzzers::regex_fuzzer {
namespace {

CliParams cli_params{};

enum class MutationKind {
    kSwapIntervals,
    kDeleteInterval,
    kDuplicateInterval,
    kReplaceIdentifier,
    kReplaceLiteral,
    kInsertReturnStatement,
    kReplaceOperator,
    kInsertBreakOrContinue,
    kReplaceFunctionCallWithBuiltin,
    kAddSwizzle,
    kNumMutationKinds
};

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    // Parse CLI parameters. `ParseCliParams` will call `exit` if some parameter
    // is invalid.
    cli_params = ParseCliParams(argc, *argv);
    // For some fuzz targets it is desirable to force the values of certain CLI
    // parameters after parsing.
    OverrideCliParams(cli_params);
    return 0;
}

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data,
                                          size_t size,
                                          size_t max_size,
                                          unsigned seed) {
    std::string wgsl_code(data, data + size);
    const std::vector<std::string> delimiters{";"};
    RandomGenerator generator(seed);

    std::string delimiter =
        delimiters[generator.GetUInt32(static_cast<uint32_t>(delimiters.size()))];

    MutationKind mutation_kind = static_cast<MutationKind>(
        generator.GetUInt32(static_cast<uint32_t>(MutationKind::kNumMutationKinds)));

    WgslMutator mutator(generator);
    switch (mutation_kind) {
        case MutationKind::kSwapIntervals:
            if (!mutator.SwapRandomIntervals(delimiter, wgsl_code)) {
                return 0;
            }
            break;

        case MutationKind::kDeleteInterval:
            if (!mutator.DeleteRandomInterval(delimiter, wgsl_code)) {
                return 0;
            }
            break;

        case MutationKind::kDuplicateInterval:
            if (!mutator.DuplicateRandomInterval(delimiter, wgsl_code)) {
                return 0;
            }
            break;

        case MutationKind::kReplaceIdentifier:
            if (!mutator.ReplaceRandomIdentifier(wgsl_code)) {
                return 0;
            }
            break;

        case MutationKind::kReplaceLiteral:
            if (!mutator.ReplaceRandomIntLiteral(wgsl_code)) {
                return 0;
            }
            break;

        case MutationKind::kInsertReturnStatement:
            if (!mutator.InsertReturnStatement(wgsl_code)) {
                return 0;
            }
            break;

        case MutationKind::kReplaceOperator:
            if (!mutator.ReplaceRandomOperator(wgsl_code)) {
                return 0;
            }
            break;

        case MutationKind::kInsertBreakOrContinue:
            if (!mutator.InsertBreakOrContinue(wgsl_code)) {
                return 0;
            }
            break;
        case MutationKind::kReplaceFunctionCallWithBuiltin:
            if (!mutator.ReplaceFunctionCallWithBuiltin(wgsl_code)) {
                return 0;
            }
            break;
        case MutationKind::kAddSwizzle:
            if (!mutator.AddSwizzle(wgsl_code)) {
                return 0;
            }
            break;
        default:
            assert(false && "Unreachable");
            return 0;
    }

    if (wgsl_code.size() > max_size) {
        return 0;
    }

    memcpy(data, wgsl_code.c_str(), wgsl_code.size());
    return wgsl_code.size();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) {
        return 0;
    }

    struct Target {
        FuzzingTarget fuzzing_target;
        OutputFormat output_format;
        const char* name;
    };

    Target targets[] = {{FuzzingTarget::kWgsl, OutputFormat::kWGSL, "WGSL"},
                        {FuzzingTarget::kHlsl, OutputFormat::kHLSL, "HLSL"},
                        {FuzzingTarget::kMsl, OutputFormat::kMSL, "MSL"},
                        {FuzzingTarget::kSpv, OutputFormat::kSpv, "SPV"}};

    for (auto target : targets) {
        if ((target.fuzzing_target & cli_params.fuzzing_target) != target.fuzzing_target) {
            continue;
        }

        TransformBuilder tb(data, size);
        tb.AddTransform<tint::transform::Robustness>();

        CommonFuzzer fuzzer(InputFormat::kWGSL, target.output_format);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());

        fuzzer.Run(data, size);
    }

    return 0;
}

}  // namespace
}  // namespace tint::fuzzers::regex_fuzzer
