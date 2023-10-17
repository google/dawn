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

#include "src/tint/lang/glsl/writer/ast_raise/combine_samplers.h"

#include <memory>
#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::glsl::writer {
namespace {

using CombineSamplersTest = ast::transform::TransformTest;

TEST_F(CombineSamplersTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePair) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn main() -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn main() -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePair_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn main() -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePairInAFunction) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn sample(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

fn main() -> vec4<f32> {
  return sample(t, s, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn sample(t_s : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, coords);
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s_1 : texture_2d<f32>;

fn main() -> vec4<f32> {
  return sample(t_s_1, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePairInAFunction_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return sample(t, s, vec2<f32>(1.0, 2.0));
}

fn sample(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

@group(0) @binding(1) var s : sampler;

@group(0) @binding(0) var t : texture_2d<f32>;
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s : texture_2d<f32>;

fn main() -> vec4<f32> {
  return sample(t_s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn sample(t_s_1 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s_1, placeholder_sampler, coords);
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePairRename) {
    auto* src = R"(
@group(0) @binding(1) var t : texture_2d<f32>;

@group(2) @binding(3) var s : sampler;

fn main() -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var fuzzy : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn main() -> vec4<f32> {
  return textureSample(fuzzy, placeholder_sampler, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    CombineSamplers::BindingMap map;
    sem::SamplerTexturePair pair;
    pair.texture_binding_point.group = 0;
    pair.texture_binding_point.binding = 1;
    pair.sampler_binding_point.group = 2;
    pair.sampler_binding_point.binding = 3;
    map[pair] = "fuzzy";
    BindingPoint placeholder{1024, 0};
    data.Add<CombineSamplers::BindingInfo>(map, placeholder);
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePairRenameMiss) {
    auto* src = R"(
@group(0) @binding(1) var t : texture_2d<f32>;

@group(2) @binding(3) var s : sampler;

fn main() -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn main() -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    CombineSamplers::BindingMap map;
    sem::SamplerTexturePair pair;
    pair.texture_binding_point.group = 3;
    pair.texture_binding_point.binding = 2;
    pair.sampler_binding_point.group = 1;
    pair.sampler_binding_point.binding = 0;
    map[pair] = "fuzzy";
    BindingPoint placeholder{1024, 0};
    data.Add<CombineSamplers::BindingInfo>(map, placeholder);
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, AliasedTypes) {
    auto* src = R"(

alias Tex2d = texture_2d<f32>;

@group(0) @binding(0) var t : Tex2d;

@group(0) @binding(1) var s : sampler;

fn sample(t : Tex2d, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

fn main() -> vec4<f32> {
  return sample(t, s, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
alias Tex2d = texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn sample(t_s : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, coords);
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s_1 : texture_2d<f32>;

fn main() -> vec4<f32> {
  return sample(t_s_1, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, AliasedTypes_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return sample(t, s, vec2<f32>(1.0, 2.0));
}

fn sample(t : Tex2d, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

@group(0) @binding(0) var t : Tex2d;
@group(0) @binding(1) var s : sampler;

alias Tex2d = texture_2d<f32>;
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s : texture_2d<f32>;

fn main() -> vec4<f32> {
  return sample(t_s, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn sample(t_s_1 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s_1, placeholder_sampler, coords);
}

alias Tex2d = texture_2d<f32>;
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePairInTwoFunctions) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn g(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

fn f(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return g(t, s, coords);
}

fn main() -> vec4<f32> {
  return f(t, s, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn g(t_s : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, coords);
}

fn f(t_s_1 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return g(t_s_1, coords);
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s_2 : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(t_s_2, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, SimplePairInTwoFunctions_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return f(t, s, vec2<f32>(1.0, 2.0));
}

fn f(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return g(t, s, coords);
}

fn g(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

@group(0) @binding(1) var s : sampler;
@group(0) @binding(0) var t : texture_2d<f32>;
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_s : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(t_s, vec2<f32>(1.0, 2.0));
}

fn f(t_s_1 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return g(t_s_1, coords);
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn g(t_s_2 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s_2, placeholder_sampler, coords);
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TwoFunctionsGenerateSamePair) {
    auto* src = R"(
@group(1) @binding(0) var tex : texture_2d<f32>;

@group(1) @binding(1) var samp : sampler;

fn f() -> vec4<f32> {
  return textureSample(tex, samp, vec2<f32>(1.0, 2.0));
}

fn g() -> vec4<f32> {
  return textureSample(tex, samp, vec2<f32>(3.0, 4.0));
}

fn main() -> vec4<f32> {
  return f() + g();
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn f() -> vec4<f32> {
  return textureSample(tex_samp, placeholder_sampler, vec2<f32>(1.0, 2.0));
}

fn g() -> vec4<f32> {
  return textureSample(tex_samp, placeholder_sampler, vec2<f32>(3.0, 4.0));
}

fn main() -> vec4<f32> {
  return (f() + g());
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, ThreeTexturesThreeSamplers) {
    auto* src = R"(
@group(0) @binding(0) var tex1 : texture_2d<f32>;
@group(0) @binding(1) var tex2 : texture_2d<f32>;
@group(0) @binding(2) var tex3 : texture_2d<f32>;

@group(1) @binding(0) var samp1 : sampler;
@group(1) @binding(1) var samp2: sampler;
@group(1) @binding(2) var samp3: sampler;

fn sample(t : texture_2d<f32>, s : sampler) -> vec4<f32> {
  return textureSample(t, s, vec2<f32>(1.0, 2.0));
}

fn main() -> vec4<f32> {
  return sample(tex1, samp1)
       + sample(tex1, samp2)
       + sample(tex1, samp3)
       + sample(tex2, samp1)
       + sample(tex2, samp2)
       + sample(tex2, samp3)
       + sample(tex3, samp1)
       + sample(tex3, samp2)
       + sample(tex3, samp3);
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn sample(t_s : texture_2d<f32>) -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex1_samp1 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex1_samp2 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex1_samp3 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex2_samp1 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex2_samp2 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex2_samp3 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex3_samp1 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex3_samp2 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex3_samp3 : texture_2d<f32>;

fn main() -> vec4<f32> {
  return ((((((((sample(tex1_samp1) + sample(tex1_samp2)) + sample(tex1_samp3)) + sample(tex2_samp1)) + sample(tex2_samp2)) + sample(tex2_samp3)) + sample(tex3_samp1)) + sample(tex3_samp2)) + sample(tex3_samp3));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TwoFunctionsTwoTexturesDiamond) {
    auto* src = R"(
@group(0) @binding(0) var tex1 : texture_2d<f32>;

@group(0) @binding(1) var tex2 : texture_2d<f32>;

@group(0) @binding(2) var samp : sampler;

fn sample(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

fn f(t1 : texture_2d<f32>, t2 : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return sample(t1, s, coords) + sample(t2, s, coords);
}

fn main() -> vec4<f32> {
  return f(tex1, tex2, samp, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn sample(t_s : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, coords);
}

fn f(t1_s : texture_2d<f32>, t2_s : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return (sample(t1_s, coords) + sample(t2_s, coords));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex1_samp : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex2_samp : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(tex1_samp, tex2_samp, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TwoFunctionsTwoSamplersDiamond) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

@group(0) @binding(1) var samp1 : sampler;

@group(0) @binding(2) var samp2 : sampler;

fn sample(t : texture_2d<f32>, s : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, coords);
}

fn f(t : texture_2d<f32>, s1 : sampler, s2 : sampler, coords : vec2<f32>) -> vec4<f32> {
  return sample(t, s1, coords) + sample(t, s2, coords);
}

fn main() -> vec4<f32> {
  return f(tex, samp1, samp2, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn sample(t_s : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t_s, placeholder_sampler, coords);
}

fn f(t_s1 : texture_2d<f32>, t_s2 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return (sample(t_s1, coords) + sample(t_s2, coords));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp1 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp2 : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(tex_samp1, tex_samp2, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, GlobalTextureLocalSampler) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

@group(0) @binding(1) var samp1 : sampler;

@group(0) @binding(2) var samp2 : sampler;

fn f(s1 : sampler, s2 : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(tex, s1, coords) + textureSample(tex, s2, coords);
}

fn main() -> vec4<f32> {
  return f(samp1, samp2, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn f(tex_s1 : texture_2d<f32>, tex_s2 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return (textureSample(tex_s1, placeholder_sampler, coords) + textureSample(tex_s2, placeholder_sampler, coords));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp1 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp2 : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(tex_samp1, tex_samp2, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, GlobalTextureLocalSampler_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return f(samp1, samp2, vec2<f32>(1.0, 2.0));
}

fn f(s1 : sampler, s2 : sampler, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(tex, s1, coords) + textureSample(tex, s2, coords);
}

@group(0) @binding(1) var samp1 : sampler;
@group(0) @binding(2) var samp2 : sampler;
@group(0) @binding(0) var tex : texture_2d<f32>;
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp1 : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp2 : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(tex_samp1, tex_samp2, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn f(tex_s1 : texture_2d<f32>, tex_s2 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return (textureSample(tex_s1, placeholder_sampler, coords) + textureSample(tex_s2, placeholder_sampler, coords));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, LocalTextureGlobalSampler) {
    auto* src = R"(
@group(0) @binding(0) var tex1 : texture_2d<f32>;

@group(0) @binding(1) var tex2 : texture_2d<f32>;

@group(0) @binding(2) var samp : sampler;

fn f(t1 : texture_2d<f32>, t2 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t1, samp, coords) + textureSample(t2, samp, coords);
}

fn main() -> vec4<f32> {
  return f(tex1, tex2, vec2<f32>(1.0, 2.0));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn f(t1_samp : texture_2d<f32>, t2_samp : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return (textureSample(t1_samp, placeholder_sampler, coords) + textureSample(t2_samp, placeholder_sampler, coords));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex1_samp : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex2_samp : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(tex1_samp, tex2_samp, vec2<f32>(1.0, 2.0));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, LocalTextureGlobalSampler_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return f(tex1, tex2, vec2<f32>(1.0, 2.0));
}

fn f(t1 : texture_2d<f32>, t2 : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return textureSample(t1, samp, coords) + textureSample(t2, samp, coords);
}

@group(0) @binding(2) var samp : sampler;
@group(0) @binding(0) var tex1 : texture_2d<f32>;
@group(0) @binding(1) var tex2 : texture_2d<f32>;
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex1_samp : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex2_samp : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(tex1_samp, tex2_samp, vec2<f32>(1.0, 2.0));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn f(t1_samp : texture_2d<f32>, t2_samp : texture_2d<f32>, coords : vec2<f32>) -> vec4<f32> {
  return (textureSample(t1_samp, placeholder_sampler, coords) + textureSample(t2_samp, placeholder_sampler, coords));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TextureLoadNoSampler) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

fn f(t : texture_2d<f32>, coords : vec2<i32>) -> vec4<f32> {
  return textureLoad(t, coords, 0);
}

fn main() -> vec4<f32> {
  return f(tex, vec2<i32>(1, 2));
}
)";
    auto* expect = R"(
fn f(t_1 : texture_2d<f32>, coords : vec2<i32>) -> vec4<f32> {
  return textureLoad(t_1, coords, 0);
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var fred : texture_2d<f32>;

fn main() -> vec4<f32> {
  return f(fred, vec2<i32>(1, 2));
}
)";

    BindingPoint placeholder{1024, 0};
    sem::SamplerTexturePair pair;
    pair.texture_binding_point.group = 0;
    pair.texture_binding_point.binding = 0;
    pair.sampler_binding_point.group = placeholder.group;
    pair.sampler_binding_point.binding = placeholder.binding;
    CombineSamplers::BindingMap map;
    map[pair] = "fred";
    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(map, placeholder);
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TextureWithAndWithoutSampler) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;
@group(0) @binding(1) var samp : sampler;

fn main() -> vec4<f32> {
  return textureLoad(tex, vec2<i32>(), 0) +
         textureSample(tex, samp, vec2<f32>());
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var barney : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn main() -> vec4<f32> {
  return (textureLoad(barney, vec2<i32>(), 0) + textureSample(barney, placeholder_sampler, vec2<f32>()));
}
)";

    BindingPoint placeholder{1024, 0};
    BindingPoint tex{0, 0};
    BindingPoint samp{0, 1};
    sem::SamplerTexturePair pair, placeholder_pair;
    pair.texture_binding_point.group = tex.group;
    pair.texture_binding_point.binding = tex.binding;
    pair.sampler_binding_point.group = samp.group;
    pair.sampler_binding_point.binding = samp.binding;
    placeholder_pair.texture_binding_point.group = tex.group;
    placeholder_pair.texture_binding_point.binding = tex.binding;
    placeholder_pair.sampler_binding_point.group = placeholder.group;
    placeholder_pair.sampler_binding_point.binding = placeholder.binding;
    CombineSamplers::BindingMap map;
    map[pair] = "barney";
    map[placeholder_pair] = "fred";
    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(map, placeholder);
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TextureSampleCompare) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_depth_2d;

@group(0) @binding(1) var samp : sampler_comparison;

fn main() -> vec4<f32> {
  return vec4<f32>(textureSampleCompare(tex, samp, vec2<f32>(1.0, 2.0), 0.5));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp : texture_depth_2d;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_comparison_sampler : sampler_comparison;

fn main() -> vec4<f32> {
  return vec4<f32>(textureSampleCompare(tex_samp, placeholder_comparison_sampler, vec2<f32>(1.0, 2.0), 0.5));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TextureSampleCompareInAFunction) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_depth_2d;

@group(0) @binding(1) var samp : sampler_comparison;

fn f(t : texture_depth_2d, s : sampler_comparison, coords : vec2<f32>) -> f32 {
  return textureSampleCompare(t, s, coords, 5.0f);
}

fn main() -> vec4<f32> {
  return vec4<f32>(f(tex, samp, vec2<f32>(1.0, 2.0)));
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_comparison_sampler : sampler_comparison;

fn f(t_s : texture_depth_2d, coords : vec2<f32>) -> f32 {
  return textureSampleCompare(t_s, placeholder_comparison_sampler, coords, 5.0f);
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp : texture_depth_2d;

fn main() -> vec4<f32> {
  return vec4<f32>(f(tex_samp, vec2<f32>(1.0, 2.0)));
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, TextureSampleCompareInAFunction_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return vec4<f32>(f(tex, samp, vec2<f32>(1.0, 2.0)));
}

fn f(t : texture_depth_2d, s : sampler_comparison, coords : vec2<f32>) -> f32 {
  return textureSampleCompare(t, s, coords, 5.0f);
}

@group(0) @binding(0) var tex : texture_depth_2d;
@group(0) @binding(1) var samp : sampler_comparison;
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp : texture_depth_2d;

fn main() -> vec4<f32> {
  return vec4<f32>(f(tex_samp, vec2<f32>(1.0, 2.0)));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_comparison_sampler : sampler_comparison;

fn f(t_s : texture_depth_2d, coords : vec2<f32>) -> f32 {
  return textureSampleCompare(t_s, placeholder_comparison_sampler, coords, 5.0f);
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, BindingPointCollision) {
    auto* src = R"(
@group(1) @binding(0) var tex : texture_2d<f32>;

@group(1) @binding(1) var samp : sampler;

@group(0) @binding(0) var<uniform> gcoords : vec2<f32>;

fn main() -> vec4<f32> {
  return textureSample(tex, samp, gcoords);
}
)";
    auto* expect = R"(
@internal(disable_validation__binding_point_collision) @group(0) @binding(0) var<uniform> gcoords : vec2<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn main() -> vec4<f32> {
  return textureSample(tex_samp, placeholder_sampler, gcoords);
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, BindingPointCollision_OutOfOrder) {
    auto* src = R"(
fn main() -> vec4<f32> {
  return textureSample(tex, samp, gcoords);
}

@group(1) @binding(1) var samp : sampler;
@group(0) @binding(0) var<uniform> gcoords : vec2<f32>;
@group(1) @binding(0) var tex : texture_2d<f32>;

)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var tex_samp : texture_2d<f32>;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var placeholder_sampler : sampler;

fn main() -> vec4<f32> {
  return textureSample(tex_samp, placeholder_sampler, gcoords);
}

@internal(disable_validation__binding_point_collision) @group(0) @binding(0) var<uniform> gcoords : vec2<f32>;
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, UnusedTextureFunctionParameter) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn f(tex: texture_2d<f32>) -> u32 {
  return 1u;
}

fn main() {
  _ = f(t);
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_1 : texture_2d<f32>;

fn f() -> u32 {
  return 1u;
}

fn main() {
  _ = f();
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, UnusedSamplerFunctionParameter) {
    auto* src = R"(
@group(0) @binding(0) var s : sampler;

fn f(sampler1: sampler) -> u32 {
  return 1u;
}

fn main() {
  _ = f(s);
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var s_1 : sampler;

fn f() -> u32 {
  return 1u;
}

fn main() {
  _ = f();
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, UnusedTextureAndSamplerFunctionParameter) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn f(tex: texture_2d<f32>, sampler1: sampler) -> u32 {
  return 1u;
}

fn main() {
  _ = f(t, s);
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var s_1 : sampler;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_1 : texture_2d<f32>;

fn f() -> u32 {
  return 1u;
}

fn main() {
  _ = f();
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, UnusedTextureFunctionParameter_Multiple) {
    auto* src = R"(
@group(0) @binding(0) var t1 : texture_2d<f32>;

@group(0) @binding(1) var t2 : texture_2d_array<f32>;

@group(0) @binding(2) var s : sampler;

fn f(tex1: texture_2d<f32>, tex2: texture_2d<f32>, tex3: texture_2d_array<f32>, sampler1: sampler) -> u32 {
  return 1u + textureNumLayers(tex3);
}

fn main() {
  _ = f(t1, t1, t2, s);
}
)";
    auto* expect = R"(
@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var s_1 : sampler;

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t1_1 : texture_2d<f32>;

fn f(tex3_1 : texture_2d_array<f32>) -> u32 {
  return (1u + textureNumLayers(tex3_1));
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t2_1 : texture_2d_array<f32>;

fn main() {
  _ = f(t2_1);
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CombineSamplersTest, UnusedTextureFunctionParameter_Nested) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn f_nested(tex: texture_2d<f32>) -> u32 {
  return 1u;
}

fn f(tex: texture_2d<f32>) -> u32 {
  return f_nested(tex);
}

fn main() {
  _ = f(t);
}
)";
    auto* expect = R"(
fn f_nested(tex_1 : texture_2d<f32>) -> u32 {
  return 1u;
}

fn f(tex_2 : texture_2d<f32>) -> u32 {
  return f_nested(tex_2);
}

@group(0) @binding(0) @internal(disable_validation__binding_point_collision) var t_1 : texture_2d<f32>;

fn main() {
  _ = f(t_1);
}
)";

    ast::transform::DataMap data;
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(), BindingPoint());
    auto got = Run<CombineSamplers>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::glsl::writer
