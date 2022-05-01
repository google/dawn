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

#ifndef SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_FUZZ_MUTATOR_H_
#define SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_FUZZ_MUTATOR_H_

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator.h"

#include "source/fuzz/fuzzer.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/pseudo_random_generator.h"

namespace tint::fuzzers::spvtools_fuzzer {

/// The mutator that uses spirv-fuzz to mutate SPIR-V.
///
/// The initial `binary` must be valid according to `target_env`. All other
/// parameters (except for the `seed` which just initializes the RNG) are from
/// the `spvtools::fuzz::Fuzzer` class.
class SpirvFuzzMutator : public Mutator {
  public:
    /// Constructor.
    /// @param target_env - the target environment for the `binary`.
    /// @param binary - the SPIR-V binary. Must be valid.
    /// @param seed - seed for the RNG.
    /// @param donors - vector of donor suppliers.
    /// @param enable_all_passes - whether to use all fuzzer passes.
    /// @param repeated_pass_strategy - the strategy to use when selecting the
    ///     next fuzzer pass.
    /// @param validate_after_each_pass - whether to validate the binary after
    ///     each fuzzer pass.
    /// @param transformation_batch_size - the maximum number of transformations
    ///     that will be applied during a single call to `Mutate`. It it's equal
    ///     to 0 then we apply as much transformations as we can until the
    ///     threshold in the spvtools::fuzz::Fuzzer is reached (see the doc for
    ///     that class for more info).
    SpirvFuzzMutator(spv_target_env target_env,
                     std::vector<uint32_t> binary,
                     uint32_t seed,
                     const std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier>& donors,
                     bool enable_all_passes,
                     spvtools::fuzz::RepeatedPassStrategy repeated_pass_strategy,
                     bool validate_after_each_pass,
                     uint32_t transformation_batch_size);

    Result Mutate() override;
    std::vector<uint32_t> GetBinary() const override;
    void LogErrors(const std::string* path, uint32_t count) const override;
    std::string GetErrors() const override;

  private:
    // The number of transformations that will be applied during a single call to
    // the `Mutate` method. Is this only a lower bound since transformations are
    // applied in batches by fuzzer passes (see docs for the
    // `spvtools::fuzz::Fuzzer` for more info).
    const uint32_t transformation_batch_size_;

    // The errors produced by the `spvtools::fuzz::Fuzzer`.
    std::unique_ptr<std::stringstream> errors_;
    std::unique_ptr<spvtools::fuzz::Fuzzer> fuzzer_;
    spvtools::ValidatorOptions validator_options_;

    // The following fields are useful for debugging.

    // The binary that the mutator is constructed with.
    const std::vector<uint32_t> original_binary_;

    // The seed that the mutator is constructed with.
    const uint32_t seed_;
};

}  // namespace tint::fuzzers::spvtools_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_FUZZ_MUTATOR_H_
