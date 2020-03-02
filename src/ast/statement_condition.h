// Copyright 2020 The Tint Authors.
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

#ifndef SRC_AST_STATEMENT_CONDITION_H_
#define SRC_AST_STATEMENT_CONDITION_H_

#include <ostream>

namespace tint {
namespace ast {

/// Type of  condition attached to a statement
enum class StatementCondition { kNone = 0, kIf, kUnless };

std::ostream& operator<<(std::ostream& out, StatementCondition condition);

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STATEMENT_CONDITION_H_
