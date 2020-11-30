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

#include <functional>
#include <vector>

#include "src/ast/builder.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/texture_type.h"

namespace tint {
namespace ast {
namespace intrinsic {
namespace test {

enum class TextureKind { kRegular, kDepth };

inline std::ostream& operator<<(std::ostream& out, const TextureKind& kind) {
  switch (kind) {
    case TextureKind::kRegular:
      out << "regular";
      break;
    case TextureKind::kDepth:
      out << "depth";
      break;
  }
  return out;
}

enum class TextureDataType { kF32, kU32, kI32 };

inline std::ostream& operator<<(std::ostream& out, const TextureDataType& ty) {
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

enum class ValidTextureOverload {
  kSample1dF32,
  kSample1dArrayF32,
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
  kSampleGradDepth2dF32,
  kSampleGradDepth2dOffsetF32,
  kSampleGradDepth2dArrayF32,
  kSampleGradDepth2dArrayOffsetF32,
  kSampleGradDepthCubeF32,
  kSampleGradDepthCubeArrayF32,
};

/// Describes a texture intrinsic overload
struct TextureOverloadCase {
  /// Constructor
  TextureOverloadCase();
  /// Constructor
  TextureOverloadCase(ValidTextureOverload,
                      const char*,
                      TextureKind,
                      type::SamplerKind,
                      type::TextureDimension,
                      TextureDataType,
                      const char*,
                      std::function<ExpressionList(Builder*)>);
  /// Copy constructor
  TextureOverloadCase(const TextureOverloadCase&);
  /// Destructor
  ~TextureOverloadCase();

  /// @return a vector containing a large number of valid texture overloads
  static std::vector<TextureOverloadCase> ValidCases();

  /// The enumerator for this overload
  ValidTextureOverload overload;
  /// A human readable description of the overload
  const char* description;
  /// The texture kind for the texture parameter
  TextureKind texture_kind;
  /// The sampler kind for the sampler parameter
  type::SamplerKind sampler_kind;
  /// The dimensions of the texture parameter
  type::TextureDimension texture_dimension;
  /// The data type of the texture parameter
  TextureDataType texture_data_type;
  /// Name of the function. e.g. `textureSample`, `textureSampleGrad`, etc
  const char* function;
  /// A function that builds the AST arguments for the overload
  std::function<ExpressionList(Builder*)> args;
};

inline std::ostream& operator<<(std::ostream& out,
                                const TextureOverloadCase& data) {
  out << "TextureOverloadCase" << static_cast<int>(data.overload) << "\n";
  out << data.description << "\n";
  out << "texture_kind:      " << data.texture_kind << "\n";
  out << "sampler_kind:      " << data.sampler_kind << "\n";
  out << "texture_dimension: " << data.texture_dimension << "\n";
  out << "texture_data_type: " << data.texture_data_type << "\n";
  return out;
}

}  // namespace test
}  // namespace intrinsic
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INTRINSIC_TEXTURE_HELPER_TEST_H_
