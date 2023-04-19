// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TYPE_MULTISAMPLED_TEXTURE_H_
#define SRC_TINT_TYPE_MULTISAMPLED_TEXTURE_H_

#include <string>

#include "src/tint/type/texture.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::type {

/// A multisampled texture type.
class MultisampledTexture final : public utils::Castable<MultisampledTexture, Texture> {
  public:
    /// Constructor
    /// @param dim the dimensionality of the texture
    /// @param type the data type of the multisampled texture
    MultisampledTexture(TextureDimension dim, const Type* type);

    /// Destructor
    ~MultisampledTexture() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the subtype of the sampled texture
    const Type* type() const { return type_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    MultisampledTexture* Clone(CloneContext& ctx) const override;

  private:
    const Type* const type_;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_MULTISAMPLED_TEXTURE_H_
