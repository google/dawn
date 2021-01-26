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
#include "src/ast/discard_statement.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/access_control_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/struct_type.h"
#include "src/type/void_type.h"
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
  auto* func = Func("a_func", {}, ty.void_, ast::StatementList{},
                    ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func));
  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Function_Terminator_Return) {
  auto* func = Func("a_func", {}, ty.void_,
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func));
  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Function_Terminator_ReturnValue) {
  auto* var_a = Var("a", ast::StorageClass::kPrivate, ty.f32);
  td.RegisterVariableForTesting(var_a);

  auto* func = Func("a_func", {}, ty.void_,
                    ast::StatementList{create<ast::ReturnStatement>(Expr("a"))},
                    ast::FunctionDecorationList{});

  ASSERT_TRUE(td.DetermineFunction(func)) << td.error();

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "a"
OpName %7 "a_func"
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
  auto* func = Func("a_func", {}, ty.void_,
                    ast::StatementList{
                        create<ast::DiscardStatement>(),
                    },
                    ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func));
  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpKill
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Function_WithParams) {
  ast::VariableList params = {Var("a", ast::StorageClass::kFunction, ty.f32),
                              Var("b", ast::StorageClass::kFunction, ty.i32)};

  auto* func = Func("a_func", params, ty.f32,
                    ast::StatementList{create<ast::ReturnStatement>(Expr("a"))},
                    ast::FunctionDecorationList{});

  td.RegisterVariableForTesting(func->params()[0]);
  td.RegisterVariableForTesting(func->params()[1]);
  EXPECT_TRUE(td.DetermineFunction(func));

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func));
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
%8 = OpLoad %2 %5
OpReturnValue %8
OpFunctionEnd
)") << DumpBuilder(b);
}

TEST_F(BuilderTest, Function_WithBody) {
  auto* func = Func("a_func", {}, ty.void_,
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func));
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
  auto* func = Func("a_func", {}, ty.void_, ast::StatementList{},
                    ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

TEST_F(BuilderTest, FunctionType_DeDuplicate) {
  auto* func1 = Func("a_func", {}, ty.void_, ast::StatementList{},
                     ast::FunctionDecorationList{});
  auto* func2 = Func("b_func", {}, ty.void_, ast::StatementList{},
                     ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func1));
  ASSERT_TRUE(b.GenerateFunction(func2));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

// https://crbug.com/tint/297
TEST_F(BuilderTest, Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   [[offset(0)]] d : f32;
  // };
  // [[binding(0), group(0)]] var<storage> data : Data;
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

  ast::StructDecorationList s_decos;
  s_decos.push_back(create<ast::StructBlockDecoration>());

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("d", ty.f32, {MemberOffset(0)})}, s_decos);

  auto* s = ty.struct_("Data", str);
  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  auto* data_var = Var("data", ast::StorageClass::kStorage, &ac, nullptr,
                       ast::VariableDecorationList{
                           create<ast::BindingDecoration>(0),
                           create<ast::GroupDecoration>(0),
                       });

  mod->AST().AddConstructedType(s);

  td.RegisterVariableForTesting(data_var);
  mod->AST().AddGlobalVariable(data_var);

  {
    auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                    MemberAccessor("data", "d"), ast::VariableDecorationList{});

    auto* func =
        Func("a", ast::VariableList{}, ty.void_,
             ast::StatementList{
                 create<ast::VariableDeclStatement>(var),
                 create<ast::ReturnStatement>(),
             },
             ast::FunctionDecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

    mod->Functions().Add(func);
  }

  {
    auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                    MemberAccessor("data", "d"), ast::VariableDecorationList{});

    auto* func =
        Func("b", ast::VariableList{}, ty.void_,
             ast::StatementList{
                 create<ast::VariableDeclStatement>(var),
                 create<ast::ReturnStatement>(),
             },
             ast::FunctionDecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

    mod->Functions().Add(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build());
  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %7 "a"
OpEntryPoint GLCompute %17 "b"
OpExecutionMode %7 LocalSize 1 1 1
OpExecutionMode %17 LocalSize 1 1 1
OpName %3 "Data"
OpMemberName %3 0 "d"
OpName %1 "data"
OpName %7 "a"
OpName %14 "v"
OpName %17 "b"
OpName %21 "v"
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
