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

#include "src/sem/depth_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"

namespace tint {
namespace ast {
namespace intrinsic {
namespace test {

using u32 = ProgramBuilder::u32;
using i32 = ProgramBuilder::i32;
using f32 = ProgramBuilder::f32;

TextureOverloadCase::TextureOverloadCase(
    ValidTextureOverload o,
    const char* desc,
    TextureKind tk,
    ast::SamplerKind sk,
    ast::TextureDimension dims,
    TextureDataType datatype,
    const char* f,
    std::function<ExpressionList(ProgramBuilder*)> a)
    : overload(o),
      description(desc),
      texture_kind(tk),
      sampler_kind(sk),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)) {}
TextureOverloadCase::TextureOverloadCase(
    ValidTextureOverload o,
    const char* desc,
    TextureKind tk,
    ast::TextureDimension dims,
    TextureDataType datatype,
    const char* f,
    std::function<ExpressionList(ProgramBuilder*)> a)
    : overload(o),
      description(desc),
      texture_kind(tk),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)) {}
TextureOverloadCase::TextureOverloadCase(
    ValidTextureOverload o,
    const char* d,
    AccessControl::Access access,
    ast::ImageFormat i,
    ast::TextureDimension dims,
    TextureDataType datatype,
    const char* f,
    std::function<ExpressionList(ProgramBuilder*)> a)
    : overload(o),
      description(d),
      texture_kind(TextureKind::kStorage),
      access_control(access),
      image_format(i),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)) {}
TextureOverloadCase::TextureOverloadCase(const TextureOverloadCase&) = default;
TextureOverloadCase::~TextureOverloadCase() = default;

