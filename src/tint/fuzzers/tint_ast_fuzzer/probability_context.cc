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

#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"

#include <cassert>

namespace tint::fuzzers::ast_fuzzer {
namespace {

const std::pair<uint32_t, uint32_t> kChanceOfChangingBinaryOperators = {30, 90};
const std::pair<uint32_t, uint32_t> kChanceOfChangingUnaryOperators = {30, 70};
const std::pair<uint32_t, uint32_t> kChanceOfDeletingStatements = {30, 70};
const std::pair<uint32_t, uint32_t> kChanceOfReplacingIdentifiers = {30, 70};
const std::pair<uint32_t, uint32_t> kChanceOfWrappingUnaryOperators = {30, 70};

}  // namespace

ProbabilityContext::ProbabilityContext(RandomGenerator* generator)
    : generator_(generator),
      chance_of_changing_binary_operators_(RandomFromRange(kChanceOfChangingBinaryOperators)),
      chance_of_changing_unary_operators_(RandomFromRange(kChanceOfChangingUnaryOperators)),
      chance_of_deleting_statements_(RandomFromRange(kChanceOfDeletingStatements)),
      chance_of_replacing_identifiers_(RandomFromRange(kChanceOfReplacingIdentifiers)),
      chance_of_wrapping_unary_operators_(RandomFromRange(kChanceOfWrappingUnaryOperators)) {
    assert(generator != nullptr && "generator must not be nullptr");
}

uint32_t ProbabilityContext::RandomFromRange(std::pair<uint32_t, uint32_t> range) {
    assert(range.first <= range.second && "Range must be non-decreasing");
    return generator_->GetUInt32(range.first,
                                 range.second + 1);  // + 1 need since range is inclusive.
}

}  // namespace tint::fuzzers::ast_fuzzer
