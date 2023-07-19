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

#include "src/tint/builtin/function.h"
#include "src/tint/resolver/builtin_structs.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

/// A parameterized builtin function test case.
struct BuiltinTestCase {
    /// The element type to test.
    TestElementType type;
    /// The builtin function.
    enum builtin::Function function;
    /// The expected SPIR-V instruction string.
    std::string spirv_inst;
};

// Tests for builtins with the signature: T = func(T)
using Builtin_1arg = SpvGeneratorImplTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_1arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
TEST_P(Builtin_1arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    Builtin_1arg,
    testing::Values(BuiltinTestCase{kI32, builtin::Function::kAbs, "SAbs"},
                    BuiltinTestCase{kF32, builtin::Function::kAbs, "FAbs"},
                    BuiltinTestCase{kF16, builtin::Function::kAbs, "FAbs"},
                    BuiltinTestCase{kF32, builtin::Function::kAcos, "Acos"},
                    BuiltinTestCase{kF16, builtin::Function::kAcos, "Acos"},
                    BuiltinTestCase{kF32, builtin::Function::kAsinh, "Asinh"},
                    BuiltinTestCase{kF16, builtin::Function::kAsinh, "Asinh"},
                    BuiltinTestCase{kF32, builtin::Function::kAcos, "Acos"},
                    BuiltinTestCase{kF16, builtin::Function::kAcos, "Acos"},
                    BuiltinTestCase{kF32, builtin::Function::kAsinh, "Asinh"},
                    BuiltinTestCase{kF16, builtin::Function::kAsinh, "Asinh"},
                    BuiltinTestCase{kF32, builtin::Function::kAtan, "Atan"},
                    BuiltinTestCase{kF16, builtin::Function::kAtan, "Atan"},
                    BuiltinTestCase{kF32, builtin::Function::kAtanh, "Atanh"},
                    BuiltinTestCase{kF16, builtin::Function::kAtanh, "Atanh"},
                    BuiltinTestCase{kF32, builtin::Function::kCeil, "Ceil"},
                    BuiltinTestCase{kF16, builtin::Function::kCeil, "Ceil"},
                    BuiltinTestCase{kF32, builtin::Function::kCos, "Cos"},
                    BuiltinTestCase{kF16, builtin::Function::kCos, "Cos"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdx, "OpDPdx"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdxCoarse, "OpDPdxCoarse"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdxFine, "OpDPdxFine"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdy, "OpDPdy"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdyCoarse, "OpDPdyCoarse"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdyFine, "OpDPdyFine"},
                    BuiltinTestCase{kF32, builtin::Function::kExp, "Exp"},
                    BuiltinTestCase{kF16, builtin::Function::kExp, "Exp"},
                    BuiltinTestCase{kF32, builtin::Function::kExp2, "Exp2"},
                    BuiltinTestCase{kF16, builtin::Function::kExp2, "Exp2"},
                    BuiltinTestCase{kF32, builtin::Function::kFloor, "Floor"},
                    BuiltinTestCase{kF16, builtin::Function::kFloor, "Floor"},
                    BuiltinTestCase{kF32, builtin::Function::kFract, "Fract"},
                    BuiltinTestCase{kF16, builtin::Function::kFract, "Fract"},
                    BuiltinTestCase{kF32, builtin::Function::kInverseSqrt, "InverseSqrt"},
                    BuiltinTestCase{kF16, builtin::Function::kInverseSqrt, "InverseSqrt"},
                    BuiltinTestCase{kF32, builtin::Function::kLog, "Log"},
                    BuiltinTestCase{kF16, builtin::Function::kLog, "Log"},
                    BuiltinTestCase{kF32, builtin::Function::kLog2, "Log2"},
                    BuiltinTestCase{kF16, builtin::Function::kLog2, "Log2"},
                    BuiltinTestCase{kF32, builtin::Function::kSin, "Sin"},
                    BuiltinTestCase{kF16, builtin::Function::kSin, "Sin"},
                    BuiltinTestCase{kF32, builtin::Function::kSqrt, "Sqrt"},
                    BuiltinTestCase{kF16, builtin::Function::kSqrt, "Sqrt"},
                    BuiltinTestCase{kF32, builtin::Function::kTan, "Tan"},
                    BuiltinTestCase{kF16, builtin::Function::kTan, "Tan"},
                    BuiltinTestCase{kF32, builtin::Function::kTrunc, "Trunc"},
                    BuiltinTestCase{kF16, builtin::Function::kTrunc, "Trunc"},
                    BuiltinTestCase{kF32, builtin::Function::kCosh, "Cosh"},
                    BuiltinTestCase{kF16, builtin::Function::kCosh, "Cosh"},
                    BuiltinTestCase{kF32, builtin::Function::kSinh, "Sinh"},
                    BuiltinTestCase{kF16, builtin::Function::kSinh, "Sinh"},
                    BuiltinTestCase{kF32, builtin::Function::kTanh, "Tanh"},
                    BuiltinTestCase{kF16, builtin::Function::kTanh, "Tanh"}));

