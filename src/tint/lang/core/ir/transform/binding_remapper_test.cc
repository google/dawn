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

#include "src/tint/lang/core/ir/transform/binding_remapper.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_BindingRemapperTest = TransformTest;

TEST_F(IR_BindingRemapperTest, NoModify_NoRemappings) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    BindingRemapperOptions options;
    Run(BindingRemapper, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, NoModify_RemappingDifferentBindingPoint) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    BindingRemapperOptions options;
    options.binding_points[tint::BindingPoint{0u, 1u}] = tint::BindingPoint{1u, 0u};
    Run(BindingRemapper, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, RemappingGroup) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(1, 2);
    b.RootBlock()->Append(buffer);

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(1, 2)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(3, 2)
}

)";

    BindingRemapperOptions options;
    options.binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{3u, 2u};
    Run(BindingRemapper, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, RemappingBindingIndex) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(1, 2);
    b.RootBlock()->Append(buffer);

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(1, 2)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(1, 3)
}

)";

    BindingRemapperOptions options;
    options.binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{1u, 3u};
    Run(BindingRemapper, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, RemappingGroupAndBindingIndex) {
    auto* buffer = b.Var("buffer", ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(1, 2);
    b.RootBlock()->Append(buffer);

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(1, 2)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<uniform, i32, read_write> = var @binding_point(3, 4)
}

)";

    BindingRemapperOptions options;
    options.binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{3u, 4u};
    Run(BindingRemapper, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, SwapTwoBindingPoints) {
    auto* buffer_a = b.Var("buffer_a", ty.ptr<uniform, i32>());
    buffer_a->SetBindingPoint(1, 2);
    b.RootBlock()->Append(buffer_a);
    auto* buffer_b = b.Var("buffer_b", ty.ptr<uniform, i32>());
    buffer_b->SetBindingPoint(3, 4);
    b.RootBlock()->Append(buffer_b);

    auto* src = R"(
%b1 = block {  # root
  %buffer_a:ptr<uniform, i32, read_write> = var @binding_point(1, 2)
  %buffer_b:ptr<uniform, i32, read_write> = var @binding_point(3, 4)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer_a:ptr<uniform, i32, read_write> = var @binding_point(3, 4)
  %buffer_b:ptr<uniform, i32, read_write> = var @binding_point(1, 2)
}

)";

    BindingRemapperOptions options;
    options.binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{3u, 4u};
    options.binding_points[tint::BindingPoint{3u, 4u}] = tint::BindingPoint{1u, 2u};
    Run(BindingRemapper, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BindingRemapperTest, BindingPointCollisionSameEntryPoint) {
    auto* buffer_a = b.Var("buffer_a", ty.ptr<uniform, i32>());
    buffer_a->SetBindingPoint(1, 2);
    b.RootBlock()->Append(buffer_a);
    auto* buffer_b = b.Var("buffer_b", ty.ptr<uniform, i32>());
    buffer_b->SetBindingPoint(3, 4);
    b.RootBlock()->Append(buffer_b);

    auto* ep = b.Function("main", mod.Types().void_(), Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Load(buffer_a);
        b.Load(buffer_b);
        b.Return(ep);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer_a:ptr<uniform, i32, read_write> = var @binding_point(1, 2)
  %buffer_b:ptr<uniform, i32, read_write> = var @binding_point(3, 4)
}

%main = @fragment func():void -> %b2 {
  %b2 = block {
    %4:i32 = load %buffer_a
    %5:i32 = load %buffer_b
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer_a:ptr<uniform, i32, read_write> = var @binding_point(0, 1)
  %buffer_b:ptr<uniform, i32, read_write> = var @binding_point(0, 1)
}

%main = @fragment func():void -> %b2 {
  %b2 = block {
    %4:i32 = load %buffer_a
    %5:i32 = load %buffer_b
    ret
  }
}
)";

    BindingRemapperOptions options;
    options.binding_points[tint::BindingPoint{1u, 2u}] = tint::BindingPoint{0u, 1u};
    options.binding_points[tint::BindingPoint{3u, 4u}] = tint::BindingPoint{0u, 1u};
    Run(BindingRemapper, options);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
