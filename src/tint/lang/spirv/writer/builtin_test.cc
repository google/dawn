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

#include "src/tint/lang/spirv/writer/common/test_helper.h"

#include "src/tint/lang/core/builtin/function.h"
#include "src/tint/lang/core/type/builtin_structs.h"

using namespace tint::number_suffixes;        // NOLINT
using namespace tint::builtin::fluent_types;  // NOLINT

namespace tint::spirv::writer {
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
using Builtin_1arg = SpirvWriterTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_1arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeScalarType(params.type), params.function, MakeScalarValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
TEST_P(Builtin_1arg, Vector) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
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
                    BuiltinTestCase{kI32, builtin::Function::kCountOneBits, "OpBitCount"},
                    BuiltinTestCase{kU32, builtin::Function::kCountOneBits, "OpBitCount"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdx, "OpDPdx"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdxCoarse, "OpDPdxCoarse"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdxFine, "OpDPdxFine"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdy, "OpDPdy"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdyCoarse, "OpDPdyCoarse"},
                    BuiltinTestCase{kF32, builtin::Function::kDpdyFine, "OpDPdyFine"},
                    BuiltinTestCase{kF32, builtin::Function::kDegrees, "Degrees"},
                    BuiltinTestCase{kF16, builtin::Function::kDegrees, "Degrees"},
                    BuiltinTestCase{kF32, builtin::Function::kExp, "Exp"},
                    BuiltinTestCase{kF16, builtin::Function::kExp, "Exp"},
                    BuiltinTestCase{kF32, builtin::Function::kExp2, "Exp2"},
                    BuiltinTestCase{kF16, builtin::Function::kExp2, "Exp2"},
                    BuiltinTestCase{kF32, builtin::Function::kFloor, "Floor"},
                    BuiltinTestCase{kF16, builtin::Function::kFloor, "Floor"},
                    BuiltinTestCase{kF32, builtin::Function::kFract, "Fract"},
                    BuiltinTestCase{kF16, builtin::Function::kFract, "Fract"},
                    BuiltinTestCase{kF32, builtin::Function::kFwidth, "OpFwidth"},
                    BuiltinTestCase{kF32, builtin::Function::kFwidthCoarse, "OpFwidthCoarse"},
                    BuiltinTestCase{kF32, builtin::Function::kFwidthFine, "OpFwidthFine"},
                    BuiltinTestCase{kF32, builtin::Function::kInverseSqrt, "InverseSqrt"},
                    BuiltinTestCase{kF16, builtin::Function::kInverseSqrt, "InverseSqrt"},
                    BuiltinTestCase{kF32, builtin::Function::kLog, "Log"},
                    BuiltinTestCase{kF16, builtin::Function::kLog, "Log"},
                    BuiltinTestCase{kF32, builtin::Function::kLog2, "Log2"},
                    BuiltinTestCase{kF16, builtin::Function::kLog2, "Log2"},
                    BuiltinTestCase{kF32, builtin::Function::kQuantizeToF16, "OpQuantizeToF16"},
                    BuiltinTestCase{kF32, builtin::Function::kRadians, "Radians"},
                    BuiltinTestCase{kF16, builtin::Function::kRadians, "Radians"},
                    BuiltinTestCase{kI32, builtin::Function::kReverseBits, "OpBitReverse"},
                    BuiltinTestCase{kU32, builtin::Function::kReverseBits, "OpBitReverse"},
                    BuiltinTestCase{kF32, builtin::Function::kRound, "RoundEven"},
                    BuiltinTestCase{kF16, builtin::Function::kRound, "RoundEven"},
                    BuiltinTestCase{kF32, builtin::Function::kSign, "FSign"},
                    BuiltinTestCase{kF16, builtin::Function::kSign, "FSign"},
                    BuiltinTestCase{kI32, builtin::Function::kSign, "SSign"},
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
TEST_F(SpirvWriterTest, Builtin_Abs_u32) {
    auto* func = b.Function("foo", MakeScalarType(kU32));
    b.Append(func->Block(), [&] {
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

TEST_F(SpirvWriterTest, Builtin_Abs_vec2u) {
    auto* func = b.Function("foo", MakeVectorType(kU32));
    b.Append(func->Block(), [&] {
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
TEST_F(SpirvWriterTest, Builtin_All_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.bool_());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAll, arg);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpirvWriterTest, Builtin_All_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAll, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAll %bool %arg");
}

// Test that any of a scalar just folds away.
TEST_F(SpirvWriterTest, Builtin_Any_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.bool_());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAny, arg);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpirvWriterTest, Builtin_Any_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.bool_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.bool_(), builtin::Function::kAny, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAny %bool %arg");
}

TEST_F(SpirvWriterTest, Builtin_Determinant_Mat4x4f) {
    auto* arg = b.FunctionParam("arg", ty.mat4x4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kDeterminant, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %9 Determinant %arg");
}

TEST_F(SpirvWriterTest, Builtin_Determinant_Mat3x3h) {
    auto* arg = b.FunctionParam("arg", ty.mat3x3<f16>());
    auto* func = b.Function("foo", ty.f16());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f16(), builtin::Function::kDeterminant, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %half %9 Determinant %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_F32) {
    auto* str = type::CreateFrexpResult(ty, mod.symbols, ty.f32());
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_f32 %9 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_F16) {
    auto* str = type::CreateFrexpResult(ty, mod.symbols, ty.f16());
    auto* arg = b.FunctionParam("arg", ty.f16());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_f16 %9 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_Vec2f) {
    auto* str = type::CreateFrexpResult(ty, mod.symbols, ty.vec2<f32>());
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_vec2_f32 %11 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Frexp_Vec3h) {
    auto* str = type::CreateFrexpResult(ty, mod.symbols, ty.vec3<f16>());
    auto* arg = b.FunctionParam("arg", ty.vec3<f16>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kFrexp, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__frexp_result_vec3_f16 %11 FrexpStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Length_vec4f) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kLength, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %8 Length %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_F32) {
    auto* str = type::CreateModfResult(ty, mod.symbols, ty.f32());
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_f32 %8 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_F16) {
    auto* str = type::CreateModfResult(ty, mod.symbols, ty.f16());
    auto* arg = b.FunctionParam("arg", ty.f16());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_f16 %8 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_Vec2f) {
    auto* str = type::CreateModfResult(ty, mod.symbols, ty.vec2<f32>());
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_vec2_f32 %9 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Modf_Vec3h) {
    auto* str = type::CreateModfResult(ty, mod.symbols, ty.vec3<f16>());
    auto* arg = b.FunctionParam("arg", ty.vec3<f16>());
    auto* func = b.Function("foo", str);
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(str, builtin::Function::kModf, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %__modf_result_vec3_f16 %9 ModfStruct %arg");
}

TEST_F(SpirvWriterTest, Builtin_Normalize_vec4f) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kNormalize, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %8 Normalize %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat2x3f) {
    auto* arg = b.FunctionParam("arg", ty.mat2x3<f32>());
    auto* func = b.Function("foo", ty.mat3x2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat3x2<f32>(), builtin::Function::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat3v2float %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat4x4f) {
    auto* arg = b.FunctionParam("arg", ty.mat4x4<f32>());
    auto* func = b.Function("foo", ty.mat4x4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat4x4<f32>(), builtin::Function::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat4v4float %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat4x3h) {
    auto* arg = b.FunctionParam("arg", ty.mat4x3<f16>());
    auto* func = b.Function("foo", ty.mat3x4<f16>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat3x4<f16>(), builtin::Function::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat3v4half %arg");
}

TEST_F(SpirvWriterTest, Builtin_Transpose_Mat2x2h) {
    auto* arg = b.FunctionParam("arg", ty.mat2x2<f16>());
    auto* func = b.Function("foo", ty.mat2x2<f16>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.mat2x2<f16>(), builtin::Function::kTranspose, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpTranspose %mat2v2half %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack2X16Float) {
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kPack2X16Float, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackHalf2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack2X16Snorm) {
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kPack2X16Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackSnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack2X16Unorm) {
    auto* arg = b.FunctionParam("arg", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kPack2X16Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackUnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack4X8Snorm) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kPack4X8Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackSnorm4x8 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Pack4X8Unorm) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kPack4X8Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %uint %9 PackUnorm4x8 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack2X16Float) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), builtin::Function::kUnpack2X16Float, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %9 UnpackHalf2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack2X16Snorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), builtin::Function::kUnpack2X16Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %9 UnpackSnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack2X16Unorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), builtin::Function::kUnpack2X16Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %9 UnpackUnorm2x16 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack4X8Snorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kUnpack4X8Snorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %9 UnpackSnorm4x8 %arg");
}

TEST_F(SpirvWriterTest, Builtin_Unpack4X8Unorm) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kUnpack4X8Unorm, arg);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %9 UnpackUnorm4x8 %arg");
}

// Tests for builtins with the signature: T = func(T, T)
using Builtin_2arg = SpirvWriterTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_2arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
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
    b.Append(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type),
               MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
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

TEST_F(SpirvWriterTest, Builtin_Cross_vec3f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), builtin::Function::kCross, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3float %9 Cross %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Distance_vec2f) {
    auto* arg1 = b.FunctionParam("arg1", MakeVectorType(kF32));
    auto* arg2 = b.FunctionParam("arg2", MakeVectorType(kF32));
    auto* func = b.Function("foo", MakeScalarType(kF32));
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(MakeScalarType(kF32), builtin::Function::kDistance, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %9 Distance %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Distance_vec3h) {
    auto* arg1 = b.FunctionParam("arg1", MakeVectorType(kF16));
    auto* arg2 = b.FunctionParam("arg2", MakeVectorType(kF16));
    auto* func = b.Function("foo", MakeScalarType(kF16));
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(MakeScalarType(kF16), builtin::Function::kDistance, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %half %9 Distance %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Dot_vec4f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpDot %float %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Dot_vec2i) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<i32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
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

TEST_F(SpirvWriterTest, Builtin_Dot_vec4u) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<u32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
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

TEST_F(SpirvWriterTest, Builtin_Ldexp_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.f32());
    auto* arg2 = b.FunctionParam("arg2", ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %float %9 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Ldexp_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.f16());
    auto* arg2 = b.FunctionParam("arg2", ty.i32());
    auto* func = b.Function("foo", ty.f16());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f16(), builtin::Function::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %half %9 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Ldexp_Vec2_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), builtin::Function::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v2float %11 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Ldexp_Vec3_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<i32>());
    auto* func = b.Function("foo", ty.vec3<f16>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f16>(), builtin::Function::kLdexp, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3half %11 Ldexp %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Reflect_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), builtin::Function::kReflect, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3float %9 Reflect %arg1 %arg2");
}

