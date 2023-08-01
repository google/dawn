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

#include "gmock/gmock.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, ModuleHeader) {
    auto spirv = writer_.Generate();
    ASSERT_TRUE(spirv) << spirv.Failure();
    auto got = Disassemble(spirv.Get());
    EXPECT_THAT(got, testing::StartsWith(R"(OpCapability Shader
OpMemoryModel Logical GLSL450
)"));
}

TEST_F(SpirvWriterTest, Unreachable) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(true);
            b.Append(ifelse->True(), [&] {  //
                b.Continue(loop);
            });
            b.Append(ifelse->False(), [&] {  //
                b.Continue(loop);
            });
            b.Unreachable();

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %3
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %11
         %10 = OpLabel
               OpBranch %6
         %11 = OpLabel
               OpBranch %6
          %9 = OpLabel
               OpUnreachable
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
