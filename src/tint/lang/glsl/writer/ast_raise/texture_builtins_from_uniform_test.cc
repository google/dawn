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

#include "src/tint/lang/glsl/writer/ast_raise/texture_builtins_from_uniform.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::glsl::writer {
namespace {

using TextureBuiltinsFromUniformTest = ast::transform::TransformTest;

TEST_F(TextureBuiltinsFromUniformTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    EXPECT_FALSE(ShouldRun<TextureBuiltinsFromUniform>(src, data));
}

TEST_F(TextureBuiltinsFromUniformTest, ShouldRunNoTextureNumLevels) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  _ = textureDimensions(t);
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    EXPECT_FALSE(ShouldRun<TextureBuiltinsFromUniform>(src, data));
}

TEST_F(TextureBuiltinsFromUniformTest, ShouldRunWithTextureNumLevels) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = textureNumLevels(t);
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    EXPECT_TRUE(ShouldRun<TextureBuiltinsFromUniform>(src, data));
}

TEST_F(TextureBuiltinsFromUniformTest, Error_MissingTransformData) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = textureNumLevels(t);
}
)";

    auto* expect =
        "error: missing transform data for tint::glsl::writer::TextureBuiltinsFromUniform";

    auto got = Run<TextureBuiltinsFromUniform>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(TextureBuiltinsFromUniformTest, BasicTextureNumLevels) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = textureNumLevels(t);
}
)";

    auto* expect = R"(
struct tint_symbol {
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var t : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = tint_symbol_1.texture_builtin_value_0;
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    // Note: Using the following EXPECT_EQ directly on BindingPointToFieldAndOffset seems to cause
    // compiler to hang. EXPECT_EQ(
    //     TextureBuiltinsFromUniformOptions::BindingPointToFieldAndOffset{
    //         {BindgPoint{0u, 0u},
    //          std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u)}},
    //     val->bindpoint_to_data);
    EXPECT_EQ(1u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, BasicTextureNumSamples) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var samples : u32 = textureNumSamples(t);
}
)";

    auto* expect = R"(
struct tint_symbol {
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var t : texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var samples : u32 = tint_symbol_1.texture_builtin_value_0;
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(1u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumSamples, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimes) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = textureNumLevels(tex);
  len = textureNumLevels(tex);
}
)";

    auto* expect = R"(
struct tint_symbol {
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var tex : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = tint_symbol_1.texture_builtin_value_0;
  len = tint_symbol_1.texture_builtin_value_0;
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(1u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimesTextureNumSamples) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = textureNumSamples(tex);
  len = textureNumSamples(tex);
}
)";

    auto* expect = R"(
struct tint_symbol {
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var tex : texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = tint_symbol_1.texture_builtin_value_0;
  len = tint_symbol_1.texture_builtin_value_0;
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(1u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumSamples, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, TextureAsFunctionParameterBasic) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

fn f(t: texture_2d<f32>) -> u32 {
  return textureNumLevels(t);
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f(tex);
}
)";

    auto* expect = R"(
struct tint_symbol_1 {
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_2 : tint_symbol_1;

@group(0) @binding(0) var tex : texture_2d<f32>;

fn f(t : texture_2d<f32>, tint_symbol : u32) -> u32 {
  return tint_symbol;
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f(tex, tint_symbol_2.texture_builtin_value_0);
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(1u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, TextureAsFunctionParameterUsedTwice) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

fn f(t: texture_2d<f32>) -> u32 {
  var len = textureNumLevels(t);
  len += textureNumLevels(t);
  return len;
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f(tex);
}
)";