TEST_F(SpirvWriterTest, Builtin_Reflect_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), builtin::Function::kReflect, arg1, arg2);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4half %9 Reflect %arg1 %arg2");
}

// Tests for builtins with the signature: T = func(T, T, T)
using Builtin_3arg = SpirvWriterTestWithParam<BuiltinTestCase>;
TEST_P(Builtin_3arg, Scalar) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
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
    b.Append(func->Block(), [&] {
        b.Call(MakeVectorType(params.type), params.function, MakeVectorValue(params.type),
               MakeVectorValue(params.type), MakeVectorValue(params.type));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.spirv_inst);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
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

TEST_F(SpirvWriterTest, Builtin_ExtractBits_Scalar_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldSExtract %int %arg %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_ExtractBits_Scalar_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldUExtract %uint %arg %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_ExtractBits_Vector_I32) {
    auto* arg = b.FunctionParam("arg", ty.vec4<i32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldSExtract %v4int %arg %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_ExtractBits_Vector_U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg, offset, count});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), builtin::Function::kExtractBits, arg, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldUExtract %v2uint %arg %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Scalar_I32) {
    auto* arg = b.FunctionParam("arg", ty.i32());
    auto* newbits = b.FunctionParam("newbits", ty.i32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.i32(), builtin::Function::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldInsert %int %arg %newbits %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Scalar_U32) {
    auto* arg = b.FunctionParam("arg", ty.u32());
    auto* newbits = b.FunctionParam("newbits", ty.u32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.u32(), builtin::Function::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldInsert %uint %arg %newbits %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Vector_I32) {
    auto* arg = b.FunctionParam("arg", ty.vec4<i32>());
    auto* newbits = b.FunctionParam("newbits", ty.vec4<i32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<i32>(), builtin::Function::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldInsert %v4int %arg %newbits %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_InsertBits_Vector_U32) {
    auto* arg = b.FunctionParam("arg", ty.vec2<u32>());
    auto* newbits = b.FunctionParam("newbits", ty.vec2<u32>());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({arg, newbits, offset, count});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec2<u32>(), builtin::Function::kInsertBits, arg, newbits, offset, count);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitFieldInsert %v2uint %arg %newbits %offset %count");
}

TEST_F(SpirvWriterTest, Builtin_FaceForward_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec3<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec3<f32>());
    auto* arg3 = b.FunctionParam("arg3", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({arg1, arg2, arg3});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), builtin::Function::kFaceForward, arg1, arg2, arg3);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v3float %10 FaceForward %arg1 %arg2 %arg3");
}

