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

#include "gmock/gmock.h"

namespace tint::writer::spirv {
namespace {

using namespace tint::number_suffixes;  // NOLINT

TEST_F(SpvGeneratorImplTest, ModuleHeader) {
    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    auto got = Disassemble(generator_.Result());
    EXPECT_THAT(got, testing::StartsWith(R"(OpCapability Shader
OpMemoryModel Logical GLSL450
)"));
}

TEST_F(SpvGeneratorImplTest, Unreachable) {
    auto* func = b.Function("foo", ty.i32());

    auto* i = b.If(true);
    i->True()->Append(b.Return(func, 10_i));
    i->False()->Append(b.Return(func, 20_i));

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Unreachable());

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeInt 32 1
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%10 = OpConstant %2 10
%11 = OpConstant %2 20
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpReturnValue %10
%7 = OpLabel
OpReturnValue %11
%5 = OpLabel
OpUnreachable
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
