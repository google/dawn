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

#include "src/tint/transform/num_workgroups_from_uniform.h"

#include <utility>

#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/transform/test_helper.h"
#include "src/tint/transform/unshadow.h"

namespace tint::transform {
namespace {

using NumWorkgroupsFromUniformTest = TransformTest;

TEST_F(NumWorkgroupsFromUniformTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    DataMap data;
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    EXPECT_FALSE(ShouldRun<NumWorkgroupsFromUniform>(src, data));
}

TEST_F(NumWorkgroupsFromUniformTest, ShouldRunHasNumWorkgroups) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main(@builtin(num_workgroups) num_wgs : vec3<u32>) {
}
)";

    DataMap data;
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    EXPECT_TRUE(ShouldRun<NumWorkgroupsFromUniform>(src, data));
}

TEST_F(NumWorkgroupsFromUniformTest, Error_MissingTransformData) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main(@builtin(num_workgroups) num_wgs : vec3<u32>) {
}
)";

    auto* expect =
        "error: missing transform data for "
        "tint::transform::NumWorkgroupsFromUniform";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(NumWorkgroupsFromUniformTest, Basic) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main(@builtin(num_workgroups) num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  num_workgroups : vec3<u32>,
}

@group(0) @binding(30) var<uniform> tint_symbol_3 : tint_symbol_2;

fn main_inner(num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}

