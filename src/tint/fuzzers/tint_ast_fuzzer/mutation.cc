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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

#include <cassert>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_binary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/delete_statement.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/replace_identifier.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/wrap_unary_operator.h"

namespace tint::fuzzers::ast_fuzzer {

Mutation::~Mutation() = default;

std::unique_ptr<Mutation> Mutation::FromMessage(const protobufs::Mutation& message) {
    switch (message.mutation_case()) {
        case protobufs::Mutation::kChangeUnaryOperator:
            return std::make_unique<MutationChangeUnaryOperator>(message.change_unary_operator());
        case protobufs::Mutation::kReplaceIdentifier:
            return std::make_unique<MutationReplaceIdentifier>(message.replace_identifier());
        case protobufs::Mutation::kChangeBinaryOperator:
            return std::make_unique<MutationChangeBinaryOperator>(message.change_binary_operator());
        case protobufs::Mutation::kDeleteStatement:
            return std::make_unique<MutationDeleteStatement>(message.delete_statement());
        case protobufs::Mutation::kWrapUnaryOperator:
            return std::make_unique<MutationWrapUnaryOperator>(message.wrap_unary_operator());
        case protobufs::Mutation::MUTATION_NOT_SET:
            assert(false && "Mutation is not set");
            break;
    }
    return nullptr;
}

}  // namespace tint::fuzzers::ast_fuzzer
