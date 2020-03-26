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

#include "src/ast/variable_statement.h"

#include "gtest/gtest.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {
namespace {

using VariableStatementTest = testing::Test;

TEST_F(VariableStatementTest, Creation) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);
  auto var_ptr = var.get();

  VariableStatement stmt(std::move(var));
  EXPECT_EQ(stmt.variable(), var_ptr);
}

TEST_F(VariableStatementTest, Creation_WithSource) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);

  VariableStatement stmt(Source{20, 2}, std::move(var));
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(VariableStatementTest, IsVariable) {
  VariableStatement s;
  EXPECT_TRUE(s.IsVariable());
}

TEST_F(VariableStatementTest, IsValid) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);
  VariableStatement stmt(std::move(var));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(VariableStatementTest, IsValid_InvalidVariable) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("", StorageClass::kNone, &f32);
  VariableStatement stmt(std::move(var));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(VariableStatementTest, IsValid_NullVariable) {
  VariableStatement stmt;
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(VariableStatementTest, ToStr) {
  type::F32Type f32;
  auto var = std::make_unique<Variable>("a", StorageClass::kNone, &f32);

  VariableStatement stmt(Source{20, 2}, std::move(var));
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  VariableStatement{
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
