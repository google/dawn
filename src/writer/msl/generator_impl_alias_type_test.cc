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
#include "src/ast/module.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitConstructedType_F32) {
  ast::type::F32 f32;
  ast::type::Alias alias(mod.RegisterSymbol("a"), "a", &f32);

  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(typedef float a;
)");
}

TEST_F(MslGeneratorImplTest, EmitConstructedType_NameCollision) {
  ast::type::F32 f32;
  ast::type::Alias alias(mod.RegisterSymbol("float"), "float", &f32);

  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(typedef float float_tint_0;
)");
}

TEST_F(MslGeneratorImplTest, EmitConstructedType_Struct) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>(
      Source{}, "a", &f32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(Source{}, 4));
  members.push_back(create<ast::StructMember>(Source{}, "b", &i32, b_deco));

  auto* str =
      create<ast::Struct>(Source{}, members, ast::StructDecorationList{});

  ast::type::Struct s(mod.RegisterSymbol("a"), "a", str);

  ASSERT_TRUE(gen.EmitConstructedType(&s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct a {
  float a;
  int b;
};
)");
}

TEST_F(MslGeneratorImplTest, EmitConstructedType_AliasStructIdent) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>(
      Source{}, "a", &f32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(Source{}, 4));
  members.push_back(create<ast::StructMember>(Source{}, "b", &i32, b_deco));

  auto* str =
      create<ast::Struct>(Source{}, members, ast::StructDecorationList{});

  ast::type::Struct s(mod.RegisterSymbol("b"), "b", str);
  ast::type::Alias alias(mod.RegisterSymbol("a"), "a", &s);

  ASSERT_TRUE(gen.EmitConstructedType(&alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(typedef b a;
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
