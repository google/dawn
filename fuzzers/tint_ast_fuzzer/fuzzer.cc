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

#include "fuzzers/tint_ast_fuzzer/cli.h"
#include "fuzzers/tint_ast_fuzzer/mt_rng.h"
#include "fuzzers/tint_ast_fuzzer/mutator.h"
#include "fuzzers/tint_ast_fuzzer/protobufs/tint_ast_fuzzer.h"
#include "fuzzers/tint_common_fuzzer.h"

#include "src/reader/wgsl/parser.h"
#include "src/writer/wgsl/generator.h"

namespace tint {
namespace fuzzers {
namespace ast_fuzzer {
namespace {

CliParams cli_params{};

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
  // Parse CLI parameters. `ParseCliParams` will call `exit` if some parameter
  // is invalid.
  cli_params = ParseCliParams(*argc, *argv);
  return 0;
}

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data,
                                          size_t size,
                                          size_t max_size,
                                          unsigned seed) {
  protobufs::MutatorState mutator_state;
  auto success = mutator_state.ParseFromArray(data, static_cast<int>(size));
  (void)success;  // This variable will be unused in release mode.
  assert(success && "Can't parse protobuf message");

  tint::Source::File file("test.wgsl", mutator_state.program());
  auto program = reader::wgsl::Parse(&file);
  protobufs::MutationSequence* mutation_sequence = nullptr;

  if (cli_params.record_mutations) {
    // If mutations are being recorded, then `mutator_state.program` is the
    // original (unmodified) program and it is necessary to replay all
    // mutations.
    mutation_sequence = mutator_state.mutable_mutation_sequence();
    program = Replay(std::move(program), *mutation_sequence);
    if (!program.IsValid()) {
      std::cout << "Replayer produced invalid WGSL:" << std::endl
                << "  seed: " << seed << std::endl
                << program.Diagnostics().str() << std::endl;
      return 0;
    }
  }

  // Run the mutator.
  MtRng mt_rng(seed);
  ProbabilityContext probability_context(&mt_rng);
  program = Mutate(std::move(program), &probability_context,
                   cli_params.enable_all_mutations,
                   cli_params.mutation_batch_size, mutation_sequence);

  if (!program.IsValid()) {
    std::cout << "Mutator produced invalid WGSL:" << std::endl
              << "  seed: " << seed << std::endl
              << program.Diagnostics().str() << std::endl;
    return 0;
  }

  if (!cli_params.record_mutations) {
    // If mutations are not being recorded, then the mutated `program` must be
    // stored into the `mutator_state`.
    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    if (!result.success) {
      std::cout << "Can't generate WGSL for valid tint::Program:" << std::endl
                << "  seed: " << seed << std::endl
                << result.error << std::endl;
      return 0;
    }
    *mutator_state.mutable_program() = result.wgsl;
  }

  if (mutator_state.ByteSizeLong() > max_size) {
    return 0;
  }

  success = mutator_state.SerializeToArray(data, static_cast<int>(max_size));
  assert(success && "Can't serialize a protobuf message");
  return mutator_state.ByteSizeLong();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size == 0) {
    return 0;
  }

  protobufs::MutatorState mutator_state;
  auto success = mutator_state.ParseFromArray(data, static_cast<int>(size));
  (void)success;  // This variable will be unused in release mode.
  assert(success && "Can't parse a protobuf message");

  std::string program_text;
  if (cli_params.record_mutations) {
    // If mutations are being recorded, then it's necessary to replay them
    // before invoking the system under test.
    Source::File file("test.wgsl", mutator_state.program());
    auto program =
        Replay(reader::wgsl::Parse(&file), mutator_state.mutation_sequence());
    assert(program.IsValid() && "Replayed program is invalid");

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    assert(result.success &&
           "Can't generate a shader for the valid tint::Program");
    program_text = result.wgsl;
  } else {
    program_text.assign(data, data + size);
  }

  std::pair<FuzzingTarget, OutputFormat> targets[] = {
      {FuzzingTarget::kWgsl, OutputFormat::kWGSL},
      {FuzzingTarget::kHlsl, OutputFormat::kHLSL},
      {FuzzingTarget::kMsl, OutputFormat::kMSL},
      {FuzzingTarget::kSpv, OutputFormat::kSpv}};

  for (auto target : targets) {
    if ((target.first & cli_params.fuzzing_target) != target.first) {
      continue;
    }

    CommonFuzzer fuzzer(InputFormat::kWGSL, target.second);
    fuzzer.EnableInspector();
    fuzzer.Run(reinterpret_cast<const uint8_t*>(program_text.data()),
               program_text.size());
  }

  return 0;
}

}  // namespace
}  // namespace ast_fuzzer
}  // namespace fuzzers
}  // namespace tint