@compute @workgroup_size(1)
fn main() {
  main_inner(tint_symbol_3.num_workgroups);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(NumWorkgroupsFromUniformTest, StructOnlyMember) {
    auto* src = R"(
struct Builtins {
  @builtin(num_workgroups) num_wgs : vec3<u32>,
};

@compute @workgroup_size(1)
fn main(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  num_workgroups : vec3<u32>,
}

@group(0) @binding(30) var<uniform> tint_symbol_3 : tint_symbol_2;

struct Builtins {
  num_wgs : vec3<u32>,
}

fn main_inner(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main() {
  main_inner(Builtins(tint_symbol_3.num_workgroups));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(NumWorkgroupsFromUniformTest, StructOnlyMember_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

struct Builtins {
  @builtin(num_workgroups) num_wgs : vec3<u32>,
};
)";

    auto* expect = R"(
struct tint_symbol_2 {
  num_workgroups : vec3<u32>,
}

@group(0) @binding(30) var<uniform> tint_symbol_3 : tint_symbol_2;

fn main_inner(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main() {
  main_inner(Builtins(tint_symbol_3.num_workgroups));
}

struct Builtins {
  num_wgs : vec3<u32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(NumWorkgroupsFromUniformTest, StructMultipleMembers) {
    auto* src = R"(
struct Builtins {
  @builtin(global_invocation_id) gid : vec3<u32>,
  @builtin(num_workgroups) num_wgs : vec3<u32>,
  @builtin(workgroup_id) wgid : vec3<u32>,
};

@compute @workgroup_size(1)
fn main(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  num_workgroups : vec3<u32>,
}

@group(0) @binding(30) var<uniform> tint_symbol_3 : tint_symbol_2;

struct Builtins {
  gid : vec3<u32>,
  num_wgs : vec3<u32>,
  wgid : vec3<u32>,
}

struct tint_symbol_1 {
  @builtin(global_invocation_id)
  gid : vec3<u32>,
  @builtin(workgroup_id)
  wgid : vec3<u32>,
}

fn main_inner(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main(tint_symbol : tint_symbol_1) {
  main_inner(Builtins(tint_symbol.gid, tint_symbol_3.num_workgroups, tint_symbol.wgid));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(NumWorkgroupsFromUniformTest, StructMultipleMembers_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

struct Builtins {
  @builtin(global_invocation_id) gid : vec3<u32>,
  @builtin(num_workgroups) num_wgs : vec3<u32>,
  @builtin(workgroup_id) wgid : vec3<u32>,
};

)";

    auto* expect = R"(
struct tint_symbol_2 {
  num_workgroups : vec3<u32>,
}

@group(0) @binding(30) var<uniform> tint_symbol_3 : tint_symbol_2;

struct tint_symbol_1 {
  @builtin(global_invocation_id)
  gid : vec3<u32>,
  @builtin(workgroup_id)
  wgid : vec3<u32>,
}

fn main_inner(in : Builtins) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main(tint_symbol : tint_symbol_1) {
  main_inner(Builtins(tint_symbol.gid, tint_symbol_3.num_workgroups, tint_symbol.wgid));
}

struct Builtins {
  gid : vec3<u32>,
  num_wgs : vec3<u32>,
  wgid : vec3<u32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(NumWorkgroupsFromUniformTest, MultipleEntryPoints) {
    auto* src = R"(
struct Builtins1 {
  @builtin(num_workgroups) num_wgs : vec3<u32>,
};

struct Builtins2 {
  @builtin(global_invocation_id) gid : vec3<u32>,
  @builtin(num_workgroups) num_wgs : vec3<u32>,
  @builtin(workgroup_id) wgid : vec3<u32>,
};

@compute @workgroup_size(1)
fn main1(in : Builtins1) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main2(in : Builtins2) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main3(@builtin(num_workgroups) num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}
)";

    auto* expect = R"(
struct tint_symbol_6 {
  num_workgroups : vec3<u32>,
}

@group(0) @binding(30) var<uniform> tint_symbol_7 : tint_symbol_6;

struct Builtins1 {
  num_wgs : vec3<u32>,
}

struct Builtins2 {
  gid : vec3<u32>,
  num_wgs : vec3<u32>,
  wgid : vec3<u32>,
}

fn main1_inner(in : Builtins1) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main1() {
  main1_inner(Builtins1(tint_symbol_7.num_workgroups));
}

struct tint_symbol_3 {
  @builtin(global_invocation_id)
  gid : vec3<u32>,
  @builtin(workgroup_id)
  wgid : vec3<u32>,
}

fn main2_inner(in : Builtins2) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main2(tint_symbol_2 : tint_symbol_3) {
  main2_inner(Builtins2(tint_symbol_2.gid, tint_symbol_7.num_workgroups, tint_symbol_2.wgid));
}

fn main3_inner(num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}

@compute @workgroup_size(1)
fn main3() {
  main3_inner(tint_symbol_7.num_workgroups);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(NumWorkgroupsFromUniformTest, NoUsages) {
    auto* src = R"(
struct Builtins {
  @builtin(global_invocation_id) gid : vec3<u32>,
  @builtin(workgroup_id) wgid : vec3<u32>,
};

@compute @workgroup_size(1)
fn main(in : Builtins) {
}
)";

    auto* expect = R"(
struct Builtins {
  gid : vec3<u32>,
  wgid : vec3<u32>,
}

struct tint_symbol_1 {
  @builtin(global_invocation_id)
  gid : vec3<u32>,
  @builtin(workgroup_id)
  wgid : vec3<u32>,
}

fn main_inner(in : Builtins) {
}

@compute @workgroup_size(1)
fn main(tint_symbol : tint_symbol_1) {
  main_inner(Builtins(tint_symbol.gid, tint_symbol.wgid));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(sem::BindingPoint{0, 30u});
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Test that group 0 binding 0 is used if no bound resource in the program and binding point is not
// specified in NumWorkgroupsFromUniform::Config.
TEST_F(NumWorkgroupsFromUniformTest, UnspecifiedBindingPoint_NoResourceBound) {
    auto* src = R"(
struct Builtins1 {
  @builtin(num_workgroups) num_wgs : vec3<u32>,
};

struct Builtins2 {
  @builtin(global_invocation_id) gid : vec3<u32>,
  @builtin(num_workgroups) num_wgs : vec3<u32>,
  @builtin(workgroup_id) wgid : vec3<u32>,
};

@compute @workgroup_size(1)
fn main1(in : Builtins1) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main2(in : Builtins2) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main3(@builtin(num_workgroups) num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}
)";

    auto* expect = R"(
struct tint_symbol_6 {
  num_workgroups : vec3<u32>,
}

@group(0) @binding(0) var<uniform> tint_symbol_7 : tint_symbol_6;

struct Builtins1 {
  num_wgs : vec3<u32>,
}

struct Builtins2 {
  gid : vec3<u32>,
  num_wgs : vec3<u32>,
  wgid : vec3<u32>,
}

fn main1_inner(in : Builtins1) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main1() {
  main1_inner(Builtins1(tint_symbol_7.num_workgroups));
}

struct tint_symbol_3 {
  @builtin(global_invocation_id)
  gid : vec3<u32>,
  @builtin(workgroup_id)
  wgid : vec3<u32>,
}

fn main2_inner(in : Builtins2) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main2(tint_symbol_2 : tint_symbol_3) {
  main2_inner(Builtins2(tint_symbol_2.gid, tint_symbol_7.num_workgroups, tint_symbol_2.wgid));
}

fn main3_inner(num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}

@compute @workgroup_size(1)
fn main3() {
  main3_inner(tint_symbol_7.num_workgroups);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    // Make binding point unspecified.
    data.Add<NumWorkgroupsFromUniform::Config>(std::nullopt);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

// Test that binding 0 of the largest used group plus 1 is used if at least one resource is bound in
// the program and binding point is not specified in NumWorkgroupsFromUniform::Config.
TEST_F(NumWorkgroupsFromUniformTest, UnspecifiedBindingPoint_MultipleResourceBound) {
    auto* src = R"(
struct Builtins1 {
  @builtin(num_workgroups) num_wgs : vec3<u32>,
};

struct Builtins2 {
  @builtin(global_invocation_id) gid : vec3<u32>,
  @builtin(num_workgroups) num_wgs : vec3<u32>,
  @builtin(workgroup_id) wgid : vec3<u32>,
};

struct S0 {
  @size(4)
  m0 : u32,
  m1 : array<u32>,
};

struct S1 {
  @size(4)
  m0 : u32,
  m1 : array<u32, 6>,
};

@group(0) @binding(0) var g2 : texture_2d<f32>;
@group(1) @binding(0) var g3 : texture_depth_2d;
@group(1) @binding(1) var g4 : texture_storage_2d<rg32float, write>;
@group(3) @binding(0) var g5 : texture_depth_cube_array;
@group(4) @binding(0) var g6 : texture_external;

@group(0) @binding(1) var<storage, read_write> g8 : S0;
@group(1) @binding(3) var<storage, read> g9 : S0;
@group(3) @binding(2) var<storage, read_write> g10 : S0;

@compute @workgroup_size(1)
fn main1(in : Builtins1) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
  g8.m0 = 1u;
}

@compute @workgroup_size(1)
fn main2(in : Builtins2) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main3(@builtin(num_workgroups) num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}
)";

    auto* expect = R"(
struct tint_symbol_6 {
  num_workgroups : vec3<u32>,
}

@group(5) @binding(0) var<uniform> tint_symbol_7 : tint_symbol_6;

struct Builtins1 {
  num_wgs : vec3<u32>,
}

struct Builtins2 {
  gid : vec3<u32>,
  num_wgs : vec3<u32>,
  wgid : vec3<u32>,
}

struct S0 {
  @size(4)
  m0 : u32,
  m1 : array<u32>,
}

struct S1 {
  @size(4)
  m0 : u32,
  m1 : array<u32, 6>,
}

@group(0) @binding(0) var g2 : texture_2d<f32>;

@group(1) @binding(0) var g3 : texture_depth_2d;

@group(1) @binding(1) var g4 : texture_storage_2d<rg32float, write>;

@group(3) @binding(0) var g5 : texture_depth_cube_array;

@group(4) @binding(0) var g6 : texture_external;

@group(0) @binding(1) var<storage, read_write> g8 : S0;

@group(1) @binding(3) var<storage, read> g9 : S0;

@group(3) @binding(2) var<storage, read_write> g10 : S0;

fn main1_inner(in : Builtins1) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
  g8.m0 = 1u;
}

@compute @workgroup_size(1)
fn main1() {
  main1_inner(Builtins1(tint_symbol_7.num_workgroups));
}

struct tint_symbol_3 {
  @builtin(global_invocation_id)
  gid : vec3<u32>,
  @builtin(workgroup_id)
  wgid : vec3<u32>,
}

fn main2_inner(in : Builtins2) {
  let groups_x = in.num_wgs.x;
  let groups_y = in.num_wgs.y;
  let groups_z = in.num_wgs.z;
}

@compute @workgroup_size(1)
fn main2(tint_symbol_2 : tint_symbol_3) {
  main2_inner(Builtins2(tint_symbol_2.gid, tint_symbol_7.num_workgroups, tint_symbol_2.wgid));
}

fn main3_inner(num_wgs : vec3<u32>) {
  let groups_x = num_wgs.x;
  let groups_y = num_wgs.y;
  let groups_z = num_wgs.z;
}

@compute @workgroup_size(1)
fn main3() {
  main3_inner(tint_symbol_7.num_workgroups);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    // Make binding point unspecified.
    data.Add<NumWorkgroupsFromUniform::Config>(std::nullopt);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO, NumWorkgroupsFromUniform>(src, data);
    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
