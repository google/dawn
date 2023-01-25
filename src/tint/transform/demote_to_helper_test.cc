// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/demote_to_helper.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using DemoteToHelperTest = TransformTest;

TEST_F(DemoteToHelperTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, ShouldRunNoDiscard) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> v : f32;

@fragment
fn foo() {
  v = 42;
}
)";

    EXPECT_FALSE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, ShouldRunDiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> v : f32;

@fragment
fn foo() {
  discard;
  v = 42;
}
)";

    EXPECT_TRUE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, ShouldRunDiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0)
var<storage, read_write> v : f32;

fn bar() {
  discard;
}

@fragment
fn foo() {
  bar();
  v = 42;
}
)";

    EXPECT_TRUE(ShouldRun<DemoteToHelper>(src));
}

TEST_F(DemoteToHelperTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInEntryPoint_DiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  v = ret.x;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInEntryPoint_DiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  discard;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  v = ret.x;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  tint_discarded = true;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInHelper_DiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  v = ret.x;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  bar(coord);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  bar(coord);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInHelper_DiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(in : f32, coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  v = ret.x;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  bar(in, coord);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar(in : f32, coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v = ret.x;
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  bar(in, coord);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, WriteInEntryPoint_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  v = ret.x;
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that no additional discards are inserted when the function unconditionally returns in a
// nested block.
TEST_F(DemoteToHelperTest, EntryPointReturn_NestedInBlock) {
    auto* src = R"(
@fragment
fn foo() {
  {
    discard;
    return;
  }
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@fragment
fn foo() {
  {
    tint_discarded = true;
    if (tint_discarded) {
      discard;
    }
    return;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that a discard statement is inserted before every return statement in an entry point that
// contains a discard.
TEST_F(DemoteToHelperTest, EntryPointReturns_DiscardInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  if (in < 1.0) {
    return ret.x;
  }
  return 2.0;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if ((in < 1.0)) {
    if (tint_discarded) {
      discard;
    }
    return ret.x;
  }
  if (tint_discarded) {
    discard;
  }
  return 2.0;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that a discard statement is inserted before every return statement in an entry point that
// calls a function that contains a discard.
TEST_F(DemoteToHelperTest, EntryPointReturns_DiscardInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  discard;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if (in == 0.0) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if (in < 1.0) {
    return ret.x;
  }
  return 2.0;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
  tint_discarded = true;
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if ((in == 0.0)) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if ((in < 1.0)) {
    if (tint_discarded) {
      discard;
    }
    return ret.x;
  }
  if (tint_discarded) {
    discard;
  }
  return 2.0;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that no return statements are modified in an entry point that does not discard.
TEST_F(DemoteToHelperTest, EntryPointReturns_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

fn bar() {
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) f32 {
  if ((in == 0.0)) {
    bar();
  }
  let ret = textureSample(t, s, coord);
  if ((in < 1.0)) {
    return ret.x;
  }
  return 2.0;
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that only functions that are part of a shader that discards are transformed.
// Functions in non-discarding stages should not have their writes masked, and non-discarding entry
// points should not have their return statements replaced.
TEST_F(DemoteToHelperTest, MultipleShaders) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v1 : f32;

@group(0) @binding(3) var<storage, read_write> v2 : f32;

fn bar_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  v1 = ret.x * 2.0;
  return ret.y * 2.0;
}

fn bar_no_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  v1 = ret.x * 2.0;
  return ret.y * 2.0;
}

@fragment
fn foo_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = bar_discard(in, coord);
  v2 = ret;
}

@fragment
fn foo_no_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = bar_no_discard(in, coord);
  if (in == 0.0) {
    return;
  }
  v2 = ret;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v1 : f32;

@group(0) @binding(3) var<storage, read_write> v2 : f32;

fn bar_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    v1 = (ret.x * 2.0);
  }
  return (ret.y * 2.0);
}

fn bar_no_discard(in : f32, coord : vec2<f32>) -> f32 {
  let ret = textureSample(t, s, coord);
  v1 = (ret.x * 2.0);
  return (ret.y * 2.0);
}

@fragment
fn foo_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = bar_discard(in, coord);
  if (!(tint_discarded)) {
    v2 = ret;
  }
  if (tint_discarded) {
    discard;
  }
}

@fragment
fn foo_no_discard(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = bar_no_discard(in, coord);
  if ((in == 0.0)) {
    return;
  }
  v2 = ret;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that we do not mask writes to invocation-private address spaces.
TEST_F(DemoteToHelperTest, InvocationPrivateWrites) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

var<private> vp : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  var vf : f32;
  vf = ret.x;
  vp = ret.y;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

var<private> vp : f32;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  var vf : f32;
  vf = ret.x;
  vp = ret.y;
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, TextureStoreInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  textureStore(t2, vec2<u32>(coord), ret);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    textureStore(t2, vec2<u32>(coord), ret);
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, TextureStoreInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

fn bar(coord : vec2<f32>, value : vec4<f32>) {
  textureStore(t2, vec2<u32>(coord), value);
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  bar(coord, ret);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

fn bar(coord : vec2<f32>, value : vec4<f32>) {
  if (!(tint_discarded)) {
    textureStore(t2, vec2<u32>(coord), value);
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  bar(coord, ret);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, TextureStore_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var t2 : texture_storage_2d<rgba8unorm, write>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  textureStore(t2, vec2<u32>(coord), ret);
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicStoreInEntryPoint) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  atomicStore(&a, i32(ret.x));
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  if (!(tint_discarded)) {
    atomicStore(&(a), i32(ret.x));
  }
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicStoreInHelper) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

fn bar(value : vec4<f32>) {
  atomicStore(&a, i32(value.x));
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if (in == 0.0) {
    discard;
  }
  let ret = textureSample(t, s, coord);
  bar(ret);
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

fn bar(value : vec4<f32>) {
  if (!(tint_discarded)) {
    atomicStore(&(a), i32(value.x));
  }
}

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let ret = textureSample(t, s, coord);
  bar(ret);
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicStore_NoDiscard) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) {
  let ret = textureSample(t, s, coord);
  atomicStore(&(a), i32(ret.x));
}
)";

    auto* expect = src;

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicBuiltinExpression) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  let v = i32(textureSample(t, s, coord).x);
  let result = v + atomicAdd(&a, v);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let v = i32(textureSample(t, s, coord).x);
  var tint_symbol : i32;
  if (!(tint_discarded)) {
    tint_symbol = atomicAdd(&(a), v);
  }
  let result = (v + tint_symbol);
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicBuiltinExpression_InForLoopContinuing) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  var result = 0;
  for (var i = 0; i < 10; i = atomicAdd(&a, 1)) {
    result += i;
  }
  result += i32(textureSample(t, s, coord).x);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  var result = 0;
  {
    var i = 0;
    loop {
      if (!((i < 10))) {
        break;
      }
      {
        result += i;
      }

      continuing {
        var tint_symbol : i32;
        if (!(tint_discarded)) {
          tint_symbol = atomicAdd(&(a), 1);
        }
        i = tint_symbol;
      }
    }
  }
  result += i32(textureSample(t, s, coord).x);
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, AtomicCompareExchangeWeak) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  var result = 0;
  if (!atomicCompareExchangeWeak(&a, i32(in), 42).exchanged) {
    let xchg = atomicCompareExchangeWeak(&a, i32(in), 42);
    result = xchg.old_value;
  }
  result += i32(textureSample(t, s, coord).x);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

struct tint_symbol_1 {
  old_value : i32,
  exchanged : bool,
}

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  var result = 0;
  var tint_symbol : tint_symbol_1;
  if (!(tint_discarded)) {
    let tint_symbol_2 = atomicCompareExchangeWeak(&(a), i32(in), 42);
    tint_symbol.old_value = tint_symbol_2.old_value;
    tint_symbol.exchanged = tint_symbol_2.exchanged;
  }
  if (!(tint_symbol.exchanged)) {
    var tint_symbol_3 : tint_symbol_1;
    if (!(tint_discarded)) {
      let tint_symbol_4 = atomicCompareExchangeWeak(&(a), i32(in), 42);
      tint_symbol_3.old_value = tint_symbol_4.old_value;
      tint_symbol_3.exchanged = tint_symbol_4.exchanged;
    }
    let xchg = tint_symbol_3;
    result = xchg.old_value;
  }
  result += i32(textureSample(t, s, coord).x);
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

// Test that no masking is generated for calls to `atomicLoad()`.
TEST_F(DemoteToHelperTest, AtomicLoad) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if (in == 0.0) {
    discard;
  }
  let v = i32(textureSample(t, s, coord).x);
  let result = v + atomicLoad(&a);
  return result;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> v : f32;

@group(0) @binding(3) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  let v = i32(textureSample(t, s, coord).x);
  let result = (v + atomicLoad(&(a)));
  if (tint_discarded) {
    discard;
  }
  return result;
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DemoteToHelperTest, PhonyAssignment) {
    auto* src = R"(
@fragment
fn foo(@location(0) in : f32) {
  if (in == 0.0) {
    discard;
  }
  _ = in;
}
)";

    auto* expect = R"(
var<private> tint_discarded = false;

@fragment
fn foo(@location(0) in : f32) {
  if ((in == 0.0)) {
    tint_discarded = true;
  }
  _ = in;
  if (tint_discarded) {
    discard;
  }
}
)";

    auto got = Run<DemoteToHelper>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
