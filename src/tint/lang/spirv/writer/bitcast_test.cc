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
struct BitcastCase {
    /// The input type.
    TestElementType in;
    /// The output type.
    TestElementType out;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};
std::string PrintCase(testing::TestParamInfo<BitcastCase> cc) {
    StringStream ss;
    ss << cc.param.in << "_to_" << cc.param.out;
    return ss.str();
}

using Bitcast = SpirvWriterTestWithParam<BitcastCase>;
TEST_P(Bitcast, Scalar) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeScalarType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeScalarType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(MakeScalarType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    if (params.in == params.out) {
        EXPECT_INST("OpReturnValue %arg");
    } else {
        EXPECT_INST("%result = OpBitcast %" + params.spirv_type_name + " %arg");
    }
}
TEST_P(Bitcast, Vector) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeVectorType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeVectorType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(MakeVectorType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    if (params.in == params.out) {
        EXPECT_INST("OpReturnValue %arg");
    } else {
        EXPECT_INST("%result = OpBitcast %v2" + params.spirv_type_name + " %arg");
    }
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Bitcast,
                         testing::Values(
                             // To f32.
                             BitcastCase{kF32, kF32, "float"},
                             BitcastCase{kI32, kF32, "float"},
                             BitcastCase{kU32, kF32, "float"},

                             // To f16.
                             BitcastCase{kF16, kF16, "half"},

                             // To i32.
                             BitcastCase{kF32, kI32, "int"},
                             BitcastCase{kI32, kI32, "int"},
                             BitcastCase{kU32, kI32, "int"},

                             // To u32.
                             BitcastCase{kF32, kU32, "uint"},
                             BitcastCase{kI32, kU32, "uint"},
                             BitcastCase{kU32, kU32, "uint"}),
                         PrintCase);

TEST_F(SpirvWriterTest, Bitcast_u32_to_vec2h) {
    auto* func = b.Function("foo", ty.vec2<f16>());
    func->SetParams({b.FunctionParam("arg", ty.u32())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.vec2<f16>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %v2half %arg");
}

TEST_F(SpirvWriterTest, Bitcast_vec2i_to_vec4h) {
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({b.FunctionParam("arg", ty.vec2<i32>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.vec4<f16>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %v4half %arg");
}

TEST_F(SpirvWriterTest, Bitcast_vec2h_to_u32) {
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({b.FunctionParam("arg", ty.vec2<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.u32(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %uint %arg");
}

TEST_F(SpirvWriterTest, Bitcast_vec4h_to_vec2i) {
    auto* func = b.Function("foo", ty.vec2<i32>());
    func->SetParams({b.FunctionParam("arg", ty.vec4<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Bitcast(ty.vec2<i32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpBitcast %v2int %arg");
}

}  // namespace
}  // namespace tint::spirv::writer
