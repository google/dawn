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

#include "src/type/access_control_type.h"
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"

namespace tint {
namespace type {
namespace {

using StructTypeTest = TestHelper;

TEST_F(StructTypeTest, Creation) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::DecorationList{});
  auto* ptr = impl;
  auto* s = ty.struct_("S", impl);
  EXPECT_EQ(s->impl(), ptr);
}

TEST_F(StructTypeTest, Is) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::DecorationList{});
  auto* s = ty.struct_("S", impl);
  type::Type* ty = s;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_TRUE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(StructTypeTest, TypeName) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::DecorationList{});
  auto* s = ty.struct_("my_struct", impl);
  EXPECT_EQ(s->type_name(), "__struct_tint_symbol_1");
}

TEST_F(StructTypeTest, FriendlyName) {
  auto* impl =
      create<ast::Struct>(ast::StructMemberList{}, ast::DecorationList{});
  auto* s = ty.struct_("my_struct", impl);
  EXPECT_EQ(s->FriendlyName(Symbols()), "my_struct");
}

}  // namespace
}  // namespace type
}  // namespace tint
