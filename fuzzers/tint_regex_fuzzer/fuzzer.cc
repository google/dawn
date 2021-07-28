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

#include "fuzzers/tint_common_fuzzer.h"
#include "fuzzers/tint_regex_fuzzer/cli.h"

#include "fuzzers/tint_regex_fuzzer/wgsl_mutator.h"

#include "src/reader/wgsl/parser.h"
#include "src/writer/wgsl/generator.h"

namespace tint {
namespace fuzzers {
namespace regex_fuzzer {
namespace {

CliParams cli_params{};

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
  // Parse CLI parameters. `ParseCliParams` will call `exit` if some parameter
  // is invalid.
  cli_params = ParseCliParams(argc, *argv);
  return 0;
}

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data,
                                          size_t size,
                                          size_t max_size,
                                          unsigned seed) {
  const std::vector<std::string> delimiters{";"};
  std::mt19937 generator(seed);
  std::uniform_int_distribution<size_t> distribution(0, delimiters.size() - 1);
  size_t ind = distribution(generator);

  return FuzzEnclosedRegions(size, max_size, delimiters[ind], data, &generator);
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
    if ((target.fuzzing_target & cli_params.fuzzing_target) !=
        target.fuzzing_target) {
      continue;
    }

    CommonFuzzer fuzzer(InputFormat::kWGSL, target.output_format);
    fuzzer.EnableInspector();
    fuzzer.Run(data, size);
  }

  return 0;
}

}  // namespace
}  // namespace regex_fuzzer
}  // namespace fuzzers
}  // namespace tint
