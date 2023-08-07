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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

TEST_F(SpirvWriterTest, Function_Empty) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we do not emit the same function type more than once.
TEST_F(SpirvWriterTest, Function_DeduplicateType) {
    auto* func_a = b.Function("func_a", ty.void_());
    b.Append(func_a->Block(), [&] {  //
        b.Return(func_a);
    });
    auto* func_b = b.Function("func_b", ty.void_());
    b.Append(func_b->Block(), [&] {  //
        b.Return(func_b);
    });
    auto* func_c = b.Function("func_c", ty.void_());
    b.Append(func_c->Block(), [&] {  //
        b.Return(func_c);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function func_a
     %func_a = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function func_b
     %func_b = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function func_c
     %func_c = OpFunction %void None %3
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Compute) {
    auto* func =
        b.Function("main", ty.void_(), ir::Function::PipelineStage::kCompute, {{32, 4, 1}});
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 32 4 1

               ; Debug Information
               OpName %main "main"  ; id %1

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Fragment) {
    auto* func = b.Function("main", ty.void_(), ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %main "main"  ; id %1

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Vertex) {
    auto* func = b.Function("main", ty.void_(), ir::Function::PipelineStage::kVertex);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint Vertex %main "main"

               ; Debug Information
               OpName %main "main"  ; id %1

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Multiple) {
    auto* f1 = b.Function("main1", ty.void_(), ir::Function::PipelineStage::kCompute, {{32, 4, 1}});
    b.Append(f1->Block(), [&] {  //
        b.Return(f1);
    });

    auto* f2 = b.Function("main2", ty.void_(), ir::Function::PipelineStage::kCompute, {{8, 2, 16}});
    b.Append(f2->Block(), [&] {  //
        b.Return(f2);
    });

    auto* f3 = b.Function("main3", ty.void_(), ir::Function::PipelineStage::kFragment);
    b.Append(f3->Block(), [&] {  //
        b.Return(f3);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint GLCompute %main1 "main1"
               OpEntryPoint GLCompute %main2 "main2"
               OpEntryPoint Fragment %main3 "main3"
               OpExecutionMode %main1 LocalSize 32 4 1
               OpExecutionMode %main2 LocalSize 8 2 16
               OpExecutionMode %main3 OriginUpperLeft

               ; Debug Information
               OpName %main1 "main1"  ; id %1
               OpName %main2 "main2"  ; id %5
               OpName %main3 "main3"  ; id %7

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main1
      %main1 = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function main2
      %main2 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function main3
      %main3 = OpFunction %void None %3
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_ReturnValue) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {  //
        b.Return(func, 42_i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %3 = OpTypeFunction %int
     %int_42 = OpConstant %int 42
       %void = OpTypeVoid
          %8 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %3
          %4 = OpLabel
               OpReturnValue %int_42
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_Parameters) {
    auto* i32 = ty.i32();
    auto* x = b.FunctionParam("x", i32);
    auto* y = b.FunctionParam("y", i32);
    auto* func = b.Function("foo", i32);
    func->SetParams({x, y});

    b.Append(func->Block(), [&] {
        auto* result = b.Add(i32, x, y);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %5 = OpTypeFunction %int %int %int
       %void = OpTypeVoid
         %10 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %5
          %x = OpFunctionParameter %int
          %y = OpFunctionParameter %int
          %6 = OpLabel
          %7 = OpIAdd %int %x %y
               OpReturnValue %7
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_Call) {
    auto* i32 = ty.i32();
    auto* x = b.FunctionParam("x", i32);
    auto* y = b.FunctionParam("y", i32);
    auto* foo = b.Function("foo", i32);
    foo->SetParams({x, y});

    b.Append(foo->Block(), [&] {
        auto* result = b.Add(i32, x, y);
        b.Return(foo, result);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        auto* result = b.Call(i32, foo, 2_i, 3_i);
        b.Return(bar);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpFunctionCall %int %foo %int_2 %int_3");
}

TEST_F(SpirvWriterTest, Function_Call_Void) {
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        auto* result = b.Call(ty.void_(), foo);
        b.Return(bar);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpFunctionCall %void %foo");
}

}  // namespace
}  // namespace tint::spirv::writer
