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

#ifndef FUZZERS_TINT_AST_FUZZER_RANDOM_NUMBER_GENERATOR_H_
#define FUZZERS_TINT_AST_FUZZER_RANDOM_NUMBER_GENERATOR_H_

#include <cassert>
#include <cstdint>
#include <vector>

namespace tint {
namespace fuzzers {
namespace ast_fuzzer {

/// Abstracts away the underlying algorithm that is used to generate random
/// numbers.
class RandomNumberGenerator {
 public:
  /// Virtual destructor.
  virtual ~RandomNumberGenerator();

  /// @brief Compute a random `uint32_t` value in the range `[0; bound)`.
  /// @param bound - the upper exclusive bound for the computed integer
  ///     (must be positive).
  /// @return the random number.
  virtual uint32_t RandomUint32(uint32_t bound) = 0;

  /// @brief Compute a random `uint64_t` value in the range `[0; bound)`.
  /// @param bound - the upper exclusive bound for the computed integer
  ///     (must be positive).
  /// @return the random number.
  virtual uint64_t RandomUint64(uint64_t bound) = 0;

  /// @return a randomly generated boolean value.
  bool RandomBool();

  /// @param percentage - must be in the range `[0; 100]`.
  /// @return `true` with `percentage` probability.
  bool ChoosePercentage(uint32_t percentage);
};

}  // namespace ast_fuzzer
}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_AST_FUZZER_RANDOM_NUMBER_GENERATOR_H_
