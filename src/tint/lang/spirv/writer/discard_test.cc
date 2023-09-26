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

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, Discard) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::ir::FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Store(buffer, 42_i);
        b.Return(ep, 0.5_f);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
   %ep_inner = OpFunction %float None %16
%front_facing = OpFunctionParameter %bool
         %17 = OpLabel
               OpSelectionMerge %18 None
               OpBranchConditional %front_facing %19 %18
         %19 = OpLabel
               OpStore %continue_execution %false
               OpBranch %18
         %18 = OpLabel
         %21 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
         %25 = OpLoad %bool %continue_execution
               OpSelectionMerge %26 None
               OpBranchConditional %25 %27 %26
         %27 = OpLabel
               OpStore %21 %int_42
               OpBranch %26
         %26 = OpLabel
         %29 = OpLoad %bool %continue_execution
         %30 = OpLogicalEqual %bool %29 %false
               OpSelectionMerge %31 None
               OpBranchConditional %30 %32 %31
         %32 = OpLabel
               OpKill
         %31 = OpLabel
               OpReturnValue %float_0_5
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
