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

#include "gtest/gtest.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/type/struct_type.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitAlias_F32) {
  ast::type::Alias alias(mod->RegisterSymbol("a"), "a", ty.f32);

  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(type a = f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructedType_Struct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.f32),
                            Member("b", ty.i32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  ast::type::Struct s(mod->RegisterSymbol("A"), "A", str);
  ast::type::Alias alias(mod->RegisterSymbol("B"), "B", &s);

  ASSERT_TRUE(gen.EmitConstructedType(&s)) << gen.error();
  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct A {
  a : f32;
  [[offset(4)]]
  b : i32;
};
type B = A;
)");
}

TEST_F(WgslGeneratorImplTest, EmitAlias_ToStruct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.f32),
                            Member("b", ty.i32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  ast::type::Struct s(mod->RegisterSymbol("A"), "A", str);
  ast::type::Alias alias(mod->RegisterSymbol("B"), "B", &s);

  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(type B = A;
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
