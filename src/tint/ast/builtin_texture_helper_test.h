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

#ifndef SRC_TINT_AST_BUILTIN_TEXTURE_HELPER_TEST_H_
#define SRC_TINT_AST_BUILTIN_TEXTURE_HELPER_TEST_H_

#include <vector>

#include "src/tint/builtin/access.h"
#include "src/tint/builtin/texel_format.h"
#include "src/tint/program_builder.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::ast::builtin::test {

/// The name of the texture global variable used by the tests.
static constexpr const char* kTextureName = "Texture";

/// The name of the sampler global variable used by the tests.
static constexpr const char* kSamplerName = "Sampler";

enum class TextureKind { kRegular, kDepth, kDepthMultisampled, kMultisampled, kStorage };
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
    kDimensionsDepthMultisampled2d,
    kDimensionsStorageWO1d,
    kDimensionsStorageWO2d,
    kDimensionsStorageWO2dArray,
    kDimensionsStorageWO3d,
    kGather2dF32,
    kGather2dOffsetF32,
    kGather2dArrayF32,
    kGather2dArrayOffsetF32,
    kGatherCubeF32,
    kGatherCubeArrayF32,
    kGatherDepth2dF32,
    kGatherDepth2dOffsetF32,
    kGatherDepth2dArrayF32,
    kGatherDepth2dArrayOffsetF32,
    kGatherDepthCubeF32,
    kGatherDepthCubeArrayF32,
    kGatherCompareDepth2dF32,
    kGatherCompareDepth2dOffsetF32,
    kGatherCompareDepth2dArrayF32,
    kGatherCompareDepth2dArrayOffsetF32,
    kGatherCompareDepthCubeF32,
    kGatherCompareDepthCubeArrayF32,
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
    kNumSamplesDepthMultisampled2d,
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
    kSampleCompareLevelDepth2dF32,
    kSampleCompareLevelDepth2dOffsetF32,
    kSampleCompareLevelDepth2dArrayF32,
    kSampleCompareLevelDepth2dArrayOffsetF32,
    kSampleCompareLevelDepthCubeF32,
    kSampleCompareLevelDepthCubeArrayF32,
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
    kLoadDepthMultisampled2dF32,
    kStoreWO1dRgba32float,       // Not permutated for all texel formats
    kStoreWO2dRgba32float,       // Not permutated for all texel formats
    kStoreWO2dArrayRgba32float,  // Not permutated for all texel formats
    kStoreWO3dRgba32float,       // Not permutated for all texel formats
};

/// @param texture_overload the ValidTextureOverload
/// @returns true if the ValidTextureOverload builtin returns no value.
bool ReturnsVoid(ValidTextureOverload texture_overload);

/// Describes a texture builtin overload
struct TextureOverloadCase {
    /// Args is a list of Expression used as arguments to the texture overload case.
    using Args = utils::Vector<const Expression*, 8>;

    /// Constructor for textureSample...() functions
    TextureOverloadCase(ValidTextureOverload,
                        const char*,
                        TextureKind,
                        type::SamplerKind,
                        type::TextureDimension,
                        TextureDataType,
                        const char*,
                        std::function<Args(ProgramBuilder*)>,
                        bool /* returns_value */);
    /// Constructor for textureLoad() functions with non-storage textures
    TextureOverloadCase(ValidTextureOverload,
                        const char*,
                        TextureKind,
                        type::TextureDimension,
                        TextureDataType,
                        const char*,
                        std::function<Args(ProgramBuilder*)>,
                        bool /* returns_value */);
    /// Constructor for textureLoad() with storage textures
    TextureOverloadCase(ValidTextureOverload,
                        const char*,
                        tint::builtin::Access,
                        tint::builtin::TexelFormat,
                        type::TextureDimension,
                        TextureDataType,
                        const char*,
                        std::function<Args(ProgramBuilder*)>,
                        bool /* returns_value */);
    /// Copy constructor
    TextureOverloadCase(const TextureOverloadCase&);

    /// Destructor
    ~TextureOverloadCase();

    /// @return a vector containing a large number (non-exhaustive) of valid
    /// texture overloads.
    static std::vector<TextureOverloadCase> ValidCases();

    /// @param builder the AST builder used for the test
    /// @returns the vector component type of the texture function return value
    Type BuildResultVectorComponentType(ProgramBuilder* builder) const;
    /// @param builder the AST builder used for the test
    /// @returns a variable holding the test texture, automatically registered as
    /// a global variable.
    const Variable* BuildTextureVariable(ProgramBuilder* builder) const;
    /// @param builder the AST builder used for the test
    /// @returns a Variable holding the test sampler, automatically registered as
    /// a global variable.
    const Variable* BuildSamplerVariable(ProgramBuilder* builder) const;

    /// The enumerator for this overload
    const ValidTextureOverload overload;
    /// A human readable description of the overload
    const char* const description;
    /// The texture kind for the texture parameter
    const TextureKind texture_kind;
    /// The sampler kind for the sampler parameter
    /// Used only when texture_kind is not kStorage
    type::SamplerKind const sampler_kind = type::SamplerKind::kSampler;
    /// The access control for the storage texture
    /// Used only when texture_kind is kStorage
    tint::builtin::Access const access = tint::builtin::Access::kReadWrite;
    /// The image format for the storage texture
    /// Used only when texture_kind is kStorage
    tint::builtin::TexelFormat const texel_format = tint::builtin::TexelFormat::kUndefined;
    /// The dimensions of the texture parameter
    type::TextureDimension const texture_dimension;
    /// The data type of the texture parameter
    const TextureDataType texture_data_type;
    /// Name of the function. e.g. `textureSample`, `textureSampleGrad`, etc
    const char* const function;
    /// A function that builds the AST arguments for the overload
    std::function<Args(ProgramBuilder*)> const args;
    /// True if the function returns a value
    const bool returns_value;
};

std::ostream& operator<<(std::ostream& out, const TextureOverloadCase& data);

}  // namespace tint::ast::builtin::test

#endif  // SRC_TINT_AST_BUILTIN_TEXTURE_HELPER_TEST_H_
