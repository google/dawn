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

#ifndef SRC_AST_TYPE_TEXTURE_TYPE_H_
#define SRC_AST_TYPE_TEXTURE_TYPE_H_

#include <ostream>
#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

class DepthTextureType;
class SampledTextureType;
class StorageTextureType;

/// The dimensionality of the texture
enum class TextureDimension {
  /// 1 dimensional texture
  k1d,
  /// 1 dimenstional array texture
  k1dArray,
  /// 2 dimensional texture
  k2d,
  /// 2 dimensional array texture
  k2dArray,
  /// 2 dimensional multi-sampled texture
  k2dMs,
  /// 2 dimensional multi-sampled array texture
  k2dMsArray,
  /// 3 dimensional texture
  k3d,
  /// cube texture
  kCube,
  /// cube array texture
  kCubeArray,
};
std::ostream& operator<<(std::ostream& out, TextureDimension dim);

/// A texture type.
class TextureType : public Type {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  explicit TextureType(TextureDimension dim);
  /// Move constructor
  TextureType(TextureType&&);
  ~TextureType() override;

  /// @returns true if the type is a texture type
  bool IsTexture() const override;

  /// @returns the texture dimension
  TextureDimension dim() const { return dim_; }

  /// @returns true if this is a depth texture
  virtual bool IsDepth() const;
  /// @returns true if this is a storage texture
  virtual bool IsStorage() const;
  /// @returns true if this is a sampled texture
  virtual bool IsSampled() const;

  /// @returns the texture as a depth texture
  const DepthTextureType* AsDepth() const;
  /// @returns the texture as a sampled texture
  const SampledTextureType* AsSampled() const;
  /// @returns the texture as a storage texture
  const StorageTextureType* AsStorage() const;

  /// @returns the texture as a depth texture
  DepthTextureType* AsDepth();
  /// @returns the texture as a sampled texture
  SampledTextureType* AsSampled();
  /// @returns the texture as a storage texture
  StorageTextureType* AsStorage();

 private:
  TextureDimension dim_ = TextureDimension::k1d;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_TEXTURE_TYPE_H_
