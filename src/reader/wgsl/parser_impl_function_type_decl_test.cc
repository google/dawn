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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionTypeDecl_Void) {
  auto* v = tm()->Get(std::make_unique<ast::type::VoidType>());

  auto* p = parser("void");
  auto* e = p->function_type_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e, v);
}

TEST_F(ParserImplTest, FunctionTypeDecl_Type) {
  auto* f32 = tm()->Get(std::make_unique<ast::type::F32Type>());
  auto* vec2 = tm()->Get(std::make_unique<ast::type::VectorType>(f32, 2));

  auto* p = parser("vec2<f32>");
  auto* e = p->function_type_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e, vec2);
}

TEST_F(ParserImplTest, FunctionTypeDecl_InvalidType) {
  auto* p = parser("vec2<invalid>");
  auto* e = p->function_type_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unknown type alias 'invalid'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
