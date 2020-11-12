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
#include "src/ast/decorated_variable.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/context.h"
#include "src/type_determiner.h"
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

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "tint_615f66756e63"
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
  auto var_a =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kFunction, &f32);
  var_a->set_is_const(true);
  params.push_back(std::move(var_a));
  auto var_b =
      std::make_unique<ast::Variable>("b", ast::StorageClass::kFunction, &i32);
  var_b->set_is_const(true);
  params.push_back(std::move(var_b));

  ast::Function func("a_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::IdentifierExpression>("a")));
  func.set_body(std::move(body));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(func.params()[0].get());
  td.RegisterVariableForTesting(func.params()[1].get());
  EXPECT_TRUE(td.DetermineFunction(&func));

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func));
  EXPECT_EQ(DumpBuilder(b), R"(OpName %4 "tint_615f66756e63"
OpName %5 "tint_61"
OpName %6 "tint_62"
%2 = OpTypeFloat 32
%3 = OpTypeInt 32 1
%1 = OpTypeFunction %2 %2 %3
%4 = OpFunction %2 None %1
%5 = OpFunctionParameter %2
%6 = OpFunctionParameter %3
%7 = OpLabel
OpReturnValue %5
OpFunctionEnd
)") << DumpBuilder(b);
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
  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "tint_615f66756e63"
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

// https://crbug.com/tint/297
TEST_F(BuilderTest, Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   [[offset(0)]] d : f32;
  // };
  // [[binding(0), set(0)]] var<storage_buffer> data : Data;
  //
  // [[stage(compute)]]
  // fn a() -> void {
  //   return;
  // }
  //
  // [[stage(compute)]]
  // fn b() -> void {
  //   return;
  // }

  ast::type::VoidType void_type;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(
      std::make_unique<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(
      std::make_unique<ast::StructMember>("d", &f32, std::move(a_deco)));

  ast::StructDecorationList s_decos;
  s_decos.push_back(std::make_unique<ast::StructBlockDecoration>(Source{}));

  auto str =
      std::make_unique<ast::Struct>(std::move(s_decos), std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadWrite, &s);

  auto data_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "data", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0, Source{}));
  decos.push_back(std::make_unique<ast::SetDecoration>(0, Source{}));
  data_var->set_decorations(std::move(decos));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  mod.AddConstructedType(&s);

  td.RegisterVariableForTesting(data_var.get());
  mod.AddGlobalVariable(std::move(data_var));

  {
    ast::VariableList params;
    auto func =
        std::make_unique<ast::Function>("a", std::move(params), &void_type);
    func->add_decoration(std::make_unique<ast::StageDecoration>(
        ast::PipelineStage::kCompute, Source{}));

    auto var = std::make_unique<ast::Variable>(
        "v", ast::StorageClass::kFunction, &f32);
    var->set_constructor(std::make_unique<ast::MemberAccessorExpression>(
        std::make_unique<ast::IdentifierExpression>("data"),
        std::make_unique<ast::IdentifierExpression>("d")));

    auto body = std::make_unique<ast::BlockStatement>();
    body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
    body->append(std::make_unique<ast::ReturnStatement>());
    func->set_body(std::move(body));

    mod.AddFunction(std::move(func));
  }

  {
    ast::VariableList params;
    auto func =
        std::make_unique<ast::Function>("b", std::move(params), &void_type);
    func->add_decoration(std::make_unique<ast::StageDecoration>(
        ast::PipelineStage::kCompute, Source{}));

    auto var = std::make_unique<ast::Variable>(
        "v", ast::StorageClass::kFunction, &f32);
    var->set_constructor(std::make_unique<ast::MemberAccessorExpression>(
        std::make_unique<ast::IdentifierExpression>("data"),
        std::make_unique<ast::IdentifierExpression>("d")));

    auto body = std::make_unique<ast::BlockStatement>();
    body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
    body->append(std::make_unique<ast::ReturnStatement>());
    func->set_body(std::move(body));

    mod.AddFunction(std::move(func));
  }

  ASSERT_TRUE(td.Determine()) << td.error();

  Builder b(&mod);
  ASSERT_TRUE(b.Build());
  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %7 "a"
OpEntryPoint GLCompute %17 "b"
OpExecutionMode %7 LocalSize 1 1 1
OpExecutionMode %17 LocalSize 1 1 1
OpName %3 "tint_44617461"
OpMemberName %3 0 "tint_64"
OpName %1 "tint_64617461"
OpName %7 "tint_61"
OpName %13 "tint_76"
OpName %17 "tint_62"
OpName %20 "tint_76"
OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
%4 = OpTypeFloat 32
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%9 = OpTypeInt 32 0
%10 = OpConstant %9 0
%11 = OpTypePointer StorageBuffer %4
%14 = OpTypePointer Function %4
%15 = OpConstantNull %4
%7 = OpFunction %6 None %5
%8 = OpLabel
%13 = OpVariable %14 Function %15
%12 = OpAccessChain %11 %1 %10
%16 = OpLoad %4 %12
OpStore %13 %16
OpReturn
OpFunctionEnd
%17 = OpFunction %6 None %5
%18 = OpLabel
%20 = OpVariable %14 Function %15
%19 = OpAccessChain %11 %1 %10
%21 = OpLoad %4 %19
OpStore %20 %21
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
