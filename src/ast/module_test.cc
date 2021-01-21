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
  EXPECT_EQ(mod->functions().size(), 0u);
}

TEST_F(ModuleTest, ToStrEmitsPreambleAndPostamble) {
  const auto str = mod->to_str();
  auto* const expected = "Module{\n}\n";
  EXPECT_EQ(str, expected);
}

TEST_F(ModuleTest, LookupFunction) {
  auto* func = Func("main", VariableList{}, ty.f32, StatementList{},
                    ast::FunctionDecorationList{});
  mod->AddFunction(func);
  EXPECT_EQ(func, mod->FindFunctionBySymbol(mod->RegisterSymbol("main")));
}

TEST_F(ModuleTest, LookupFunctionMissing) {
  EXPECT_EQ(nullptr, mod->FindFunctionBySymbol(mod->RegisterSymbol("Missing")));
}

TEST_F(ModuleTest, IsValid_Empty) {
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_GlobalVariable) {
  auto* var = Var("var", StorageClass::kInput, ty.f32);
  mod->AddGlobalVariable(var);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Null_GlobalVariable) {
  mod->AddGlobalVariable(nullptr);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Invalid_GlobalVariable) {
  auto* var = Var("var", StorageClass::kInput, nullptr);
  mod->AddGlobalVariable(var);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Alias) {
  auto* alias = ty.alias("alias", ty.f32);
  mod->AddConstructedType(alias);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Null_Alias) {
  mod->AddConstructedType(nullptr);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Struct) {
  auto* st = ty.struct_("name", {});
  auto* alias = ty.alias("name", st);
  mod->AddConstructedType(alias);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Struct_EmptyName) {
  auto* st = ty.struct_("", {});
  auto* alias = ty.alias("name", st);
  mod->AddConstructedType(alias);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Function) {
  auto* func = Func("main", VariableList(), ty.f32, StatementList{},
                    ast::FunctionDecorationList{});

  mod->AddFunction(func);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Null_Function) {
  mod->AddFunction(nullptr);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ModuleTest, IsValid_Invalid_Function) {
  auto* func = Func("main", VariableList{}, nullptr, StatementList{},
                    ast::FunctionDecorationList{});

  mod->AddFunction(func);
  EXPECT_FALSE(mod->IsValid());
}

}  // namespace
}  // namespace ast
}  // namespace tint
