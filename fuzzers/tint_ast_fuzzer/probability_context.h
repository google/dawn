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

#ifndef FUZZERS_TINT_AST_FUZZER_PROBABILITY_CONTEXT_H_
#define FUZZERS_TINT_AST_FUZZER_PROBABILITY_CONTEXT_H_

#include <utility>
#include <vector>

#include "fuzzers/tint_ast_fuzzer/random_number_generator.h"

namespace tint {
namespace fuzzers {
namespace ast_fuzzer {

/// This class is intended to be used by the `MutationFinder`s to introduce some
/// variance to the mutation process.
class ProbabilityContext {
 public:
  /// Initializes this instance with a random number generator.
  /// @param rng - may not be a `nullptr`. Must remain in scope as long as this
  ///     instance exists.
  explicit ProbabilityContext(RandomNumberGenerator* rng);

  /// @copydoc RandomNumberGenerator::RandomBool
  bool RandomBool() { return rng_->RandomBool(); }

  /// @copydoc RandomNumberGenerator::ChoosePercentage
  bool ChoosePercentage(uint32_t percentage) {
    return rng_->ChoosePercentage(percentage);
  }

  /// Returns a random value in the range `[0; arr.size())`.
  /// @tparam T - type of the elements in the vector.
  /// @param arr - may not be empty.
  /// @return the random index in the `arr`.
  template <typename T>
  size_t GetRandomIndex(const std::vector<T>& arr) {
    return static_cast<size_t>(rng_->RandomUint64(arr.size()));
  }

  /// @return the probability of replacing some identifier with some other one.
  uint32_t GetChanceOfReplacingIdentifiers() const {
    return chance_of_replacing_identifiers_;
  }

 private:
  /// @param range - a pair of integers `a` and `b` s.t. `a <= b`.
  /// @return an random number in the range `[a; b]`.
  uint32_t RandomFromRange(std::pair<uint32_t, uint32_t> range);

  RandomNumberGenerator* rng_;

  uint32_t chance_of_replacing_identifiers_;
};

}  // namespace ast_fuzzer
}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_AST_FUZZER_PROBABILITY_CONTEXT_H_
