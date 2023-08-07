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

/// A parameterized test case.
struct ConvertCase {
    /// The input type.
    TestElementType in;
    /// The output type.
    TestElementType out;
    /// The expected SPIR-V instruction.
    std::string spirv_inst;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};
std::string PrintCase(testing::TestParamInfo<ConvertCase> cc) {
    StringStream ss;
    ss << cc.param.in << "_to_" << cc.param.out;
    return ss.str();
}

using Convert = SpirvWriterTestWithParam<ConvertCase>;
TEST_P(Convert, Scalar) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeScalarType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeScalarType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(MakeScalarType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %" + params.spirv_type_name + " %arg");
}
TEST_P(Convert, Vector) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeVectorType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeVectorType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(MakeVectorType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2" + params.spirv_type_name + " %arg");
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Convert,
                         testing::Values(
                             // To f32.
                             ConvertCase{kF16, kF32, "OpFConvert", "float"},
                             ConvertCase{kI32, kF32, "OpConvertSToF", "float"},
                             ConvertCase{kU32, kF32, "OpConvertUToF", "float"},
                             ConvertCase{kBool, kF32, "OpSelect", "float"},

                             // To f16.
                             ConvertCase{kF32, kF16, "OpFConvert", "half"},
                             ConvertCase{kI32, kF16, "OpConvertSToF", "half"},
                             ConvertCase{kU32, kF16, "OpConvertUToF", "half"},
                             ConvertCase{kBool, kF16, "OpSelect", "half"},

                             // To i32.
                             ConvertCase{kF32, kI32, "OpConvertFToS", "int"},
                             ConvertCase{kF16, kI32, "OpConvertFToS", "int"},
                             ConvertCase{kU32, kI32, "OpBitcast", "int"},
                             ConvertCase{kBool, kI32, "OpSelect", "int"},

                             // To u32.
                             ConvertCase{kF32, kU32, "OpConvertFToU", "uint"},
                             ConvertCase{kF16, kU32, "OpConvertFToU", "uint"},
                             ConvertCase{kI32, kU32, "OpBitcast", "uint"},
                             ConvertCase{kBool, kU32, "OpSelect", "uint"},

                             // To bool.
                             ConvertCase{kF32, kBool, "OpFUnordNotEqual", "bool"},
                             ConvertCase{kF16, kBool, "OpFUnordNotEqual", "bool"},
                             ConvertCase{kI32, kBool, "OpINotEqual", "bool"},
                             ConvertCase{kU32, kBool, "OpINotEqual", "bool"}),
                         PrintCase);

TEST_F(SpirvWriterTest, Convert_Mat2x3_F16_to_F32) {
    auto* func = b.Function("foo", ty.mat2x3<f32>());
    func->SetParams({b.FunctionParam("arg", ty.mat2x3<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.mat2x3<f32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %11 = OpCompositeExtract %v3half %arg 0
         %12 = OpFConvert %v3float %11
         %13 = OpCompositeExtract %v3half %arg 1
         %14 = OpFConvert %v3float %13
     %result = OpCompositeConstruct %mat2v3float %12 %14
)");
}

TEST_F(SpirvWriterTest, Convert_Mat4x2_F32_to_F16) {
    auto* func = b.Function("foo", ty.mat4x2<f16>());
    func->SetParams({b.FunctionParam("arg", ty.mat4x2<f32>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.mat4x2<f16>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %11 = OpCompositeExtract %v2float %arg 0
         %12 = OpFConvert %v2half %11
         %13 = OpCompositeExtract %v2float %arg 1
         %14 = OpFConvert %v2half %13
         %15 = OpCompositeExtract %v2float %arg 2
         %16 = OpFConvert %v2half %15
         %17 = OpCompositeExtract %v2float %arg 3
         %18 = OpFConvert %v2half %17
     %result = OpCompositeConstruct %mat4v2half %12 %14 %16 %18
)");
}

}  // namespace
}  // namespace tint::spirv::writer
