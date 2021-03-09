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

#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Function_Empty) {
  Func("a_func", {}, ty.void_(), ast::StatementList{},
       ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  auto* func = program->AST().Functions()[0];
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
  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  auto* func = program->AST().Functions()[0];
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
  Global("a", ty.f32(), ast::StorageClass::kPrivate);

  Func("a_func", {}, ty.void_(),
       ast::StatementList{create<ast::ReturnStatement>(Expr("a"))},
       ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  auto* var_a = program->AST().GlobalVariables()[0];
  auto* func = program->AST().Functions()[0];

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
  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           create<ast::DiscardStatement>(),
       },
       ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  auto* func = program->AST().Functions()[0];
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
  ast::VariableList params = {Var("a", ty.f32(), ast::StorageClass::kFunction),
                              Var("b", ty.i32(), ast::StorageClass::kFunction)};

  Func("a_func", params, ty.f32(),
       ast::StatementList{create<ast::ReturnStatement>(Expr("a"))},
       ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  auto* func = program->AST().Functions()[0];
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
  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  auto* func = program->AST().Functions()[0];
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
  Func("a_func", {}, ty.void_(), ast::StatementList{},
       ast::FunctionDecorationList{});

  spirv::Builder& b = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_TRUE(b.GenerateFunction(func));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

TEST_F(BuilderTest, FunctionType_DeDuplicate) {
  auto* func1 = Func("a_func", {}, ty.void_(), ast::StatementList{},
                     ast::FunctionDecorationList{});
  auto* func2 = Func("b_func", {}, ty.void_(), ast::StatementList{},
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
      ast::StructMemberList{Member("d", ty.f32(), {MemberOffset(0)})}, s_decos);

  auto* s = ty.struct_("Data", str);
  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("data", &ac, ast::StorageClass::kStorage, nullptr,
         ast::VariableDecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  AST().AddConstructedType(s);

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("a", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             create<ast::VariableDeclStatement>(var),
             create<ast::ReturnStatement>(),
         },
         ast::FunctionDecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute),
         });
  }

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("b", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             create<ast::VariableDeclStatement>(var),
             create<ast::ReturnStatement>(),
         },
         ast::FunctionDecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute),
         });
  }

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
