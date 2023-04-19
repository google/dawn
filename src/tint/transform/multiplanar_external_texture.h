// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_TRANSFORM_MULTIPLANAR_EXTERNAL_TEXTURE_H_
#define SRC_TINT_TRANSFORM_MULTIPLANAR_EXTERNAL_TEXTURE_H_

#include <unordered_map>
#include <utility>

#include "src/tint/ast/struct_member.h"
#include "src/tint/builtin/function.h"
#include "src/tint/sem/binding_point.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Within the MultiplanarExternalTexture transform, each instance of a
/// texture_external binding is unpacked into two texture_2d<f32> bindings
/// representing two possible planes of a texture and a uniform buffer binding
/// representing a struct of parameters. Calls to textureLoad or
/// textureSampleLevel that contain a texture_external parameter will be
/// transformed into a newly generated version of the function, which can
/// perform the desired operation on a single RGBA plane or on separate Y and UV
/// planes, and do colorspace conversions including yuv->rgb conversion, gamma
/// decoding, gamut conversion, and gamma encoding steps. Specifically
// for BT.709 to SRGB conversion, it takes the fast path only doing the yuv->rgb
// step and skipping all other steps.
class MultiplanarExternalTexture final
    : public utils::Castable<MultiplanarExternalTexture, Transform> {
  public:
    /// This struct identifies the binding groups and locations for new bindings to
    /// use when transforming a texture_external instance.
    using BindingPoints = sem::external_texture::BindingPoints;

    /// BindingsMap is a map where the key is the binding location of a
    /// texture_external and the value is a struct containing the desired
    /// locations for new bindings expanded from the texture_external instance.
    using BindingsMap = sem::external_texture::BindingsMap;

    /// NewBindingPoints is consumed by the MultiplanarExternalTexture transform.
    /// Data holds information about location of each texture_external binding and
    /// which binding slots it should expand into.
    struct NewBindingPoints final : public utils::Castable<Data, transform::Data> {
        /// Constructor
        /// @param bm a map to the new binding slots to use.
        explicit NewBindingPoints(BindingsMap bm);

        /// Destructor
        ~NewBindingPoints() override;

        /// A map of new binding points to use.
        const BindingsMap bindings_map;
    };

    /// Constructor
    MultiplanarExternalTexture();
    /// Destructor
    ~MultiplanarExternalTexture() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_MULTIPLANAR_EXTERNAL_TEXTURE_H_
