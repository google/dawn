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
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitAliasType_F32) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("a", &f32);

  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(type a = f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructedType_Struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(
      create<ast::StructMember>("a", &f32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>("b", &i32, b_deco));

  auto* str = create<ast::Struct>();
  str->set_members(members);

  ast::type::StructType s("A", str);
  ast::type::AliasType alias("B", &s);

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

TEST_F(WgslGeneratorImplTest, EmitAliasType_ToStruct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(
      create<ast::StructMember>("a", &f32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>("b", &i32, b_deco));

  auto* str = create<ast::Struct>();
  str->set_members(members);

  ast::type::StructType s("A", str);
  ast::type::AliasType alias("B", &s);

  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(type B = A;
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
