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
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, VariableIdentDecl_Parses) {
  auto* p = parser("my_var : f32");
  std::string name;
  ast::type::Type* type;
  std::tie(name, type) = p->variable_ident_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_EQ(name, "my_var");
  ASSERT_NE(type, nullptr);
  ASSERT_TRUE(type->IsF32());
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingIdent) {
  auto* p = parser(": f32");
  std::string name;
  ast::type::Type* type;
  std::tie(name, type) = p->variable_ident_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_EQ(name, "");
  ASSERT_EQ(type, nullptr);

  auto t = p->next();
  ASSERT_TRUE(t.IsColon());
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingColon) {
  auto* p = parser("my_var f32");
  auto r = p->variable_ident_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:8: missing : for identifier declaration");
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingType) {
  auto* p = parser("my_var :");
  auto r = p->variable_ident_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:9: invalid type for identifier declaration");
}

TEST_F(ParserImplTest, VariableIdentDecl_InvalidIdent) {
  auto* p = parser("123 : f32");
  std::string name;
  ast::type::Type* type;
  std::tie(name, type) = p->variable_ident_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_EQ(name, "");
  ASSERT_EQ(type, nullptr);

  auto t = p->next();
  ASSERT_TRUE(t.IsSintLiteral());
}

TEST_F(ParserImplTest, VariableIdentDecl_InvalidType) {
  auto* p = parser("my_var : invalid");
  auto r = p->variable_ident_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:10: unknown type alias 'invalid'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
