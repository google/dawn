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

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

TEST_F(SpvGeneratorImplTest, Construct_Vector) {
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({
        b.FunctionParam(ty.i32()),
        b.FunctionParam(ty.i32()),
        b.FunctionParam(ty.i32()),
        b.FunctionParam(ty.i32()),
    });

    b.With(func->Block(), [&] { b.Return(func, b.Construct(ty.vec4<i32>(), func->Params())); });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%3 = OpTypeInt 32 1
%2 = OpTypeVector %3 4
%8 = OpTypeFunction %2 %3 %3 %3 %3
%1 = OpFunction %2 None %8
%4 = OpFunctionParameter %3
%5 = OpFunctionParameter %3
%6 = OpFunctionParameter %3
%7 = OpFunctionParameter %3
%9 = OpLabel
%10 = OpCompositeConstruct %2 %4 %5 %6 %7
OpReturnValue %10
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Construct_Matrix) {
    auto* func = b.Function("foo", ty.mat3x4<f32>());
    func->SetParams({
        b.FunctionParam(ty.vec4<f32>()),
        b.FunctionParam(ty.vec4<f32>()),
        b.FunctionParam(ty.vec4<f32>()),
    });

    b.With(func->Block(), [&] { b.Return(func, b.Construct(ty.mat3x4<f32>(), func->Params())); });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%4 = OpTypeFloat 32
%3 = OpTypeVector %4 4
%2 = OpTypeMatrix %3 3
%8 = OpTypeFunction %2 %3 %3 %3
%1 = OpFunction %2 None %8
%5 = OpFunctionParameter %3
%6 = OpFunctionParameter %3
%7 = OpFunctionParameter %3
%9 = OpLabel
%10 = OpCompositeConstruct %2 %5 %6 %7
OpReturnValue %10
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Construct_Array) {
    auto* func = b.Function("foo", ty.array<f32, 4>());
    func->SetParams({
        b.FunctionParam(ty.f32()),
        b.FunctionParam(ty.f32()),
        b.FunctionParam(ty.f32()),
        b.FunctionParam(ty.f32()),
    });

    b.With(func->Block(), [&] { b.Return(func, b.Construct(ty.array<f32, 4>(), func->Params())); });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %2 ArrayStride 4
%3 = OpTypeFloat 32
%5 = OpTypeInt 32 0
%4 = OpConstant %5 4
%2 = OpTypeArray %3 %4
%10 = OpTypeFunction %2 %3 %3 %3 %3
%1 = OpFunction %2 None %10
%6 = OpFunctionParameter %3
%7 = OpFunctionParameter %3
%8 = OpFunctionParameter %3
%9 = OpFunctionParameter %3
%11 = OpLabel
%12 = OpCompositeConstruct %2 %6 %7 %8 %9
OpReturnValue %12
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Construct_Struct) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.i32()},
                                                   {mod.symbols.Register("b"), ty.u32()},
                                                   {mod.symbols.Register("c"), ty.vec4<f32>()},
                                               });

    auto* func = b.Function("foo", str);
    func->SetParams({
        b.FunctionParam(ty.i32()),
        b.FunctionParam(ty.u32()),
        b.FunctionParam(ty.vec4<f32>()),
    });

    b.With(func->Block(), [&] { b.Return(func, b.Construct(str, func->Params())); });

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpMemberName %2 0 "a"
OpMemberName %2 1 "b"
OpMemberName %2 2 "c"
OpName %2 "MyStruct"
OpMemberDecorate %2 0 Offset 0
OpMemberDecorate %2 1 Offset 4
OpMemberDecorate %2 2 Offset 16
%3 = OpTypeInt 32 1
%4 = OpTypeInt 32 0
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 4
%2 = OpTypeStruct %3 %4 %5
%10 = OpTypeFunction %2 %3 %4 %5
%1 = OpFunction %2 None %10
%7 = OpFunctionParameter %3
%8 = OpFunctionParameter %4
%9 = OpFunctionParameter %5
%11 = OpLabel
%12 = OpCompositeConstruct %2 %7 %8 %9
OpReturnValue %12
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
