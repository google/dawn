// Copyright 2020 The Tint Authors.
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

#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/wgsl/test_helper.h"

#include "gmock/gmock.h"

namespace tint::writer::wgsl {
namespace {

struct BinaryData {
    const char* result;
    ast::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    utils::StringStream str;
    str << data.op;
    out << str.str();
    return out;
}
using WgslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(WgslBinaryTest, Emit) {
    auto params = GetParam();

    auto op_ty = [&]() {
        if (params.op == ast::BinaryOp::kLogicalAnd || params.op == ast::BinaryOp::kLogicalOr) {
            return ty.bool_();
        } else {
            return ty.u32();
        }
    };

    GlobalVar("left", op_ty(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", op_ty(), builtin::AddressSpace::kPrivate);
    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslBinaryTest,
    testing::Values(BinaryData{"(left & right)", ast::BinaryOp::kAnd},
                    BinaryData{"(left | right)", ast::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", ast::BinaryOp::kXor},
                    BinaryData{"(left && right)", ast::BinaryOp::kLogicalAnd},
                    BinaryData{"(left || right)", ast::BinaryOp::kLogicalOr},
                    BinaryData{"(left == right)", ast::BinaryOp::kEqual},
                    BinaryData{"(left != right)", ast::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", ast::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", ast::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", ast::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", ast::BinaryOp::kGreaterThanEqual},
                    BinaryData{"(left << right)", ast::BinaryOp::kShiftLeft},
                    BinaryData{"(left >> right)", ast::BinaryOp::kShiftRight},
                    BinaryData{"(left + right)", ast::BinaryOp::kAdd},
                    BinaryData{"(left - right)", ast::BinaryOp::kSubtract},
                    BinaryData{"(left * right)", ast::BinaryOp::kMultiply},
                    BinaryData{"(left / right)", ast::BinaryOp::kDivide},
                    BinaryData{"(left % right)", ast::BinaryOp::kModulo}));

}  // namespace
}  // namespace tint::writer::wgsl
