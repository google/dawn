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

#include "src/ast/module.h"

#include <sstream>
#include <utility>

#include "gmock/gmock.h"
#include "src/ast/function.h"
#include "src/ast/test_helper.h"
#include "src/ast/variable.h"
#include "src/type/alias_type.h"
#include "src/type/f32_type.h"
#include "src/type/struct_type.h"

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
                    ast::FunctionDecorationList{});

  Program program(std::move(*this));
  EXPECT_EQ(func,
            program.AST().Functions().Find(program.Symbols().Get("main")));
}

TEST_F(ModuleTest, LookupFunctionMissing) {
  Program program(std::move(*this));
  EXPECT_EQ(nullptr,
            program.AST().Functions().Find(program.Symbols().Get("Missing")));
}

TEST_F(ModuleTest, IsValid_Empty) {
  Program program(std::move(*this));
  EXPECT_TRUE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_GlobalVariable) {
  Global("var", StorageClass::kInput, ty.f32());
  Program program(std::move(*this));
  EXPECT_TRUE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Null_GlobalVariable) {
  AST().AddGlobalVariable(nullptr);
  Program program(std::move(*this));
  EXPECT_FALSE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Invalid_GlobalVariable) {
  Global("var", StorageClass::kInput, nullptr);
  Program program(std::move(*this));
  EXPECT_FALSE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Alias) {
  auto* alias = ty.alias("alias", ty.f32());
  AST().AddConstructedType(alias);
  Program program(std::move(*this));
  EXPECT_TRUE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Null_Alias) {
  AST().AddConstructedType(nullptr);
  Program program(std::move(*this));
  EXPECT_FALSE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Struct) {
  auto* st = ty.struct_("name", {});
  auto* alias = ty.alias("name", st);
  AST().AddConstructedType(alias);
  Program program(std::move(*this));
  EXPECT_TRUE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Struct_EmptyName) {
  auto* st = ty.struct_("", {});
  auto* alias = ty.alias("name", st);
  AST().AddConstructedType(alias);
  Program program(std::move(*this));
  EXPECT_FALSE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Function) {
  Func("main", VariableList(), ty.f32(), StatementList{},
       ast::FunctionDecorationList{});

  Program program(std::move(*this));
  EXPECT_TRUE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Null_Function) {
  AST().Functions().Add(nullptr);
  Program program(std::move(*this));
  EXPECT_FALSE(program.AST().IsValid());
}

TEST_F(ModuleTest, IsValid_Invalid_Function) {
  Func("main", VariableList{}, nullptr, StatementList{},
       ast::FunctionDecorationList{});

  Program program(std::move(*this));
  EXPECT_FALSE(program.AST().IsValid());
}

}  // namespace
}  // namespace ast
}  // namespace tint