    auto* expect = R"(
struct tint_symbol_1 {
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_2 : tint_symbol_1;

@group(0) @binding(0) var tex : texture_2d<f32>;

fn f(t : texture_2d<f32>, tint_symbol : u32) -> u32 {
  var len = tint_symbol;
  len += tint_symbol;
  return len;
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f(tex, tint_symbol_2.texture_builtin_value_0);
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(1u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, TextureAsFunctionParameterMultipleParameters) {
    auto* src = R"(
@group(0) @binding(0) var tex1 : texture_2d<f32>;
@group(0) @binding(1) var tex2 : texture_2d<f32>;
@group(0) @binding(2) var tex3 : texture_2d<f32>;

fn f(t1: texture_2d<f32>, t2: texture_2d<f32>, t3: texture_2d<f32>) -> u32 {
  return textureNumLevels(t1) + textureNumLevels(t2) + textureNumLevels(t3);
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f(tex1, tex2, tex3);
}
)";

    auto* expect = R"(
struct tint_symbol_3 {
  texture_builtin_value_0 : u32,
  texture_builtin_value_1 : u32,
  texture_builtin_value_2 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_4 : tint_symbol_3;

@group(0) @binding(0) var tex1 : texture_2d<f32>;

@group(0) @binding(1) var tex2 : texture_2d<f32>;

@group(0) @binding(2) var tex3 : texture_2d<f32>;

fn f(t1 : texture_2d<f32>, t2 : texture_2d<f32>, t3 : texture_2d<f32>, tint_symbol : u32, tint_symbol_1 : u32, tint_symbol_2 : u32) -> u32 {
  return ((tint_symbol + tint_symbol_1) + tint_symbol_2);
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f(tex1, tex2, tex3, tint_symbol_4.texture_builtin_value_0, tint_symbol_4.texture_builtin_value_1, tint_symbol_4.texture_builtin_value_2);
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(3u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 4u),
              val->bindpoint_to_data.at(BindingPoint{0, 1}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 8u),
              val->bindpoint_to_data.at(BindingPoint{0, 2}));
}

TEST_F(TextureBuiltinsFromUniformTest, TextureAsFunctionParameterNested) {
    auto* src = R"(
@group(0) @binding(0) var tex : texture_2d<f32>;

fn f2(tt: texture_2d<f32>) -> u32 {
  return textureNumLevels(tt);
}

fn f1(t: texture_2d<f32>) -> u32 {
  return f2(t);
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f1(tex);
}
)";

    auto* expect = R"(
struct tint_symbol_2 {
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_3 : tint_symbol_2;

@group(0) @binding(0) var tex : texture_2d<f32>;

fn f2(tt : texture_2d<f32>, tint_symbol : u32) -> u32 {
  return tint_symbol;
}

fn f1(t : texture_2d<f32>, tint_symbol_1 : u32) -> u32 {
  return f2(t, tint_symbol_1);
}

@compute @workgroup_size(1)
fn main() {
  var len : u32 = f1(tex, tint_symbol_3.texture_builtin_value_0);
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(1u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, TextureAsFunctionParameterMixed) {
    auto* src = R"(
@group(0) @binding(0) var tex0 : texture_2d<f32>;
@group(0) @binding(1) var tex1 : texture_2d<f32>;
@group(0) @binding(2) var tex2 : texture_2d<f32>;
@group(0) @binding(3) var tex3 : texture_2d<f32>;
@group(0) @binding(4) var tex4 : texture_2d_array<f32>; // unused for textureNumLevels

fn f_nested(t1: texture_2d<f32>, t2: texture_2d<f32>) -> u32 {
  return textureNumLevels(t1) + textureNumLevels(t2);
}

fn f1(a: u32, t: texture_2d<f32>) -> u32 {
  return a + f_nested(t, tex1) + textureNumLevels(tex3);
}

@compute @workgroup_size(1)
fn main() {
  _ = textureNumLayers(tex4);
  _ = f1(9u, tex0);
  _ = f_nested(tex2, tex2);
  _ = f_nested(tex1, tex0);
}
)";

    auto* expect = R"(
struct tint_symbol_3 {
  texture_builtin_value_0 : u32,
  texture_builtin_value_1 : u32,
  texture_builtin_value_2 : u32,
  texture_builtin_value_3 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_4 : tint_symbol_3;

@group(0) @binding(0) var tex0 : texture_2d<f32>;

@group(0) @binding(1) var tex1 : texture_2d<f32>;

@group(0) @binding(2) var tex2 : texture_2d<f32>;

@group(0) @binding(3) var tex3 : texture_2d<f32>;

@group(0) @binding(4) var tex4 : texture_2d_array<f32>;

fn f_nested(t1 : texture_2d<f32>, t2 : texture_2d<f32>, tint_symbol : u32, tint_symbol_1 : u32) -> u32 {
  return (tint_symbol + tint_symbol_1);
}

fn f1(a : u32, t : texture_2d<f32>, tint_symbol_2 : u32) -> u32 {
  return ((a + f_nested(t, tex1, tint_symbol_2, tint_symbol_4.texture_builtin_value_0)) + tint_symbol_4.texture_builtin_value_1);
}

@compute @workgroup_size(1)
fn main() {
  _ = textureNumLayers(tex4);
  _ = f1(9u, tex0, tint_symbol_4.texture_builtin_value_2);
  _ = f_nested(tex2, tex2, tint_symbol_4.texture_builtin_value_3, tint_symbol_4.texture_builtin_value_3);
  _ = f_nested(tex1, tex0, tint_symbol_4.texture_builtin_value_0, tint_symbol_4.texture_builtin_value_2);
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(4u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 1}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 4u),
              val->bindpoint_to_data.at(BindingPoint{0, 3}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 8u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 12u),
              val->bindpoint_to_data.at(BindingPoint{0, 2}));
}

TEST_F(TextureBuiltinsFromUniformTest, MultipleTextures) {
    auto* src = R"(
@group(0) @binding(0) var t0 : texture_2d<f32>;
@group(0) @binding(1) var t1 : texture_multisampled_2d<f32>;
@group(0) @binding(2) var t2 : texture_2d_array<f32>;
@group(0) @binding(3) var t3 : texture_cube<f32>;
@group(0) @binding(4) var t4 : texture_depth_2d;
@group(1) @binding(0) var t5 : texture_depth_multisampled_2d;

@compute @workgroup_size(1)
fn main() {
  _ = textureNumLevels(t0);
  _ = textureNumSamples(t1);
  _ = textureNumLevels(t2);
  _ = textureNumLevels(t3);
  _ = textureNumLevels(t4);
  _ = textureNumSamples(t5);
}
)";

    auto* expect = R"(
struct tint_symbol {
  texture_builtin_value_0 : u32,
  texture_builtin_value_1 : u32,
  texture_builtin_value_2 : u32,
  texture_builtin_value_3 : u32,
  texture_builtin_value_4 : u32,
  texture_builtin_value_5 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var t0 : texture_2d<f32>;

@group(0) @binding(1) var t1 : texture_multisampled_2d<f32>;

@group(0) @binding(2) var t2 : texture_2d_array<f32>;

@group(0) @binding(3) var t3 : texture_cube<f32>;

@group(0) @binding(4) var t4 : texture_depth_2d;

@group(1) @binding(0) var t5 : texture_depth_multisampled_2d;

@compute @workgroup_size(1)
fn main() {
  _ = tint_symbol_1.texture_builtin_value_0;
  _ = tint_symbol_1.texture_builtin_value_1;
  _ = tint_symbol_1.texture_builtin_value_2;
  _ = tint_symbol_1.texture_builtin_value_3;
  _ = tint_symbol_1.texture_builtin_value_4;
  _ = tint_symbol_1.texture_builtin_value_5;
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(6u, val->bindpoint_to_data.size());
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumSamples, 4u),
              val->bindpoint_to_data.at(BindingPoint{0, 1}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 8u),
              val->bindpoint_to_data.at(BindingPoint{0, 2}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 12u),
              val->bindpoint_to_data.at(BindingPoint{0, 3}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 16u),
              val->bindpoint_to_data.at(BindingPoint{0, 4}));
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumSamples, 20u),
              val->bindpoint_to_data.at(BindingPoint{1, 0}));
}

TEST_F(TextureBuiltinsFromUniformTest, BindingPointExist) {
    auto* src = R"(
struct tint_symbol {
  foo : array<vec4<u32>, 1u>,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var t : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = textureNumLevels(t);
}
)";

    auto* expect = R"(
struct tint_symbol {
  foo : array<vec4<u32>, 1u>,
  texture_builtin_value_0 : u32,
}

@group(0) @binding(30) var<uniform> tint_symbol_1 : tint_symbol;

@group(0) @binding(0) var t : texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = tint_symbol_1.texture_builtin_value_0;
}
)";

    TextureBuiltinsFromUniform::Config cfg({0, 30u});

    ast::transform::DataMap data;
    data.Add<TextureBuiltinsFromUniform::Config>(std::move(cfg));

    auto got = Run<TextureBuiltinsFromUniform>(src, data);

    EXPECT_EQ(expect, str(got));
    auto* val = got.data.Get<TextureBuiltinsFromUniform::Result>();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(std::make_pair(TextureBuiltinsFromUniformOptions::Field::TextureNumLevels, 0u),
              val->bindpoint_to_data.at(BindingPoint{0, 0}));
}

}  // namespace
}  // namespace tint::glsl::writer
