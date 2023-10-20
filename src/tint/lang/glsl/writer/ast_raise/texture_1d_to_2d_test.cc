// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/ast_raise/texture_1d_to_2d.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::glsl::writer {
namespace {

using Texture1DTo2DTest = ast::transform::TransformTest;

TEST_F(Texture1DTo2DTest, EmptyModule) {
    auto* src = "";

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Texture1DTo2DTest, Global1DDeclAndTextureDimensionsInCallStmt) {
    auto* src = R"(
@group(0) @binding(0) var st : texture_storage_1d<r32uint, write>;

@group(0) @binding(1) var t : texture_1d<f32>;

fn main() {
  textureStore(st, clamp(5, 0, i32((textureDimensions(st) - 1))), vec4u(0, 0, 0, 0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) var st : texture_storage_2d<r32uint, write>;

@group(0) @binding(1) var t : texture_2d<f32>;

fn main() {
  textureStore(st, vec2<i32>(clamp(5, 0, i32((textureDimensions(st).x - 1))), 0), vec4u(0, 0, 0, 0));
}
)";

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
    EXPECT_FALSE(ShouldRun<Texture1DTo2D>(src, data));
}

TEST_F(Texture1DTo2DTest, PrivateIntNoop) {
    auto* src = R"(
var<private> i : i32;
)";

    ast::transform::DataMap data;
    EXPECT_FALSE(ShouldRun<Texture1DTo2D>(src, data));
}

TEST_F(Texture1DTo2DTest, GlobalMatrixNoop) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> m : mat2x2<f32>;
)";

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
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

    ast::transform::DataMap data;
    auto got = Run<Texture1DTo2D>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::glsl::writer
