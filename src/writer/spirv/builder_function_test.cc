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

#include <string>

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Function_Empty) {
  ast::type::VoidType void_type;
  ast::Function func("a_func", {}, &void_type);

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func));

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "a_func"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");

  ASSERT_GE(b.functions().size(), 1u);
  const auto& ret = b.functions()[0];
  EXPECT_EQ(DumpInstruction(ret.declaration()), R"(%3 = OpFunction %2 None %1
)");
}

TEST_F(BuilderTest, Function_WithParams) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::VariableList params;
  params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &f32));
  params.push_back(
      std::make_unique<ast::Variable>("b", ast::StorageClass::kFunction, &i32));

  ast::Function func("a_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::IdentifierExpression>("a")));
  func.set_body(std::move(body));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func));
  EXPECT_EQ(DumpBuilder(b), R"(OpName %4 "a_func"
OpName %5 "a"
OpName %6 "b"
%2 = OpTypeFloat 32
%3 = OpTypeInt 32 1
%1 = OpTypeFunction %2 %2 %3
%4 = OpFunction %2 None %1
%5 = OpFunctionParameter %2
%6 = OpFunctionParameter %3
%7 = OpLabel
OpReturnValue %5
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Function_WithBody) {
  ast::type::VoidType void_type;

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::Function func("a_func", {}, &void_type);
  func.set_body(std::move(body));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func));
  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, FunctionType) {
  ast::type::VoidType void_type;
  ast::Function func("a_func", {}, &void_type);

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

TEST_F(BuilderTest, FunctionType_DeDuplicate) {
  ast::type::VoidType void_type;
  ast::Function func1("a_func", {}, &void_type);
  ast::Function func2("b_func", {}, &void_type);

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func1));
  ASSERT_TRUE(b.GenerateFunction(&func2));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