// Test that abs of an unsigned value just folds away.
TEST_F(SpvGeneratorImplTest, Builtin_Abs_u32) {
    auto* func = b.Function("foo", MakeScalarType(kU32));
    b.With(func->Block(), [&] {
        auto* arg = MakeScalarValue(kU32);
        auto* result = b.Call(MakeScalarType(kU32), builtin::Function::kAbs, arg);
        b.Return(func, result);
        mod.SetName(arg, "arg");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %uint None %3
          %4 = OpLabel
               OpReturnValue %arg
               OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Builtin_Abs_vec2u) {
    auto* func = b.Function("foo", MakeVectorType(kU32));
    b.With(func->Block(), [&] {
        auto* arg = MakeVectorValue(kU32);
        auto* result = b.Call(MakeVectorType(kU32), builtin::Function::kAbs, arg);
        b.Return(func, result);
        mod.SetName(arg, "arg");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %v2uint None %4
          %5 = OpLabel
               OpReturnValue %arg
               OpFunctionEnd
)");
}

// Test that all of a scalar just folds away.
TEST_F(SpvGeneratorImplTest, Builtin_All_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.bool_());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAll, arg);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_All_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAll, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAll %bool %arg");
}

// Test that any of a scalar just folds away.
TEST_F(SpvGeneratorImplTest, Builtin_Any_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.bool_());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAny, arg);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Any_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAny, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAny %bool %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Frexp_F32) {
    auto* str = resolver::CreateFrexpResult(ty, mod.symbols, ty.f32());
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_f32 %9 FrexpStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Frexp_F16) {
    auto* str = resolver::CreateFrexpResult(ty, mod.symbols, ty.f16());
    auto* arg = b.FunctionParam("arg", ty.f16());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_f16 %9 FrexpStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Frexp_Vec2f) {
    auto* str = resolver::CreateFrexpResult(ty, mod.symbols, ty.vec2<f32>());
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_vec2_f32 %11 FrexpStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Frexp_Vec3h) {
    auto* str = resolver::CreateFrexpResult(ty, mod.symbols, ty.vec3<f16>());
    auto* arg = b.FunctionParam("arg", ty.vec3<f16>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_vec3_f16 %11 FrexpStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Length_vec4f) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kLength, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %8 Length %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Modf_F32) {
    auto* str = resolver::CreateModfResult(ty, mod.symbols, ty.f32());
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_f32 %8 ModfStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Modf_F16) {
    auto* str = resolver::CreateModfResult(ty, mod.symbols, ty.f16());
    auto* arg = b.FunctionParam("arg", ty.f16());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_f16 %8 ModfStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Modf_Vec2f) {
    auto* str = resolver::CreateModfResult(ty, mod.symbols, ty.vec2<f32>());
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_vec2_f32 %9 ModfStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Modf_Vec3h) {
    auto* str = resolver::CreateModfResult(ty, mod.symbols, ty.vec3<f16>());
    auto* arg = b.FunctionParam("arg", ty.vec3<f16>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_vec3_f16 %9 ModfStruct %arg");
}

TEST_F(SpvGeneratorImplTest, Builtin_Normalize_vec4f) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kNormalize, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %8 Normalize %arg");
}

// Tests for builtins with the signature: T = func(T, T)
using Builtin_2arg = SpvGeneratorImplTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_2arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type),
               MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
TEST_P(Builtin_2arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type),
               MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         Builtin_2arg,
                         testing::Values(BuiltinTestCase{kF32, builtin::Function::kAtan2, "Atan2"},
                                         BuiltinTestCase{kF32, builtin::Function::kMax, "FMax"},
                                         BuiltinTestCase{kI32, builtin::Function::kMax, "SMax"},
                                         BuiltinTestCase{kU32, builtin::Function::kMax, "UMax"},
                                         BuiltinTestCase{kF32, builtin::Function::kMin, "FMin"},
                                         BuiltinTestCase{kI32, builtin::Function::kMin, "SMin"},
                                         BuiltinTestCase{kU32, builtin::Function::kMin, "UMin"},
                                         BuiltinTestCase{kF32, builtin::Function::kPow, "Pow"},
                                         BuiltinTestCase{kF16, builtin::Function::kPow, "Pow"},
                                         BuiltinTestCase{kF32, builtin::Function::kStep, "Step"},
                                         BuiltinTestCase{kF16, builtin::Function::kStep, "Step"}));

TEST_F(SpvGeneratorImplTest, Builtin_Cross_vec3f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({arg1, arg2});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), builtin::Function::kCross, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3float %9 Cross %arg1 %arg2");
}

