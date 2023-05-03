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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Function_Empty) {
    Func("a_func", utils::Empty, ty.void_(), utils::Empty);

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
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

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
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);

    Func("a_func", utils::Empty, ty.f32(), utils::Vector{Return("a")}, utils::Empty);

    spirv::Builder& b = Build();

    auto* var_a = program->AST().GlobalVariables()[0];
    auto* func = program->AST().Functions()[0];

    ASSERT_TRUE(b.GenerateGlobalVariable(var_a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "a"
OpName %6 "a_func"
%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeFunction %3
%6 = OpFunction %3 None %5
%7 = OpLabel
%8 = OpLoad %3 %1
OpReturnValue %8
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Function_Terminator_Discard) {
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Discard(),
         });

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
    Func("a_func",
         utils::Vector{
             Param("a", ty.f32()),
             Param("b", ty.i32()),
         },
         ty.f32(), utils::Vector{Return("a")}, utils::Empty);

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
OpReturnValue %5
OpFunctionEnd
)") << DumpBuilder(b);
}

TEST_F(BuilderTest, Function_WithBody) {
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

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
    Func("a_func", utils::Empty, ty.void_(), utils::Empty, utils::Empty);

    spirv::Builder& b = Build();

    auto* func = program->AST().Functions()[0];
    ASSERT_TRUE(b.GenerateFunction(func));
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

TEST_F(BuilderTest, FunctionType_DeDuplicate) {
    auto* func1 = Func("a_func", utils::Empty, ty.void_(), utils::Empty, utils::Empty);
    auto* func2 = Func("b_func", utils::Empty, ty.void_(), utils::Empty, utils::Empty);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func1));
    ASSERT_TRUE(b.GenerateFunction(func2));
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

// https://crbug.com/tint/297
TEST_F(BuilderTest, Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
    // struct Data {
    //   d : f32;
    // };
    // @binding(0) @group(0) var<storage> data : Data;
    //
    // @compute @workgroup_size(1)
    // fn a() {
    //   return;
    // }
    //
    // @compute @workgroup_size(1)
    // fn b() {
    //   return;
    // }

    auto* s = Structure("Data", utils::Vector{Member("d", ty.f32())});

    GlobalVar("data", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(0_a));

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("a", utils::Empty, ty.void_(),
             utils::Vector{
                 Decl(var),
                 Return(),
             },
             utils::Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    }

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("b", utils::Empty, ty.void_(),
             utils::Vector{
                 Decl(var),
                 Return(),
             },
             utils::Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    }

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());
    EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %8 "a"
OpEntryPoint GLCompute %18 "b"
OpExecutionMode %8 LocalSize 1 1 1
OpExecutionMode %18 LocalSize 1 1 1
OpName %3 "data_block"
OpMemberName %3 0 "inner"
OpName %4 "Data"
OpMemberName %4 0 "d"
OpName %1 "data"
OpName %8 "a"
OpName %15 "v"
OpName %18 "b"
OpName %22 "v"
OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %4 0 Offset 0
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
%5 = OpTypeFloat 32
%4 = OpTypeStruct %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 0
%12 = OpTypePointer StorageBuffer %5
%16 = OpTypePointer Function %5
%17 = OpConstantNull %5
%8 = OpFunction %7 None %6
%9 = OpLabel
%15 = OpVariable %16 Function %17
%13 = OpAccessChain %12 %1 %11 %11
%14 = OpLoad %5 %13
OpStore %15 %14
OpReturn
OpFunctionEnd
%18 = OpFunction %7 None %6
%19 = OpLabel
%22 = OpVariable %16 Function %17
%20 = OpAccessChain %12 %1 %11 %11
%21 = OpLoad %5 %20
OpStore %22 %21
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
