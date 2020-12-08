// Copyright 2020 The Tint Authors.
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

#include "src/ast/intrinsic_texture_helper_test.h"

#include "src/ast/type_constructor_expression.h"

namespace tint {
namespace ast {
namespace intrinsic {
namespace test {

TextureOverloadCase::TextureOverloadCase() = default;
TextureOverloadCase::TextureOverloadCase(
    ValidTextureOverload o,
    const char* d,
    TextureKind tk,
    type::SamplerKind sk,
    type::TextureDimension td,
    TextureDataType tdt,
    const char* f,
    std::function<ExpressionList(Builder*)> a)
    : overload(o),
      description(d),
      texture_kind(tk),
      sampler_kind(sk),
      texture_dimension(td),
      texture_data_type(tdt),
      function(f),
      args(std::move(a)) {}
TextureOverloadCase::TextureOverloadCase(const TextureOverloadCase&) = default;
TextureOverloadCase::~TextureOverloadCase() = default;

std::vector<TextureOverloadCase> TextureOverloadCase::ValidCases() {
  return {{
              ValidTextureOverload::kSample1dF32,
              "textureSample(t : texture_1d<f32>,\n"
              "              s : sampler,\n"
              "              coords : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k1d,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                return b->ExprList("texture",  // t
                                   "sampler",  // s
                                   1.0f);      // coords
              },
          },
          {
              ValidTextureOverload::kSample1dArrayF32,
              "textureSample(t : texture_1d_array<f32>,\n"
              "              s : sampler,\n"
              "              coords : f32,\n"
              "              array_index : u32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k1dArray,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                return b->ExprList("texture",  // t
                                   "sampler",  // s
                                   1.0f,       // coords
                                   2);         // array_index
              },
          },
          {
              ValidTextureOverload::kSample2dF32,
              "textureSample(t : texture_2d<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                // t
                                   "sampler",                // s
                                   b->vec2<f32>(1.f, 2.f));  // coords
              },
          },
          {
              ValidTextureOverload::kSample2dOffsetF32,
              "textureSample(t : texture_2d<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>\n"
              "              offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   b->vec2<i32>(3, 4));     // offset
              },
          },
          {
              ValidTextureOverload::kSample2dArrayF32,
              "textureSample(t : texture_2d_array<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>,\n"
              "              array_index : u32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3);                      // array_index
              },
          },
          {
              ValidTextureOverload::kSample2dArrayOffsetF32,
              "textureSample(t : texture_2d_array<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>,\n"
              "              array_index : u32\n"
              "              offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   b->vec2<i32>(4, 5));     // offset
              },
          },
          {
              ValidTextureOverload::kSample3dF32,
              "textureSample(t : texture_3d<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec3<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                     // t
                                   "sampler",                     // s
                                   b->vec3<f32>(1.f, 2.f, 3.f));  // coords
              },
          },
          {
              ValidTextureOverload::kSample3dOffsetF32,
              "textureSample(t : texture_3d<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec3<f32>\n"
              "              offset : vec3<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   b->vec3<i32>(4, 5, 6));       // offset
              },
          },
          {
              ValidTextureOverload::kSampleCubeF32,
              "textureSample(t : texture_cube<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec3<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCube,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                     // t
                                   "sampler",                     // s
                                   b->vec3<f32>(1.f, 2.f, 3.f));  // coords
              },
          },
          {
              ValidTextureOverload::kSampleCubeArrayF32,
              "textureSample(t : texture_cube_array<f32>,\n"
              "              s : sampler,\n"
              "              coords : vec3<f32>,\n"
              "              array_index : u32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCubeArray,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4);                           // array_index
              },
          },
          {
              ValidTextureOverload::kSampleDepth2dF32,
              "textureSample(t : texture_depth_2d,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                // t
                                   "sampler",                // s
                                   b->vec2<f32>(1.f, 2.f));  // coords
              },
          },
          {
              ValidTextureOverload::kSampleDepth2dOffsetF32,
              "textureSample(t : texture_depth_2d,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>\n"
              "              offset : vec2<i32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   b->vec2<i32>(3, 4));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleDepth2dArrayF32,
              "textureSample(t : texture_depth_2d_array,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>,\n"
              "              array_index : u32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3);                      // array_index
              },
          },
          {
              ValidTextureOverload::kSampleDepth2dArrayOffsetF32,
              "textureSample(t : texture_depth_2d_array,\n"
              "              s : sampler,\n"
              "              coords : vec2<f32>,\n"
              "              array_index : u32\n"
              "              offset : vec2<i32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   b->vec2<i32>(4, 5));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleDepthCubeF32,
              "textureSample(t : texture_depth_cube,\n"
              "              s : sampler,\n"
              "              coords : vec3<f32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCube,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                     // t
                                   "sampler",                     // s
                                   b->vec3<f32>(1.f, 2.f, 3.f));  // coords
              },
          },
          {
              ValidTextureOverload::kSampleDepthCubeArrayF32,
              "textureSample(t : texture_depth_cube_array,\n"
              "              s : sampler,\n"
              "              coords : vec3<f32>,\n"
              "              array_index : u32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCubeArray,
              TextureDataType::kF32,
              "textureSample",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4);                           // array_index
              },
          },
          {
              ValidTextureOverload::kSampleBias2dF32,
              "textureSampleBias(t : texture_2d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>,\n"
              "                  bias : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3.f);                    // bias
              },
          },
          {
              ValidTextureOverload::kSampleBias2dOffsetF32,
              "textureSampleBias(t : texture_2d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>,\n"
              "                  bias : f32,\n"
              "                  offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3.f,                     // bias
                                   b->vec2<i32>(4, 5));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleBias2dArrayF32,
              "textureSampleBias(t : texture_2d_array<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>,\n"
              "                  array_index : u32,\n"
              "                  bias : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   4,                       // array_index
                                   3.f);                    // bias
              },
          },
          {
              ValidTextureOverload::kSampleBias2dArrayOffsetF32,
              "textureSampleBias(t : texture_2d_array<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>,\n"
              "                  array_index : u32,\n"
              "                  bias : f32,\n"
              "                  offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   4.f,                     // bias
                                   b->vec2<i32>(5, 6));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleBias3dF32,
              "textureSampleBias(t : texture_3d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  bias : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4.f);                         // bias
              },
          },
          {
              ValidTextureOverload::kSampleBias3dOffsetF32,
              "textureSampleBias(t : texture_3d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  bias : f32,\n"
              "                  offset : vec3<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4.f,                          // bias
                                   b->vec3<i32>(5, 6, 7));       // offset
              },
          },
          {
              ValidTextureOverload::kSampleBiasCubeF32,
              "textureSampleBias(t : texture_cube<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  bias : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCube,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4.f);                         // bias
              },
          },
          {
              ValidTextureOverload::kSampleBiasCubeArrayF32,
              "textureSampleBias(t : texture_cube_array<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  array_index : u32,\n"
              "                  bias : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCubeArray,
              TextureDataType::kF32,
              "textureSampleBias",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   3,                            // array_index
                                   4.f);                         // bias
              },
          },
          {
              ValidTextureOverload::kSampleLevel2dF32,
              "textureSampleLevel(t : texture_2d<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   level : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3.f);                    // level
              },
          },
          {
              ValidTextureOverload::kSampleLevel2dOffsetF32,
              "textureSampleLevel(t : texture_2d<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   level : f32,\n"
              "                   offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3.f,                     // level
                                   b->vec2<i32>(4, 5));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleLevel2dArrayF32,
              "textureSampleLevel(t : texture_2d_array<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   array_index : u32,\n"
              "                   level : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   4.f);                    // level
              },
          },
          {
              ValidTextureOverload::kSampleLevel2dArrayOffsetF32,
              "textureSampleLevel(t : texture_2d_array<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   array_index : u32,\n"
              "                   level : f32,\n"
              "                   offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   4.f,                     // level
                                   b->vec2<i32>(5, 6));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleLevel3dF32,
              "textureSampleLevel(t : texture_3d<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec3<f32>,\n"
              "                   level : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4.f);                         // level
              },
          },
          {
              ValidTextureOverload::kSampleLevel3dOffsetF32,
              "textureSampleLevel(t : texture_3d<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec3<f32>,\n"
              "                   level : f32,\n"
              "                   offset : vec3<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4.f,                          // level
                                   b->vec3<i32>(5, 6, 7));       // offset
              },
          },
          {
              ValidTextureOverload::kSampleLevelCubeF32,
              "textureSampleLevel(t : texture_cube<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec3<f32>,\n"
              "                   level : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCube,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4.f);                         // level
              },
          },
          {
              ValidTextureOverload::kSampleLevelCubeArrayF32,
              "textureSampleLevel(t : texture_cube_array<f32>,\n"
              "                   s : sampler,\n"
              "                   coords : vec3<f32>,\n"
              "                   array_index : u32,\n"
              "                   level : f32) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCubeArray,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4,                            // array_index
                                   5.f);                         // level
              },
          },
          {
              ValidTextureOverload::kSampleLevelDepth2dF32,
              "textureSampleLevel(t : texture_depth_2d,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   level : u32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3);                      // level
              },
          },
          {
              ValidTextureOverload::kSampleLevelDepth2dOffsetF32,
              "textureSampleLevel(t : texture_depth_2d,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   level : u32,\n"
              "                   offset : vec2<i32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // level
                                   b->vec2<i32>(4, 5));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleLevelDepth2dArrayF32,
              "textureSampleLevel(t : texture_depth_2d_array,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   array_index : u32,\n"
              "                   level : u32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   4);                      // level
              },
          },
          {
              ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32,
              "textureSampleLevel(t : texture_depth_2d_array,\n"
              "                   s : sampler,\n"
              "                   coords : vec2<f32>,\n"
              "                   array_index : u32,\n"
              "                   level : u32,\n"
              "                   offset : vec2<i32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   4,                       // level
                                   b->vec2<i32>(5, 6));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleLevelDepthCubeF32,
              "textureSampleLevel(t : texture_depth_cube,\n"
              "                   s : sampler,\n"
              "                   coords : vec3<f32>,\n"
              "                   level : u32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCube,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4);                           // level
              },
          },
          {
              ValidTextureOverload::kSampleLevelDepthCubeArrayF32,
              "textureSampleLevel(t : texture_depth_cube_array,\n"
              "                   s : sampler,\n"
              "                   coords : vec3<f32>,\n"
              "                   array_index : u32,\n"
              "                   level : u32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCubeArray,
              TextureDataType::kF32,
              "textureSampleLevel",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4,                            // array_index
                                   5);                           // level
              },
          },
          {
              ValidTextureOverload::kSampleGrad2dF32,
              "textureSampleGrad(t : texture_2d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>\n"
              "                  ddx : vec2<f32>,\n"
              "                  ddy : vec2<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                  // t
                                   "sampler",                  // s
                                   b->vec2<f32>(1.0f, 2.0f),   // coords
                                   b->vec2<f32>(3.0f, 4.0f),   // ddx
                                   b->vec2<f32>(5.0f, 6.0f));  // ddy
              },
          },
          {
              ValidTextureOverload::kSampleGrad2dOffsetF32,
              "textureSampleGrad(t : texture_2d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>,\n"
              "                  ddx : vec2<f32>,\n"
              "                  ddy : vec2<f32>,\n"
              "                  offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   b->vec2<f32>(3.f, 4.f),  // ddx
                                   b->vec2<f32>(5.f, 6.f),  // ddy
                                   b->vec2<i32>(7, 8));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleGrad2dArrayF32,
              "textureSampleGrad(t : texture_2d_array<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>,\n"
              "                  array_index : u32,\n"
              "                  ddx : vec2<f32>,\n"
              "                  ddy : vec2<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                // t
                                   "sampler",                // s
                                   b->vec2<f32>(1.f, 2.f),   // coords
                                   3,                        // array_index
                                   b->vec2<f32>(4.f, 5.f),   // ddx
                                   b->vec2<f32>(6.f, 7.f));  // ddy
              },
          },
          {
              ValidTextureOverload::kSampleGrad2dArrayOffsetF32,
              "textureSampleGrad(t : texture_2d_array<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec2<f32>,\n"
              "                  array_index : u32,\n"
              "                  ddx : vec2<f32>,\n"
              "                  ddy : vec2<f32>,\n"
              "                  offset : vec2<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3,                       // array_index
                                   b->vec2<f32>(4.f, 5.f),  // ddx
                                   b->vec2<f32>(6.f, 7.f),  // ddy
                                   b->vec2<i32>(8, 9));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleGrad3dF32,
              "textureSampleGrad(t : texture_3d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  ddx : vec3<f32>,\n"
              "                  ddy : vec3<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                     // t
                                   "sampler",                     // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),   // coords
                                   b->vec3<f32>(4.f, 5.f, 6.f),   // ddx
                                   b->vec3<f32>(7.f, 8.f, 9.f));  // ddy
              },
          },
          {
              ValidTextureOverload::kSampleGrad3dOffsetF32,
              "textureSampleGrad(t : texture_3d<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  ddx : vec3<f32>,\n"
              "                  ddy : vec3<f32>,\n"
              "                  offset : vec3<i32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::k3d,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   b->vec3<f32>(4.f, 5.f, 6.f),  // ddx
                                   b->vec3<f32>(7.f, 8.f, 9.f),  // ddy
                                   b->vec3<i32>(10, 11, 12));    // offset
              },
          },
          {
              ValidTextureOverload::kSampleGradCubeF32,
              "textureSampleGrad(t : texture_cube<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  ddx : vec3<f32>,\n"
              "                  ddy : vec3<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCube,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                     // t
                                   "sampler",                     // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),   // coords
                                   b->vec3<f32>(4.f, 5.f, 6.f),   // ddx
                                   b->vec3<f32>(7.f, 8.f, 9.f));  // ddy
              },
          },
          {
              ValidTextureOverload::kSampleGradCubeArrayF32,
              "textureSampleGrad(t : texture_cube_array<f32>,\n"
              "                  s : sampler,\n"
              "                  coords : vec3<f32>,\n"
              "                  array_index : u32,\n"
              "                  ddx : vec3<f32>,\n"
              "                  ddy : vec3<f32>) -> vec4<f32>",
              TextureKind::kRegular,
              type::SamplerKind::kSampler,
              type::TextureDimension::kCubeArray,
              TextureDataType::kF32,
              "textureSampleGrad",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4,                            // array_index
                                   b->vec3<f32>(5.f, 6.f, 7.f),  // ddx
                                   b->vec3<f32>(8.f, 9.f, 10.f));  // ddy
              },
          },
          {
              ValidTextureOverload::kSampleGradDepth2dF32,
              "textureSampleCompare(t : texture_depth_2d,\n"
              "                     s : sampler_comparison,\n"
              "                     coords : vec2<f32>,\n"
              "                     depth_ref : f32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kComparisonSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleCompare",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3.f);                    // depth_ref
              },
          },
          {
              ValidTextureOverload::kSampleGradDepth2dOffsetF32,
              "textureSampleCompare(t : texture_depth_2d,\n"
              "                     s : sampler_comparison,\n"
              "                     coords : vec2<f32>,\n"
              "                     depth_ref : f32,\n"
              "                     offset : vec2<i32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kComparisonSampler,
              type::TextureDimension::k2d,
              TextureDataType::kF32,
              "textureSampleCompare",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   3.f,                     // depth_ref
                                   b->vec2<i32>(4, 5));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleGradDepth2dArrayF32,
              "textureSampleCompare(t : texture_depth_2d_array,\n"
              "                     s : sampler_comparison,\n"
              "                     coords : vec2<f32>,\n"
              "                     array_index : u32,\n"
              "                     depth_ref : f32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kComparisonSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleCompare",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   4,                       // array_index
                                   3.f);                    // depth_ref
              },
          },
          {
              ValidTextureOverload::kSampleGradDepth2dArrayOffsetF32,
              "textureSampleCompare(t : texture_depth_2d_array,\n"
              "                     s : sampler_comparison,\n"
              "                     coords : vec2<f32>,\n"
              "                     array_index : u32,\n"
              "                     depth_ref : f32,\n"
              "                     offset : vec2<i32>) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kComparisonSampler,
              type::TextureDimension::k2dArray,
              TextureDataType::kF32,
              "textureSampleCompare",
              [](Builder* b) {
                using f32 = Builder::f32;
                using i32 = Builder::i32;
                return b->ExprList("texture",               // t
                                   "sampler",               // s
                                   b->vec2<f32>(1.f, 2.f),  // coords
                                   4,                       // array_index
                                   3.f,                     // depth_ref
                                   b->vec2<i32>(5, 6));     // offset
              },
          },
          {
              ValidTextureOverload::kSampleGradDepthCubeF32,
              "textureSampleCompare(t : texture_depth_cube,\n"
              "                     s : sampler_comparison,\n"
              "                     coords : vec3<f32>,\n"
              "                     depth_ref : f32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kComparisonSampler,
              type::TextureDimension::kCube,
              TextureDataType::kF32,
              "textureSampleCompare",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4.f);                         // depth_ref
              },
          },
          {
              ValidTextureOverload::kSampleGradDepthCubeArrayF32,
              "textureSampleCompare(t : texture_depth_cube_array,\n"
              "                     s : sampler_comparison,\n"
              "                     coords : vec3<f32>,\n"
              "                     array_index : u32,\n"
              "                     depth_ref : f32) -> f32",
              TextureKind::kDepth,
              type::SamplerKind::kComparisonSampler,
              type::TextureDimension::kCubeArray,
              TextureDataType::kF32,
              "textureSampleCompare",
              [](Builder* b) {
                using f32 = Builder::f32;
                return b->ExprList("texture",                    // t
                                   "sampler",                    // s
                                   b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                                   4,                            // array_index
                                   5.f);                         // depth_ref
              },
          }};
}

}  // namespace test
}  // namespace intrinsic
}  // namespace ast
}  // namespace tint
