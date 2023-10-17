// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, Function_Empty) {
    Func("a_func", tint::Empty, ty.void_(), tint::Empty);

    Builder& b = Build();

    auto* func = program->AST().Functions()[0];
    ASSERT_TRUE(b.GenerateFunction(func));
    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpirvASTPrinterTest, Function_Terminator_Return) {
    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Return(),
         });

    Builder& b = Build();

    auto* func = program->AST().Functions()[0];
    ASSERT_TRUE(b.GenerateFunction(func));
    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpirvASTPrinterTest, Function_Terminator_ReturnValue) {
    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);

    Func("a_func", tint::Empty, ty.f32(), Vector{Return("a")}, tint::Empty);

    Builder& b = Build();

    auto* var_a = program->AST().GlobalVariables()[0];
    auto* func = program->AST().Functions()[0];

    ASSERT_TRUE(b.GenerateGlobalVariable(var_a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();
    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %1 "a"
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

TEST_F(SpirvASTPrinterTest, Function_Terminator_Discard) {
    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Discard(),
         });

    Builder& b = Build();

    auto* func = program->AST().Functions()[0];
    ASSERT_TRUE(b.GenerateFunction(func));
    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpKill
OpFunctionEnd
)");
}

TEST_F(SpirvASTPrinterTest, Function_WithParams) {
    Func("a_func",
         Vector{
             Param("a", ty.f32()),
             Param("b", ty.i32()),
         },
         ty.f32(), Vector{Return("a")}, tint::Empty);

    Builder& b = Build();

    auto* func = program->AST().Functions()[0];
    ASSERT_TRUE(b.GenerateFunction(func));
    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %4 "a_func"
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
)") << DumpModule(b.Module());
}

TEST_F(SpirvASTPrinterTest, Function_WithBody) {
    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Return(),
         });

    Builder& b = Build();

    auto* func = program->AST().Functions()[0];
    ASSERT_TRUE(b.GenerateFunction(func));
    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpirvASTPrinterTest, FunctionType) {
    Func("a_func", tint::Empty, ty.void_(), tint::Empty, tint::Empty);

    Builder& b = Build();

    auto* func = program->AST().Functions()[0];
    ASSERT_TRUE(b.GenerateFunction(func));
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

TEST_F(SpirvASTPrinterTest, FunctionType_DeDuplicate) {
    auto* func1 = Func("a_func", tint::Empty, ty.void_(), tint::Empty, tint::Empty);
    auto* func2 = Func("b_func", tint::Empty, ty.void_(), tint::Empty, tint::Empty);

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func1));
    ASSERT_TRUE(b.GenerateFunction(func2));
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

// https://crbug.com/tint/297
TEST_F(SpirvASTPrinterTest, Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
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

    auto* s = Structure("Data", Vector{Member("d", ty.f32())});

    GlobalVar("data", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
              Binding(0_a), Group(0_a));

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("a", tint::Empty, ty.void_(),
             Vector{
                 Decl(var),
                 Return(),
             },
             Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    }

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("b", tint::Empty, ty.void_(),
             Vector{
                 Decl(var),
                 Return(),
             },
             Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    }

    Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());
    EXPECT_EQ(DumpModule(b.Module()), R"(OpCapability Shader
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
}  // namespace tint::spirv::writer
