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

#ifndef SRC_AST_INTRINSIC_TEXTURE_HELPER_TEST_H_
#define SRC_AST_INTRINSIC_TEXTURE_HELPER_TEST_H_

#include <vector>

#include "src/ast/access_control.h"
#include "src/program_builder.h"
#include "src/sem/storage_texture_type.h"

namespace tint {
namespace ast {
namespace intrinsic {
namespace test {

enum class TextureKind { kRegular, kDepth, kMultisampled, kStorage };
enum class TextureDataType { kF32, kU32, kI32 };

std::ostream& operator<<(std::ostream& out, const TextureKind& kind);
std::ostream& operator<<(std::ostream& out, const TextureDataType& ty);

/// Non-exhaustive list of valid texture overloads
enum class ValidTextureOverload {
  kDimensions1d,
  kDimensions2d,
  kDimensions2dLevel,
  kDimensions2dArray,
  kDimensions2dArrayLevel,
  kDimensions3d,
  kDimensions3dLevel,
  kDimensionsCube,
  kDimensionsCubeLevel,
  kDimensionsCubeArray,
  kDimensionsCubeArrayLevel,
  kDimensionsMultisampled2d,
  kDimensionsDepth2d,
  kDimensionsDepth2dLevel,
  kDimensionsDepth2dArray,
  kDimensionsDepth2dArrayLevel,
  kDimensionsDepthCube,
  kDimensionsDepthCubeLevel,
  kDimensionsDepthCubeArray,
  kDimensionsDepthCubeArrayLevel,
  kDimensionsStorageRO1d,
  kDimensionsStorageRO2d,
  kDimensionsStorageRO2dArray,
  kDimensionsStorageRO3d,
  kDimensionsStorageWO1d,
  kDimensionsStorageWO2d,
  kDimensionsStorageWO2dArray,
  kDimensionsStorageWO3d,
  kNumLayers2dArray,
  kNumLayersCubeArray,
  kNumLayersDepth2dArray,
  kNumLayersDepthCubeArray,
  kNumLayersStorageWO2dArray,
  kNumLevels2d,
  kNumLevels2dArray,
  kNumLevels3d,
  kNumLevelsCube,
  kNumLevelsCubeArray,
  kNumLevelsDepth2d,
  kNumLevelsDepth2dArray,
  kNumLevelsDepthCube,
  kNumLevelsDepthCubeArray,
  kNumSamplesMultisampled2d,
  kSample1dF32,
  kSample2dF32,
  kSample2dOffsetF32,
  kSample2dArrayF32,
  kSample2dArrayOffsetF32,
  kSample3dF32,
  kSample3dOffsetF32,
  kSampleCubeF32,
  kSampleCubeArrayF32,
  kSampleDepth2dF32,
  kSampleDepth2dOffsetF32,
  kSampleDepth2dArrayF32,
  kSampleDepth2dArrayOffsetF32,
  kSampleDepthCubeF32,
  kSampleDepthCubeArrayF32,
  kSampleBias2dF32,
  kSampleBias2dOffsetF32,
  kSampleBias2dArrayF32,
  kSampleBias2dArrayOffsetF32,
  kSampleBias3dF32,
  kSampleBias3dOffsetF32,
  kSampleBiasCubeF32,
  kSampleBiasCubeArrayF32,
  kSampleLevel2dF32,
  kSampleLevel2dOffsetF32,
  kSampleLevel2dArrayF32,
  kSampleLevel2dArrayOffsetF32,
  kSampleLevel3dF32,
  kSampleLevel3dOffsetF32,
  kSampleLevelCubeF32,
  kSampleLevelCubeArrayF32,
  kSampleLevelDepth2dF32,
  kSampleLevelDepth2dOffsetF32,
  kSampleLevelDepth2dArrayF32,
  kSampleLevelDepth2dArrayOffsetF32,
  kSampleLevelDepthCubeF32,
  kSampleLevelDepthCubeArrayF32,
  kSampleGrad2dF32,
  kSampleGrad2dOffsetF32,
  kSampleGrad2dArrayF32,
  kSampleGrad2dArrayOffsetF32,
  kSampleGrad3dF32,
  kSampleGrad3dOffsetF32,
  kSampleGradCubeF32,
  kSampleGradCubeArrayF32,
  kSampleCompareDepth2dF32,
  kSampleCompareDepth2dOffsetF32,
  kSampleCompareDepth2dArrayF32,
  kSampleCompareDepth2dArrayOffsetF32,
  kSampleCompareDepthCubeF32,
  kSampleCompareDepthCubeArrayF32,
  kLoad1dLevelF32,
  kLoad1dLevelU32,
  kLoad1dLevelI32,
  kLoad2dLevelF32,
  kLoad2dLevelU32,
  kLoad2dLevelI32,
  kLoad2dArrayLevelF32,
  kLoad2dArrayLevelU32,
  kLoad2dArrayLevelI32,
  kLoad3dLevelF32,
  kLoad3dLevelU32,
  kLoad3dLevelI32,
  kLoadMultisampled2dF32,
  kLoadMultisampled2dU32,
  kLoadMultisampled2dI32,
  kLoadDepth2dLevelF32,
  kLoadDepth2dArrayLevelF32,
  kLoadStorageRO1dRgba32float,  // Not permutated for all texel formats
  kLoadStorageRO2dRgba8unorm,
  kLoadStorageRO2dRgba8snorm,
  kLoadStorageRO2dRgba8uint,
  kLoadStorageRO2dRgba8sint,
  kLoadStorageRO2dRgba16uint,
  kLoadStorageRO2dRgba16sint,
  kLoadStorageRO2dRgba16float,
  kLoadStorageRO2dR32uint,
  kLoadStorageRO2dR32sint,
  kLoadStorageRO2dR32float,
  kLoadStorageRO2dRg32uint,
  kLoadStorageRO2dRg32sint,
  kLoadStorageRO2dRg32float,
  kLoadStorageRO2dRgba32uint,
  kLoadStorageRO2dRgba32sint,
  kLoadStorageRO2dRgba32float,
  kLoadStorageRO2dArrayRgba32float,  // Not permutated for all texel formats
  kLoadStorageRO3dRgba32float,       // Not permutated for all texel formats
  kStoreWO1dRgba32float,             // Not permutated for all texel formats
  kStoreWO2dRgba32float,             // Not permutated for all texel formats
  kStoreWO2dArrayRgba32float,        // Not permutated for all texel formats
  kStoreWO3dRgba32float,             // Not permutated for all texel formats
};

/// Describes a texture intrinsic overload
struct TextureOverloadCase {
  /// Constructor for textureSample...() functions
  TextureOverloadCase(ValidTextureOverload,
                      const char*,
                      TextureKind,
                      ast::SamplerKind,
                      ast::TextureDimension,
                      TextureDataType,
                      const char*,
                      std::function<ExpressionList(ProgramBuilder*)>);
  /// Constructor for textureLoad() functions with non-storage textures
  TextureOverloadCase(ValidTextureOverload,
                      const char*,
                      TextureKind,
                      ast::TextureDimension,
                      TextureDataType,
                      const char*,
                      std::function<ExpressionList(ProgramBuilder*)>);
  /// Constructor for textureLoad() with storage textures
  TextureOverloadCase(ValidTextureOverload,
                      const char*,
                      AccessControl::Access,
                      ast::ImageFormat,
                      ast::TextureDimension,
                      TextureDataType,
                      const char*,
                      std::function<ExpressionList(ProgramBuilder*)>);
  /// Copy constructor
  TextureOverloadCase(const TextureOverloadCase&);
  /// Destructor
  ~TextureOverloadCase();

