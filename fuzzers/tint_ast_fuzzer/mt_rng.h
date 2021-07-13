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

#ifndef FUZZERS_TINT_AST_FUZZER_MT_RNG_H_
#define FUZZERS_TINT_AST_FUZZER_MT_RNG_H_

#include <random>

#include "fuzzers/tint_ast_fuzzer/random_number_generator.h"

namespace tint {
namespace fuzzers {
namespace ast_fuzzer {

/// The random number generator that uses STL's Mersenne Twister (std::mt19937)
/// under the hood.
class MtRng : public RandomNumberGenerator {
 public:
  /// @brief Initializes this RNG with some `seed`.
  /// @param seed - passed down to the `std::mt19937`.
  explicit MtRng(uint32_t seed);

  uint32_t RandomUint32(uint32_t bound) override;
  uint64_t RandomUint64(uint64_t bound) override;

 private:
  std::mt19937 rng_;
};

}  // namespace ast_fuzzer
}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_AST_FUZZER_MT_RNG_H_
