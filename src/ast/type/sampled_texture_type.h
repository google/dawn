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

#ifndef SRC_AST_TYPE_SAMPLED_TEXTURE_TYPE_H_
#define SRC_AST_TYPE_SAMPLED_TEXTURE_TYPE_H_

#include <string>

#include "src/ast/type/texture_type.h"

namespace tint {
namespace ast {
namespace type {

/// A sampled texture type.
class SampledTextureType : public TextureType {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  /// @param type the data type of the sampled texture
  SampledTextureType(TextureDimension dim, Type* type);
  /// Move constructor
  SampledTextureType(SampledTextureType&&);
  ~SampledTextureType() override;

  /// @returns true if the type is a sampled texture type
  bool IsSampled() const override;

  /// @returns the subtype of the sampled texture
  Type* type() const { return type_; }

  /// @returns the name for this type
  std::string type_name() const override;

 private:
  Type* type_ = nullptr;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_SAMPLED_TEXTURE_TYPE_H_
