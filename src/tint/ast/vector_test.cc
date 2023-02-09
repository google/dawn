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

#include "src/tint/ast/vector.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstVectorTest = TestHelper;

TEST_F(AstVectorTest, Creation) {
    auto* v = create<Vector>(ty.i32(), 2u);
    ASSERT_TRUE(v->type->Is<ast::TypeName>());
    EXPECT_EQ(Symbols().NameFor(v->type->As<ast::TypeName>()->name->symbol), "i32");
    EXPECT_EQ(v->width, 2u);
}

TEST_F(AstVectorTest, FriendlyName) {
    auto* v = create<Vector>(ty.f32(), 3u);
    EXPECT_EQ(v->FriendlyName(Symbols()), "vec3<f32>");
}

}  // namespace
}  // namespace tint::ast
