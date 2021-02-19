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

#include "src/program.h"

#include <sstream>
#include <utility>

#include "gmock/gmock.h"
#include "src/ast/function.h"
#include "src/ast/return_statement.h"
#include "src/ast/test_helper.h"
#include "src/ast/variable.h"
#include "src/type/alias_type.h"
#include "src/type/f32_type.h"
#include "src/type/struct_type.h"

namespace tint {
namespace {

using ProgramTest = ast::TestHelper;

TEST_F(ProgramTest, Unbuilt) {
  Program program;
  EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, Creation) {
  Program program(std::move(*this));
  EXPECT_EQ(program.AST().Functions().size(), 0u);
}

TEST_F(ProgramTest, ToStrEmitsPreambleAndPostamble) {
  Program program(std::move(*this));
  const auto str = program.to_str();
  auto* const expected = "Module{\n}\n";
  EXPECT_EQ(str, expected);
}

TEST_F(ProgramTest, IsValid_Empty) {
  Program program(std::move(*this));
  EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_GlobalVariable) {
  Global("var", ty.f32(), ast::StorageClass::kInput);

  Program program(std::move(*this));
  EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Null_GlobalVariable) {
  AST().AddGlobalVariable(nullptr);

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Invalid_GlobalVariable) {
  Global("var", nullptr, ast::StorageClass::kInput);

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Alias) {
  auto* alias = ty.alias("alias", ty.f32());
  AST().AddConstructedType(alias);

  Program program(std::move(*this));
  EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Null_Alias) {
  AST().AddConstructedType(nullptr);

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Struct) {
  auto* st = ty.struct_("name", {});
  auto* alias = ty.alias("name", st);
  AST().AddConstructedType(alias);

  Program program(std::move(*this));
  EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Struct_EmptyName) {
  auto* st = ty.struct_("", {});
  auto* alias = ty.alias("name", st);
  AST().AddConstructedType(alias);

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Function) {
  Func("main", ast::VariableList(), ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  Program program(std::move(*this));
  EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Null_Function) {
  AST().AddFunction(nullptr);

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Invalid_Function) {
  Func("main", ast::VariableList{}, nullptr, ast::StatementList{},
       ast::FunctionDecorationList{});

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, IsValid_Invalid_UnknownVar) {
  Func("main", ast::VariableList{}, nullptr,
       ast::StatementList{
           create<ast::ReturnStatement>(Expr("unknown_ident")),
       },
       ast::FunctionDecorationList{});

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
  EXPECT_NE(program.Diagnostics().count(), 0u);
}

TEST_F(ProgramTest, IsValid_GeneratesError) {
  AST().AddGlobalVariable(nullptr);

  Program program(std::move(*this));
  EXPECT_FALSE(program.IsValid());
  EXPECT_EQ(program.Diagnostics().count(), 1u);
  EXPECT_EQ(program.Diagnostics().error_count(), 1u);
  EXPECT_EQ(program.Diagnostics().begin()->message,
            "invalid program generated");
}
}  // namespace
}  // namespace tint
