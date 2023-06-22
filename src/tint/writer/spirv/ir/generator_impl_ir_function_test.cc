// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Function_Empty) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

// Test that we do not emit the same function type more than once.
TEST_F(SpvGeneratorImplTest, Function_DeduplicateType) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    generator_.EmitFunction(func);
    generator_.EmitFunction(func);
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeVoid
%3 = OpTypeFunction %2
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Compute) {
    auto* func =
        b.Function("main", ty.void_(), ir::Function::PipelineStage::kCompute, {{32, 4, 1}});
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint GLCompute %1 "main"
OpExecutionMode %1 LocalSize 32 4 1
OpName %1 "main"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Fragment) {
    auto* func = b.Function("main", ty.void_(), ir::Function::PipelineStage::kFragment);
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpName %1 "main"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Vertex) {
    auto* func = b.Function("main", ty.void_(), ir::Function::PipelineStage::kVertex);
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint Vertex %1 "main"
OpName %1 "main"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Multiple) {
    auto* f1 = b.Function("main1", ty.void_(), ir::Function::PipelineStage::kCompute, {{32, 4, 1}});
    f1->Block()->Append(b.Return(f1));

    auto* f2 = b.Function("main2", ty.void_(), ir::Function::PipelineStage::kCompute, {{8, 2, 16}});
    f2->Block()->Append(b.Return(f2));

    auto* f3 = b.Function("main3", ty.void_(), ir::Function::PipelineStage::kFragment);
    f3->Block()->Append(b.Return(f3));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(f1);
    generator_.EmitFunction(f2);
    generator_.EmitFunction(f3);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint GLCompute %1 "main1"
OpEntryPoint GLCompute %5 "main2"
OpEntryPoint Fragment %7 "main3"
OpExecutionMode %1 LocalSize 32 4 1
OpExecutionMode %5 LocalSize 8 2 16
OpExecutionMode %7 OriginUpperLeft
OpName %1 "main1"
OpName %5 "main2"
OpName %7 "main3"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
%5 = OpFunction %2 None %3
%6 = OpLabel
OpReturn
OpFunctionEnd
%7 = OpFunction %2 None %3
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_ReturnValue) {
    auto* func = b.Function("foo", ty.i32());
    func->Block()->Append(b.Return(func, i32(42)));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeInt 32 1
%3 = OpTypeFunction %2
%5 = OpConstant %2 42
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturnValue %5
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_Parameters) {
    auto* i32 = ty.i32();
    auto* x = b.FunctionParam(i32);
    auto* y = b.FunctionParam(i32);
    auto* func = b.Function("foo", i32);
    func->SetParams({x, y});
    mod.SetName(x, "x");
    mod.SetName(y, "y");

    b.With(func->Block(), [&] {
        auto* result = b.Add(i32, x, y);
        b.Return(func, result);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpName %3 "x"
OpName %4 "y"
%2 = OpTypeInt 32 1
%5 = OpTypeFunction %2 %2 %2
%1 = OpFunction %2 None %5
%3 = OpFunctionParameter %2
%4 = OpFunctionParameter %2
%6 = OpLabel
%7 = OpIAdd %2 %3 %4
OpReturnValue %7
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_Call) {
    auto* i32_ty = ty.i32();
    auto* x = b.FunctionParam(i32_ty);
    auto* y = b.FunctionParam(i32_ty);
    auto* foo = b.Function("foo", i32_ty);
    foo->SetParams({x, y});

    b.With(foo->Block(), [&] {
        auto* result = b.Add(i32_ty, x, y);
        b.Return(foo, result);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.With(bar->Block(), [&] {
        b.Call(i32_ty, foo, i32(2), i32(3));
        b.Return(bar);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(foo);
    generator_.EmitFunction(bar);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpName %8 "bar"
%2 = OpTypeInt 32 1
%5 = OpTypeFunction %2 %2 %2
%9 = OpTypeVoid
%10 = OpTypeFunction %9
%13 = OpConstant %2 2
%14 = OpConstant %2 3
%1 = OpFunction %2 None %5
%3 = OpFunctionParameter %2
%4 = OpFunctionParameter %2
%6 = OpLabel
%7 = OpIAdd %2 %3 %4
OpReturnValue %7
OpFunctionEnd
%8 = OpFunction %9 None %10
%11 = OpLabel
%12 = OpFunctionCall %2 %1 %13 %14
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_Call_Void) {
    auto* foo = b.Function("foo", ty.void_());
    foo->Block()->Append(b.Return(foo));

    auto* bar = b.Function("bar", ty.void_());
    b.With(bar->Block(), [&] {
        b.Call(ty.void_(), foo, utils::Empty);
        b.Return(bar);
    });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(foo);
    generator_.EmitFunction(bar);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpName %5 "bar"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
%5 = OpFunction %2 None %3
%6 = OpLabel
%7 = OpFunctionCall %2 %1
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
