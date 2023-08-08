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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
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
using WgslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(WgslBinaryTest, Emit) {
    auto params = GetParam();

    auto op_ty = [&] {
        if (params.op == core::BinaryOp::kLogicalAnd || params.op == core::BinaryOp::kLogicalOr) {
            return ty.bool_();
        } else {
            return ty.u32();
        }
    };

    GlobalVar("left", op_ty(), core::AddressSpace::kPrivate);
    GlobalVar("right", op_ty(), core::AddressSpace::kPrivate);
    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslBinaryTest,
    testing::Values(BinaryData{"(left & right)", core::BinaryOp::kAnd},
                    BinaryData{"(left | right)", core::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", core::BinaryOp::kXor},
                    BinaryData{"(left && right)", core::BinaryOp::kLogicalAnd},
                    BinaryData{"(left || right)", core::BinaryOp::kLogicalOr},
                    BinaryData{"(left == right)", core::BinaryOp::kEqual},
                    BinaryData{"(left != right)", core::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", core::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", core::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", core::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", core::BinaryOp::kGreaterThanEqual},
                    BinaryData{"(left << right)", core::BinaryOp::kShiftLeft},
                    BinaryData{"(left >> right)", core::BinaryOp::kShiftRight},
                    BinaryData{"(left + right)", core::BinaryOp::kAdd},
                    BinaryData{"(left - right)", core::BinaryOp::kSubtract},
                    BinaryData{"(left * right)", core::BinaryOp::kMultiply},
                    BinaryData{"(left / right)", core::BinaryOp::kDivide},
                    BinaryData{"(left % right)", core::BinaryOp::kModulo}));

}  // namespace
}  // namespace tint::wgsl::writer
