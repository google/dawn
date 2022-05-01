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
#include "src/tint/ast/access.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/bool.h"
#include "src/tint/ast/f32.h"
#include "src/tint/ast/i32.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/ast/struct.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/ast/texture.h"
#include "src/tint/ast/u32.h"
#include "src/tint/ast/vector.h"

namespace tint::ast {
namespace {

using AstAliasTest = TestHelper;

TEST_F(AstAliasTest, Create) {
    auto* u32 = create<U32>();
    auto* a = Alias("a_type", u32);
    EXPECT_EQ(a->name, Symbol(1, ID()));
    EXPECT_EQ(a->type, u32);
}

}  // namespace
}  // namespace tint::ast
