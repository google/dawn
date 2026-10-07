// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/format_string.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/string.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/type/string.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_FormatStringTest = IRTestHelper;

TEST_F(IR_FormatStringTest, Usage) {
    auto* arg1 = b.Constant("hello ");
    auto* arg2 = b.Constant(42_i);
    auto* fs = b.FormatString(arg1, arg2);

    EXPECT_THAT(arg1->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{fs, 0u}));
    EXPECT_THAT(arg2->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{fs, 1u}));
}

TEST_F(IR_FormatStringTest, Result) {
    auto* arg = b.Constant("hello");
    auto* fs = b.FormatString(arg);

    EXPECT_EQ(fs->Results().Length(), 1u);
    EXPECT_TRUE(fs->Result(0)->Is<InstructionResult>());
    EXPECT_EQ(fs, fs->Result(0)->Instruction());
    EXPECT_TRUE(fs->Result(0)->Type()->Is<core::type::String>());
}

TEST_F(IR_FormatStringTest, Clone) {
    auto* arg1 = b.Constant("val: ");
    auto* arg2 = b.Constant(42_i);
    auto* fs = b.FormatString(arg1, arg2);

    auto* new_fs = clone_ctx.Clone(fs);

    EXPECT_NE(fs, new_fs);
    EXPECT_NE(fs->Result(0), new_fs->Result(0));
    EXPECT_TRUE(new_fs->Result(0)->Type()->Is<core::type::String>());

    auto elements = new_fs->Elements();
    ASSERT_EQ(2u, elements.size());

    auto* val0 = elements[0]->As<Constant>()->Value();
    EXPECT_EQ("val: ", val0->As<core::constant::String>()->Value());

    auto* val1 = elements[1]->As<Constant>()->Value();
    EXPECT_EQ(42, val1->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_FormatStringTest, CloneEmpty) {
    auto* fs = b.FormatString();

    auto* new_fs = clone_ctx.Clone(fs);
    EXPECT_NE(fs, new_fs);
    EXPECT_NE(fs->Result(0), new_fs->Result(0));
    EXPECT_TRUE(new_fs->Result(0)->Type()->Is<core::type::String>());
    EXPECT_TRUE(new_fs->Elements().empty());
}

TEST_F(IR_FormatStringTest, Disassemble) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.FormatString(b.Constant("hello "), 42_i, b.Constant(" world"));
        b.Return(f);
    });

    EXPECT_EQ(str(), R"(
%my_func = func():void {
  $B1: {
    %2:string = format_string "hello ", 42i, " world"
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::core::ir
