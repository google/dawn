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

#include "gmock/gmock.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/helpers/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRLetTest = helpers::IRProgramTest;

TEST_F(ProgramToIRLetTest, Constant) {
    WrapInFunction(Let("a", Expr(42_i)));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:i32 = let 42i
    ret
  }
}
)");
}

TEST_F(ProgramToIRLetTest, BinaryOp) {
    WrapInFunction(Let("a", Add(1_i, 2_i)));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:i32 = let 3i
    ret
  }
}
)");
}

TEST_F(ProgramToIRLetTest, Chain) {
    WrapInFunction(Let("a", Expr(1_i)),  //
                   Let("b", Expr("a")),  //
                   Let("c", Expr("b")));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:i32 = let 1i
    %b:i32 = let %a
    %c:i32 = let %b
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
