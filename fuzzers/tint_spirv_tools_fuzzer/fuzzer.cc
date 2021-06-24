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

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "fuzzers/tint_common_fuzzer.h"
#include "fuzzers/tint_spirv_tools_fuzzer/cli.h"
#include "fuzzers/tint_spirv_tools_fuzzer/mutator_cache.h"
#include "fuzzers/tint_spirv_tools_fuzzer/spirv_fuzz_mutator.h"
#include "fuzzers/tint_spirv_tools_fuzzer/spirv_opt_mutator.h"
#include "fuzzers/tint_spirv_tools_fuzzer/spirv_reduce_mutator.h"
#include "fuzzers/tint_spirv_tools_fuzzer/util.h"

namespace tint {
namespace fuzzers {
namespace spvtools_fuzzer {
namespace {

struct Context {
  const FuzzerCliParams params;
  std::unique_ptr<MutatorCache> mutator_cache;
};

Context* context = nullptr;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
  auto params = ParseFuzzerCliParams(*argc, *argv);
  auto mutator_cache =
      params.mutator_cache_size
          ? std::make_unique<MutatorCache>(params.mutator_cache_size)
          : nullptr;
  context = new Context{std::move(params), std::move(mutator_cache)};
  return 0;
}

std::unique_ptr<Mutator> CreateMutator(const std::vector<uint32_t>& binary,
                                       unsigned seed) {
  std::vector<MutatorType> types;
  types.reserve(3);

  // Determine which mutator we will be using for `binary` at random.
  auto cli_mutator_type = context->params.mutator_type;
  if ((MutatorType::kFuzz & cli_mutator_type) == MutatorType::kFuzz) {
    types.push_back(MutatorType::kFuzz);
  }
  if ((MutatorType::kReduce & cli_mutator_type) == MutatorType::kReduce) {
    types.push_back(MutatorType::kReduce);
  }
  if ((MutatorType::kOpt & cli_mutator_type) == MutatorType::kOpt) {
    types.push_back(MutatorType::kOpt);
  }

  assert(!types.empty() && "At least one mutator type must be specified");
  std::mt19937 rng(seed);
  auto mutator_type =
      types[std::uniform_int_distribution<size_t>(0, types.size() - 1)(rng)];

  const auto& mutator_params = context->params.mutator_params;
  switch (mutator_type) {
    case MutatorType::kFuzz:
      return std::make_unique<SpirvFuzzMutator>(
          mutator_params.target_env, binary, seed, mutator_params.donors,
          mutator_params.enable_all_fuzzer_passes,
          mutator_params.repeated_pass_strategy,
          mutator_params.validate_after_each_fuzzer_pass,
          mutator_params.transformation_batch_size);
    case MutatorType::kReduce:
      return std::make_unique<SpirvReduceMutator>(
          mutator_params.target_env, binary, seed,
          mutator_params.reduction_batch_size,
          mutator_params.enable_all_reduce_passes,
          mutator_params.validate_after_each_reduce_pass);
    case MutatorType::kOpt:
      return std::make_unique<SpirvOptMutator>(
          mutator_params.target_env, seed, binary,
          mutator_params.validate_after_each_opt_pass,
          mutator_params.opt_batch_size);
    default:
      assert(false && "All mutator types must be handled above");
      return nullptr;
  }
}

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data,
                                          size_t size,
                                          size_t max_size,
                                          unsigned seed) {
  std::vector<uint32_t> binary(size / sizeof(uint32_t));
  std::memcpy(binary.data(), data, size);

  MutatorCache dummy_cache(1);
  auto* mutator_cache = context->mutator_cache.get();
  if (!mutator_cache) {
    // Use a dummy cache if the user has decided not to use a real cache.
    // The dummy cache will be destroyed when we return from this function but
    // it will save us from writing all the `if (mutator_cache)` below.
    mutator_cache = &dummy_cache;
  }

  if (!mutator_cache->Get(binary)) {
    // Assign a mutator to the binary if it doesn't have one yet.
    mutator_cache->Put(binary, CreateMutator(binary, seed));
  }

  auto* mutator = mutator_cache->Get(binary);
  assert(mutator && "Mutator must be present in the cache");

  auto result = mutator->Mutate();

  if (result.GetStatus() == Mutator::Status::kInvalid) {
    // The binary is invalid - log the error and remove the mutator from the
    // cache.
    util::LogMutatorError(*mutator, context->params.error_dir);
    mutator_cache->Remove(binary);
    return 0;
  }

  if (!result.IsChanged()) {
    // The mutator didn't change the binary this time. This could be due to the
    // fact that we've reached the number of mutations we can apply (e.g. the
    // number of transformations in spirv-fuzz) or the mutator was just unlucky.
    // Either way, there is no harm in destroying mutator and maybe trying again
    // later (i.e. if libfuzzer decides to do so).
    mutator_cache->Remove(binary);
    return 0;
  }

  // At this point the binary is valid and was changed by the mutator.

  auto mutated = mutator->GetBinary();
  auto mutated_bytes_size = mutated.size() * sizeof(uint32_t);
  if (mutated_bytes_size > max_size) {
    // The binary is too big. It's unlikely that we'll reduce its size by
    // applying the mutator one more time.
    mutator_cache->Remove(binary);
    return 0;
  }

  if (result.GetStatus() == Mutator::Status::kComplete) {
    // Reassign the mutator to the mutated binary in the cache so that we can
    // access later.
    mutator_cache->Put(mutated, mutator_cache->Remove(binary));
  } else {
    // If the binary is valid and was changed but is not `kComplete`, then the
    // mutator has reached some limit on the number of mutations.
    mutator_cache->Remove(binary);
  }

  std::memcpy(data, mutated.data(), mutated_bytes_size);
  return mutated_bytes_size;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size == 0) {
    return 0;
  }

  CommonFuzzer spv_to_wgsl(InputFormat::kSpv, OutputFormat::kWGSL);
  spv_to_wgsl.EnableInspector();
  spv_to_wgsl.Run(data, size);
  if (spv_to_wgsl.HasErrors()) {
    util::LogSpvError(spv_to_wgsl.GetErrors(), data, size,
                      context->params.error_dir);
    return 0;
  }

  const auto* writer =
      static_cast<const writer::wgsl::Generator*>(spv_to_wgsl.GetWriter());

  assert(writer && writer->error().empty() &&
         "Errors should have already been handled");

  auto wgsl = writer->result();

  std::pair<FuzzingTarget, OutputFormat> targets[] = {
      {FuzzingTarget::kHlsl, OutputFormat::kHLSL},
      {FuzzingTarget::kMsl, OutputFormat::kMSL},
      {FuzzingTarget::kSpv, OutputFormat::kSpv},
      {FuzzingTarget::kWgsl, OutputFormat::kWGSL}};

  for (auto target : targets) {
    if ((target.first & context->params.fuzzing_target) != target.first) {
      continue;
    }

    CommonFuzzer fuzzer(InputFormat::kWGSL, target.second);
    fuzzer.EnableInspector();
    fuzzer.Run(reinterpret_cast<const uint8_t*>(wgsl.data()), wgsl.size());
    if (fuzzer.HasErrors()) {
      util::LogWgslError(fuzzer.GetErrors(), data, size, wgsl, target.second,
                         context->params.error_dir);
    }
  }

  return 0;
}

}  // namespace
}  // namespace spvtools_fuzzer
}  // namespace fuzzers
}  // namespace tint
