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

#ifndef SRC_TYPE_TEXTURE_TYPE_H_
#define SRC_TYPE_TEXTURE_TYPE_H_

#include "src/type/type.h"

namespace tint {
namespace type {

/// The dimensionality of the texture
enum class TextureDimension {
  /// Invalid texture
  kNone = -1,
  /// 1 dimensional texture
  k1d,
  /// 2 dimensional texture
  k2d,
  /// 2 dimensional array texture
  k2dArray,
  /// 3 dimensional texture
  k3d,
  /// cube texture
  kCube,
  /// cube array texture
  kCubeArray,
};
std::ostream& operator<<(std::ostream& out, TextureDimension dim);

/// @param dim the TextureDimension to query
/// @return true if the given TextureDimension is an array texture
bool IsTextureArray(TextureDimension dim);

/// Returns the number of axes in the coordinate for a dimensionality.
///  None -> 0
///  1D -> 1
///  2D, 2DArray -> 2
///  3D, Cube, CubeArray -> 3
/// @param dim the TextureDimension to query
/// @return number of dimensions in a coordinate for the dimensionality
int NumCoordinateAxes(TextureDimension dim);

/// A texture type.
class Texture : public Castable<Texture, Type> {
 public:
  /// Constructor
  /// @param dim the dimensionality of the texture
  explicit Texture(TextureDimension dim);
  /// Move constructor
  Texture(Texture&&);
  ~Texture() override;

  /// @returns the texture dimension
  TextureDimension dim() const { return dim_; }

 private:
  TextureDimension const dim_;
};

}  // namespace type
}  // namespace tint

#endif  // SRC_TYPE_TEXTURE_TYPE_H_
