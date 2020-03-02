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
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, TypeDecl_ParsesType) {
  auto tm = TypeManager::Instance();
  auto i32 = tm->Get(std::make_unique<ast::type::I32Type>());

  ParserImpl p{"type a = i32"};
  auto t = p.type_alias();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->type()->IsI32());
  ASSERT_EQ(t->type(), i32);

  TypeManager::Destroy();
}

TEST_F(ParserImplTest, TypeDecl_ParsesStruct) {
  ParserImpl p{"type a = struct { b: i32; c: f32;}"};
  auto t = p.type_alias();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t->name(), "a");
  ASSERT_TRUE(t->type()->IsStruct());

  auto s = t->type()->AsStruct();
  EXPECT_EQ(s->impl()->members().size(), 2);
}

TEST_F(ParserImplTest, TypeDecl_MissingIdent) {
  ParserImpl p{"type = i32"};
  auto t = p.type_alias();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing identifier for type alias");
}

TEST_F(ParserImplTest, TypeDecl_InvalidIdent) {
  ParserImpl p{"type 123 = i32"};
  auto t = p.type_alias();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing identifier for type alias");
}

TEST_F(ParserImplTest, TypeDecl_MissingEqual) {
  ParserImpl p{"type a i32"};
  auto t = p.type_alias();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing = for type alias");
}

TEST_F(ParserImplTest, TypeDecl_InvalidType) {
  ParserImpl p{"type a = B"};
  auto t = p.type_alias();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p.error(), "1:10: unknown type alias 'B'");
}

TEST_F(ParserImplTest, TypeDecl_InvalidStruct) {
  ParserImpl p{"type a = [[block]] {}"};
  auto t = p.type_alias();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(t, nullptr);
  EXPECT_EQ(p.error(), "1:20: missing struct declaration");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