TEST_F(SpvGeneratorImplTest, Builtin_Distance_vec2f) {
    auto* arg1 = b.FunctionParam("arg1", MakeVectorType(kF32));
    auto* arg2 = b.FunctionParam("arg2", MakeVectorType(kF32));
    auto* func = b.Function("foo", MakeScalarType(kF32));
    func->SetParams({arg1, arg2});
    b.With(func->Block(), [&] {
        auto* result = b.Call(MakeScalarType(kF32), builtin::Function::kDistance, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %9 Distance %arg1 %arg2");
}

TEST_F(SpvGeneratorImplTest, Builtin_Distance_vec3h) {
    auto* arg1 = b.FunctionParam("arg1", MakeVectorType(kF16));
    auto* arg2 = b.FunctionParam("arg2", MakeVectorType(kF16));
    auto* func = b.Function("foo", MakeScalarType(kF16));
    func->SetParams({arg1, arg2});
    b.With(func->Block(), [&] {
        auto* result = b.Call(MakeScalarType(kF16), builtin::Function::kDistance, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %half %9 Distance %arg1 %arg2");
}

TEST_F(SpvGeneratorImplTest, Builtin_Dot_vec4f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpDot %float %arg1 %arg2");
}

TEST_F(SpvGeneratorImplTest, Builtin_Dot_vec2i) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<i32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpCompositeExtract %int %arg1 0
          %9 = OpCompositeExtract %int %arg2 0
         %10 = OpIMul %int %8 %9
         %11 = OpCompositeExtract %int %arg1 1
         %12 = OpCompositeExtract %int %arg2 1
         %13 = OpIMul %int %11 %12
     %result = OpIAdd %int %10 %13
)");
}

TEST_F(SpvGeneratorImplTest, Builtin_Dot_vec4u) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<u32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});
    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpCompositeExtract %uint %arg1 0
          %9 = OpCompositeExtract %uint %arg2 0
         %10 = OpIMul %uint %8 %9
         %11 = OpCompositeExtract %uint %arg1 1
         %12 = OpCompositeExtract %uint %arg2 1
         %13 = OpIMul %uint %11 %12
         %14 = OpIAdd %uint %10 %13
         %15 = OpCompositeExtract %uint %arg1 2
         %16 = OpCompositeExtract %uint %arg2 2
         %17 = OpIMul %uint %15 %16
         %18 = OpIAdd %uint %14 %17
         %19 = OpCompositeExtract %uint %arg1 3
         %20 = OpCompositeExtract %uint %arg2 3
         %21 = OpIMul %uint %19 %20
     %result = OpIAdd %uint %18 %21
)");
}

// Tests for builtins with the signature: T = func(T, T, T)
using Builtin_3arg = SpvGeneratorImplTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_3arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type),
               MakeScalarValue(params.type), MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
TEST_P(Builtin_3arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type),
               MakeVectorValue(params.type), MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    Builtin_3arg,
    testing::Values(BuiltinTestCase{kF32, builtin::Function::kClamp, "NClamp"},
                    BuiltinTestCase{kI32, builtin::Function::kClamp, "SClamp"},
                    BuiltinTestCase{kU32, builtin::Function::kClamp, "UClamp"},
                    BuiltinTestCase{kF32, builtin::Function::kFma, "Fma"},
                    BuiltinTestCase{kF16, builtin::Function::kFma, "Fma"},
                    BuiltinTestCase{kF32, builtin::Function::kMix, "Mix"},
                    BuiltinTestCase{kF16, builtin::Function::kMix, "Mix"},
                    BuiltinTestCase{kF32, builtin::Function::kSmoothstep, "SmoothStep"},
                    BuiltinTestCase{kF16, builtin::Function::kSmoothstep, "SmoothStep"}));

TEST_F(SpvGeneratorImplTest, Builtin_Mix_VectorOperands_ScalarFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* factor = b.FunctionParam("factor", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, factor});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kMix, arg1, arg2, factor);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%9 = OpCompositeConstruct %v4float %factor %factor %factor %factor");
    EXPECT_INST("%result = OpExtInst %v4float %11 FMix %arg1 %arg2 %9");
}

TEST_F(SpvGeneratorImplTest, Builtin_Mix_VectorOperands_VectorFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* factor = b.FunctionParam("factor", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, factor});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kMix, arg1, arg2, factor);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %10 FMix %arg1 %arg2 %factor");
}

TEST_F(SpvGeneratorImplTest, Builtin_Select_ScalarCondition_ScalarOperands) {
    auto* argf = b.FunctionParam("argf", ty.i32());
    auto* argt = b.FunctionParam("argt", ty.i32());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpSelect %int %cond %argt %argf");
}

TEST_F(SpvGeneratorImplTest, Builtin_Select_VectorCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpSelect %v4int %cond %argt %argf");
}

TEST_F(SpvGeneratorImplTest, Builtin_Select_ScalarCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%11 = OpCompositeConstruct %v4bool %cond %cond %cond %cond");
    EXPECT_INST("%result = OpSelect %v4int %11 %argt %argf");
}

TEST_F(SpvGeneratorImplTest, Builtin_StorageBarrier) {
    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_72");
}

TEST_F(SpvGeneratorImplTest, Builtin_WorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_264");
}

}  // namespace
}  // namespace tint::writer::spirv
