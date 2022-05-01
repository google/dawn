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

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/spirv_fuzz_mutator.h"

#include <fstream>
#include <utility>

#include "source/opt/build_module.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/util.h"

namespace tint::fuzzers::spvtools_fuzzer {

SpirvFuzzMutator::SpirvFuzzMutator(
    spv_target_env target_env,
    std::vector<uint32_t> binary,
    unsigned seed,
    const std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier>& donors,
    bool enable_all_passes,
    spvtools::fuzz::RepeatedPassStrategy repeated_pass_strategy,
    bool validate_after_each_pass,
    uint32_t transformation_batch_size)
    : transformation_batch_size_(transformation_batch_size),
      errors_(std::make_unique<std::stringstream>()),
      fuzzer_(nullptr),
      validator_options_(),
      original_binary_(std::move(binary)),
      seed_(seed) {
    auto ir_context =
        spvtools::BuildModule(target_env, spvtools::fuzz::fuzzerutil::kSilentMessageConsumer,
                              original_binary_.data(), original_binary_.size());
    assert(ir_context && "|binary| is invalid");

    auto transformation_context = std::make_unique<spvtools::fuzz::TransformationContext>(
        std::make_unique<spvtools::fuzz::FactManager>(ir_context.get()), validator_options_);

    auto fuzzer_context = std::make_unique<spvtools::fuzz::FuzzerContext>(
        std::make_unique<spvtools::fuzz::PseudoRandomGenerator>(seed),
        spvtools::fuzz::FuzzerContext::GetMinFreshId(ir_context.get()), false);
    fuzzer_ = std::make_unique<spvtools::fuzz::Fuzzer>(
        std::move(ir_context), std::move(transformation_context), std::move(fuzzer_context),
        util::GetBufferMessageConsumer(errors_.get()), donors, enable_all_passes,
        repeated_pass_strategy, validate_after_each_pass, validator_options_);
}

Mutator::Result SpirvFuzzMutator::Mutate() {
    // The assertion will fail in |fuzzer_->Run| if the previous fuzzing led to
    // invalid module.
    auto result = fuzzer_->Run(transformation_batch_size_);
    switch (result.status) {
        case spvtools::fuzz::Fuzzer::Status::kComplete:
            return {Mutator::Status::kComplete, result.is_changed};
        case spvtools::fuzz::Fuzzer::Status::kModuleTooBig:
        case spvtools::fuzz::Fuzzer::Status::kTransformationLimitReached:
            return {Mutator::Status::kLimitReached, result.is_changed};
        case spvtools::fuzz::Fuzzer::Status::kFuzzerStuck:
            return {Mutator::Status::kStuck, result.is_changed};
        case spvtools::fuzz::Fuzzer::Status::kFuzzerPassLedToInvalidModule:
            return {Mutator::Status::kInvalid, result.is_changed};
    }
}

std::vector<uint32_t> SpirvFuzzMutator::GetBinary() const {
    std::vector<uint32_t> result;
    fuzzer_->GetIRContext()->module()->ToBinary(&result, true);
    return result;
}

std::string SpirvFuzzMutator::GetErrors() const {
    return errors_->str();
}

void SpirvFuzzMutator::LogErrors(const std::string* path, uint32_t count) const {
    auto message = GetErrors();
    std::cout << count << " | SpirvFuzzMutator (seed: " << seed_ << ")" << std::endl;
    std::cout << message << std::endl;

    if (path) {
        auto prefix = *path + std::to_string(count);

        // Write errors to file.
        std::ofstream(prefix + ".fuzzer.log") << "seed: " << seed_ << std::endl
                                              << message << std::endl;

        // Write the invalid SPIR-V binary.
        util::WriteBinary(prefix + ".fuzzer.invalid.spv", GetBinary());

        // Write the original SPIR-V binary.
        util::WriteBinary(prefix + ".fuzzer.original.spv", original_binary_);

        // Write transformations.
        google::protobuf::util::JsonOptions options;
        options.add_whitespace = true;
        std::string json;
        google::protobuf::util::MessageToJsonString(fuzzer_->GetTransformationSequence(), &json,
                                                    options);
        std::ofstream(prefix + ".fuzzer.transformations.json") << json << std::endl;

        std::ofstream binary_transformations(prefix + ".fuzzer.transformations.binary",
                                             std::ios::binary | std::ios::out);
        fuzzer_->GetTransformationSequence().SerializeToOstream(&binary_transformations);
    }
}

}  // namespace tint::fuzzers::spvtools_fuzzer
