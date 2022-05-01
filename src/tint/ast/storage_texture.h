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

#ifndef SRC_TINT_AST_STORAGE_TEXTURE_H_
#define SRC_TINT_AST_STORAGE_TEXTURE_H_

#include <string>

#include "src/tint/ast/access.h"
#include "src/tint/ast/texture.h"

namespace tint::ast {

/// The texel format in the storage texture
enum class TexelFormat {
    kNone = -1,
    kRgba8Unorm,
    kRgba8Snorm,
    kRgba8Uint,
    kRgba8Sint,
    kRgba16Uint,
    kRgba16Sint,
    kRgba16Float,
    kR32Uint,
    kR32Sint,
    kR32Float,
    kRg32Uint,
    kRg32Sint,
    kRg32Float,
    kRgba32Uint,
    kRgba32Sint,
    kRgba32Float,
};

/// @param out the std::ostream to write to
/// @param format the TexelFormat
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, TexelFormat format);

/// A storage texture type.
class StorageTexture final : public Castable<StorageTexture, Texture> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node
    /// @param dim the dimensionality of the texture
    /// @param format the image format of the texture
    /// @param subtype the storage subtype. Use SubtypeFor() to calculate this.
    /// @param access_control the access control for the texture.
    StorageTexture(ProgramID pid,
                   const Source& src,
                   TextureDimension dim,
                   TexelFormat format,
                   const Type* subtype,
                   Access access_control);

    /// Move constructor
    StorageTexture(StorageTexture&&);
    ~StorageTexture() override;

    /// @param symbols the program's symbol table
    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const SymbolTable& symbols) const override;

    /// Clones this type and all transitive types using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned type
    const StorageTexture* Clone(CloneContext* ctx) const override;

    /// @param format the storage texture image format
    /// @param builder the ProgramBuilder used to build the returned type
    /// @returns the storage texture subtype for the given TexelFormat
    static Type* SubtypeFor(TexelFormat format, ProgramBuilder& builder);

    /// The image format
    const TexelFormat format;

    /// The storage subtype
    const Type* const type;

    /// The access control
    const Access access;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_STORAGE_TEXTURE_H_
