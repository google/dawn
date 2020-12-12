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

#include "src/ast/variable_decl_statement.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {
namespace {

using VariableDeclStatementTest = TestHelper;

TEST_F(VariableDeclStatementTest, Creation) {
  type::F32 f32;
  auto* var = create<Variable>(Source{}, "a", StorageClass::kNone, &f32, false,
                               nullptr, ast::VariableDecorationList{});

  VariableDeclStatement stmt(Source{}, var);
  EXPECT_EQ(stmt.variable(), var);
}

TEST_F(VariableDeclStatementTest, Creation_WithSource) {
  type::F32 f32;
  auto* var = create<Variable>(Source{}, "a", StorageClass::kNone, &f32, false,
                               nullptr, ast::VariableDecorationList{});

  VariableDeclStatement stmt(Source{Source::Location{20, 2}}, var);
  auto src = stmt.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(VariableDeclStatementTest, IsVariableDecl) {
  type::F32 f32;
  auto* var = create<Variable>(Source{}, "a", StorageClass::kNone, &f32, false,
                               nullptr, ast::VariableDecorationList{});

  VariableDeclStatement stmt(Source{}, var);
  EXPECT_TRUE(stmt.Is<VariableDeclStatement>());
}

TEST_F(VariableDeclStatementTest, IsValid) {
  type::F32 f32;
  auto* var = create<Variable>(Source{}, "a", StorageClass::kNone, &f32, false,
                               nullptr, ast::VariableDecorationList{});
  VariableDeclStatement stmt(Source{}, var);
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(VariableDeclStatementTest, IsValid_InvalidVariable) {
  type::F32 f32;
  auto* var = create<Variable>(Source{}, "", StorageClass::kNone, &f32, false,
                               nullptr, ast::VariableDecorationList{});
  VariableDeclStatement stmt(Source{}, var);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(VariableDeclStatementTest, IsValid_NullVariable) {
  VariableDeclStatement stmt(Source{}, nullptr);
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(VariableDeclStatementTest, ToStr) {
  type::F32 f32;
  auto* var = create<Variable>(Source{}, "a", StorageClass::kNone, &f32, false,
                               nullptr, ast::VariableDecorationList{});

  VariableDeclStatement stmt(Source{Source::Location{20, 2}}, var);
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  VariableDeclStatement{
    Variable{
      a
      none
      __f32
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
