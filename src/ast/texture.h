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

#ifndef SRC_AST_TEXTURE_H_
#define SRC_AST_TEXTURE_H_

#include "src/ast/type.h"

namespace tint {
namespace ast {

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

/// @param out the std::ostream to write to
/// @param dim the TextureDimension
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, TextureDimension dim);

/// @param dim the TextureDimension to query
/// @return true if the given TextureDimension is an array texture
bool IsTextureArray(TextureDimension dim);

/// Returns the number of axes in the coordinate used for accessing
/// the texture, where an access is one of: sampling, fetching, load,
/// or store.
///  None -> 0
///  1D -> 1
///  2D, 2DArray -> 2
///  3D, Cube, CubeArray -> 3
/// Note: To sample a cube texture, the coordinate has 3 dimensions,
/// but textureDimensions on a cube or cube array returns a 2-element
/// size, representing the (x,y) size of each cube face, in texels.
/// @param dim the TextureDimension to query
/// @return number of dimensions in a coordinate for the dimensionality
int NumCoordinateAxes(TextureDimension dim);

/// A texture type.
class Texture : public Castable<Texture, Type> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  /// @param dim the dimensionality of the texture
  Texture(ProgramID program_id, const Source& source, TextureDimension dim);
  /// Move constructor
  Texture(Texture&&);
  ~Texture() override;

  /// @returns the texture dimension
  TextureDimension dim() const { return dim_; }

 private:
  TextureDimension const dim_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TEXTURE_H_
