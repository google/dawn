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

#ifndef SRC_TINT_LANG_CORE_TYPE_TEXTURE_H_
#define SRC_TINT_LANG_CORE_TYPE_TEXTURE_H_

#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::core::type {

/// A texture type.
class Texture : public Castable<Texture, Type> {
  public:
    /// Constructor
    /// @param hash the unique hash of the node
    /// @param dim the dimensionality of the texture
    Texture(size_t hash, TextureDimension dim);
    /// Destructor
    ~Texture() override;

    /// @returns the texture dimension
    TextureDimension dim() const { return dim_; }

  private:
    TextureDimension const dim_;
};

/// @param dim the type::TextureDimension to query
/// @return true if the given type::TextureDimension is an array texture
bool IsTextureArray(core::type::TextureDimension dim);

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
/// @param dim the type::TextureDimension to query
/// @return number of dimensions in a coordinate for the dimensionality
int NumCoordinateAxes(core::type::TextureDimension dim);

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_TEXTURE_H_
