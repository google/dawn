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

#include "fuzzers/tint_ast_fuzzer/random_number_generator.h"

namespace tint {
namespace fuzzers {
namespace ast_fuzzer {

RandomNumberGenerator::~RandomNumberGenerator() = default;

bool RandomNumberGenerator::RandomBool() {
  return RandomUint32(2);
}

bool RandomNumberGenerator::ChoosePercentage(uint32_t percentage) {
  assert(percentage <= 100 && "|percentage| is invalid");
  // 100 is used as a bound instead of 101 because otherwise it would be
  // possible to return `false` when `percentage == 100` holds. This would
  // happen when the result of `RandomUint32` is 100 as well.
  return RandomUint32(100) < percentage;
}

}  // namespace ast_fuzzer
}  // namespace fuzzers
}  // namespace tint
