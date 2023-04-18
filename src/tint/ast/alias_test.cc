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

#include "src/tint/ast/alias.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/builtin/access.h"

namespace tint::ast {
namespace {

using AstAliasTest = TestHelper;

TEST_F(AstAliasTest, Create) {
    auto u32 = ty.u32();
    auto* a = Alias("a_type", u32);
    CheckIdentifier(a->name, "a_type");
    CheckIdentifier(a->type, "u32");
}

}  // namespace
}  // namespace tint::ast
