// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/glsl/writer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::glsl::writer {
namespace {

struct BinaryData {
    const char* result;
    core::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using GlslWriterBinaryTest = GlslWriterTestWithParam<BinaryData>;
TEST_P(GlslWriterBinaryTest, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    Options opts{};
    opts.disable_polyfill_integer_div_mod = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint left = 1u;
  uint right = 2u;
  uint val = )" + params.result +
                                R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    GlslWriterTest,
    GlslWriterBinaryTest,
    testing::Values(BinaryData{"(left + right)", core::BinaryOp::kAdd},
                    BinaryData{"(left - right)", core::BinaryOp::kSubtract},
                    BinaryData{"(left * right)", core::BinaryOp::kMultiply},
                    BinaryData{"(left / right)", core::BinaryOp::kDivide},
                    BinaryData{"(left % right)", core::BinaryOp::kModulo},
                    BinaryData{"(left & right)", core::BinaryOp::kAnd},
                    BinaryData{"(left | right)", core::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", core::BinaryOp::kXor},
                    BinaryData{"(left << (right & 31u))", core::BinaryOp::kShiftLeft},
                    BinaryData{"(left >> (right & 31u))", core::BinaryOp::kShiftRight}));

using GlslWriterBinaryBoolTest = GlslWriterTestWithParam<BinaryData>;
TEST_P(GlslWriterBinaryBoolTest, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.bool_(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint left = 1u;
  uint right = 2u;
  bool val = )" + params.result +
                                R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    GlslWriterTest,
    GlslWriterBinaryBoolTest,
    testing::Values(BinaryData{"(left == right)", core::BinaryOp::kEqual},
                    BinaryData{"(left != right)", core::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", core::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", core::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", core::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", core::BinaryOp::kGreaterThanEqual}));

// TODO(dsinclair): Test int_div_mod polyfil
TEST_F(GlslWriterTest, DISABLED_Binary_Int_Div_Polyfill) {}

// TODO(dsinclair): Test int_div_mod polyfil
TEST_F(GlslWriterTest, DISABLED_Binary_Int_Mod_Polyfill) {}

// TODO(dsinclair): Polyfill conversion to relational functions
TEST_F(GlslWriterTest, DISABLED_Binary_Relational_Vector) {}

// TODO(dsinclair): Bitwise bool
TEST_F(GlslWriterTest, DISABLED_Binary_Bitwise_Bool) {}

// TODO(dsinclair): Float Modulo
TEST_F(GlslWriterTest, DISABLED_Binary_Float_Modulo) {}

}  // namespace
}  // namespace tint::glsl::writer
