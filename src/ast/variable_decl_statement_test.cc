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

#include "gtest/gtest.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {
namespace {

using VariableDeclStatementTest = testing::Test;

TEST_F(VariableDeclStatementTest, Creation) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);
  auto* var_ptr = var.get();

  VariableDeclStatement stmt(std::move(var));
  EXPECT_EQ(stmt.variable(), var_ptr);
}

TEST_F(VariableDeclStatementTest, Creation_WithSource) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);

  VariableDeclStatement stmt(Source{20, 2}, std::move(var));
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(VariableDeclStatementTest, IsVariableDecl) {
  VariableDeclStatement s;
  EXPECT_TRUE(s.IsVariableDecl());
}

TEST_F(VariableDeclStatementTest, IsValid) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);
  VariableDeclStatement stmt(std::move(var));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(VariableDeclStatementTest, IsValid_InvalidVariable) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("", StorageClass::kNone, &f32);
  VariableDeclStatement stmt(std::move(var));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(VariableDeclStatementTest, IsValid_NullVariable) {
  VariableDeclStatement stmt;
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(VariableDeclStatementTest, ToStr) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);

  VariableDeclStatement stmt(Source{20, 2}, std::move(var));
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
