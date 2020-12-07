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
#include "src/ast/type/alias_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {
namespace {

using ModuleTest = TestHelper;

TEST_F(ModuleTest, Creation) {
  Module m;

  EXPECT_EQ(m.functions().size(), 0u);
}

TEST_F(ModuleTest, ToStrEmitsPreambleAndPostamble) {
  Module m;
  const auto str = m.to_str();
  auto* const expected = "Module{\n}\n";
  EXPECT_EQ(str, expected);
}

TEST_F(ModuleTest, LookupFunction) {
  type::F32 f32;
  Module m;

  auto* func =
      create<Function>(Source{}, "main", VariableList{}, &f32,
                       create<BlockStatement>(), ast::FunctionDecorationList{});
  m.AddFunction(func);
  EXPECT_EQ(func, m.FindFunctionByName("main"));
}

TEST_F(ModuleTest, LookupFunctionMissing) {
  Module m;
  EXPECT_EQ(nullptr, m.FindFunctionByName("Missing"));
}

TEST_F(ModuleTest, IsValid_Empty) {
  Module m;
  EXPECT_TRUE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_GlobalVariable) {
  type::F32 f32;
  auto* var = create<Variable>(Source{}, "var", StorageClass::kInput, &f32);

  Module m;
  m.AddGlobalVariable(var);
  EXPECT_TRUE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Null_GlobalVariable) {
  Module m;
  m.AddGlobalVariable(nullptr);
  EXPECT_FALSE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Invalid_GlobalVariable) {
  auto* var = create<Variable>(Source{}, "var", StorageClass::kInput, nullptr);

  Module m;
  m.AddGlobalVariable(var);
  EXPECT_FALSE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Alias) {
  type::F32 f32;
  type::Alias alias("alias", &f32);

  Module m;
  m.AddConstructedType(&alias);
  EXPECT_TRUE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Null_Alias) {
  Module m;
  m.AddConstructedType(nullptr);
  EXPECT_FALSE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Struct) {
  type::F32 f32;
  type::Struct st("name", {});
  type::Alias alias("name", &st);

  Module m;
  m.AddConstructedType(&alias);
  EXPECT_TRUE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Struct_EmptyName) {
  type::F32 f32;
  type::Struct st("", {});
  type::Alias alias("name", &st);

  Module m;
  m.AddConstructedType(&alias);
  EXPECT_FALSE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Function) {
  type::F32 f32;
  auto* func =
      create<Function>(Source{}, "main", VariableList(), &f32,
                       create<BlockStatement>(), ast::FunctionDecorationList{});

  Module m;
  m.AddFunction(func);
  EXPECT_TRUE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Null_Function) {
  Module m;
  m.AddFunction(nullptr);
  EXPECT_FALSE(m.IsValid());
}

TEST_F(ModuleTest, IsValid_Invalid_Function) {
  VariableList p;
  auto* func = create<Function>(Source{}, "", p, nullptr, nullptr,
                                ast::FunctionDecorationList{});

  Module m;
  m.AddFunction(func);
  EXPECT_FALSE(m.IsValid());
}

}  // namespace
}  // namespace ast
}  // namespace tint
