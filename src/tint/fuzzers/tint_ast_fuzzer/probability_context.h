// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
