// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

#include "gmock/gmock.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, ModuleHeader) {
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability Shader");
    EXPECT_INST("OpMemoryModel Logical GLSL450");
}

TEST_F(SpirvWriterTest, ModuleHeader_VulkanMemoryModel) {
    Options opts;
    opts.use_vulkan_memory_model = true;

    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("OpExtension \"SPV_KHR_vulkan_memory_model\"");
    EXPECT_INST("OpCapability VulkanMemoryModel");
    EXPECT_INST("OpCapability VulkanMemoryModelDeviceScope");
    EXPECT_INST("OpMemoryModel Logical Vulkan");
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

// Test that we fail gracefully when a function has too many parameters.
// See crbug.com/354748060.
TEST_F(SpirvWriterTest, TooManyFunctionParameters) {
    Vector<core::ir::FunctionParam*, 256> params;
    for (uint32_t i = 0; i < 256; i++) {
        params.Push(b.FunctionParam(ty.i32()));
    }
    auto* func = b.Function("foo", ty.void_());
    func->SetParams(std::move(params));
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    EXPECT_FALSE(Generate());
    EXPECT_THAT(Error(),
                testing::HasSubstr(
                    "Function 'foo' has more than 255 parameters after running Tint transforms"));
}

}  // namespace
}  // namespace tint::spirv::writer
