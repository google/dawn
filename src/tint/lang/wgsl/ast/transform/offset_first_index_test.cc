// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/offset_first_index.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::ast::transform {
namespace {

using OffsetFirstIndexTest = TransformTest;

TEST_F(OffsetFirstIndexTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<OffsetFirstIndex>(src));
}

TEST_F(OffsetFirstIndexTest, ShouldRunNoVertexIndexNoInstanceIndex) {
    auto* src = R"(
@fragment
fn entry() {
  return;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(std::nullopt, std::nullopt);
    EXPECT_FALSE(ShouldRun<OffsetFirstIndex>(src, std::move(config)));
}

TEST_F(OffsetFirstIndexTest, ShouldRunNoVertexIndex) {
    auto* src = R"(
@fragment
fn entry() {
  return;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(std::nullopt, 0);
    EXPECT_TRUE(ShouldRun<OffsetFirstIndex>(src, std::move(config)));
}

TEST_F(OffsetFirstIndexTest, ShouldRunNoInstanceIndex) {
    auto* src = R"(
@fragment
fn entry() {
  return;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    EXPECT_TRUE(ShouldRun<OffsetFirstIndex>(src, std::move(config)));
}

TEST_F(OffsetFirstIndexTest, ShouldRun) {
    auto* src = R"(
@fragment
fn entry() {
  return;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, 4);
    EXPECT_TRUE(ShouldRun<OffsetFirstIndex>(src, std::move(config)));
}

TEST_F(OffsetFirstIndexTest, ShouldRunVertexStageWithVertexIndex) {
    auto* src = R"(
@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, 4);
    EXPECT_TRUE(ShouldRun<OffsetFirstIndex>(src, std::move(config)));
}

TEST_F(OffsetFirstIndexTest, ShouldRunVertexStageWithInstanceIndex) {
    auto* src = R"(
@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, 4);
    EXPECT_TRUE(ShouldRun<OffsetFirstIndex>(src, std::move(config)));
}

TEST_F(OffsetFirstIndexTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<OffsetFirstIndex>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicVertexShader) {
    auto* src = R"(
@vertex
fn entry() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";
    auto* expect = src;

    auto got = Run<OffsetFirstIndex>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleVertexIndex) {
    auto* src = R"(
fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test(vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
}

var<push_constant> push_constants : PushConstants;

fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleVertexIndex_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test(vert_idx);
  return vec4<f32>();
}

fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
}

var<push_constant> push_constants : PushConstants;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}

fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleInstanceIndex) {
    auto* src = R"(
fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}

@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test(inst_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  @size(4)
  padding_0 : u32,
  /* @offset(4) */
  first_instance : u32,
}

var<push_constant> push_constants : PushConstants;

fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}

@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(inst_idx) + push_constants.first_instance));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(std::nullopt, 4);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleInstanceIndex_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test(inst_idx);
  return vec4<f32>();
}

fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  @size(4)
  padding_0 : u32,
  /* @offset(4) */
  first_instance : u32,
}

var<push_constant> push_constants : PushConstants;

@vertex
fn entry(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(inst_idx) + push_constants.first_instance));
  return vec4<f32>();
}

fn test(inst_idx : u32) -> u32 {
  return inst_idx;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(std::nullopt, 4);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleBothIndexes) {
    auto* src = R"(
fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return instance_idx + vert_idx;
}

struct Inputs {
  @builtin(instance_index) instance_idx : u32,
  @builtin(vertex_index) vert_idx : u32,
};

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
  /* @offset(4) */
  first_instance : u32,
}

var<push_constant> push_constants : PushConstants;

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}

struct Inputs {
  @builtin(instance_index)
  instance_idx : u32,
  @builtin(vertex_index)
  vert_idx : u32,
}

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(inputs.instance_idx) + push_constants.first_instance), (bitcast<u32>(inputs.vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, 4);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleBothIndexes_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}

struct Inputs {
  @builtin(instance_index) instance_idx : u32,
  @builtin(vertex_index) vert_idx : u32,
};

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return instance_idx + vert_idx;
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
  /* @offset(4) */
  first_instance : u32,
}

var<push_constant> push_constants : PushConstants;

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(inputs.instance_idx) + push_constants.first_instance), (bitcast<u32>(inputs.vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}

struct Inputs {
  @builtin(instance_index)
  instance_idx : u32,
  @builtin(vertex_index)
  vert_idx : u32,
}

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, 4);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleBothIndexes_OffsetsOutOfOrder) {
    auto* src = R"(
@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}

struct Inputs {
  @builtin(vertex_index) vert_idx : u32,
  @builtin(instance_index) instance_idx : u32,
};

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return instance_idx + vert_idx;
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_instance : u32,
  /* @offset(4) */
  first_vertex : u32,
}

var<push_constant> push_constants : PushConstants;

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(inputs.instance_idx) + push_constants.first_instance), (bitcast<u32>(inputs.vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}

struct Inputs {
  @builtin(vertex_index)
  vert_idx : u32,
  @builtin(instance_index)
  instance_idx : u32,
}

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(4, 0);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleBothIndexesVertexDisabled) {
    auto* src = R"(
fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return instance_idx + vert_idx;
}

struct Inputs {
  @builtin(instance_index) instance_idx : u32,
  @builtin(vertex_index) vert_idx : u32,
};

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_instance : u32,
}

