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
    utils::StringStream ss;
    ss << cc.param.in << "_to_" << cc.param.out;
    return ss.str();
}

using Convert = SpvGeneratorImplTestWithParam<ConvertCase>;
TEST_P(Convert, Scalar) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeScalarType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeScalarType(params.in))});
    b.With(func->Block(), [&] {
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
    b.With(func->Block(), [&] {
        auto* result = b.Convert(MakeVectorType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2" + params.spirv_type_name + " %arg");
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
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

}  // namespace
}  // namespace tint::writer::spirv
