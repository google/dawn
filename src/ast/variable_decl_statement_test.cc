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

namespace tint {
namespace ast {
namespace {

using VariableDeclStatementTest = TestHelper;

TEST_F(VariableDeclStatementTest, Creation) {
  auto* var = Var("a", ty.f32(), StorageClass::kNone);

  auto* stmt = create<VariableDeclStatement>(var);
  EXPECT_EQ(stmt->variable(), var);
}

TEST_F(VariableDeclStatementTest, Creation_WithSource) {
  auto* var = Var("a", ty.f32(), StorageClass::kNone);

  auto* stmt =
      create<VariableDeclStatement>(Source{Source::Location{20, 2}}, var);
  auto src = stmt->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(VariableDeclStatementTest, IsVariableDecl) {
  auto* var = Var("a", ty.f32(), StorageClass::kNone);

  auto* stmt = create<VariableDeclStatement>(var);
  EXPECT_TRUE(stmt->Is<VariableDeclStatement>());
}

TEST_F(VariableDeclStatementTest, IsValid) {
  auto* var = Var("a", ty.f32(), StorageClass::kNone);
  auto* stmt = create<VariableDeclStatement>(var);
  EXPECT_TRUE(stmt->IsValid());
}

TEST_F(VariableDeclStatementTest, IsValid_InvalidVariable) {
  auto* var = Var("", ty.f32(), StorageClass::kNone);
  auto* stmt = create<VariableDeclStatement>(var);
  EXPECT_FALSE(stmt->IsValid());
}

TEST_F(VariableDeclStatementTest, IsValid_NullVariable) {
  auto* stmt = create<VariableDeclStatement>(nullptr);
  EXPECT_FALSE(stmt->IsValid());
}

TEST_F(VariableDeclStatementTest, ToStr) {
  auto* var = Var("a", ty.f32(), StorageClass::kNone);

  auto* stmt =
      create<VariableDeclStatement>(Source{Source::Location{20, 2}}, var);
  EXPECT_EQ(str(stmt), R"(VariableDeclStatement{
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
