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

#ifndef SRC_TINT_SEM_SAMPLED_TEXTURE_H_
#define SRC_TINT_SEM_SAMPLED_TEXTURE_H_

#include <string>

#include "src/tint/sem/texture.h"

namespace tint::sem {

/// A sampled texture type.
class SampledTexture final : public Castable<SampledTexture, Texture> {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  /// @param type the data type of the sampled texture
  SampledTexture(ast::TextureDimension dim, const Type* type);
  /// Move constructor
  SampledTexture(SampledTexture&&);
  ~SampledTexture() override;

  /// @returns a hash of the type.
  size_t Hash() const override;

  /// @param other the other type to compare against
  /// @returns true if the this type is equal to the given type
  bool Equals(const Type& other) const override;

  /// @returns the subtype of the sampled texture
  Type* type() const { return const_cast<Type*>(type_); }

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  const Type* const type_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_SAMPLED_TEXTURE_H_
