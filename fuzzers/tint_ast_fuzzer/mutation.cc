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

#include "fuzzers/tint_ast_fuzzer/mutation.h"

#include <cassert>

#include "fuzzers/tint_ast_fuzzer/mutations/replace_identifier.h"

namespace tint {
namespace fuzzers {
namespace ast_fuzzer {

Mutation::~Mutation() = default;

std::unique_ptr<Mutation> Mutation::FromMessage(
    const protobufs::Mutation& message) {
  switch (message.mutation_case()) {
    case protobufs::Mutation::kReplaceIdentifier:
      return std::make_unique<MutationReplaceIdentifier>(
          message.replace_identifier());
    case protobufs::Mutation::MUTATION_NOT_SET:
      assert(false && "Mutation is not set");
      break;
  }
  return nullptr;
}

}  // namespace ast_fuzzer
}  // namespace fuzzers
}  // namespace tint