std::ostream& operator<<(std::ostream& out, const TextureKind& kind) {
  switch (kind) {
    case TextureKind::kRegular:
      out << "regular";
      break;
    case TextureKind::kDepth:
      out << "depth";
      break;
    case TextureKind::kMultisampled:
      out << "multisampled";
      break;
    case TextureKind::kStorage:
      out << "storage";
      break;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const TextureDataType& ty) {
  switch (ty) {
    case TextureDataType::kF32:
      out << "f32";
      break;
    case TextureDataType::kU32:
      out << "u32";
      break;
    case TextureDataType::kI32:
      out << "i32";
      break;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const TextureOverloadCase& data) {
  out << "TextureOverloadCase " << static_cast<int>(data.overload) << "\n";
  out << data.description << "\n";
  out << "texture_kind:      " << data.texture_kind << "\n";
  out << "sampler_kind:      ";
  if (data.texture_kind != TextureKind::kStorage) {
    out << data.sampler_kind;
  } else {
    out << "<unused>";
  }
  out << "\n";
  out << "access_control:    " << data.access_control << "\n";
  out << "image_format:      " << data.image_format << "\n";
  out << "texture_dimension: " << data.texture_dimension << "\n";
  out << "texture_data_type: " << data.texture_data_type << "\n";
  return out;
}

ast::Type* TextureOverloadCase::buildResultVectorComponentType(
    ProgramBuilder* b) const {
  switch (texture_data_type) {
    case ast::intrinsic::test::TextureDataType::kF32:
      return b->ty.f32();
    case ast::intrinsic::test::TextureDataType::kU32:
      return b->ty.u32();
    case ast::intrinsic::test::TextureDataType::kI32:
      return b->ty.i32();
  }

  TINT_UNREACHABLE(b->Diagnostics());
  return {};
}

ast::Variable* TextureOverloadCase::buildTextureVariable(
    ProgramBuilder* b) const {
  DecorationList decos = {
      b->create<ast::GroupDecoration>(0),
      b->create<ast::BindingDecoration>(0),
  };
  switch (texture_kind) {
    case ast::intrinsic::test::TextureKind::kRegular:
      return b->Global("texture",
                       b->ty.sampled_texture(texture_dimension,
                                             buildResultVectorComponentType(b)),
                       ast::StorageClass::kNone, nullptr, decos);

    case ast::intrinsic::test::TextureKind::kDepth:
      return b->Global("texture", b->ty.depth_texture(texture_dimension),
                       ast::StorageClass::kNone, nullptr, decos);

    case ast::intrinsic::test::TextureKind::kMultisampled:
      return b->Global(
          "texture",
          b->ty.multisampled_texture(texture_dimension,
                                     buildResultVectorComponentType(b)),
          ast::StorageClass::kNone, nullptr, decos);

    case ast::intrinsic::test::TextureKind::kStorage: {
      auto* st = b->ty.storage_texture(texture_dimension, image_format);
      auto* ac = b->ty.access(access_control, st);
      return b->Global("texture", ac, ast::StorageClass::kNone, nullptr, decos);
    }
  }

  TINT_UNREACHABLE(b->Diagnostics());
  return nullptr;
}

ast::Variable* TextureOverloadCase::buildSamplerVariable(
    ProgramBuilder* b) const {
  DecorationList decos = {
      b->create<ast::GroupDecoration>(0),
      b->create<ast::BindingDecoration>(1),
  };
  return b->Global("sampler", b->ty.sampler(sampler_kind),
                   ast::StorageClass::kNone, nullptr, decos);
}

std::vector<TextureOverloadCase> TextureOverloadCase::ValidCases() {
  return {
      {
          ValidTextureOverload::kDimensions1d,
          "textureDimensions(t : texture_1d<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k1d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensions2d,
          "textureDimensions(t : texture_2d<f32>) -> vec2<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensions2dLevel,
          "textureDimensions(t     : texture_2d<f32>,\n"
          "                  level : i32) -> vec2<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensions2dArray,
          "textureDimensions(t : texture_2d_array<f32>) -> vec2<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensions2dArrayLevel,
          "textureDimensions(t     : texture_2d_array<f32>,\n"
          "                  level : i32) -> vec2<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensions3d,
          "textureDimensions(t : texture_3d<f32>) -> vec3<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensions3dLevel,
          "textureDimensions(t     : texture_3d<f32>,\n"
          "                  level : i32) -> vec3<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensionsCube,
          "textureDimensions(t : texture_cube<f32>) -> vec3<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsCubeLevel,
          "textureDimensions(t     : texture_cube<f32>,\n"
          "                  level : i32) -> vec3<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensionsCubeArray,
          "textureDimensions(t : texture_cube_array<f32>) -> vec3<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsCubeArrayLevel,
          "textureDimensions(t     : texture_cube_array<f32>,\n"
          "                  level : i32) -> vec3<i32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensionsMultisampled2d,
          "textureDimensions(t : texture_multisampled_2d<f32>)-> vec2<i32>",
          TextureKind::kMultisampled,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsDepth2d,
          "textureDimensions(t : texture_depth_2d) -> vec2<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsDepth2dLevel,
          "textureDimensions(t     : texture_depth_2d,\n"
          "                  level : i32) -> vec2<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensionsDepth2dArray,
          "textureDimensions(t : texture_depth_2d_array) -> vec2<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsDepth2dArrayLevel,
          "textureDimensions(t     : texture_depth_2d_array,\n"
          "                  level : i32) -> vec2<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensionsDepthCube,
          "textureDimensions(t : texture_depth_cube) -> vec3<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsDepthCubeLevel,
          "textureDimensions(t     : texture_depth_cube,\n"
          "                  level : i32) -> vec3<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensionsDepthCubeArray,
          "textureDimensions(t : texture_depth_cube_array) -> vec3<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsDepthCubeArrayLevel,
          "textureDimensions(t     : texture_depth_cube_array,\n"
          "                  level : i32) -> vec3<i32>",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture", 1); },
      },
      {
          ValidTextureOverload::kDimensionsStorageRO1d,
          "textureDimensions(t : texture_storage_1d<rgba32float>) -> i32",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k1d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsStorageRO2d,
          "textureDimensions(t : texture_storage_2d<rgba32float>) -> "
          "vec2<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsStorageRO2dArray,
          "textureDimensions(t : texture_storage_2d_array<rgba32float>) -> "
          "vec2<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsStorageRO3d,
          "textureDimensions(t : texture_storage_3d<rgba32float>) -> "
          "vec3<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsStorageWO1d,
          "textureDimensions(t : texture_storage_1d<rgba32float>) -> i32",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k1d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsStorageWO2d,
          "textureDimensions(t : texture_storage_2d<rgba32float>) -> "
          "vec2<i32>",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsStorageWO2dArray,
          "textureDimensions(t : texture_storage_2d_array<rgba32float>) -> "
          "vec2<i32>",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kDimensionsStorageWO3d,
          "textureDimensions(t : texture_storage_3d<rgba32float>) -> "
          "vec3<i32>",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureDimensions",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLayers2dArray,
          "textureNumLayers(t : texture_2d_array<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureNumLayers",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLayersCubeArray,
          "textureNumLayers(t : texture_cube_array<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureNumLayers",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLayersDepth2dArray,
          "textureNumLayers(t : texture_depth_2d_array) -> i32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureNumLayers",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLayersDepthCubeArray,
          "textureNumLayers(t : texture_depth_cube_array) -> i32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureNumLayers",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLayersStorageWO2dArray,
          "textureNumLayers(t : texture_storage_2d_array<rgba32float>) -> i32",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureNumLayers",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevels2d,
          "textureNumLevels(t : texture_2d<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevels2dArray,
          "textureNumLevels(t : texture_2d_array<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevels3d,
          "textureNumLevels(t : texture_3d<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevelsCube,
          "textureNumLevels(t : texture_cube<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevelsCubeArray,
          "textureNumLevels(t : texture_cube_array<f32>) -> i32",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevelsDepth2d,
          "textureNumLevels(t : texture_depth_2d) -> i32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevelsDepth2dArray,
          "textureNumLevels(t : texture_depth_2d_array) -> i32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevelsDepthCube,
          "textureNumLevels(t : texture_depth_cube) -> i32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumLevelsDepthCubeArray,
          "textureNumLevels(t : texture_depth_cube_array) -> i32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureNumLevels",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kNumSamplesMultisampled2d,
          "textureNumSamples(t : texture_multisampled_2d<f32>) -> i32",
          TextureKind::kMultisampled,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureNumSamples",
          [](ProgramBuilder* b) { return b->ExprList("texture"); },
      },
      {
          ValidTextureOverload::kSample1dF32,
          "textureSample(t      : texture_1d<f32>,\n"
          "              s      : sampler,\n"
          "              coords : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k1d,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",  // t
                               "sampler",  // s
                               1.0f);      // coords
          },
      },
      {
          ValidTextureOverload::kSample2dF32,
          "textureSample(t      : texture_2d<f32>,\n"
          "              s      : sampler,\n"
          "              coords : vec2<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                // t
                               "sampler",                // s
                               b->vec2<f32>(1.f, 2.f));  // coords
          },
      },
      {
          ValidTextureOverload::kSample2dOffsetF32,
          "textureSample(t      : texture_2d<f32>,\n"
          "              s      : sampler,\n"
          "              coords : vec2<f32>\n"
          "              offset : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               b->vec2<i32>(3, 4));     // offset
          },
      },
      {
          ValidTextureOverload::kSample2dArrayF32,
          "textureSample(t           : texture_2d_array<f32>,\n"
          "              s           : sampler,\n"
          "              coords      : vec2<f32>,\n"
          "              array_index : i32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3);                      // array_index
          },
      },
      {
          ValidTextureOverload::kSample2dArrayOffsetF32,
          "textureSample(t           : texture_2d_array<f32>,\n"
          "              s           : sampler,\n"
          "              coords      : vec2<f32>,\n"
          "              array_index : i32\n"
          "              offset      : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3,                       // array_index
                               b->vec2<i32>(4, 5));     // offset
          },
      },
      {
          ValidTextureOverload::kSample3dF32,
          "textureSample(t      : texture_3d<f32>,\n"
          "              s      : sampler,\n"
          "              coords : vec3<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                     // t
                               "sampler",                     // s
                               b->vec3<f32>(1.f, 2.f, 3.f));  // coords
          },
      },
      {
          ValidTextureOverload::kSample3dOffsetF32,
          "textureSample(t      : texture_3d<f32>,\n"
          "              s      : sampler,\n"
          "              coords : vec3<f32>\n"
          "              offset : vec3<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               b->vec3<i32>(4, 5, 6));       // offset
          },
      },
      {
          ValidTextureOverload::kSampleCubeF32,
          "textureSample(t      : texture_cube<f32>,\n"
          "              s      : sampler,\n"
          "              coords : vec3<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                     // t
                               "sampler",                     // s
                               b->vec3<f32>(1.f, 2.f, 3.f));  // coords
          },
      },
      {
          ValidTextureOverload::kSampleCubeArrayF32,
          "textureSample(t           : texture_cube_array<f32>,\n"
          "              s           : sampler,\n"
          "              coords      : vec3<f32>,\n"
          "              array_index : i32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4);                           // array_index
          },
      },
      {
          ValidTextureOverload::kSampleDepth2dF32,
          "textureSample(t      : texture_depth_2d,\n"
          "              s      : sampler,\n"
          "              coords : vec2<f32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                // t
                               "sampler",                // s
                               b->vec2<f32>(1.f, 2.f));  // coords
          },
      },
      {
          ValidTextureOverload::kSampleDepth2dOffsetF32,
          "textureSample(t      : texture_depth_2d,\n"
          "              s      : sampler,\n"
          "              coords : vec2<f32>\n"
          "              offset : vec2<i32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               b->vec2<i32>(3, 4));     // offset
          },
      },
      {
          ValidTextureOverload::kSampleDepth2dArrayF32,
          "textureSample(t           : texture_depth_2d_array,\n"
          "              s           : sampler,\n"
          "              coords      : vec2<f32>,\n"
          "              array_index : i32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3);                      // array_index
          },
      },
      {
          ValidTextureOverload::kSampleDepth2dArrayOffsetF32,
          "textureSample(t           : texture_depth_2d_array,\n"
          "              s           : sampler,\n"
          "              coords      : vec2<f32>,\n"
          "              array_index : i32\n"
          "              offset      : vec2<i32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3,                       // array_index
                               b->vec2<i32>(4, 5));     // offset
          },
      },
      {
          ValidTextureOverload::kSampleDepthCubeF32,
          "textureSample(t      : texture_depth_cube,\n"
          "              s      : sampler,\n"
          "              coords : vec3<f32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                     // t
                               "sampler",                     // s
                               b->vec3<f32>(1.f, 2.f, 3.f));  // coords
          },
      },
      {
          ValidTextureOverload::kSampleDepthCubeArrayF32,
          "textureSample(t           : texture_depth_cube_array,\n"
          "              s           : sampler,\n"
          "              coords      : vec3<f32>,\n"
          "              array_index : i32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureSample",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4);                           // array_index
          },
      },
      {
          ValidTextureOverload::kSampleBias2dF32,
          "textureSampleBias(t      : texture_2d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec2<f32>,\n"
          "                  bias   : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3.f);                    // bias
          },
      },
      {
          ValidTextureOverload::kSampleBias2dOffsetF32,
          "textureSampleBias(t      : texture_2d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec2<f32>,\n"
          "                  bias   : f32,\n"
          "                  offset : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3.f,                     // bias
                               b->vec2<i32>(4, 5));     // offset
          },
      },
      {
          ValidTextureOverload::kSampleBias2dArrayF32,
          "textureSampleBias(t           : texture_2d_array<f32>,\n"
          "                  s           : sampler,\n"
          "                  coords      : vec2<f32>,\n"
          "                  array_index : i32,\n"
          "                  bias        : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               4,                       // array_index
                               3.f);                    // bias
          },
      },
      {
          ValidTextureOverload::kSampleBias2dArrayOffsetF32,
          "textureSampleBias(t           : texture_2d_array<f32>,\n"
          "                  s           : sampler,\n"
          "                  coords      : vec2<f32>,\n"
          "                  array_index : i32,\n"
          "                  bias        : f32,\n"
          "                  offset      : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
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
          "textureSampleBias(t      : texture_3d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec3<f32>,\n"
          "                  bias   : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4.f);                         // bias
          },
      },
      {
          ValidTextureOverload::kSampleBias3dOffsetF32,
          "textureSampleBias(t      : texture_3d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec3<f32>,\n"
          "                  bias   : f32,\n"
          "                  offset : vec3<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4.f,                          // bias
                               b->vec3<i32>(5, 6, 7));       // offset
          },
      },
      {
          ValidTextureOverload::kSampleBiasCubeF32,
          "textureSampleBias(t      : texture_cube<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec3<f32>,\n"
          "                  bias   : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4.f);                         // bias
          },
      },
      {
          ValidTextureOverload::kSampleBiasCubeArrayF32,
          "textureSampleBias(t           : texture_cube_array<f32>,\n"
          "                  s           : sampler,\n"
          "                  coords      : vec3<f32>,\n"
          "                  array_index : i32,\n"
          "                  bias        : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureSampleBias",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               3,                            // array_index
                               4.f);                         // bias
          },
      },
      {
          ValidTextureOverload::kSampleLevel2dF32,
          "textureSampleLevel(t      : texture_2d<f32>,\n"
          "                   s      : sampler,\n"
          "                   coords : vec2<f32>,\n"
          "                   level  : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3.f);                    // level
          },
      },
      {
          ValidTextureOverload::kSampleLevel2dOffsetF32,
          "textureSampleLevel(t      : texture_2d<f32>,\n"
          "                   s      : sampler,\n"
          "                   coords : vec2<f32>,\n"
          "                   level  : f32,\n"
          "                   offset : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3.f,                     // level
                               b->vec2<i32>(4, 5));     // offset
          },
      },
      {
          ValidTextureOverload::kSampleLevel2dArrayF32,
          "textureSampleLevel(t           : texture_2d_array<f32>,\n"
          "                   s           : sampler,\n"
          "                   coords      : vec2<f32>,\n"
          "                   array_index : i32,\n"
          "                   level       : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3,                       // array_index
                               4.f);                    // level
          },
      },
      {
          ValidTextureOverload::kSampleLevel2dArrayOffsetF32,
          "textureSampleLevel(t           : texture_2d_array<f32>,\n"
          "                   s           : sampler,\n"
          "                   coords      : vec2<f32>,\n"
          "                   array_index : i32,\n"
          "                   level       : f32,\n"
          "                   offset      : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
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
          "textureSampleLevel(t      : texture_3d<f32>,\n"
          "                   s      : sampler,\n"
          "                   coords : vec3<f32>,\n"
          "                   level  : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4.f);                         // level
          },
      },
      {
          ValidTextureOverload::kSampleLevel3dOffsetF32,
          "textureSampleLevel(t      : texture_3d<f32>,\n"
          "                   s      : sampler,\n"
          "                   coords : vec3<f32>,\n"
          "                   level  : f32,\n"
          "                   offset : vec3<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4.f,                          // level
                               b->vec3<i32>(5, 6, 7));       // offset
          },
      },
      {
          ValidTextureOverload::kSampleLevelCubeF32,
          "textureSampleLevel(t      : texture_cube<f32>,\n"
          "                   s      : sampler,\n"
          "                   coords : vec3<f32>,\n"
          "                   level  : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4.f);                         // level
          },
      },
      {
          ValidTextureOverload::kSampleLevelCubeArrayF32,
          "textureSampleLevel(t           : texture_cube_array<f32>,\n"
          "                   s           : sampler,\n"
          "                   coords      : vec3<f32>,\n"
          "                   array_index : i32,\n"
          "                   level       : f32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4,                            // array_index
                               5.f);                         // level
          },
      },
      {
          ValidTextureOverload::kSampleLevelDepth2dF32,
          "textureSampleLevel(t      : texture_depth_2d,\n"
          "                   s      : sampler,\n"
          "                   coords : vec2<f32>,\n"
          "                   level  : i32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3);                      // level
          },
      },
      {
          ValidTextureOverload::kSampleLevelDepth2dOffsetF32,
          "textureSampleLevel(t      : texture_depth_2d,\n"
          "                   s      : sampler,\n"
          "                   coords : vec2<f32>,\n"
          "                   level  : i32,\n"
          "                   offset : vec2<i32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3,                       // level
                               b->vec2<i32>(4, 5));     // offset
          },
      },
      {
          ValidTextureOverload::kSampleLevelDepth2dArrayF32,
          "textureSampleLevel(t           : texture_depth_2d_array,\n"
          "                   s           : sampler,\n"
          "                   coords      : vec2<f32>,\n"
          "                   array_index : i32,\n"
          "                   level       : i32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3,                       // array_index
                               4);                      // level
          },
      },
      {
          ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32,
          "textureSampleLevel(t           : texture_depth_2d_array,\n"
          "                   s           : sampler,\n"
          "                   coords      : vec2<f32>,\n"
          "                   array_index : i32,\n"
          "                   level       : i32,\n"
          "                   offset      : vec2<i32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
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
          "textureSampleLevel(t      : texture_depth_cube,\n"
          "                   s      : sampler,\n"
          "                   coords : vec3<f32>,\n"
          "                   level  : i32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4);                           // level
          },
      },
      {
          ValidTextureOverload::kSampleLevelDepthCubeArrayF32,
          "textureSampleLevel(t           : texture_depth_cube_array,\n"
          "                   s           : sampler,\n"
          "                   coords      : vec3<f32>,\n"
          "                   array_index : i32,\n"
          "                   level       : i32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureSampleLevel",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4,                            // array_index
                               5);                           // level
          },
      },
      {
          ValidTextureOverload::kSampleGrad2dF32,
          "textureSampleGrad(t      : texture_2d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec2<f32>\n"
          "                  ddx    : vec2<f32>,\n"
          "                  ddy    : vec2<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                  // t
                               "sampler",                  // s
                               b->vec2<f32>(1.0f, 2.0f),   // coords
                               b->vec2<f32>(3.0f, 4.0f),   // ddx
                               b->vec2<f32>(5.0f, 6.0f));  // ddy
          },
      },
      {
          ValidTextureOverload::kSampleGrad2dOffsetF32,
          "textureSampleGrad(t      : texture_2d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec2<f32>,\n"
          "                  ddx    : vec2<f32>,\n"
          "                  ddy    : vec2<f32>,\n"
          "                  offset : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
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
          "textureSampleGrad(t           : texture_2d_array<f32>,\n"
          "                  s           : sampler,\n"
          "                  coords      : vec2<f32>,\n"
          "                  array_index : i32,\n"
          "                  ddx         : vec2<f32>,\n"
          "                  ddy         : vec2<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
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
          "textureSampleGrad(t           : texture_2d_array<f32>,\n"
          "                  s           : sampler,\n"
          "                  coords      : vec2<f32>,\n"
          "                  array_index : i32,\n"
          "                  ddx         : vec2<f32>,\n"
          "                  ddy         : vec2<f32>,\n"
          "                  offset      : vec2<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
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
          "textureSampleGrad(t      : texture_3d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec3<f32>,\n"
          "                  ddx    : vec3<f32>,\n"
          "                  ddy    : vec3<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                     // t
                               "sampler",                     // s
                               b->vec3<f32>(1.f, 2.f, 3.f),   // coords
                               b->vec3<f32>(4.f, 5.f, 6.f),   // ddx
                               b->vec3<f32>(7.f, 8.f, 9.f));  // ddy
          },
      },
      {
          ValidTextureOverload::kSampleGrad3dOffsetF32,
          "textureSampleGrad(t      : texture_3d<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec3<f32>,\n"
          "                  ddx    : vec3<f32>,\n"
          "                  ddy    : vec3<f32>,\n"
          "                  offset : vec3<i32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
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
          "textureSampleGrad(t      : texture_cube<f32>,\n"
          "                  s      : sampler,\n"
          "                  coords : vec3<f32>,\n"
          "                  ddx    : vec3<f32>,\n"
          "                  ddy    : vec3<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                     // t
                               "sampler",                     // s
                               b->vec3<f32>(1.f, 2.f, 3.f),   // coords
                               b->vec3<f32>(4.f, 5.f, 6.f),   // ddx
                               b->vec3<f32>(7.f, 8.f, 9.f));  // ddy
          },
      },
      {
          ValidTextureOverload::kSampleGradCubeArrayF32,
          "textureSampleGrad(t           : texture_cube_array<f32>,\n"
          "                  s           : sampler,\n"
          "                  coords      : vec3<f32>,\n"
          "                  array_index : i32,\n"
          "                  ddx         : vec3<f32>,\n"
          "                  ddy         : vec3<f32>) -> vec4<f32>",
          TextureKind::kRegular,
          ast::SamplerKind::kSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureSampleGrad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                      // t
                               "sampler",                      // s
                               b->vec3<f32>(1.f, 2.f, 3.f),    // coords
                               4,                              // array_index
                               b->vec3<f32>(5.f, 6.f, 7.f),    // ddx
                               b->vec3<f32>(8.f, 9.f, 10.f));  // ddy
          },
      },
      {
          ValidTextureOverload::kSampleCompareDepth2dF32,
          "textureSampleCompare(t         : texture_depth_2d,\n"
          "                     s         : sampler_comparison,\n"
          "                     coords    : vec2<f32>,\n"
          "                     depth_ref : f32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kComparisonSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleCompare",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3.f);                    // depth_ref
          },
      },
      {
          ValidTextureOverload::kSampleCompareDepth2dOffsetF32,
          "textureSampleCompare(t         : texture_depth_2d,\n"
          "                     s         : sampler_comparison,\n"
          "                     coords    : vec2<f32>,\n"
          "                     depth_ref : f32,\n"
          "                     offset    : vec2<i32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kComparisonSampler,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureSampleCompare",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               3.f,                     // depth_ref
                               b->vec2<i32>(4, 5));     // offset
          },
      },
      {
          ValidTextureOverload::kSampleCompareDepth2dArrayF32,
          "textureSampleCompare(t           : texture_depth_2d_array,\n"
          "                     s           : sampler_comparison,\n"
          "                     coords      : vec2<f32>,\n"
          "                     array_index : i32,\n"
          "                     depth_ref   : f32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kComparisonSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleCompare",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               4,                       // array_index
                               3.f);                    // depth_ref
          },
      },
      {
          ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32,
          "textureSampleCompare(t           : texture_depth_2d_array,\n"
          "                     s           : sampler_comparison,\n"
          "                     coords      : vec2<f32>,\n"
          "                     array_index : i32,\n"
          "                     depth_ref   : f32,\n"
          "                     offset      : vec2<i32>) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kComparisonSampler,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureSampleCompare",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               "sampler",               // s
                               b->vec2<f32>(1.f, 2.f),  // coords
                               4,                       // array_index
                               3.f,                     // depth_ref
                               b->vec2<i32>(5, 6));     // offset
          },
      },
      {
          ValidTextureOverload::kSampleCompareDepthCubeF32,
          "textureSampleCompare(t         : texture_depth_cube,\n"
          "                     s         : sampler_comparison,\n"
          "                     coords    : vec3<f32>,\n"
          "                     depth_ref : f32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kComparisonSampler,
          ast::TextureDimension::kCube,
          TextureDataType::kF32,
          "textureSampleCompare",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4.f);                         // depth_ref
          },
      },
      {
          ValidTextureOverload::kSampleCompareDepthCubeArrayF32,
          "textureSampleCompare(t           : texture_depth_cube_array,\n"
          "                     s           : sampler_comparison,\n"
          "                     coords      : vec3<f32>,\n"
          "                     array_index : i32,\n"
          "                     depth_ref   : f32) -> f32",
          TextureKind::kDepth,
          ast::SamplerKind::kComparisonSampler,
          ast::TextureDimension::kCubeArray,
          TextureDataType::kF32,
          "textureSampleCompare",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                    // t
                               "sampler",                    // s
                               b->vec3<f32>(1.f, 2.f, 3.f),  // coords
                               4,                            // array_index
                               5.f);                         // depth_ref
          },
      },
      {
          ValidTextureOverload::kLoad1dLevelF32,
          "textureLoad(t      : texture_1d<f32>,\n"
          "            coords : i32,\n"
          "            level  : i32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::TextureDimension::k1d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",  // t
                               1,          // coords
                               3);         // level
          },
      },
      {
          ValidTextureOverload::kLoad1dLevelU32,
          "textureLoad(t      : texture_1d<u32>,\n"
          "            coords : i32,\n"
          "            level  : i32) -> vec4<u32>",
          TextureKind::kRegular,
          ast::TextureDimension::k1d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",  // t
                               1,          // coords
                               3);         // level
          },
      },
      {
          ValidTextureOverload::kLoad1dLevelI32,
          "textureLoad(t      : texture_1d<i32>,\n"
          "            coords : i32,\n"
          "            level  : i32) -> vec4<i32>",
          TextureKind::kRegular,
          ast::TextureDimension::k1d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",  // t
                               1,          // coords
                               3);         // level
          },
      },
      {
          ValidTextureOverload::kLoad2dLevelF32,
          "textureLoad(t      : texture_2d<f32>,\n"
          "            coords : vec2<i32>,\n"
          "            level  : i32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // level
          },
      },
      {
          ValidTextureOverload::kLoad2dLevelU32,
          "textureLoad(t      : texture_2d<u32>,\n"
          "            coords : vec2<i32>,\n"
          "            level  : i32) -> vec4<u32>",
          TextureKind::kRegular,
          ast::TextureDimension::k2d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // level
          },
      },
      {
          ValidTextureOverload::kLoad2dLevelI32,
          "textureLoad(t      : texture_2d<i32>,\n"
          "            coords : vec2<i32>,\n"
          "            level  : i32) -> vec4<i32>",
          TextureKind::kRegular,
          ast::TextureDimension::k2d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // level
          },
      },
      {
          ValidTextureOverload::kLoad2dArrayLevelF32,
          "textureLoad(t           : texture_2d_array<f32>,\n"
          "            coords      : vec2<i32>,\n"
          "            array_index : i32,\n"
          "            level       : i32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3,                   // array_index
                               4);                  // level
          },
      },
      {
          ValidTextureOverload::kLoad2dArrayLevelU32,
          "textureLoad(t           : texture_2d_array<u32>,\n"
          "            coords      : vec2<i32>,\n"
          "            array_index : i32,\n"
          "            level       : i32) -> vec4<u32>",
          TextureKind::kRegular,
          ast::TextureDimension::k2dArray,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3,                   // array_index
                               4);                  // level
          },
      },
      {
          ValidTextureOverload::kLoad2dArrayLevelI32,
          "textureLoad(t           : texture_2d_array<i32>,\n"
          "            coords      : vec2<i32>,\n"
          "            array_index : i32,\n"
          "            level       : i32) -> vec4<i32>",
          TextureKind::kRegular,
          ast::TextureDimension::k2dArray,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3,                   // array_index
                               4);                  // level
          },
      },
      {
          ValidTextureOverload::kLoad3dLevelF32,
          "textureLoad(t      : texture_3d<f32>,\n"
          "            coords : vec3<i32>,\n"
          "            level  : i32) -> vec4<f32>",
          TextureKind::kRegular,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",              // t
                               b->vec3<i32>(1, 2, 3),  // coords
                               4);                     // level
          },
      },
      {
          ValidTextureOverload::kLoad3dLevelU32,
          "textureLoad(t      : texture_3d<u32>,\n"
          "            coords : vec3<i32>,\n"
          "            level  : i32) -> vec4<u32>",
          TextureKind::kRegular,
          ast::TextureDimension::k3d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",              // t
                               b->vec3<i32>(1, 2, 3),  // coords
                               4);                     // level
          },
      },
      {
          ValidTextureOverload::kLoad3dLevelI32,
          "textureLoad(t      : texture_3d<i32>,\n"
          "            coords : vec3<i32>,\n"
          "            level  : i32) -> vec4<i32>",
          TextureKind::kRegular,
          ast::TextureDimension::k3d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",              // t
                               b->vec3<i32>(1, 2, 3),  // coords
                               4);                     // level
          },
      },
      {
          ValidTextureOverload::kLoadMultisampled2dF32,
          "textureLoad(t            : texture_multisampled_2d<f32>,\n"
          "            coords       : vec2<i32>,\n"
          "            sample_index : i32) -> vec4<f32>",
          TextureKind::kMultisampled,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // sample_index
          },
      },
      {
          ValidTextureOverload::kLoadMultisampled2dU32,
          "textureLoad(t            : texture_multisampled_2d<u32>,\n"
          "            coords       : vec2<i32>,\n"
          "            sample_index : i32) -> vec4<u32>",
          TextureKind::kMultisampled,
          ast::TextureDimension::k2d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // sample_index
          },
      },
      {
          ValidTextureOverload::kLoadMultisampled2dI32,
          "textureLoad(t            : texture_multisampled_2d<i32>,\n"
          "            coords       : vec2<i32>,\n"
          "            sample_index : i32) -> vec4<i32>",
          TextureKind::kMultisampled,
          ast::TextureDimension::k2d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // sample_index
          },
      },
      {
          ValidTextureOverload::kLoadDepth2dLevelF32,
          "textureLoad(t      : texture_depth_2d,\n"
          "            coords : vec2<i32>,\n"
          "            level  : i32) -> f32",
          TextureKind::kDepth,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // level
          },
      },
      {
          ValidTextureOverload::kLoadDepth2dArrayLevelF32,
          "textureLoad(t           : texture_depth_2d_array,\n"
          "            coords      : vec2<i32>,\n"
          "            array_index : i32,\n"
          "            level       : i32) -> f32",
          TextureKind::kDepth,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3,                   // array_index
                               4);                  // level
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO1dRgba32float,
          "textureLoad(t      : texture_storage_1d<rgba32float>,\n"
          "            coords : i32) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k1d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",  // t
                               1);         // coords
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba8unorm,
          "textureLoad(t           : texture_storage_2d<rgba8unorm>,\n"
          "            coords      : vec2<i32>) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba8Unorm,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba8snorm,
          "textureLoad(t           : texture_storage_2d<rgba8snorm>,\n"
          "            coords      : vec2<i32>) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba8Snorm,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba8uint,
          "textureLoad(t           : texture_storage_2d<rgba8uint>,\n"
          "            coords      : vec2<i32>) -> vec4<u32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba8Uint,
          ast::TextureDimension::k2d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba8sint,
          "textureLoad(t           : texture_storage_2d<rgba8sint>,\n"
          "            coords      : vec2<i32>) -> vec4<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba8Sint,
          ast::TextureDimension::k2d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba16uint,
          "textureLoad(t           : texture_storage_2d<rgba16uint>,\n"
          "            coords      : vec2<i32>) -> vec4<u32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba16Uint,
          ast::TextureDimension::k2d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba16sint,
          "textureLoad(t           : texture_storage_2d<rgba16sint>,\n"
          "            coords      : vec2<i32>) -> vec4<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba16Sint,
          ast::TextureDimension::k2d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba16float,
          "textureLoad(t           : texture_storage_2d<rgba16float>,\n"
          "            coords      : vec2<i32>) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba16Float,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dR32uint,
          "textureLoad(t           : texture_storage_2d<r32uint>,\n"
          "            coords      : vec2<i32>) -> vec4<u32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kR32Uint,
          ast::TextureDimension::k2d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dR32sint,
          "textureLoad(t           : texture_storage_2d<r32sint>,\n"
          "            coords      : vec2<i32>) -> vec4<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kR32Sint,
          ast::TextureDimension::k2d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dR32float,
          "textureLoad(t           : texture_storage_2d<r32float>,\n"
          "            coords      : vec2<i32>) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kR32Float,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRg32uint,
          "textureLoad(t           : texture_storage_2d<rg32uint>,\n"
          "            coords      : vec2<i32>) -> vec4<u32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRg32Uint,
          ast::TextureDimension::k2d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRg32sint,
          "textureLoad(t           : texture_storage_2d<rg32sint>,\n"
          "            coords      : vec2<i32>) -> vec4<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRg32Sint,
          ast::TextureDimension::k2d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRg32float,
          "textureLoad(t           : texture_storage_2d<rg32float>,\n"
          "            coords      : vec2<i32>) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRg32Float,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba32uint,
          "textureLoad(t           : texture_storage_2d<rgba32uint>,\n"
          "            coords      : vec2<i32>) -> vec4<u32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Uint,
          ast::TextureDimension::k2d,
          TextureDataType::kU32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba32sint,
          "textureLoad(t           : texture_storage_2d<rgba32sint>,\n"
          "            coords      : vec2<i32>) -> vec4<i32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Sint,
          ast::TextureDimension::k2d,
          TextureDataType::kI32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dRgba32float,
          "textureLoad(t           : texture_storage_2d<rgba32float>,\n"
          "            coords      : vec2<i32>) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",            // t
                               b->vec2<i32>(1, 2));  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO2dArrayRgba32float,
          "textureLoad(t           : "
          "texture_storage_2d_array<rgba32float>,\n"
          "            coords      : vec2<i32>,\n"
          "            array_index : i32) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3);                  // array_index
          },
      },
      {
          ValidTextureOverload::kLoadStorageRO3dRgba32float,
          "textureLoad(t      : texture_storage_3d<rgba32float>,\n"
          "            coords : vec3<i32>) -> vec4<f32>",
          ast::AccessControl::kReadOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureLoad",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",               // t
                               b->vec3<i32>(1, 2, 3));  // coords
          },
      },
      {
          ValidTextureOverload::kStoreWO1dRgba32float,
          "textureStore(t      : texture_storage_1d<rgba32float>,\n"
          "             coords : i32,\n"
          "             value  : vec4<T>)",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k1d,
          TextureDataType::kF32,
          "textureStore",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                          // t
                               1,                                  // coords
                               b->vec4<f32>(2.f, 3.f, 4.f, 5.f));  // value
          },
      },
      {
          ValidTextureOverload::kStoreWO2dRgba32float,
          "textureStore(t      : texture_storage_2d<rgba32float>,\n"
          "             coords : vec2<i32>,\n"
          "             value  : vec4<T>)",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2d,
          TextureDataType::kF32,
          "textureStore",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                          // t
                               b->vec2<i32>(1, 2),                 // coords
                               b->vec4<f32>(3.f, 4.f, 5.f, 6.f));  // value
          },
      },
      {
          ValidTextureOverload::kStoreWO2dArrayRgba32float,
          "textureStore(t           : texture_storage_2d_array<rgba32float>,\n"
          "             coords      : vec2<i32>,\n"
          "             array_index : i32,\n"
          "             value       : vec4<T>)",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k2dArray,
          TextureDataType::kF32,
          "textureStore",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",           // t
                               b->vec2<i32>(1, 2),  // coords
                               3,                   // array_index
                               b->vec4<f32>(4.f, 5.f, 6.f, 7.f));  // value
          },
      },
      {
          ValidTextureOverload::kStoreWO3dRgba32float,
          "textureStore(t      : texture_storage_3d<rgba32float>,\n"
          "             coords : vec3<i32>,\n"
          "             value  : vec4<T>)",
          ast::AccessControl::kWriteOnly,
          ast::ImageFormat::kRgba32Float,
          ast::TextureDimension::k3d,
          TextureDataType::kF32,
          "textureStore",
          [](ProgramBuilder* b) {
            return b->ExprList("texture",                          // t
                               b->vec3<i32>(1, 2, 3),              // coords
                               b->vec4<f32>(4.f, 5.f, 6.f, 7.f));  // value
          },
      },
  };
}

}  // namespace test
}  // namespace intrinsic
}  // namespace ast
}  // namespace tint