TEST_F(SpirvWriterTest, Builtin_FaceForward_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f16>());
    auto* arg3 = b.FunctionParam("arg3", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({arg1, arg2, arg3});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), builtin::Function::kFaceForward, arg1, arg2, arg3);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4half %10 FaceForward %arg1 %arg2 %arg3");
}

TEST_F(SpirvWriterTest, Builtin_Mix_VectorOperands_ScalarFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* factor = b.FunctionParam("factor", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, factor});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kMix, arg1, arg2, factor);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%9 = OpCompositeConstruct %v4float %factor %factor %factor %factor");
    EXPECT_INST("%result = OpExtInst %v4float %11 FMix %arg1 %arg2 %9");
}

TEST_F(SpirvWriterTest, Builtin_Mix_VectorOperands_VectorFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* factor = b.FunctionParam("factor", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, factor});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kMix, arg1, arg2, factor);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %10 FMix %arg1 %arg2 %factor");
}

TEST_F(SpirvWriterTest, Builtin_Refract_F32) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* i = b.FunctionParam("i", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, i});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kRefract, arg1, arg2, i);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4float %10 Refract %arg1 %arg2 %i");
}

TEST_F(SpirvWriterTest, Builtin_Refract_F16) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f16>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f16>());
    auto* i = b.FunctionParam("i", ty.f16());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({arg1, arg2, i});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), builtin::Function::kRefract, arg1, arg2, i);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpExtInst %v4half %10 Refract %arg1 %arg2 %i");
}

