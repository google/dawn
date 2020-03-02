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
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, GlobalDecl_Semicolon) {
  ParserImpl p(";");
  p.global_decl();
  ASSERT_FALSE(p.has_error()) << p.error();
}

TEST_F(ParserImplTest, GlobalDecl_Import) {
  ParserImpl p{R"(import "GLSL.std.430" as glsl;)"};
  p.global_decl();
  ASSERT_FALSE(p.has_error()) << p.error();

  auto m = p.module();
  ASSERT_EQ(1, m.imports().size());

  const auto& import = m.imports()[0];
  EXPECT_EQ("GLSL.std.430", import->path());
  EXPECT_EQ("glsl", import->name());
}

TEST_F(ParserImplTest, GlobalDecl_Import_Invalid) {
  ParserImpl p{R"(import as glsl;)"};
  p.global_decl();

  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:8: missing path for import");
}

TEST_F(ParserImplTest, GlobalDecl_Import_Invalid_MissingSemicolon) {
  ParserImpl p{R"(import "GLSL.std.430" as glsl)"};
  p.global_decl();

  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:30: missing ';' for import");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable) {
  ParserImpl p{"var<out> a : vec2<i32> = vec2<i32>(1, 2);"};
  p.global_decl();
  ASSERT_FALSE(p.has_error()) << p.error();

  auto m = p.module();
  ASSERT_EQ(m.global_variables().size(), 1);

  auto v = m.global_variables()[0].get();
  EXPECT_EQ(v->name(), "a");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_Invalid) {
  ParserImpl p{"var<out> a : vec2<invalid>;"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:19: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_MissingSemicolon) {
  ParserImpl p{"var<out> a : vec2<i32>"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:23: missing ';' for variable declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant) {
  ParserImpl p{"const a : i32 = 2;"};
  p.global_decl();
  ASSERT_FALSE(p.has_error()) << p.error();

  auto m = p.module();
  ASSERT_EQ(m.global_variables().size(), 1);

  auto v = m.global_variables()[0].get();
  EXPECT_EQ(v->name(), "a");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant_Invalid) {
  ParserImpl p{"const a : vec2<i32>;"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:20: missing = for const declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant_MissingSemicolon) {
  ParserImpl p{"const a : vec2<i32> = vec2<i32>(1, 2)"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:38: missing ';' for constant declaration");
}

TEST_F(ParserImplTest, GlobalDecl_EntryPoint) {
  ParserImpl p{"entry_point vertex = main;"};
  p.global_decl();
  ASSERT_FALSE(p.has_error()) << p.error();

  auto m = p.module();
  ASSERT_EQ(m.entry_points().size(), 1);
  EXPECT_EQ(m.entry_points()[0]->name(), "main");
}

TEST_F(ParserImplTest, GlobalDecl_EntryPoint_Invalid) {
  ParserImpl p{"entry_point main;"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:13: missing pipeline stage for entry point");
}

TEST_F(ParserImplTest, GlobalDecl_EntryPoint_MissingSemicolon) {
  ParserImpl p{"entry_point vertex = main"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:26: missing ';' for entry point");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias) {
  ParserImpl p{"type A = i32;"};
  p.global_decl();
  ASSERT_FALSE(p.has_error()) << p.error();

  auto m = p.module();
  ASSERT_EQ(m.alias_types().size(), 1);
  EXPECT_EQ(m.alias_types()[0]->name(), "A");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_Invalid) {
  ParserImpl p{"type A = invalid;"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:10: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_MissingSemicolon) {
  ParserImpl p{"type A = i32"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:13: missing ';' for type alias");
}

TEST_F(ParserImplTest, GlobalDecl_Function) {
  ParserImpl p{"fn main() -> void { return; }"};
  p.global_decl();
  ASSERT_FALSE(p.has_error()) << p.error();

  auto m = p.module();
  ASSERT_EQ(m.functions().size(), 1);
  EXPECT_EQ(m.functions()[0]->name(), "main");
}

TEST_F(ParserImplTest, GlobalDecl_Function_Invalid) {
  ParserImpl p{"fn main() -> { return; }"};
  p.global_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:14: unable to determine function return type");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
