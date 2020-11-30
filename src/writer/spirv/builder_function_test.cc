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
#include "src/ast/discard_statement.h"
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
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Function_Empty) {
  ast::type::Void void_type;
  ast::Function func("a_func", {}, &void_type, create<ast::BlockStatement>());

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

TEST_F(BuilderTest, Function_Terminator_Return) {
  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());

  ast::Function func("a_func", {}, &void_type, body);

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

TEST_F(BuilderTest, Function_Terminator_ReturnValue) {
  ast::type::Void void_type;
  ast::type::F32 f32;

  auto* var_a = create<ast::Variable>("a", ast::StorageClass::kPrivate, &f32);
  td.RegisterVariableForTesting(var_a);

  auto* body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::IdentifierExpression>("a")));
  ASSERT_TRUE(td.DetermineResultType(body)) << td.error();

  ast::Function func("a_func", {}, &void_type, body);

  ASSERT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "tint_61"
OpName %7 "tint_615f66756e63"
%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%7 = OpFunction %6 None %5
%8 = OpLabel
%9 = OpLoad %3 %1
OpReturnValue %9
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Function_Terminator_Discard) {
  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());

  ast::Function func("a_func", {}, &void_type, body);

  ASSERT_TRUE(b.GenerateFunction(&func));
  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpKill
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Function_WithParams) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::VariableList params;
  auto* var_a = create<ast::Variable>("a", ast::StorageClass::kFunction, &f32);
  var_a->set_is_const(true);
  params.push_back(var_a);
  auto* var_b = create<ast::Variable>("b", ast::StorageClass::kFunction, &i32);
  var_b->set_is_const(true);
  params.push_back(var_b);

  auto* body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::IdentifierExpression>("a")));
  ast::Function func("a_func", params, &f32, body);

  td.RegisterVariableForTesting(func.params()[0]);
  td.RegisterVariableForTesting(func.params()[1]);
  EXPECT_TRUE(td.DetermineFunction(&func));

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
  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());

  ast::Function func("a_func", {}, &void_type, body);

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
  ast::type::Void void_type;
  ast::Function func("a_func", {}, &void_type, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

TEST_F(BuilderTest, FunctionType_DeDuplicate) {
  ast::type::Void void_type;
  ast::Function func1("a_func", {}, &void_type, create<ast::BlockStatement>());
  ast::Function func2("b_func", {}, &void_type, create<ast::BlockStatement>());

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

  ast::type::Void void_type;
  ast::type::F32 f32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("d", &f32, a_deco));

  ast::StructDecorationList s_decos;
  s_decos.push_back(create<ast::StructBlockDecoration>(Source{}));

  auto* str = create<ast::Struct>(s_decos, members);

  ast::type::Struct s("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, &s);

  auto* data_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("data", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(0, Source{}));
  data_var->set_decorations(decos);

  mod->AddConstructedType(&s);

  td.RegisterVariableForTesting(data_var);
  mod->AddGlobalVariable(data_var);

  {
    ast::VariableList params;
    auto* var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
    var->set_constructor(create<ast::MemberAccessorExpression>(
        create<ast::IdentifierExpression>("data"),
        create<ast::IdentifierExpression>("d")));

    auto* body = create<ast::BlockStatement>();
    body->append(create<ast::VariableDeclStatement>(var));
    body->append(create<ast::ReturnStatement>());

    auto* func = create<ast::Function>("a", params, &void_type, body);
    func->add_decoration(
        create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}));

    mod->AddFunction(func);
  }

  {
    ast::VariableList params;
    auto* var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
    var->set_constructor(create<ast::MemberAccessorExpression>(
        create<ast::IdentifierExpression>("data"),
        create<ast::IdentifierExpression>("d")));

    auto* body = create<ast::BlockStatement>();
    body->append(create<ast::VariableDeclStatement>(var));
    body->append(create<ast::ReturnStatement>());

    auto* func = create<ast::Function>("b", params, &void_type, body);
    func->add_decoration(
        create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}));

    mod->AddFunction(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();

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
OpName %14 "tint_76"
OpName %17 "tint_62"
OpName %21 "tint_76"
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
%15 = OpTypePointer Function %4
%16 = OpConstantNull %4
%7 = OpFunction %6 None %5
%8 = OpLabel
%14 = OpVariable %15 Function %16
%12 = OpAccessChain %11 %1 %10
%13 = OpLoad %4 %12
OpStore %14 %13
OpReturn
OpFunctionEnd
%17 = OpFunction %6 None %5
%18 = OpLabel
%21 = OpVariable %15 Function %16
%19 = OpAccessChain %11 %1 %10
%20 = OpLoad %4 %19
OpStore %21 %20
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
