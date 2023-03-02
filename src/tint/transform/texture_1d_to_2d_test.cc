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

#include "src/tint/transform/texture_1d_to_2d.h"
#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using Texture1DTo2DTest = TransformTest;

TEST_F(Texture1DTo2DTest, EmptyModule) {
    auto* src = "";

    DataMap data;
    EXPECT_FALSE(ShouldRun<Texture1DTo2D>(src, data));
}

TEST_F(Texture1DTo2DTest, Global1DDecl) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_1d<f32>;

@group(0) @binding(1) var s : sampler;
)";
    auto* expect = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, Global1DDeclAndSample) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_1d<f32>;

@group(0) @binding(1) var s : sampler;

fn main() -> vec4<f32> {
  return textureSample(t, s, 0.5);
}
)";
    auto* expect = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn main() -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(0.5, 0.5));
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, Global1DDeclAndLoad) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_1d<f32>;

fn main() -> vec4<f32> {
  return textureLoad(t, 1, 0);
}
)";
    auto* expect = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn main() -> vec4<f32> {
  return textureLoad(t, vec2<i32>(1, 0), 0);
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, Global1DDeclAndTextureDimensions) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_1d<f32>;

fn main() -> u32 {
  return textureDimensions(t);
}
)";
    auto* expect = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn main() -> u32 {
  return textureDimensions(t).x;
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, Global1DDeclAndTextureNumLevels) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_1d<f32>;

fn main() -> u32 {
  return textureNumLevels(t);
}
)";
    auto* expect = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn main() -> u32 {
  return textureNumLevels(t);
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, Global1DDeclAndTextureDimensionsInCallStmt) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_1d<f32>;

fn main() {
  _ = textureDimensions(t);
}
)";
    auto* expect = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn main() {
  _ = textureDimensions(t).x;
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, GlobalStorage1DDecl) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_storage_1d<r32float, write>;
)";
    auto* expect = R"(
@group(0) @binding(0) var t : texture_storage_2d<r32float, write>;
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, Global2DDeclAndSample) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn main() -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(0.5, 1.5));
}
)";

    DataMap data;
    EXPECT_FALSE(ShouldRun<Texture1DTo2D>(src, data));
}

TEST_F(Texture1DTo2DTest, PrivateIntNoop) {
    auto* src = R"(
var<private> i : i32;
)";

    DataMap data;
    EXPECT_FALSE(ShouldRun<Texture1DTo2D>(src, data));
}

TEST_F(Texture1DTo2DTest, GlobalMatrixNoop) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> m : mat2x2<f32>;
)";

    DataMap data;
    EXPECT_FALSE(ShouldRun<Texture1DTo2D>(src, data));
}

TEST_F(Texture1DTo2DTest, Texture1DFuncParam) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_1d<f32>;

@group(0) @binding(1) var samp : sampler;

fn f(t : texture_1d<f32>, s : sampler) -> vec4<f32> {
  return textureSample(t, s, 0.7);
}

fn main() -> vec4<f32> {
  return f(tex, samp);
}
)";
    auto* expect = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

@group(0) @binding(1) var samp : sampler;

fn f(t : texture_2d<f32>, s : sampler) -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(0.69999999999999995559, 0.5));
}

fn main() -> vec4<f32> {
  return f(tex, samp);
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, TextureStorage1DFuncParam) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_storage_1d<rgba8unorm, write>;

fn f(t : texture_storage_1d<rgba8unorm, write>) {
  textureStore(t, 3, vec4<f32>(42.0, 21.0, 84.0, 10.5));
}

fn main() {
  f(tex);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var tex : texture_storage_2d<rgba8unorm, write>;

fn f(t : texture_storage_2d<rgba8unorm, write>) {
  textureStore(t, vec2<i32>(3, 0), vec4<f32>(42.0, 21.0, 84.0, 10.5));
}

fn main() {
  f(tex);
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, TextureAndNonTextureBuiltin) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_1d<i32>;

fn d() {
  _ = textureLoad(tex, 1, 0);
  let l = sin(3.0);
}
)";

    auto* expect = R"(
@group(0) @binding(0) var tex : texture_2d<i32>;

fn d() {
  _ = textureLoad(tex, vec2<i32>(1, 0), 0);
  let l = sin(3.0);
}
)";

    DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
