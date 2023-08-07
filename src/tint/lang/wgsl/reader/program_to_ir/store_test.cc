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

using namespace tint::number_suffixes;  // NOLINT

using ProgramToIRStoreTest = helpers::IRProgramTest;

TEST_F(ProgramToIRStoreTest, EmitStatement_Assign) {
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate);

    auto* expr = Assign("a", 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%b1 = block {  # root
  %a:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    store %a, 4u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
