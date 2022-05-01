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

#include <memory>
#include <string>

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/cli.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/spirv_fuzz_mutator.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/spirv_opt_mutator.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/spirv_reduce_mutator.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/util.h"

/// This tool is used to debug *mutators*. It uses CLI arguments similar to the
/// ones used by the fuzzer. To debug some mutator you just need to specify the
/// mutator type, the seed and the path to the SPIR-V binary that triggered the
/// error. This tool will run the mutator on the binary until the error is
/// produced or the mutator returns `kLimitReached`.
///
/// Note that this is different from debugging the fuzzer by specifying input
/// files to test. The difference is that the latter will not execute any
/// mutator (it will only run the LLVMFuzzerTestOneInput function) whereas this
/// tool is useful when one of the spirv-tools mutators crashes or produces an
/// invalid binary in LLVMFuzzerCustomMutator.
int main(int argc, const char** argv) {
    auto params = tint::fuzzers::spvtools_fuzzer::ParseMutatorDebuggerCliParams(argc, argv);

    std::unique_ptr<tint::fuzzers::spvtools_fuzzer::Mutator> mutator;
    const auto& mutator_params = params.mutator_params;
    switch (params.mutator_type) {
        case tint::fuzzers::spvtools_fuzzer::MutatorType::kFuzz:
            mutator = std::make_unique<tint::fuzzers::spvtools_fuzzer::SpirvFuzzMutator>(
                mutator_params.target_env, params.original_binary, params.seed,
                mutator_params.donors, mutator_params.enable_all_fuzzer_passes,
                mutator_params.repeated_pass_strategy,
                mutator_params.validate_after_each_fuzzer_pass,
                mutator_params.transformation_batch_size);
            break;
        case tint::fuzzers::spvtools_fuzzer::MutatorType::kReduce:
            mutator = std::make_unique<tint::fuzzers::spvtools_fuzzer::SpirvReduceMutator>(
                mutator_params.target_env, params.original_binary, params.seed,
                mutator_params.reduction_batch_size, mutator_params.enable_all_reduce_passes,
                mutator_params.validate_after_each_reduce_pass);
            break;
        case tint::fuzzers::spvtools_fuzzer::MutatorType::kOpt:
            mutator = std::make_unique<tint::fuzzers::spvtools_fuzzer::SpirvOptMutator>(
                mutator_params.target_env, params.seed, params.original_binary,
                mutator_params.validate_after_each_opt_pass, mutator_params.opt_batch_size);
            break;
        default:
            assert(false && "All mutator types must've been handled");
            return 1;
    }

    while (true) {
        auto result = mutator->Mutate();
        if (result.GetStatus() == tint::fuzzers::spvtools_fuzzer::Mutator::Status::kInvalid) {
            std::cerr << mutator->GetErrors() << std::endl;
            return 0;
        }
        if (result.GetStatus() == tint::fuzzers::spvtools_fuzzer::Mutator::Status::kLimitReached) {
            break;
        }
    }
}
