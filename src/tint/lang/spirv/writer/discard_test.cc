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

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

TEST_F(SpirvWriterTest, Discard) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(ir::FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), ir::Function::PipelineStage::kFragment);
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
   %ep_inner = OpFunction %float None %18
%front_facing = OpFunctionParameter %bool
         %19 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %front_facing %21 %20
         %21 = OpLabel
               OpStore %continue_execution %false
               OpBranch %20
         %20 = OpLabel
         %23 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
         %27 = OpLoad %bool %continue_execution
               OpSelectionMerge %28 None
               OpBranchConditional %27 %29 %28
         %29 = OpLabel
               OpStore %23 %int_42
               OpBranch %28
         %28 = OpLabel
         %31 = OpLoad %bool %continue_execution
               OpSelectionMerge %32 None
               OpBranchConditional %31 %33 %32
         %33 = OpLabel
               OpKill
         %32 = OpLabel
               OpReturnValue %float_0_5
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
