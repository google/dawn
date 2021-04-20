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

#ifndef SRC_AST_STORAGE_TEXTURE_H_
#define SRC_AST_STORAGE_TEXTURE_H_

#include <string>

#include "src/ast/texture.h"

namespace tint {
namespace ast {

class Manager;

/// The image format in the storage texture
enum class ImageFormat {
  kNone = -1,
  kR8Unorm,
  kR8Snorm,
  kR8Uint,
  kR8Sint,
  kR16Uint,
  kR16Sint,
  kR16Float,
  kRg8Unorm,
  kRg8Snorm,
  kRg8Uint,
  kRg8Sint,
  kR32Uint,
  kR32Sint,
  kR32Float,
  kRg16Uint,
  kRg16Sint,
  kRg16Float,
  kRgba8Unorm,
  kRgba8UnormSrgb,
  kRgba8Snorm,
  kRgba8Uint,
  kRgba8Sint,
  kBgra8Unorm,
  kBgra8UnormSrgb,
  kRgb10A2Unorm,
  kRg11B10Float,
  kRg32Uint,
  kRg32Sint,
  kRg32Float,
  kRgba16Uint,
  kRgba16Sint,
  kRgba16Float,
  kRgba32Uint,
  kRgba32Sint,
  kRgba32Float,
};

/// @param out the std::ostream to write to
/// @param format the ImageFormat
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, ImageFormat format);

/// A storage texture type.
class StorageTexture : public Castable<StorageTexture, Texture> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  /// @param dim the dimensionality of the texture
  /// @param format the image format of the texture
  /// @param subtype the storage subtype. Use SubtypeFor() to calculate this.
  StorageTexture(ProgramID program_id,
                 const Source& source,
                 TextureDimension dim,
                 ImageFormat format,
                 Type* subtype);

  /// Move constructor
  StorageTexture(StorageTexture&&);
  ~StorageTexture() override;

  /// @returns the storage subtype
  Type* type() const { return subtype_; }

  /// @returns the image format
  ImageFormat image_format() const { return image_format_; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  StorageTexture* Clone(CloneContext* ctx) const override;

  /// @param format the storage texture image format
  /// @param builder the ProgramBuilder used to build the returned type
  /// @returns the storage texture subtype for the given ImageFormat
  static Type* SubtypeFor(ImageFormat format, ProgramBuilder& builder);

 private:
  ImageFormat const image_format_;
  Type* const subtype_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STORAGE_TEXTURE_H_
