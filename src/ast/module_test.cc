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

#include "gtest/gtest-spi.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using ModuleTest = TestHelper;

TEST_F(ModuleTest, Creation) {
  EXPECT_EQ(Program(std::move(*this)).AST().Functions().size(), 0u);
}

TEST_F(ModuleTest, ToStrEmitsPreambleAndPostamble) {
  const auto str = Program(std::move(*this)).to_str();
  auto* const expected = "Module{\n}\n";
  EXPECT_EQ(str, expected);
}

TEST_F(ModuleTest, LookupFunction) {
  auto* func = Func("main", VariableList{}, ty.f32(), StatementList{},
                    ast::DecorationList{});

  Program program(std::move(*this));
  EXPECT_EQ(func,
            program.AST().Functions().Find(program.Symbols().Get("main")));
}

TEST_F(ModuleTest, LookupFunctionMissing) {
  Program program(std::move(*this));
  EXPECT_EQ(nullptr,
            program.AST().Functions().Find(program.Symbols().Get("Missing")));
}

TEST_F(ModuleTest, Assert_Null_GlobalVariable) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder builder;
        builder.AST().AddGlobalVariable(nullptr);
      },
      "internal compiler error");
}

TEST_F(ModuleTest, Assert_Invalid_GlobalVariable) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder builder;
        builder.Global("var", nullptr, StorageClass::kInput);
      },
      "internal compiler error");
}

TEST_F(ModuleTest, Assert_Null_ConstructedType) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder builder;
        builder.AST().AddConstructedType(nullptr);
      },
      "internal compiler error");
}

TEST_F(ModuleTest, Assert_DifferentProgramID_Function) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.AST().AddFunction(b2.create<ast::Function>(
            b2.Symbols().Register("func"), VariableList{}, b2.ty.f32(),
            b2.Block(), DecorationList{}, DecorationList{}));
      },
      "internal compiler error");
}

TEST_F(ModuleTest, Assert_DifferentProgramID_GlobalVariable) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.AST().AddGlobalVariable(
            b2.Var("var", b2.ty.i32(), ast::StorageClass::kPrivate));
      },
      "internal compiler error");
}

TEST_F(ModuleTest, Assert_Null_Function) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder builder;
        builder.AST().AddFunction(nullptr);
      },
      "internal compiler error");
}

}  // namespace
}  // namespace ast
}  // namespace tint
