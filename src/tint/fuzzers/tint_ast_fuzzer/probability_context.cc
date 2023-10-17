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
