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

#include "src/tint/lang/core/ir/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::ir::transform {
namespace {

using IR_AddEmptyEntryPointTest = TransformTest;

TEST_F(IR_AddEmptyEntryPointTest, EmptyModule) {
    auto* expect = R"(
%unused_entry_point = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";

    Run<AddEmptyEntryPoint>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_AddEmptyEntryPointTest, ExistingEntryPoint) {
    auto* ep = b.Function("main", mod.Types().void_(), Function::PipelineStage::kFragment);
    ep->Block()->Append(b.Return(ep));

    auto* expect = R"(
%main = @fragment func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";

    Run<AddEmptyEntryPoint>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
