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

#ifndef SRC_TINT_TYPE_STORAGE_TEXTURE_H_
#define SRC_TINT_TYPE_STORAGE_TEXTURE_H_

#include <string>

#include "src/tint/builtin/access.h"
#include "src/tint/builtin/texel_format.h"
#include "src/tint/type/texture.h"
#include "src/tint/type/texture_dimension.h"

// Forward declarations
namespace tint::type {
class Manager;
}  // namespace tint::type

namespace tint::type {

/// A storage texture type.
class StorageTexture final : public utils::Castable<StorageTexture, Texture> {
  public:
    /// Constructor
    /// @param dim the dimensionality of the texture
    /// @param format the texel format of the texture
    /// @param access the access control type of the texture
    /// @param subtype the storage subtype. Use SubtypeFor() to calculate this.
    StorageTexture(TextureDimension dim,
                   builtin::TexelFormat format,
                   builtin::Access access,
                   Type* subtype);

    /// Destructor
    ~StorageTexture() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the storage subtype
    Type* type() const { return subtype_; }

    /// @returns the texel format
    builtin::TexelFormat texel_format() const { return texel_format_; }

    /// @returns the access control
    builtin::Access access() const { return access_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @param format the storage texture image format
    /// @param type_mgr the Manager used to build the returned type
    /// @returns the storage texture subtype for the given TexelFormat
    static Type* SubtypeFor(builtin::TexelFormat format, Manager& type_mgr);

    /// @param ctx the clone context
    /// @returns a clone of this type
    StorageTexture* Clone(CloneContext& ctx) const override;

  private:
    builtin::TexelFormat const texel_format_;
    builtin::Access const access_;
    Type* const subtype_;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_STORAGE_TEXTURE_H_
