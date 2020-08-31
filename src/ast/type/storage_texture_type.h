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

#ifndef SRC_AST_TYPE_STORAGE_TEXTURE_TYPE_H_
#define SRC_AST_TYPE_STORAGE_TEXTURE_TYPE_H_

#include <string>

#include "src/ast/type/texture_type.h"

namespace tint {
namespace ast {
namespace type {

/// The access value of the storage texture
enum class StorageAccess { kRead, kWrite };
std::ostream& operator<<(std::ostream& out, StorageAccess dim);

/// The image format in the storage texture
enum class ImageFormat {
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
std::ostream& operator<<(std::ostream& out, ImageFormat dim);

/// A storage texture type.
class StorageTextureType : public TextureType {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  /// @param access the access type for the texture
  /// @param format the image format of the texture
  StorageTextureType(TextureDimension dim,
                     StorageAccess access,
                     ImageFormat format);

  /// Move constructor
  StorageTextureType(StorageTextureType&&);
  ~StorageTextureType() override;

  /// @returns true if the type is a storage texture type
  bool IsStorage() const override;

  /// @param type the subtype of the storage texture
  void set_type(Type* const type);

  /// @returns the subtype of the storage texture set with set_type
  Type* type() const;

  /// @returns the storage access
  StorageAccess access() const { return storage_access_; }

  /// @returns the image format
  ImageFormat image_format() const { return image_format_; }

  /// @returns the name for this type
  std::string type_name() const override;

 private:
  Type* type_ = nullptr;
  StorageAccess storage_access_ = StorageAccess::kRead;
  ImageFormat image_format_ = ImageFormat::kRgba32Float;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_STORAGE_TEXTURE_TYPE_H_