  /// @return a vector containing a large number (non-exhaustive) of valid
  /// texture overloads.
  static std::vector<TextureOverloadCase> ValidCases();

  /// @param builder the AST builder used for the test
  /// @returns the vector component type of the texture function return value
  ast::Type* buildResultVectorComponentType(ProgramBuilder* builder) const;
  /// @param builder the AST builder used for the test
  /// @returns a variable holding the test texture, automatically registered as
  /// a global variable.
  ast::Variable* buildTextureVariable(ProgramBuilder* builder) const;
  /// @param builder the AST builder used for the test
  /// @returns a Variable holding the test sampler, automatically registered as
  /// a global variable.
  ast::Variable* buildSamplerVariable(ProgramBuilder* builder) const;

  /// The enumerator for this overload
  ValidTextureOverload const overload;
  /// A human readable description of the overload
  const char* const description;
  /// The texture kind for the texture parameter
  TextureKind const texture_kind;
  /// The sampler kind for the sampler parameter
  /// Used only when texture_kind is not kStorage
  ast::SamplerKind const sampler_kind = ast::SamplerKind::kSampler;
  /// The access control for the storage texture
  /// Used only when texture_kind is kStorage
  AccessControl::Access const access_control = AccessControl::kReadWrite;
  /// The image format for the storage texture
  /// Used only when texture_kind is kStorage
  ast::ImageFormat const image_format = ast::ImageFormat::kNone;
  /// The dimensions of the texture parameter
  ast::TextureDimension const texture_dimension;
  /// The data type of the texture parameter
  TextureDataType const texture_data_type;
  /// Name of the function. e.g. `textureSample`, `textureSampleGrad`, etc
  const char* const function;
  /// A function that builds the AST arguments for the overload
  std::function<ExpressionList(ProgramBuilder*)> const args;
};

std::ostream& operator<<(std::ostream& out, const TextureOverloadCase& data);

}  // namespace test
}  // namespace intrinsic
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INTRINSIC_TEXTURE_HELPER_TEST_H_
