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

using SpvGeneratorImplTest_Access = SpvGeneratorImplTest;

TEST_F(SpvGeneratorImplTest_Access, Array_Value_ConstantIndex) {
    auto* arr_val = b.FunctionParam(ty.array(ty.i32(), 4));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arr_val});

    auto sb = b.With(func->StartTarget());
    sb.Access(ty.i32(), arr_val, 1_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %3 ArrayStride 4
%2 = OpTypeVoid
%4 = OpTypeInt 32 1
%6 = OpTypeInt 32 0
%5 = OpConstant %6 4
%3 = OpTypeArray %4 %5
%8 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %8
%7 = OpFunctionParameter %3
%9 = OpLabel
%10 = OpCompositeExtract %4 %7 1
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Array_Pointer_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());

    auto sb = b.With(func->StartTarget());
    auto* arr_var = sb.Var(ty.ptr<function, array<i32, 4>>());
    sb.Access(ty.ptr<function, i32>(), arr_var, 1_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %7 ArrayStride 4
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeInt 32 1
%10 = OpTypeInt 32 0
%9 = OpConstant %10 4
%7 = OpTypeArray %8 %9
%6 = OpTypePointer Function %7
%12 = OpTypePointer Function %8
%13 = OpConstant %10 1
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%11 = OpAccessChain %12 %5 %13
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Array_Pointer_DynamicIndex) {
    auto* func = b.Function("foo", ty.void_());

    auto sb = b.With(func->StartTarget());
    auto* idx_var = sb.Var(ty.ptr<function, i32>());
    auto* idx = sb.Load(idx_var);
    auto* arr_var = sb.Var(ty.ptr<function, array<i32, 4>>());
    sb.Access(ty.ptr<function, i32>(), arr_var, idx);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %11 ArrayStride 4
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%13 = OpTypeInt 32 0
%12 = OpConstant %13 4
%11 = OpTypeArray %7 %12
%10 = OpTypePointer Function %11
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpVariable %10 Function
%8 = OpLoad %7 %5
%14 = OpAccessChain %6 %9 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Matrix_Value_ConstantIndex) {
    auto* mat_val = b.FunctionParam(ty.mat2x2(ty.f32()));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({mat_val});

    auto sb = b.With(func->StartTarget());
    sb.Access(ty.vec2(ty.f32()), mat_val, 1_u);
    sb.Access(ty.f32(), mat_val, 1_u, 0_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%5 = OpTypeFloat 32
%4 = OpTypeVector %5 2
%3 = OpTypeMatrix %4 2
%7 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %7
%6 = OpFunctionParameter %3
%8 = OpLabel
%9 = OpCompositeExtract %4 %6 1
%10 = OpCompositeExtract %5 %6 1 0
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Matrix_Pointer_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());

    auto sb = b.With(func->StartTarget());
    auto* mat_var = sb.Var(ty.ptr<function, mat2x2<f32>>());
    sb.Access(ty.ptr<function, vec2<f32>>(), mat_var, 1_u);
    sb.Access(ty.ptr<function, f32>(), mat_var, 1_u, 0_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 2
%7 = OpTypeMatrix %8 2
%6 = OpTypePointer Function %7
%11 = OpTypePointer Function %8
%13 = OpTypeInt 32 0
%12 = OpConstant %13 1
%15 = OpTypePointer Function %9
%16 = OpConstant %13 0
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%10 = OpAccessChain %11 %5 %12
%14 = OpAccessChain %15 %5 %12 %16
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Matrix_Pointer_DynamicIndex) {
    auto* func = b.Function("foo", ty.void_());

    auto sb = b.With(func->StartTarget());
    auto* idx_var = sb.Var(ty.ptr<function, i32>());
    auto* idx = sb.Load(idx_var);
    auto* mat_var = sb.Var(ty.ptr<function, mat2x2<f32>>());
    sb.Access(ty.ptr<function, vec2<f32>>(), mat_var, idx);
    sb.Access(ty.ptr<function, f32>(), mat_var, idx, idx);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%13 = OpTypeFloat 32
%12 = OpTypeVector %13 2
%11 = OpTypeMatrix %12 2
%10 = OpTypePointer Function %11
%15 = OpTypePointer Function %12
%17 = OpTypePointer Function %13
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpVariable %10 Function
%8 = OpLoad %7 %5
%14 = OpAccessChain %15 %9 %8
%16 = OpAccessChain %17 %9 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Value_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    auto* vec_val = b.FunctionParam(ty.vec4(ty.i32()));
    func->SetParams({vec_val});

    auto sb = b.With(func->StartTarget());
    sb.Access(ty.i32(), vec_val, 1_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 4
%6 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %6
%5 = OpFunctionParameter %3
%7 = OpLabel
%8 = OpCompositeExtract %4 %5 1
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Value_DynamicIndex) {
    auto* func = b.Function("foo", ty.void_());
    auto* vec_val = b.FunctionParam(ty.vec4(ty.i32()));
    func->SetParams({vec_val});

    auto sb = b.With(func->StartTarget());
    auto* idx_var = sb.Var(ty.ptr<function, i32>());
    auto* idx = sb.Load(idx_var);
    sb.Access(ty.i32(), vec_val, idx);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 4
%6 = OpTypeFunction %2 %3
%9 = OpTypePointer Function %4
%1 = OpFunction %2 None %6
%5 = OpFunctionParameter %3
%7 = OpLabel
%8 = OpVariable %9 Function
%10 = OpLoad %4 %8
%11 = OpVectorExtractDynamic %4 %5 %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Pointer_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());

    auto sb = b.With(func->StartTarget());
    auto* vec_var = sb.Var(ty.ptr<function, vec4<i32>>());
    sb.Access(ty.ptr<function, i32>(), vec_var, 1_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 4
%6 = OpTypePointer Function %7
%10 = OpTypePointer Function %8
%12 = OpTypeInt 32 0
%11 = OpConstant %12 1
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpAccessChain %10 %5 %11
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Vector_Pointer_DynamicIndex) {
    auto* func = b.Function("foo", ty.void_());

    auto sb = b.With(func->StartTarget());
    auto* idx_var = sb.Var(ty.ptr<function, i32>());
    auto* idx = sb.Load(idx_var);
    auto* vec_var = sb.Var(ty.ptr<function, vec4<i32>>());
    sb.Access(ty.ptr<function, i32>(), vec_var, idx);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%11 = OpTypeVector %7 4
%10 = OpTypePointer Function %11
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%9 = OpVariable %10 Function
%8 = OpLoad %7 %5
%12 = OpAccessChain %6 %9 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, NestedVector_Value_DynamicIndex) {
    auto* val = b.FunctionParam(ty.array(ty.array(ty.vec4(ty.i32()), 4), 4));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({val});

    auto sb = b.With(func->StartTarget());
    auto* idx_var = sb.Var(ty.ptr<function, i32>());
    auto* idx = sb.Load(idx_var);
    sb.Access(ty.i32(), val, 1_u, 2_u, idx);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpDecorate %4 ArrayStride 16
OpDecorate %3 ArrayStride 64
%2 = OpTypeVoid
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 4
%8 = OpTypeInt 32 0
%7 = OpConstant %8 4
%4 = OpTypeArray %5 %7
%3 = OpTypeArray %4 %7
%10 = OpTypeFunction %2 %3
%13 = OpTypePointer Function %6
%1 = OpFunction %2 None %10
%9 = OpFunctionParameter %3
%11 = OpLabel
%12 = OpVariable %13 Function
%14 = OpLoad %6 %12
%16 = OpCompositeExtract %5 %9 1 2
%15 = OpVectorExtractDynamic %6 %16 %14
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Struct_Value_ConstantIndex) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* str_val = b.FunctionParam(str);
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({str_val});

    auto sb = b.With(func->StartTarget());
    sb.Access(ty.i32(), str_val, 1_u);
    sb.Access(ty.i32(), str_val, 1_u, 2_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpMemberName %3 0 "a"
OpMemberName %3 1 "b"
OpName %3 "MyStruct"
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %3 1 Offset 16
%2 = OpTypeVoid
%4 = OpTypeFloat 32
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 4
%3 = OpTypeStruct %4 %5
%8 = OpTypeFunction %2 %3
%1 = OpFunction %2 None %8
%7 = OpFunctionParameter %3
%9 = OpLabel
%10 = OpCompositeExtract %6 %7 1
%11 = OpCompositeExtract %6 %7 1 2
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest_Access, Struct_Pointer_ConstantIndex) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* func = b.Function("foo", ty.void_());

    auto sb = b.With(func->StartTarget());
    auto* str_var = sb.Var(ty.ptr(function, str, read_write));
    sb.Access(ty.ptr<function, i32>(), str_var, 1_u);
    sb.Access(ty.ptr<function, i32>(), str_var, 1_u, 2_u);
    sb.Return(func);

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpMemberName %7 0 "a"
OpMemberName %7 1 "b"
OpName %7 "MyStruct"
OpMemberDecorate %7 0 Offset 0
OpMemberDecorate %7 1 Offset 16
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeFloat 32
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 4
%7 = OpTypeStruct %8 %9
%6 = OpTypePointer Function %7
%12 = OpTypePointer Function %10
%14 = OpTypeInt 32 0
%13 = OpConstant %14 1
%16 = OpConstant %14 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%11 = OpAccessChain %12 %5 %13
%15 = OpAccessChain %12 %5 %13 %16
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
