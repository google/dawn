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

#include "src/tint/transform/canonicalize_entry_point_io.h"

#include "src/tint/transform/test_helper.h"
#include "src/tint/transform/unshadow.h"

namespace tint::transform {
namespace {

using CanonicalizeEntryPointIOTest = TransformTest;

TEST_F(CanonicalizeEntryPointIOTest, Error_MissingTransformData) {
    auto* src = "";

    auto* expect =
        "error: missing transform data for "
        "tint::transform::CanonicalizeEntryPointIO";

    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, NoShaderIO) {
    // Test that we do not introduce wrapper functions when there is no shader IO
    // to process.
    auto* src = R"(
@fragment
fn frag_main() {
}

@compute @workgroup_size(1)
fn comp_main() {
}
)";

    auto* expect = src;

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_Spirv) {
    auto* src = R"(
@fragment
fn frag_main(@location(1) loc1 : f32,
             @location(2) @interpolate(flat) loc2 : vec4<u32>,
             @builtin(position) coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}
)";

    auto* expect = R"(
@location(1) @internal(disable_validation__ignore_address_space) var<__in> loc1_1 : f32;

@location(2) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> loc2_1 : vec4<u32>;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__in> coord_1 : vec4<f32>;

fn frag_main_inner(loc1 : f32, loc2 : vec4<u32>, coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}

@fragment
fn frag_main() {
  frag_main_inner(loc1_1, loc2_1, coord_1);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_Msl) {
    auto* src = R"(
@fragment
fn frag_main(@location(1) loc1 : f32,
             @location(2) @interpolate(flat) loc2 : vec4<u32>,
             @builtin(position) coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
}

fn frag_main_inner(loc1 : f32, loc2 : vec4<u32>, coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}

@fragment
fn frag_main(@builtin(position) coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1, tint_symbol.loc2, coord);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameters_Hlsl) {
    auto* src = R"(
@fragment
fn frag_main(@location(1) loc1 : f32,
             @location(2) @interpolate(flat) loc2 : vec4<u32>,
             @builtin(position) coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
  @builtin(position)
  coord : vec4<f32>,
}

fn frag_main_inner(loc1 : f32, loc2 : vec4<u32>, coord : vec4<f32>) {
  var col : f32 = (coord.x * loc1);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1, tint_symbol.loc2, tint_symbol.coord);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameter_TypeAlias) {
    auto* src = R"(
alias myf32 = f32;

@fragment
fn frag_main(@location(1) loc1 : myf32) {
  var x : myf32 = loc1;
}
)";

    auto* expect = R"(
alias myf32 = f32;

struct tint_symbol_1 {
  @location(1)
  loc1 : f32,
}

fn frag_main_inner(loc1 : myf32) {
  var x : myf32 = loc1;
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Parameter_TypeAlias_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(@location(1) loc1 : myf32) {
  var x : myf32 = loc1;
}

alias myf32 = f32;
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(1)
  loc1 : f32,
}

fn frag_main_inner(loc1 : myf32) {
  var x : myf32 = loc1;
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc1);
}

alias myf32 = f32;
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_Spirv) {
    auto* src = R"(
struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};

@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> loc0_1 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> loc1_1 : f32;

@location(2) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> loc2_1 : vec4<u32>;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__in> coord_1 : vec4<f32>;

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

@fragment
fn frag_main() {
  frag_main_inner(loc0_1, FragLocations(loc1_1, loc2_1), FragBuiltins(coord_1));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_Spirv_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> loc0_1 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> loc1_1 : f32;

@location(2) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> loc2_1 : vec4<u32>;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__in> coord_1 : vec4<f32>;

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

@fragment
fn frag_main() {
  frag_main_inner(loc0_1, FragLocations(loc1_1, loc2_1), FragBuiltins(coord_1));
}

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_kMsl) {
    auto* src = R"(
struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};

@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}
)";

    auto* expect = R"(
struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}

struct tint_symbol_1 {
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

@fragment
fn frag_main(@builtin(position) coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(coord));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_kMsl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

@fragment
fn frag_main(@builtin(position) coord : vec4<f32>, tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(coord));
}

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_Hlsl) {
    auto* src = R"(
struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};

@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}
)";

    auto* expect = R"(
struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}

struct tint_symbol_1 {
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
  @builtin(position)
  coord : vec4<f32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(tint_symbol.coord));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_Hlsl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(@location(0) loc0 : f32,
             locations : FragLocations,
             builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

struct FragBuiltins {
  @builtin(position) coord : vec4<f32>,
};
struct FragLocations {
  @location(1) loc1 : f32,
  @location(2) @interpolate(flat) loc2 : vec4<u32>,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  loc0 : f32,
  @location(1)
  loc1 : f32,
  @location(2) @interpolate(flat)
  loc2 : vec4<u32>,
  @builtin(position)
  coord : vec4<f32>,
}

fn frag_main_inner(loc0 : f32, locations : FragLocations, builtins : FragBuiltins) {
  var col : f32 = ((builtins.coord.x * locations.loc1) + loc0);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(tint_symbol.loc0, FragLocations(tint_symbol.loc1, tint_symbol.loc2), FragBuiltins(tint_symbol.coord));
}

struct FragBuiltins {
  coord : vec4<f32>,
}

struct FragLocations {
  loc1 : f32,
  loc2 : vec4<u32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_NonStruct_Spirv) {
    auto* src = R"(
@fragment
fn frag_main() -> @builtin(frag_depth) f32 {
  return 1.0;
}
)";

    auto* expect = R"(
@builtin(frag_depth) @internal(disable_validation__ignore_address_space) var<__out> value : f32;

fn frag_main_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main() {
  let inner_result = frag_main_inner();
  value = inner_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_NonStruct_Msl) {
    auto* src = R"(
@fragment
fn frag_main() -> @builtin(frag_depth) f32 {
  return 1.0;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(frag_depth)
  value : f32,
}

fn frag_main_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_NonStruct_Hlsl) {
    auto* src = R"(
@fragment
fn frag_main() -> @builtin(frag_depth) f32 {
  return 1.0;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(frag_depth)
  value : f32,
}

fn frag_main_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Spirv) {
    auto* src = R"(
struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__out> color_1 : vec4<f32>;

@builtin(frag_depth) @internal(disable_validation__ignore_address_space) var<__out> depth_1 : f32;

@builtin(sample_mask) @internal(disable_validation__ignore_address_space) var<__out> mask_1 : array<u32, 1u>;

struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() {
  let inner_result = frag_main_inner();
  color_1 = inner_result.color;
  depth_1 = inner_result.depth;
  mask_1[0i] = inner_result.mask;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Spirv_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__out> color_1 : vec4<f32>;

@builtin(frag_depth) @internal(disable_validation__ignore_address_space) var<__out> depth_1 : f32;

@builtin(sample_mask) @internal(disable_validation__ignore_address_space) var<__out> mask_1 : array<u32, 1u>;

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() {
  let inner_result = frag_main_inner();
  color_1 = inner_result.color;
  depth_1 = inner_result.depth;
  mask_1[0i] = inner_result.mask;
}

struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Msl) {
    auto* src = R"(
struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

    auto* expect = R"(
struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}

struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Msl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}

struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Hlsl) {
    auto* src = R"(
struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}
)";

    auto* expect = R"(
struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}

struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Return_Struct_Hlsl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

struct FragOutput {
  @location(0) color : vec4<f32>,
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  color : vec4<f32>,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> FragOutput {
  var output : FragOutput;
  output.depth = 1.0;
  output.mask = 7u;
  output.color = vec4<f32>(0.5, 0.5, 0.5, 1.0);
  return output;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.color = inner_result.color;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = inner_result.mask;
  return wrapper_result;
}

struct FragOutput {
  color : vec4<f32>,
  depth : f32,
  mask : u32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Spirv) {
    auto* src = R"(
struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> value_1 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> mul_1 : f32;

@location(0) @internal(disable_validation__ignore_address_space) var<__in> value_2 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> mul_2 : f32;

struct FragmentInput {
  value : f32,
  mul : f32,
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1() {
  frag_main1_inner(FragmentInput(value_1, mul_1));
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2() {
  frag_main2_inner(FragmentInput(value_2, mul_2));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Spirv_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> value_1 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> mul_1 : f32;

@location(0) @internal(disable_validation__ignore_address_space) var<__in> value_2 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> mul_2 : f32;

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1() {
  frag_main1_inner(FragmentInput(value_1, mul_1));
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2() {
  frag_main2_inner(FragmentInput(value_2, mul_2));
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct FragmentInput {
  value : f32,
  mul : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Msl) {
    auto* src = R"(
struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}
)";

    auto* expect = R"(
struct FragmentInput {
  value : f32,
  mul : f32,
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Msl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct FragmentInput {
  value : f32,
  mul : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Hlsl) {
    auto* src = R"(
struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}
)";

    auto* expect = R"(
struct FragmentInput {
  value : f32,
  mul : f32,
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, StructParameters_SharedDeviceFunction_Hlsl_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main1(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

fn foo(x : FragmentInput) -> f32 {
  return x.value * x.mul;
}

struct FragmentInput {
  @location(0) value : f32,
  @location(1) mul : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.value, tint_symbol.mul));
}

struct tint_symbol_3 {
  @location(0)
  value : f32,
  @location(1)
  mul : f32,
}

fn frag_main2_inner(inputs : FragmentInput) {
  var x : f32 = foo(inputs);
}

@fragment
fn frag_main2(tint_symbol_2 : tint_symbol_3) {
  frag_main2_inner(FragmentInput(tint_symbol_2.value, tint_symbol_2.mul));
}

fn foo(x : FragmentInput) -> f32 {
  return (x.value * x.mul);
}

struct FragmentInput {
  value : f32,
  mul : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_ModuleScopeVariable) {
    auto* src = R"(
struct FragmentInput {
  @location(0) col1 : f32,
  @location(1) col2 : f32,
};

var<private> global_inputs : FragmentInput;

fn foo() -> f32 {
  return global_inputs.col1 * 0.5;
}

fn bar() -> f32 {
  return global_inputs.col2 * 2.0;
}

@fragment
fn frag_main1(inputs : FragmentInput) {
 global_inputs = inputs;
 var r : f32 = foo();
 var g : f32 = bar();
}
)";

    auto* expect = R"(
struct FragmentInput {
  col1 : f32,
  col2 : f32,
}

var<private> global_inputs : FragmentInput;

fn foo() -> f32 {
  return (global_inputs.col1 * 0.5);
}

fn bar() -> f32 {
  return (global_inputs.col2 * 2.0);
}

struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  global_inputs = inputs;
  var r : f32 = foo();
  var g : f32 = bar();
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.col1, tint_symbol.col2));
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_ModuleScopeVariable_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main1(inputs : FragmentInput) {
 global_inputs = inputs;
 var r : f32 = foo();
 var g : f32 = bar();
}

fn foo() -> f32 {
  return global_inputs.col1 * 0.5;
}

fn bar() -> f32 {
  return global_inputs.col2 * 2.0;
}

var<private> global_inputs : FragmentInput;

struct FragmentInput {
  @location(0) col1 : f32,
  @location(1) col2 : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main1_inner(inputs : FragmentInput) {
  global_inputs = inputs;
  var r : f32 = foo();
  var g : f32 = bar();
}

@fragment
fn frag_main1(tint_symbol : tint_symbol_1) {
  frag_main1_inner(FragmentInput(tint_symbol.col1, tint_symbol.col2));
}

fn foo() -> f32 {
  return (global_inputs.col1 * 0.5);
}

fn bar() -> f32 {
  return (global_inputs.col2 * 2.0);
}

var<private> global_inputs : FragmentInput;

struct FragmentInput {
  col1 : f32,
  col2 : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_TypeAliases) {
    auto* src = R"(
alias myf32 = f32;

struct FragmentInput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

struct FragmentOutput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

@fragment
fn frag_main(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}
)";

    auto* expect = R"(
alias myf32 = f32;

struct FragmentInput {
  col1 : myf32,
  col2 : myf32,
}

struct FragmentOutput {
  col1 : myf32,
  col2 : myf32,
}

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main_inner(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(MyFragmentInput(tint_symbol.col1, tint_symbol.col2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_TypeAliases_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

struct FragmentInput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

struct FragmentOutput {
  @location(0) col1 : myf32,
  @location(1) col2 : myf32,
};

alias myf32 = f32;
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  col1 : f32,
  @location(1)
  col2 : f32,
}

fn frag_main_inner(inputs : MyFragmentInput) -> MyFragmentOutput {
  var x : myf32 = foo(inputs);
  return MyFragmentOutput(x, inputs.col2);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(MyFragmentInput(tint_symbol.col1, tint_symbol.col2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  return wrapper_result;
}

alias MyFragmentInput = FragmentInput;

alias MyFragmentOutput = FragmentOutput;

fn foo(x : MyFragmentInput) -> myf32 {
  return x.col1;
}

struct FragmentInput {
  col1 : myf32,
  col2 : myf32,
}

struct FragmentOutput {
  col1 : myf32,
  col2 : myf32,
}

alias myf32 = f32;
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InterpolateAttributes) {
    auto* src = R"(
struct VertexOut {
  @builtin(position) pos : vec4<f32>,
  @location(1) @interpolate(flat) loc1 : f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
  @location(3) @interpolate(perspective, centroid) loc3 : f32,
};

struct FragmentIn {
  @location(1) @interpolate(flat) loc1 : f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
};

@vertex
fn vert_main() -> VertexOut {
  return VertexOut();
}

@fragment
fn frag_main(inputs : FragmentIn,
             @location(3) @interpolate(perspective, centroid) loc3 : f32) {
  let x = inputs.loc1 + inputs.loc2 + loc3;
}
)";

    auto* expect = R"(
struct VertexOut {
  pos : vec4<f32>,
  loc1 : f32,
  loc2 : f32,
  loc3 : f32,
}

struct FragmentIn {
  loc1 : f32,
  loc2 : f32,
}

struct tint_symbol {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc1 = inner_result.loc1;
  wrapper_result.loc2 = inner_result.loc2;
  wrapper_result.loc3 = inner_result.loc3;
  return wrapper_result;
}

struct tint_symbol_2 {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
}

fn frag_main_inner(inputs : FragmentIn, loc3 : f32) {
  let x = ((inputs.loc1 + inputs.loc2) + loc3);
}

@fragment
fn frag_main(tint_symbol_1 : tint_symbol_2) {
  frag_main_inner(FragmentIn(tint_symbol_1.loc1, tint_symbol_1.loc2), tint_symbol_1.loc3);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InterpolateAttributes_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(inputs : FragmentIn,
             @location(3) @interpolate(perspective, centroid) loc3 : f32) {
  let x = inputs.loc1 + inputs.loc2 + loc3;
}

@vertex
fn vert_main() -> VertexOut {
  return VertexOut();
}

struct VertexOut {
  @builtin(position) pos : vec4<f32>,
  @location(1) @interpolate(flat) loc1 : f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
  @location(3) @interpolate(perspective, centroid) loc3 : f32,
};

struct FragmentIn {
  @location(1) @interpolate(flat) loc1: f32,
  @location(2) @interpolate(linear, sample) loc2 : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
}

fn frag_main_inner(inputs : FragmentIn, loc3 : f32) {
  let x = ((inputs.loc1 + inputs.loc2) + loc3);
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) {
  frag_main_inner(FragmentIn(tint_symbol.loc1, tint_symbol.loc2), tint_symbol.loc3);
}

struct tint_symbol_2 {
  @location(1) @interpolate(flat)
  loc1 : f32,
  @location(2) @interpolate(linear, sample)
  loc2 : f32,
  @location(3) @interpolate(perspective, centroid)
  loc3 : f32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn vert_main() -> tint_symbol_2 {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol_2;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc1 = inner_result.loc1;
  wrapper_result.loc2 = inner_result.loc2;
  wrapper_result.loc3 = inner_result.loc3;
  return wrapper_result;
}

struct VertexOut {
  pos : vec4<f32>,
  loc1 : f32,
  loc2 : f32,
  loc3 : f32,
}

struct FragmentIn {
  loc1 : f32,
  loc2 : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InterpolateAttributes_Integers_Spirv) {
    // Test that we add a Flat attribute to integers that are vertex outputs and
    // fragment inputs, but not vertex inputs or fragment outputs.
    auto* src = R"(
struct VertexIn {
  @location(0) i : i32,
  @location(1) u : u32,
  @location(2) vi : vec4<i32>,
  @location(3) vu : vec4<u32>,
};

struct VertexOut {
  @location(0) @interpolate(flat) i : i32,
  @location(1) @interpolate(flat) u : u32,
  @location(2) @interpolate(flat) vi : vec4<i32>,
  @location(3) @interpolate(flat) vu : vec4<u32>,
  @builtin(position) pos : vec4<f32>,
};

struct FragmentInterface {
  @location(0) @interpolate(flat) i : i32,
  @location(1) @interpolate(flat) u : u32,
  @location(2) @interpolate(flat) vi : vec4<i32>,
  @location(3) @interpolate(flat) vu : vec4<u32>,
};

@vertex
fn vert_main(in : VertexIn) -> VertexOut {
  return VertexOut(in.i, in.u, in.vi, in.vu, vec4<f32>());
}

@fragment
fn frag_main(inputs : FragmentInterface) -> FragmentInterface {
  return inputs;
}
)";

    auto* expect =
        R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> i_1 : i32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> u_1 : u32;

@location(2) @internal(disable_validation__ignore_address_space) var<__in> vi_1 : vec4<i32>;

@location(3) @internal(disable_validation__ignore_address_space) var<__in> vu_1 : vec4<u32>;

@location(0) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> i_2 : i32;

@location(1) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> u_2 : u32;

@location(2) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> vi_2 : vec4<i32>;

@location(3) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> vu_2 : vec4<u32>;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> pos_1 : vec4<f32>;

@location(0) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> i_3 : i32;

@location(1) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> u_3 : u32;

@location(2) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> vi_3 : vec4<i32>;

@location(3) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> vu_3 : vec4<u32>;

@location(0) @internal(disable_validation__ignore_address_space) var<__out> i_4 : i32;

@location(1) @internal(disable_validation__ignore_address_space) var<__out> u_4 : u32;

@location(2) @internal(disable_validation__ignore_address_space) var<__out> vi_4 : vec4<i32>;

@location(3) @internal(disable_validation__ignore_address_space) var<__out> vu_4 : vec4<u32>;

struct VertexIn {
  i : i32,
  u : u32,
  vi : vec4<i32>,
  vu : vec4<u32>,
}

struct VertexOut {
  i : i32,
  u : u32,
  vi : vec4<i32>,
  vu : vec4<u32>,
  pos : vec4<f32>,
}

struct FragmentInterface {
  i : i32,
  u : u32,
  vi : vec4<i32>,
  vu : vec4<u32>,
}

fn vert_main_inner(in : VertexIn) -> VertexOut {
  return VertexOut(in.i, in.u, in.vi, in.vu, vec4<f32>());
}

@vertex
fn vert_main() {
  let inner_result = vert_main_inner(VertexIn(i_1, u_1, vi_1, vu_1));
  i_2 = inner_result.i;
  u_2 = inner_result.u;
  vi_2 = inner_result.vi;
  vu_2 = inner_result.vu;
  pos_1 = inner_result.pos;
}

fn frag_main_inner(inputs : FragmentInterface) -> FragmentInterface {
  return inputs;
}

@fragment
fn frag_main() {
  let inner_result_1 = frag_main_inner(FragmentInterface(i_3, u_3, vi_3, vu_3));
  i_4 = inner_result_1.i;
  u_4 = inner_result_1.u;
  vi_4 = inner_result_1.vi;
  vu_4 = inner_result_1.vu;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InterpolateAttributes_Integers_Spirv_OutOfOrder) {
    // Test that we add a Flat attribute to integers that are vertex outputs and
    // fragment inputs, but not vertex inputs or fragment outputs.
    auto* src = R"(
@vertex
fn vert_main(in : VertexIn) -> VertexOut {
  return VertexOut(in.i, in.u, in.vi, in.vu, vec4<f32>());
}

@fragment
fn frag_main(inputs : FragmentInterface) -> FragmentInterface {
  return inputs;
}

struct VertexIn {
  @location(0) i : i32,
  @location(1) u : u32,
  @location(2) vi : vec4<i32>,
  @location(3) vu : vec4<u32>,
};

struct VertexOut {
  @location(0) @interpolate(flat) i : i32,
  @location(1) @interpolate(flat) u : u32,
  @location(2) @interpolate(flat) vi : vec4<i32>,
  @location(3) @interpolate(flat) vu : vec4<u32>,
  @builtin(position) pos : vec4<f32>,
};

struct FragmentInterface {
  @location(0) @interpolate(flat) i : i32,
  @location(1) @interpolate(flat) u : u32,
  @location(2) @interpolate(flat) vi : vec4<i32>,
  @location(3) @interpolate(flat) vu : vec4<u32>,
};
)";

    auto* expect =
        R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> i_1 : i32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> u_1 : u32;

@location(2) @internal(disable_validation__ignore_address_space) var<__in> vi_1 : vec4<i32>;

@location(3) @internal(disable_validation__ignore_address_space) var<__in> vu_1 : vec4<u32>;

@location(0) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> i_2 : i32;

@location(1) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> u_2 : u32;

@location(2) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> vi_2 : vec4<i32>;

@location(3) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__out> vu_2 : vec4<u32>;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> pos_1 : vec4<f32>;

@location(0) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> i_3 : i32;

@location(1) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> u_3 : u32;

@location(2) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> vi_3 : vec4<i32>;

@location(3) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> vu_3 : vec4<u32>;

@location(0) @internal(disable_validation__ignore_address_space) var<__out> i_4 : i32;

@location(1) @internal(disable_validation__ignore_address_space) var<__out> u_4 : u32;

@location(2) @internal(disable_validation__ignore_address_space) var<__out> vi_4 : vec4<i32>;

@location(3) @internal(disable_validation__ignore_address_space) var<__out> vu_4 : vec4<u32>;

fn vert_main_inner(in : VertexIn) -> VertexOut {
  return VertexOut(in.i, in.u, in.vi, in.vu, vec4<f32>());
}

@vertex
fn vert_main() {
  let inner_result = vert_main_inner(VertexIn(i_1, u_1, vi_1, vu_1));
  i_2 = inner_result.i;
  u_2 = inner_result.u;
  vi_2 = inner_result.vi;
  vu_2 = inner_result.vu;
  pos_1 = inner_result.pos;
}

fn frag_main_inner(inputs : FragmentInterface) -> FragmentInterface {
  return inputs;
}

@fragment
fn frag_main() {
  let inner_result_1 = frag_main_inner(FragmentInterface(i_3, u_3, vi_3, vu_3));
  i_4 = inner_result_1.i;
  u_4 = inner_result_1.u;
  vi_4 = inner_result_1.vi;
  vu_4 = inner_result_1.vu;
}

struct VertexIn {
  i : i32,
  u : u32,
  vi : vec4<i32>,
  vu : vec4<u32>,
}

struct VertexOut {
  i : i32,
  u : u32,
  vi : vec4<i32>,
  vu : vec4<u32>,
  pos : vec4<f32>,
}

struct FragmentInterface {
  i : i32,
  u : u32,
  vi : vec4<i32>,
  vu : vec4<u32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InvariantAttributes) {
    auto* src = R"(
struct VertexOut {
  @builtin(position) @invariant pos : vec4<f32>,
};

@vertex
fn main1() -> VertexOut {
  return VertexOut();
}

@vertex
fn main2() -> @builtin(position) @invariant vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct VertexOut {
  pos : vec4<f32>,
}

struct tint_symbol {
  @builtin(position) @invariant
  pos : vec4<f32>,
}

fn main1_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn main1() -> tint_symbol {
  let inner_result = main1_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  @builtin(position) @invariant
  value : vec4<f32>,
}

fn main2_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn main2() -> tint_symbol_1 {
  let inner_result_1 = main2_inner();
  var wrapper_result_1 : tint_symbol_1;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, InvariantAttributes_OutOfOrder) {
    auto* src = R"(
@vertex
fn main1() -> VertexOut {
  return VertexOut();
}

@vertex
fn main2() -> @builtin(position) @invariant vec4<f32> {
  return vec4<f32>();
}

struct VertexOut {
  @builtin(position) @invariant pos : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position) @invariant
  pos : vec4<f32>,
}

fn main1_inner() -> VertexOut {
  return VertexOut();
}

@vertex
fn main1() -> tint_symbol {
  let inner_result = main1_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  @builtin(position) @invariant
  value : vec4<f32>,
}

fn main2_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn main2() -> tint_symbol_1 {
  let inner_result_1 = main2_inner();
  var wrapper_result_1 : tint_symbol_1;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}

struct VertexOut {
  pos : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_LayoutAttributes) {
    auto* src = R"(
struct FragmentInput {
  @size(16) @location(1) value : f32,
  @builtin(position) @align(32) coord : vec4<f32>,
  @location(0) @interpolate(linear, sample) @align(128) loc0 : f32,
};

struct FragmentOutput {
  @size(16) @location(1) @interpolate(flat) value : f32,
};

@fragment
fn frag_main(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(inputs.coord.x * inputs.value + inputs.loc0);
}
)";

    auto* expect = R"(
struct FragmentInput {
  @size(16)
  value : f32,
  @align(32)
  coord : vec4<f32>,
  @align(128)
  loc0 : f32,
}

struct FragmentOutput {
  @size(16)
  value : f32,
}

struct tint_symbol_1 {
  @location(0) @interpolate(linear, sample)
  loc0 : f32,
  @location(1)
  value : f32,
  @builtin(position)
  coord : vec4<f32>,
}

struct tint_symbol_2 {
  @location(1)
  value : f32,
}

fn frag_main_inner(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(((inputs.coord.x * inputs.value) + inputs.loc0));
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(FragmentInput(tint_symbol.value, tint_symbol.coord, tint_symbol.loc0));
  var wrapper_result : tint_symbol_2;
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, Struct_LayoutAttributes_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(inputs.coord.x * inputs.value + inputs.loc0);
}

struct FragmentInput {
  @size(16) @location(1) value : f32,
  @builtin(position) @align(32) coord : vec4<f32>,
  @location(0) @interpolate(linear, sample) @align(128) loc0 : f32,
};

struct FragmentOutput {
  @size(16) @location(1) @interpolate(flat) value : f32,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0) @interpolate(linear, sample)
  loc0 : f32,
  @location(1)
  value : f32,
  @builtin(position)
  coord : vec4<f32>,
}

struct tint_symbol_2 {
  @location(1)
  value : f32,
}

fn frag_main_inner(inputs : FragmentInput) -> FragmentOutput {
  return FragmentOutput(((inputs.coord.x * inputs.value) + inputs.loc0));
}

@fragment
fn frag_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = frag_main_inner(FragmentInput(tint_symbol.value, tint_symbol.coord, tint_symbol.loc0));
  var wrapper_result : tint_symbol_2;
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}

struct FragmentInput {
  @size(16)
  value : f32,
  @align(32)
  coord : vec4<f32>,
  @align(128)
  loc0 : f32,
}

struct FragmentOutput {
  @size(16)
  value : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SortedMembers) {
    auto* src = R"(
struct VertexOutput {
  @location(1) @interpolate(flat) b : u32,
  @builtin(position) pos : vec4<f32>,
  @location(3) @interpolate(flat) d : u32,
  @location(0) a : f32,
  @location(2) @interpolate(flat) c : i32,
};

struct FragmentInputExtra {
  @location(3) @interpolate(flat) d : u32,
  @builtin(position) pos : vec4<f32>,
  @location(0) a : f32,
};

@vertex
fn vert_main() -> VertexOutput {
  return VertexOutput();
}

@fragment
fn frag_main(@builtin(front_facing) ff : bool,
             @location(2) @interpolate(flat) c : i32,
             inputs : FragmentInputExtra,
             @location(1) @interpolate(flat) b : u32) {
}
)";

    auto* expect = R"(
struct VertexOutput {
  b : u32,
  pos : vec4<f32>,
  d : u32,
  a : f32,
  c : i32,
}

struct FragmentInputExtra {
  d : u32,
  pos : vec4<f32>,
  a : f32,
}

struct tint_symbol {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOutput {
  return VertexOutput();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.b = inner_result.b;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.d = inner_result.d;
  wrapper_result.a = inner_result.a;
  wrapper_result.c = inner_result.c;
  return wrapper_result;
}

struct tint_symbol_2 {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
  @builtin(front_facing)
  ff : bool,
}

fn frag_main_inner(ff : bool, c : i32, inputs : FragmentInputExtra, b : u32) {
}

@fragment
fn frag_main(tint_symbol_1 : tint_symbol_2) {
  frag_main_inner(tint_symbol_1.ff, tint_symbol_1.c, FragmentInputExtra(tint_symbol_1.d, tint_symbol_1.pos, tint_symbol_1.a), tint_symbol_1.b);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SortedMembers_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main() -> VertexOutput {
  return VertexOutput();
}

@fragment
fn frag_main(@builtin(front_facing) ff : bool,
             @location(2) @interpolate(flat) c : i32,
             inputs : FragmentInputExtra,
             @location(1) @interpolate(flat) b : u32) {
}

struct VertexOutput {
  @location(1) @interpolate(flat) b : u32,
  @builtin(position) pos : vec4<f32>,
  @location(3) @interpolate(flat) d : u32,
  @location(0) a : f32,
  @location(2) @interpolate(flat) c : i32,
};

struct FragmentInputExtra {
  @location(3) @interpolate(flat) d : u32,
  @builtin(position) pos : vec4<f32>,
  @location(0) a : f32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertexOutput {
  return VertexOutput();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.b = inner_result.b;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.d = inner_result.d;
  wrapper_result.a = inner_result.a;
  wrapper_result.c = inner_result.c;
  return wrapper_result;
}

struct tint_symbol_2 {
  @location(0)
  a : f32,
  @location(1) @interpolate(flat)
  b : u32,
  @location(2) @interpolate(flat)
  c : i32,
  @location(3) @interpolate(flat)
  d : u32,
  @builtin(position)
  pos : vec4<f32>,
  @builtin(front_facing)
  ff : bool,
}

fn frag_main_inner(ff : bool, c : i32, inputs : FragmentInputExtra, b : u32) {
}

@fragment
fn frag_main(tint_symbol_1 : tint_symbol_2) {
  frag_main_inner(tint_symbol_1.ff, tint_symbol_1.c, FragmentInputExtra(tint_symbol_1.d, tint_symbol_1.pos, tint_symbol_1.a), tint_symbol_1.b);
}

struct VertexOutput {
  b : u32,
  pos : vec4<f32>,
  d : u32,
  a : f32,
  c : i32,
}

struct FragmentInputExtra {
  d : u32,
  pos : vec4<f32>,
  a : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, DontRenameSymbols) {
    auto* src = R"(
@fragment
fn tint_symbol_1(@location(0) col : f32) {
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  @location(0)
  col : f32,
}

fn tint_symbol_1_inner(col : f32) {
}

@fragment
fn tint_symbol_1(tint_symbol : tint_symbol_2) {
  tint_symbol_1_inner(tint_symbol.col);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_VoidNoReturn) {
    auto* src = R"(
@fragment
fn frag_main() {
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() {
}

@fragment
fn frag_main() -> tint_symbol {
  frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_VoidWithReturn) {
    auto* src = R"(
@fragment
fn frag_main() {
  return;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() {
  return;
}

@fragment
fn frag_main() -> tint_symbol {
  frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_WithAuthoredMask) {
    auto* src = R"(
@fragment
fn frag_main() -> @builtin(sample_mask) u32 {
  return 7u;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  value : u32,
}

fn frag_main_inner() -> u32 {
  return 7u;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = (inner_result & 3u);
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_WithoutAuthoredMask) {
    auto* src = R"(
@fragment
fn frag_main() -> @location(0) f32 {
  return 1.0;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithAuthoredMask) {
    auto* src = R"(
struct Output {
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
  @location(0) value : f32,
};

@fragment
fn frag_main() -> Output {
  return Output(0.5, 7u, 1.0);
}
)";

    auto* expect = R"(
struct Output {
  depth : f32,
  mask : u32,
  value : f32,
}

struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 7u, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = (inner_result.mask & 3u);
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithAuthoredMask_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> Output {
  return Output(0.5, 7u, 1.0);
}

struct Output {
  @builtin(frag_depth) depth : f32,
  @builtin(sample_mask) mask : u32,
  @location(0) value : f32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 7u, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.mask = (inner_result.mask & 3u);
  wrapper_result.value = inner_result.value;
  return wrapper_result;
}

struct Output {
  depth : f32,
  mask : u32,
  value : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithoutAuthoredMask) {
    auto* src = R"(
struct Output {
  @builtin(frag_depth) depth : f32,
  @location(0) value : f32,
};

@fragment
fn frag_main() -> Output {
  return Output(0.5, 1.0);
}
)";

    auto* expect = R"(
struct Output {
  depth : f32,
  value : f32,
}

struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.value = inner_result.value;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_StructWithoutAuthoredMask_OutOfOrder) {
    auto* src = R"(
@fragment
fn frag_main() -> Output {
  return Output(0.5, 1.0);
}

struct Output {
  @builtin(frag_depth) depth : f32,
  @location(0) value : f32,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @location(0)
  value : f32,
  @builtin(frag_depth)
  depth : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main_inner() -> Output {
  return Output(0.5, 1.0);
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.depth = inner_result.depth;
  wrapper_result.value = inner_result.value;
  wrapper_result.fixed_sample_mask = 3u;
  return wrapper_result;
}

struct Output {
  depth : f32,
  value : f32,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_MultipleShaders) {
    auto* src = R"(
@fragment
fn frag_main1() -> @builtin(sample_mask) u32 {
  return 7u;
}

@fragment
fn frag_main2() -> @location(0) f32 {
  return 1.0;
}

@vertex
fn vert_main1() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}

@compute @workgroup_size(1)
fn comp_main1() {
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(sample_mask)
  value : u32,
}

fn frag_main1_inner() -> u32 {
  return 7u;
}

@fragment
fn frag_main1() -> tint_symbol {
  let inner_result = frag_main1_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = (inner_result & 3u);
  return wrapper_result;
}

struct tint_symbol_1 {
  @location(0)
  value : f32,
  @builtin(sample_mask)
  fixed_sample_mask : u32,
}

fn frag_main2_inner() -> f32 {
  return 1.0;
}

@fragment
fn frag_main2() -> tint_symbol_1 {
  let inner_result_1 = frag_main2_inner();
  var wrapper_result_1 : tint_symbol_1;
  wrapper_result_1.value = inner_result_1;
  wrapper_result_1.fixed_sample_mask = 3u;
  return wrapper_result_1;
}

struct tint_symbol_2 {
  @builtin(position)
  value : vec4<f32>,
}

fn vert_main1_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn vert_main1() -> tint_symbol_2 {
  let inner_result_2 = vert_main1_inner();
  var wrapper_result_2 : tint_symbol_2;
  wrapper_result_2.value = inner_result_2;
  return wrapper_result_2;
}

@compute @workgroup_size(1)
fn comp_main1() {
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03u);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, FixedSampleMask_AvoidNameClash) {
    auto* src = R"(
struct FragOut {
  @location(0) fixed_sample_mask : vec4<f32>,
  @location(1) fixed_sample_mask_1 : vec4<f32>,
};

@fragment
fn frag_main() -> FragOut {
  return FragOut();
}
)";

    auto* expect = R"(
struct FragOut {
  fixed_sample_mask : vec4<f32>,
  fixed_sample_mask_1 : vec4<f32>,
}

struct tint_symbol {
  @location(0)
  fixed_sample_mask : vec4<f32>,
  @location(1)
  fixed_sample_mask_1 : vec4<f32>,
  @builtin(sample_mask)
  fixed_sample_mask_2 : u32,
}

fn frag_main_inner() -> FragOut {
  return FragOut();
}

@fragment
fn frag_main() -> tint_symbol {
  let inner_result = frag_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.fixed_sample_mask = inner_result.fixed_sample_mask;
  wrapper_result.fixed_sample_mask_1 = inner_result.fixed_sample_mask_1;
  wrapper_result.fixed_sample_mask_2 = 3u;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl, 0x03);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnNonStruct_Spirv) {
    auto* src = R"(
@vertex
fn vert_main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> value : vec4<f32>;

@builtin(__point_size) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size : f32;

fn vert_main_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn vert_main() {
  let inner_result = vert_main_inner();
  value = inner_result;
  vertex_point_size = 1.0f;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnNonStruct_Msl) {
    auto* src = R"(
@vertex
fn vert_main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  value : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size : f32,
}

fn vert_main_inner() -> vec4<f32> {
  return vec4<f32>();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.value = inner_result;
  wrapper_result.vertex_point_size = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnStruct_Spirv) {
    auto* src = R"(
struct VertOut {
  @builtin(position) pos : vec4<f32>,
};

@vertex
fn vert_main() -> VertOut {
  return VertOut();
}
)";

    auto* expect = R"(
@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> pos_1 : vec4<f32>;

@builtin(__point_size) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size : f32;

struct VertOut {
  pos : vec4<f32>,
}

fn vert_main_inner() -> VertOut {
  return VertOut();
}

@vertex
fn vert_main() {
  let inner_result = vert_main_inner();
  pos_1 = inner_result.pos;
  vertex_point_size = 1.0f;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnStruct_Spirv_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main() -> VertOut {
  return VertOut();
}

struct VertOut {
  @builtin(position) pos : vec4<f32>,
};
)";

    auto* expect = R"(
@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> pos_1 : vec4<f32>;

@builtin(__point_size) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size : f32;

fn vert_main_inner() -> VertOut {
  return VertOut();
}

@vertex
fn vert_main() {
  let inner_result = vert_main_inner();
  pos_1 = inner_result.pos;
  vertex_point_size = 1.0f;
}

struct VertOut {
  pos : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnStruct_Msl) {
    auto* src = R"(
struct VertOut {
  @builtin(position) pos : vec4<f32>,
};

@vertex
fn vert_main() -> VertOut {
  return VertOut();
}
)";

    auto* expect = R"(
struct VertOut {
  pos : vec4<f32>,
}

struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size : f32,
}

fn vert_main_inner() -> VertOut {
  return VertOut();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.vertex_point_size = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_ReturnStruct_Msl_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main() -> VertOut {
  return VertOut();
}

struct VertOut {
  @builtin(position) pos : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size : f32,
}

fn vert_main_inner() -> VertOut {
  return VertOut();
}

@vertex
fn vert_main() -> tint_symbol {
  let inner_result = vert_main_inner();
  var wrapper_result : tint_symbol;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.vertex_point_size = 1.0f;
  return wrapper_result;
}

struct VertOut {
  pos : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Spirv) {
    auto* src = R"(
var<private> vertex_point_size : f32;
var<private> vertex_point_size_1 : f32;
var<private> vertex_point_size_2 : f32;

struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : f32,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};

@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> collide_2 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> collide_3 : f32;

@location(0) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size_3 : f32;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size_1_1 : vec4<f32>;

@builtin(__point_size) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size_4 : f32;

var<private> vertex_point_size : f32;

var<private> vertex_point_size_1 : f32;

var<private> vertex_point_size_2 : f32;

struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : f32,
  vertex_point_size_1 : vec4<f32>,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main() {
  let inner_result = vert_main_inner(VertIn1(collide_2), VertIn2(collide_3));
  vertex_point_size_3 = inner_result.vertex_point_size;
  vertex_point_size_1_1 = inner_result.vertex_point_size_1;
  vertex_point_size_4 = 1.0f;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Spirv_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}

struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

var<private> vertex_point_size : f32;
var<private> vertex_point_size_1 : f32;
var<private> vertex_point_size_2 : f32;

struct VertOut {
  @location(0) vertex_point_size : f32,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};
)";

    auto* expect = R"(
@location(0) @internal(disable_validation__ignore_address_space) var<__in> collide_2 : f32;

@location(1) @internal(disable_validation__ignore_address_space) var<__in> collide_3 : f32;

@location(0) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size_3 : f32;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size_1_1 : vec4<f32>;

@builtin(__point_size) @internal(disable_validation__ignore_address_space) var<__out> vertex_point_size_4 : f32;

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main() {
  let inner_result = vert_main_inner(VertIn1(collide_2), VertIn2(collide_3));
  vertex_point_size_3 = inner_result.vertex_point_size;
  vertex_point_size_1_1 = inner_result.vertex_point_size_1;
  vertex_point_size_4 = 1.0f;
}

struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

var<private> vertex_point_size : f32;

var<private> vertex_point_size_1 : f32;

var<private> vertex_point_size_2 : f32;

struct VertOut {
  vertex_point_size : f32,
  vertex_point_size_1 : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Msl) {
    auto* src = R"(
struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};

@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}
)";

    auto* expect = R"(
struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}

struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Msl_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}

struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}

struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kMsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Hlsl) {
    auto* src = R"(
struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};

@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}
)";

    auto* expect = R"(
struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}

struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, EmitVertexPointSize_AvoidNameClash_Hlsl_OutOfOrder) {
    auto* src = R"(
@vertex
fn vert_main(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = collide.collide + collide_1.collide;
  return VertOut();
}

struct VertIn1 {
  @location(0) collide : f32,
};

struct VertIn2 {
  @location(1) collide : f32,
};

struct VertOut {
  @location(0) vertex_point_size : vec4<f32>,
  @builtin(position) vertex_point_size_1 : vec4<f32>,
};
)";

    auto* expect = R"(
struct tint_symbol_1 {
  @location(0)
  collide : f32,
  @location(1)
  collide_2 : f32,
}

struct tint_symbol_2 {
  @location(0)
  vertex_point_size : vec4<f32>,
  @builtin(position)
  vertex_point_size_1 : vec4<f32>,
  @builtin(__point_size)
  vertex_point_size_2 : f32,
}

fn vert_main_inner(collide : VertIn1, collide_1 : VertIn2) -> VertOut {
  let x = (collide.collide + collide_1.collide);
  return VertOut();
}

@vertex
fn vert_main(tint_symbol : tint_symbol_1) -> tint_symbol_2 {
  let inner_result = vert_main_inner(VertIn1(tint_symbol.collide), VertIn2(tint_symbol.collide_2));
  var wrapper_result : tint_symbol_2;
  wrapper_result.vertex_point_size = inner_result.vertex_point_size;
  wrapper_result.vertex_point_size_1 = inner_result.vertex_point_size_1;
  wrapper_result.vertex_point_size_2 = 1.0f;
  return wrapper_result;
}

struct VertIn1 {
  collide : f32,
}

struct VertIn2 {
  collide : f32,
}

struct VertOut {
  vertex_point_size : vec4<f32>,
  vertex_point_size_1 : vec4<f32>,
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kHlsl,
                                               0xFFFFFFFF, true);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, SpirvSampleMaskBuiltins) {
    auto* src = R"(
@fragment
fn main(@builtin(sample_index) sample_index : u32,
        @builtin(sample_mask) mask_in : u32
        ) -> @builtin(sample_mask) u32 {
  return mask_in;
}
)";

    auto* expect = R"(
@builtin(sample_index) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> sample_index_1 : u32;

@builtin(sample_mask) @interpolate(flat) @internal(disable_validation__ignore_address_space) var<__in> mask_in_1 : array<u32, 1u>;

@builtin(sample_mask) @internal(disable_validation__ignore_address_space) var<__out> value : array<u32, 1u>;

fn main_inner(sample_index : u32, mask_in : u32) -> u32 {
  return mask_in;
}

@fragment
fn main() {
  let inner_result = main_inner(sample_index_1, mask_in_1[0i]);
  value[0i] = inner_result;
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kSpirv);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, GLSLSampleMaskBuiltins) {
    auto* src = R"(
@fragment
fn fragment_main(@builtin(sample_index) sample_index : u32,
                 @builtin(sample_mask) mask_in : u32
                 ) -> @builtin(sample_mask) u32 {
  return mask_in;
}
)";

    auto* expect = R"(
@builtin(sample_index) @internal(disable_validation__ignore_address_space) var<__in> gl_SampleID : i32;

@builtin(sample_mask) @internal(disable_validation__ignore_address_space) var<__in> gl_SampleMaskIn : array<i32, 1u>;

@builtin(sample_mask) @internal(disable_validation__ignore_address_space) var<__out> gl_SampleMask : array<i32, 1u>;

fn fragment_main(sample_index : u32, mask_in : u32) -> u32 {
  return mask_in;
}

@fragment
fn main() {
  let inner_result = fragment_main(bitcast<u32>(gl_SampleID), bitcast<u32>(gl_SampleMaskIn[0i]));
  gl_SampleMask[0i] = bitcast<i32>(inner_result);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kGlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CanonicalizeEntryPointIOTest, GLSLVertexInstanceIndexBuiltins) {
    auto* src = R"(
@vertex
fn vertex_main(@builtin(vertex_index) vertexID : u32,
               @builtin(instance_index) instanceID : u32
               ) -> @builtin(position) vec4<f32> {
  return vec4<f32>(f32(vertexID) + f32(instanceID));
}
)";

    auto* expect = R"(
@builtin(vertex_index) @internal(disable_validation__ignore_address_space) var<__in> gl_VertexID : i32;

@builtin(instance_index) @internal(disable_validation__ignore_address_space) var<__in> gl_InstanceID : i32;

@builtin(position) @internal(disable_validation__ignore_address_space) var<__out> gl_Position : vec4<f32>;

fn vertex_main(vertexID : u32, instanceID : u32) -> vec4<f32> {
  return vec4<f32>((f32(vertexID) + f32(instanceID)));
}

@vertex
fn main() {
  let inner_result = vertex_main(bitcast<u32>(gl_VertexID), bitcast<u32>(gl_InstanceID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
}
)";

    DataMap data;
    data.Add<CanonicalizeEntryPointIO::Config>(CanonicalizeEntryPointIO::ShaderStyle::kGlsl);
    auto got = Run<Unshadow, CanonicalizeEntryPointIO>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