var<push_constant> push_constants : PushConstants;

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}

struct Inputs {
  @builtin(instance_index)
  instance_idx : u32,
  @builtin(vertex_index)
  vert_idx : u32,
}

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test((bitcast<u32>(inputs.instance_idx) + push_constants.first_instance), inputs.vert_idx);
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(std::nullopt, 0);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleBothIndexesInstanceDisabled) {
    auto* src = R"(
fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return instance_idx + vert_idx;
}

struct Inputs {
  @builtin(instance_index) instance_idx : u32,
  @builtin(vertex_index) vert_idx : u32,
};

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
}

var<push_constant> push_constants : PushConstants;

fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}

struct Inputs {
  @builtin(instance_index)
  instance_idx : u32,
  @builtin(vertex_index)
  vert_idx : u32,
}

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, (bitcast<u32>(inputs.vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, BasicModuleBothIndexesBothDisabled) {
    auto* src = R"(
fn test(instance_idx : u32, vert_idx : u32) -> u32 {
  return (instance_idx + vert_idx);
}

struct Inputs {
  @builtin(instance_index)
  instance_idx : u32,
  @builtin(vertex_index)
  vert_idx : u32,
}

@vertex
fn entry(inputs : Inputs) -> @builtin(position) vec4<f32> {
  test(inputs.instance_idx, inputs.vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = src;

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(std::nullopt, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

// Test a shader with a user-declared struct called PushConstants, to force
// renaming of the Tint-provided PushConstants struct.
TEST_F(OffsetFirstIndexTest, ForceRenamingPushConstantsStruct) {
    auto* src = R"(
struct PushConstants {
  f : f32,
}

@group(0) @binding(0) var<uniform> p : PushConstants;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>(f32(vert_idx) + p.f);
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants_1 {
  /* @offset(0) */
  first_vertex : u32,
}

var<push_constant> push_constants : PushConstants_1;

struct PushConstants {
  f : f32,
}

@group(0) @binding(0) var<uniform> p : PushConstants;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>((f32((bitcast<u32>(vert_idx) + push_constants.first_vertex)) + p.f));
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

// Test a shader with a user-declared variable called push_constants, to force
// renaming of the Tint-provided push_constants variable.
TEST_F(OffsetFirstIndexTest, ForceRenamingPushConstantsVar) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> push_constants : u32;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>(f32(vert_idx));
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
}

var<push_constant> push_constants_1 : PushConstants;

@group(0) @binding(0) var<uniform> push_constants : u32;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>(f32((bitcast<u32>(vert_idx) + push_constants_1.first_vertex)));
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, NestedCalls) {
    auto* src = R"(
fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2(vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
}

var<push_constant> push_constants : PushConstants;

fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2((bitcast<u32>(vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, NestedCalls_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2(vert_idx);
  return vec4<f32>();
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
}

var<push_constant> push_constants : PushConstants;

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func2((bitcast<u32>(vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}

fn func2(vert_idx : u32) -> u32 {
  return func1(vert_idx);
}

fn func1(vert_idx : u32) -> u32 {
  return vert_idx;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, std::nullopt);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, MultipleEntryPoints) {
    auto* src = R"(
fn func(i : u32) -> u32 {
  return i;
}

@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx);
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx + inst_idx);
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(inst_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
  /* @offset(4) */
  first_instance : u32,
}

var<push_constant> push_constants : PushConstants;

fn func(i : u32) -> u32 {
  return i;
}

@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func((bitcast<u32>(vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(((bitcast<u32>(vert_idx) + push_constants.first_vertex) + (bitcast<u32>(inst_idx) + push_constants.first_instance)));
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func((bitcast<u32>(inst_idx) + push_constants.first_instance));
  return vec4<f32>();
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, 4);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

TEST_F(OffsetFirstIndexTest, MultipleEntryPoints_OutOfOrder) {
    auto* src = R"(
@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx);
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(vert_idx + inst_idx);
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(inst_idx);
  return vec4<f32>();
}

fn func(i : u32) -> u32 {
  return i;
}
)";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct PushConstants {
  /* @offset(0) */
  first_vertex : u32,
  /* @offset(4) */
  first_instance : u32,
}

var<push_constant> push_constants : PushConstants;

@vertex
fn entry_a(@builtin(vertex_index) vert_idx : u32) -> @builtin(position) vec4<f32> {
  func((bitcast<u32>(vert_idx) + push_constants.first_vertex));
  return vec4<f32>();
}

@vertex
fn entry_b(@builtin(vertex_index) vert_idx : u32, @builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func(((bitcast<u32>(vert_idx) + push_constants.first_vertex) + (bitcast<u32>(inst_idx) + push_constants.first_instance)));
  return vec4<f32>();
}

@vertex
fn entry_c(@builtin(instance_index) inst_idx : u32) -> @builtin(position) vec4<f32> {
  func((bitcast<u32>(inst_idx) + push_constants.first_instance));
  return vec4<f32>();
}

fn func(i : u32) -> u32 {
  return i;
}
)";

    DataMap config;
    config.Add<OffsetFirstIndex::Config>(0, 4);
    auto got = Run<OffsetFirstIndex>(src, std::move(config));

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
