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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_PROBABILITY_CONTEXT_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_PROBABILITY_CONTEXT_H_

#include <utility>
#include <vector>

#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::ast_fuzzer {

/// This class is intended to be used by the `MutationFinder`s to introduce some
/// variance to the mutation process.
class ProbabilityContext {
  public:
    /// Initializes this instance with a random number generator.
    /// @param generator - must not be a `nullptr`. Must remain in scope as long
    /// as this
    ///     instance exists.
    explicit ProbabilityContext(RandomGenerator* generator);

    /// Get random bool with even odds
    /// @returns true 50% of the time and false %50 of time.
    bool RandomBool() { return generator_->GetBool(); }

    /// Get random bool with weighted odds
    /// @param percentage - likelihood of true being returned
    /// @returns true |percentage|% of the time, and false (100 - |percentage|)%
    /// of the time.
    bool ChoosePercentage(uint32_t percentage) { return generator_->GetWeightedBool(percentage); }

    /// Returns a random value in the range `[0; arr.size())`.
    /// @tparam T - type of the elements in the vector.
    /// @param arr - may not be empty.
    /// @return the random index in the `arr`.
    template <typename T>
    size_t GetRandomIndex(const std::vector<T>& arr) {
        return static_cast<size_t>(generator_->GetUInt64(arr.size()));
    }

    /// @return the probability of replacing some binary operator with another.
    uint32_t GetChanceOfChangingBinaryOperators() const {
        return chance_of_changing_binary_operators_;
    }

    /// @return the probability of changing operator for an unary expression.
    uint32_t GetChanceOfChangingUnaryOperators() const {
        return chance_of_changing_unary_operators_;
    }

    /// @return the probability of changing operator for a binary expression.
    uint32_t GetChanceOfDeletingStatements() const { return chance_of_deleting_statements_; }

    /// @return the probability of replacing some identifier with some other one.
    uint32_t GetChanceOfReplacingIdentifiers() const { return chance_of_replacing_identifiers_; }

    /// @return the probability of wrapping an expression in a unary operator.
    uint32_t GetChanceOfWrappingUnaryOperators() const {
        return chance_of_wrapping_unary_operators_;
    }

  private:
    /// @param range - a pair of integers `a` and `b` s.t. `a <= b`.
    /// @return an random number in the range `[a; b]`.
    uint32_t RandomFromRange(std::pair<uint32_t, uint32_t> range);

    RandomGenerator* generator_;

    uint32_t chance_of_changing_binary_operators_;
    uint32_t chance_of_changing_unary_operators_;
    uint32_t chance_of_deleting_statements_;
    uint32_t chance_of_replacing_identifiers_;
    uint32_t chance_of_wrapping_unary_operators_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_PROBABILITY_CONTEXT_H_
