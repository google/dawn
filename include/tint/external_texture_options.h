// Copyright 2023 The Tint Authors.
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

#ifndef INCLUDE_TINT_EXTERNAL_TEXTURE_OPTIONS_H_
#define INCLUDE_TINT_EXTERNAL_TEXTURE_OPTIONS_H_

#include <unordered_map>

#include "src/tint/lang/wgsl/sem/external_texture.h"

namespace tint {

/// Options used to specify mappings of binding points for external textures.
struct ExternalTextureOptions {
    /// This struct identifies the binding groups and locations for new bindings to
    /// use when transforming a texture_external instance.
    using BindingPoints = sem::external_texture::BindingPoints;

    /// BindingsMap is a map where the key is the binding location of a
    /// texture_external and the value is a struct containing the desired
    /// locations for new bindings expanded from the texture_external instance.
    using BindingsMap = sem::external_texture::BindingsMap;

    /// A map of new binding points to use.
    BindingsMap bindings_map;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(bindings_map);
};

}  // namespace tint

#endif  // INCLUDE_TINT_EXTERNAL_TEXTURE_OPTIONS_H_
