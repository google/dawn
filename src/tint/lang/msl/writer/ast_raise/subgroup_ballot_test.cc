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

#include "src/tint/lang/msl/writer/ast_raise/subgroup_ballot.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::msl::writer {
namespace {

using SubgroupBallotTest = ast::transform::TransformTest;

TEST_F(SubgroupBallotTest, EmptyModule) {
    auto* src = "";

    EXPECT_FALSE(ShouldRun<SubgroupBallot>(src));
}

TEST_F(SubgroupBallotTest, DirectUse) {
    auto* src = R"(
enable chromium_experimental_subgroups;

@compute @workgroup_size(64)
fn foo() {
  let pred = true;
  let x : vec4u = subgroupBallot(pred);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_subgroups;

@internal(simd_ballot) @internal(disable_validation__function_has_no_body)
fn tint_msl_simd_ballot(pred : bool) -> vec2<u32>

var<private> tint_subgroup_size_mask : vec4<u32>;

fn tint_msl_subgroup_ballot(pred : bool) -> vec4<u32> {
  let tint_symbol = vec4<u32>(tint_msl_simd_ballot(pred), 0u, 0u);
  return (tint_symbol & tint_subgroup_size_mask);
}

@compute @workgroup_size(64)
fn foo(@builtin(subgroup_size) tint_subgroup_size : u32) {
  {
    let gt = (tint_subgroup_size > 32u);
    tint_subgroup_size_mask[0u] = select((4294967295u >> (32u - tint_subgroup_size)), 4294967295u, gt);
    tint_subgroup_size_mask[1u] = select(0u, (4294967295u >> (64u - tint_subgroup_size)), gt);
  }
  let pred = true;
  let x : vec4u = tint_msl_subgroup_ballot(pred);
}
)";

    auto got = Run<SubgroupBallot>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubgroupBallotTest, IndirectUse) {
    auto* src = R"(
enable chromium_experimental_subgroups;

fn bar() -> vec4u {
  let pred = true;
  return subgroupBallot(pred);
}

@compute @workgroup_size(64)
fn foo() {
  let x: vec4u = bar();
}
)";

    auto* expect =
        R"(
enable chromium_experimental_subgroups;

@internal(simd_ballot) @internal(disable_validation__function_has_no_body)
fn tint_msl_simd_ballot(pred : bool) -> vec2<u32>

var<private> tint_subgroup_size_mask : vec4<u32>;

fn tint_msl_subgroup_ballot(pred : bool) -> vec4<u32> {
  let tint_symbol = vec4<u32>(tint_msl_simd_ballot(pred), 0u, 0u);
  return (tint_symbol & tint_subgroup_size_mask);
}

fn bar() -> vec4u {
  let pred = true;
  return tint_msl_subgroup_ballot(pred);
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

    auto got = Run<SubgroupBallot>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubgroupBallotTest, PreexistingSubgroupSizeBuiltin) {
    auto* src = R"(
enable chromium_experimental_subgroups;

@compute @workgroup_size(64)
fn foo(@builtin(workgroup_id) group_id: vec3u,
       @builtin(subgroup_size) size : u32,
       @builtin(local_invocation_index) index : u32) {
  let sz = size;
  let pred = true;
  let x : vec4u = subgroupBallot(pred);
}
)";

    auto* expect =
        R"(
enable chromium_experimental_subgroups;

@internal(simd_ballot) @internal(disable_validation__function_has_no_body)
fn tint_msl_simd_ballot(pred : bool) -> vec2<u32>

var<private> tint_subgroup_size_mask : vec4<u32>;

fn tint_msl_subgroup_ballot(pred : bool) -> vec4<u32> {
  let tint_symbol = vec4<u32>(tint_msl_simd_ballot(pred), 0u, 0u);
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
  let pred = true;
  let x : vec4u = tint_msl_subgroup_ballot(pred);
}
)";

    auto got = Run<SubgroupBallot>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::msl::writer
