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

#include "src/tint/lang/core/ir/unary.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

/// A parameterized test case.
struct UnaryTestCase {
    /// The element type to test.
    TestElementType type;
    /// The unary operation.
    enum ir::Unary::Kind kind;
    /// The expected SPIR-V instruction.
    std::string spirv_inst;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};

using Arithmetic = SpirvWriterTestWithParam<UnaryTestCase>;
TEST_P(Arithmetic, Scalar) {
    auto params = GetParam();

    auto* arg = b.FunctionParam("arg", MakeScalarType(params.type));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Unary(params.kind, MakeScalarType(params.type), arg);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %" + params.spirv_type_name + " %arg");
}
TEST_P(Arithmetic, Vector) {
    auto params = GetParam();

    auto* arg = b.FunctionParam("arg", MakeVectorType(params.type));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Unary(params.kind, MakeVectorType(params.type), arg);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2" + params.spirv_type_name + " %arg");
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Unary,
    Arithmetic,
    testing::Values(UnaryTestCase{kI32, ir::Unary::Kind::kComplement, "OpNot", "int"},
                    UnaryTestCase{kU32, ir::Unary::Kind::kComplement, "OpNot", "uint"},
                    UnaryTestCase{kI32, ir::Unary::Kind::kNegation, "OpSNegate", "int"},
                    UnaryTestCase{kF32, ir::Unary::Kind::kNegation, "OpFNegate", "float"},
                    UnaryTestCase{kF16, ir::Unary::Kind::kNegation, "OpFNegate", "half"}));

}  // namespace
}  // namespace tint::spirv::writer
