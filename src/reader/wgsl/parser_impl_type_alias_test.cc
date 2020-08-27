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
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, TypeDecl_ParsesType) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser("type a = i32");
  auto* t = p->type_alias();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->type()->IsI32());
  ASSERT_EQ(t->type(), i32);
}

TEST_F(ParserImplTest, TypeDecl_ParsesStruct) {
  auto* p = parser("type a = struct { b: i32; c: f32;}");
  auto* t = p->type_alias();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t->name(), "a");
  ASSERT_TRUE(t->type()->IsStruct());

  auto* s = t->type()->AsStruct();
  EXPECT_EQ(s->impl()->members().size(), 2u);
}

TEST_F(ParserImplTest, TypeDecl_MissingIdent) {
  auto* p = parser("type = i32");
  auto* t = p->type_alias();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing identifier for type alias");
}

TEST_F(ParserImplTest, TypeDecl_InvalidIdent) {
  auto* p = parser("type 123 = i32");
  auto* t = p->type_alias();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing identifier for type alias");
}

TEST_F(ParserImplTest, TypeDecl_MissingEqual) {
  auto* p = parser("type a i32");
  auto* t = p->type_alias();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing = for type alias");
}

TEST_F(ParserImplTest, TypeDecl_InvalidType) {
  auto* p = parser("type a = B");
  auto* t = p->type_alias();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:10: unknown type alias 'B'");
}

TEST_F(ParserImplTest, TypeDecl_InvalidStruct) {
  auto* p = parser("type a = [[block]] {}");
  auto* t = p->type_alias();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:20: missing struct declaration");
}

TEST_F(ParserImplTest, TypeDecl_Struct_WithStride) {
  auto* p = parser(
      "type a = [[block]] struct { [[offset 0]] data: [[stride 4]] array<f32>; "
      "}");
  auto* t = p->type_alias();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t->name(), "a");
  ASSERT_TRUE(t->type()->IsStruct());

  auto* s = t->type()->AsStruct();
  EXPECT_EQ(s->impl()->members().size(), 1u);

  const auto* ty = s->impl()->members()[0]->type();
  ASSERT_TRUE(ty->IsArray());
  const auto* arr = ty->AsArray();
  EXPECT_TRUE(arr->has_array_stride());
  EXPECT_EQ(arr->array_stride(), 4u);
}

// This was failing due to not finding the missing ;. https://crbug.com/tint/218
TEST_F(ParserImplTest, TypeDecl_Struct_Empty) {
  auto* p = parser("type str = struct {};");
  p->global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto module = p->module();
  ASSERT_EQ(module.alias_types().size(), 1u);

  auto* t = module.alias_types()[0];
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t->name(), "str");

  ASSERT_TRUE(t->type()->IsStruct());
  auto* s = t->type()->AsStruct();
  EXPECT_EQ(s->impl()->members().size(), 0u);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
