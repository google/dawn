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

#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Block) {
    // Note, this test uses shadow variables which aren't allowed in WGSL but
    // serves to prove the block code is pushing new scopes as needed.
    auto* inner = Block(Decl(Var("var", ty.f32())), Assign("var", 2_f));
    auto* outer = Block(Decl(Var("var", ty.f32())), Assign("var", 1_f), inner, Assign("var", 3_f));

    WrapInFunction(outer);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateStatement(outer)) << b.Diagnostics();
    EXPECT_FALSE(b.has_error());

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
%5 = OpConstant %3 1
%7 = OpConstant %3 2
%8 = OpConstant %3 3
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().variables()),
              R"(%1 = OpVariable %2 Function %4
%6 = OpVariable %2 Function %4
)");

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpStore %1 %5
OpStore %6 %7
OpStore %1 %8
)");
}

}  // namespace
}  // namespace tint::writer::spirv
