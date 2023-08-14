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

#include "src/tint/lang/wgsl/ast/transform/msl_subgroup_ballot.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using MslSubgroupBallotTest = TransformTest;

TEST_F(MslSubgroupBallotTest, EmptyModule) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<MslSubgroupBallot>(src));
}

TEST_F(MslSubgroupBallotTest, DirectUse) {
    auto* src = R"(
enable chromium_experimental_subgroups;

@compute @workgroup_size(64)
fn foo() {
  let x : vec4u = subgroupBallot();
}
)";

    auto* expect =
        R"(
enable chromium_experimental_subgroups;

@internal(simd_active_threads_mask) @internal(disable_validation__function_has_no_body)
fn tint_msl_simd_active_threads_mask() -> vec2<u32>

var<private> tint_subgroup_size_mask : vec4<u32>;

fn tint_msl_subgroup_ballot() -> vec4<u32> {
  let tint_symbol = vec4<u32>(tint_msl_simd_active_threads_mask(), 0u, 0u);
  return (tint_symbol & tint_subgroup_size_mask);
}

@compute @workgroup_size(64)
fn foo(@builtin(subgroup_size) tint_subgroup_size : u32) {
  {
    let gt = (tint_subgroup_size > 32u);
    tint_subgroup_size_mask[0u] = select((4294967295u >> (32u - tint_subgroup_size)), 4294967295u, gt);
    tint_subgroup_size_mask[1u] = select(0u, (4294967295u >> (64u - tint_subgroup_size)), gt);
  }
  let x : vec4u = tint_msl_subgroup_ballot();
}
)";

    auto got = Run<MslSubgroupBallot>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MslSubgroupBallotTest, IndirectUse) {
    auto* src = R"(
enable chromium_experimental_subgroups;

fn bar() -> vec4u {
  return subgroupBallot();
}

@compute @workgroup_size(64)
fn foo() {
  let x: vec4u = bar();
}
)";

    auto* expect =
        R"(
enable chromium_experimental_subgroups;

@internal(simd_active_threads_mask) @internal(disable_validation__function_has_no_body)
fn tint_msl_simd_active_threads_mask() -> vec2<u32>

var<private> tint_subgroup_size_mask : vec4<u32>;

fn tint_msl_subgroup_ballot() -> vec4<u32> {
  let tint_symbol = vec4<u32>(tint_msl_simd_active_threads_mask(), 0u, 0u);
  return (tint_symbol & tint_subgroup_size_mask);
}

fn bar() -> vec4u {
  return tint_msl_subgroup_ballot();
}

@compute @workgroup_size(64)
fn foo(@builtin(subgroup_size) tint_subgroup_size : u32) {
  {
    let gt = (tint_subgroup_size > 32u);
    tint_subgroup_size_mask[0u] = select((4294967295u >> (32u - tint_subgroup_size)), 4294967295u, gt);
    tint_subgroup_size_mask[1u] = select(0u, (4294967295u >> (64u - tint_subgroup_size)), gt);
  }
  let x : vec4u = bar();
}
)";

    auto got = Run<MslSubgroupBallot>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MslSubgroupBallotTest, PreexistingSubgroupSizeBuiltin) {
    auto* src = R"(
enable chromium_experimental_subgroups;

@compute @workgroup_size(64)
fn foo(@builtin(workgroup_id) group_id: vec3u,
       @builtin(subgroup_size) size : u32,
       @builtin(local_invocation_index) index : u32) {
  let sz = size;
  let x : vec4u = subgroupBallot();
}
)";

    auto* expect =
        R"(
enable chromium_experimental_subgroups;

@internal(simd_active_threads_mask) @internal(disable_validation__function_has_no_body)
fn tint_msl_simd_active_threads_mask() -> vec2<u32>

var<private> tint_subgroup_size_mask : vec4<u32>;

fn tint_msl_subgroup_ballot() -> vec4<u32> {
  let tint_symbol = vec4<u32>(tint_msl_simd_active_threads_mask(), 0u, 0u);
  return (tint_symbol & tint_subgroup_size_mask);
}

@compute @workgroup_size(64)
fn foo(@builtin(workgroup_id) group_id : vec3u, @builtin(subgroup_size) size : u32, @builtin(local_invocation_index) index : u32) {
  {
    let gt = (size > 32u);
    tint_subgroup_size_mask[0u] = select((4294967295u >> (32u - size)), 4294967295u, gt);
    tint_subgroup_size_mask[1u] = select(0u, (4294967295u >> (64u - size)), gt);
  }
  let sz = size;
  let x : vec4u = tint_msl_subgroup_ballot();
}
)";

    auto got = Run<MslSubgroupBallot>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
