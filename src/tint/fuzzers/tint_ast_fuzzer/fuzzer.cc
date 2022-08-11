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

#include <cstddef>
#include <cstdint>

#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/cli.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/override_cli_params.h"
#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/transform_builder.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/writer/wgsl/generator.h"
#include "testing/libfuzzer/libfuzzer_exports.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

CliParams cli_params{};

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
    Source::File file("test.wgsl", {reinterpret_cast<char*>(data), size});
    auto program = reader::wgsl::Parse(&file);
    if (!program.IsValid()) {
        std::cout << "Trying to mutate an invalid program:" << std::endl
                  << program.Diagnostics().str() << std::endl;
        return 0;
    }

    // Run the mutator.
    RandomGenerator generator(seed);
    ProbabilityContext probability_context(&generator);
    program = Mutate(std::move(program), &probability_context, cli_params.enable_all_mutations,
                     cli_params.mutation_batch_size, nullptr);

    if (!program.IsValid()) {
        std::cout << "Mutator produced invalid WGSL:" << std::endl
                  << "  seed: " << seed << std::endl
                  << program.Diagnostics().str() << std::endl;
        return 0;
    }

    auto result = writer::wgsl::Generate(&program, writer::wgsl::Options());
    if (!result.success) {
        std::cout << "Can't generate WGSL for a valid tint::Program:" << std::endl
                  << result.error << std::endl;
        return 0;
    }

    if (result.wgsl.size() > max_size) {
        return 0;
    }

    // No need to worry about the \0 here. The reason is that if \0 is included by
    // developer by mistake, it will be considered a part of the string and will
    // cause all sorts of strange bugs. Thus, unless `data` below is used as a raw
    // C string, the \0 symbol should be ignored.
    std::memcpy(  // NOLINT - clang-tidy warns about lack of null termination.
        data, result.wgsl.data(), result.wgsl.size());
    return result.wgsl.size();
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
        if (fuzzer.HasErrors()) {
            std::cout << "Fuzzing " << target.name << " produced an error" << std::endl;
            auto printer = tint::diag::Printer::create(stderr, true);
            tint::diag::Formatter{}.format(fuzzer.Diagnostics(), printer.get());
        }
    }

    return 0;
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
