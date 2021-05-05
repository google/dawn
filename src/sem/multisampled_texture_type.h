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

#ifndef SRC_SEM_MULTISAMPLED_TEXTURE_TYPE_H_
#define SRC_SEM_MULTISAMPLED_TEXTURE_TYPE_H_

#include <string>

#include "src/sem/texture_type.h"

namespace tint {
namespace sem {

/// A multisampled texture type.
class MultisampledTexture : public Castable<MultisampledTexture, Texture> {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  /// @param type the data type of the multisampled texture
  MultisampledTexture(ast::TextureDimension dim, const Type* type);
  /// Move constructor
  MultisampledTexture(MultisampledTexture&&);
  ~MultisampledTexture() override;

  /// @returns the subtype of the sampled texture
  Type* type() const { return const_cast<Type*>(type_); }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  const Type* const type_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_MULTISAMPLED_TEXTURE_TYPE_H_