TEST_F(SpirvWriterTest, Builtin_Select_ScalarCondition_ScalarOperands) {
    auto* argf = b.FunctionParam("argf", ty.i32());
    auto* argt = b.FunctionParam("argt", ty.i32());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpSelect %int %cond %argt %argf");
}

TEST_F(SpirvWriterTest, Builtin_Select_VectorCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpSelect %v4int %cond %argt %argf");
}

TEST_F(SpirvWriterTest, Builtin_Select_ScalarCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%11 = OpCompositeConstruct %v4bool %cond %cond %cond %cond");
    EXPECT_INST("%result = OpSelect %v4int %11 %argt %argf");
}

TEST_F(SpirvWriterTest, Builtin_StorageBarrier) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_72");
}

TEST_F(SpirvWriterTest, Builtin_WorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), builtin::Function::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpControlBarrier %uint_2 %uint_2 %uint_264");
}

TEST_F(SpirvWriterTest, Builtin_ArrayLength) {
    auto* var = b.Var("var", ty.ptr(storage, ty.runtime_array(ty.i32())));
    var->SetBindingPoint(0, 0);
    b.RootBlock()->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* ptr = b.Let("ptr", var);
        auto* result = b.Call(ty.u32(), builtin::Function::kArrayLength, ptr);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpVariable %_ptr_StorageBuffer_tint_symbol_1 StorageBuffer");
    EXPECT_INST("%result = OpArrayLength %uint %1 0");
}

TEST_F(SpirvWriterTest, Builtin_ArrayLength_WithStruct) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str = ty.Struct(mod.symbols.New("Buffer"), {
                                                         {mod.symbols.New("a"), ty.i32()},
                                                         {mod.symbols.New("b"), ty.i32()},
                                                         {mod.symbols.New("arr"), arr},
                                                     });
    auto* var = b.Var("var", ty.ptr(storage, str));
    var->SetBindingPoint(0, 0);
    b.RootBlock()->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* ptr = b.Let("ptr", b.Access(ty.ptr(storage, arr), var, 2_u));
        auto* result = b.Call(ty.u32(), builtin::Function::kArrayLength, ptr);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpVariable %_ptr_StorageBuffer_tint_symbol StorageBuffer");
    EXPECT_INST("%result = OpArrayLength %uint %1 2");
}

}  // namespace
}  // namespace tint::spirv::writer
