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

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, EmitAliasType_F32) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("a", &f32);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitAliasType(&alias)) << g.error();
  EXPECT_EQ(g.result(), R"(typedef float a;
)");
}

TEST_F(MslGeneratorImplTest, EmitAliasType_NameCollision) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("float", &f32);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitAliasType(&alias)) << g.error();
  EXPECT_EQ(g.result(), R"(typedef float float_tint_0;
)");
}

TEST_F(MslGeneratorImplTest, EmitAliasType_Struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &f32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &i32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  ast::type::AliasType alias("a", &s);

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitAliasType(&alias)) << g.error();
  EXPECT_EQ(g.result(), R"(typedef struct {
  float a;
  int b;
} a;
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
