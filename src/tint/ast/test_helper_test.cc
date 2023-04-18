// Copyright 2023 The Tint Authors.
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

#include "src/tint/ast/test_helper.h"

namespace tint::ast {

using namespace tint::number_suffixes;  // NOLINT

using AstCheckIdentifierTest = TestHelper;

TEST_F(AstCheckIdentifierTest, NonTemplated) {
    CheckIdentifier(Ident("abc"), "abc");
}

TEST_F(AstCheckIdentifierTest, TemplatedScalars) {
    CheckIdentifier(Ident("abc", 1_i, 2_u, 3_f, 4_h, 5_a, 6._a, true),  //
                    Template("abc", 1_i, 2_u, 3_f, 4_h, 5_a, 6._a, true));
}

TEST_F(AstCheckIdentifierTest, TemplatedIdentifiers) {
    CheckIdentifier(Ident("abc", "one", "two", "three"),  //
                    Template("abc", "one", "two", "three"));
}

TEST_F(AstCheckIdentifierTest, NestedTemplate) {
    CheckIdentifier(Ident("abc", "pre", Ident("nested", 42_a), "post"),  //
                    Template("abc", "pre", Template("nested", 42_a), "post"));
}

}  // namespace tint::ast
