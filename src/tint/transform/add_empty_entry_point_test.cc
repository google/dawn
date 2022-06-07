// Copyright 2021 The Tint Authors.
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

#include "src/tint/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using AddEmptyEntryPointTest = TransformTest;

TEST_F(AddEmptyEntryPointTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_TRUE(ShouldRun<AddEmptyEntryPoint>(src));
}

TEST_F(AddEmptyEntryPointTest, ShouldRunExistingEntryPoint) {
    auto* src = R"(
@compute @workgroup_size(1)
fn existing() {}
)";

    EXPECT_FALSE(ShouldRun<AddEmptyEntryPoint>(src));
}

TEST_F(AddEmptyEntryPointTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = R"(
@compute @workgroup_size(1i)
fn unused_entry_point() {
}
)";

    auto got = Run<AddEmptyEntryPoint>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddEmptyEntryPointTest, ExistingEntryPoint) {
    auto* src = R"(
@fragment
fn main() {
}
)";

    auto* expect = src;

    auto got = Run<AddEmptyEntryPoint>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddEmptyEntryPointTest, NameClash) {
    auto* src = R"(var<private> unused_entry_point : f32;)";

    auto* expect = R"(
@compute @workgroup_size(1i)
fn unused_entry_point_1() {
}

var<private> unused_entry_point : f32;
)";

    auto got = Run<AddEmptyEntryPoint>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
