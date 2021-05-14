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

#ifndef SRC_SEM_STORAGE_TEXTURE_TYPE_H_
#define SRC_SEM_STORAGE_TEXTURE_TYPE_H_

#include <string>

#include "src/ast/access_control.h"
#include "src/ast/storage_texture.h"
#include "src/sem/texture_type.h"

namespace tint {
namespace sem {

class Manager;

/// A storage texture type.
class StorageTexture : public Castable<StorageTexture, Texture> {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  /// @param format the image format of the texture
  /// @param access_control the access control type of the texture
  /// @param subtype the storage subtype. Use SubtypeFor() to calculate this.
  StorageTexture(ast::TextureDimension dim,
                 ast::ImageFormat format,
                 ast::AccessControl::Access access_control,
                 sem::Type* subtype);

  /// Move constructor
  StorageTexture(StorageTexture&&);
  ~StorageTexture() override;

  /// @returns the storage subtype
  Type* type() const { return subtype_; }

  /// @returns the image format
  ast::ImageFormat image_format() const { return image_format_; }

  /// @returns the access control
  ast::AccessControl::Access access_control() const { return access_control_; }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// @param format the storage texture image format
  /// @param type_mgr the sem::Manager used to build the returned type
  /// @returns the storage texture subtype for the given ImageFormat
  static sem::Type* SubtypeFor(ast::ImageFormat format, sem::Manager& type_mgr);

 private:
  ast::ImageFormat const image_format_;
  ast::AccessControl::Access const access_control_;
  Type* const subtype_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_STORAGE_TEXTURE_TYPE_H_
