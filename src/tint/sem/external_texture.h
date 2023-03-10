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

#ifndef SRC_TINT_SEM_EXTERNAL_TEXTURE_H_
#define SRC_TINT_SEM_EXTERNAL_TEXTURE_H_

#include <unordered_map>

#include "src/tint/sem/binding_point.h"

namespace tint::sem::external_texture {

/// This struct identifies the binding groups and locations for new bindings to
/// use when transforming a texture_external instance.
struct BindingPoints {
    /// The desired binding location of the texture_2d representing plane #1 when
    /// a texture_external binding is expanded.
    BindingPoint plane_1;
    /// The desired binding location of the ExternalTextureParams uniform when a
    /// texture_external binding is expanded.
    BindingPoint params;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(plane_1, params);
};

/// BindingsMap is a map where the key is the binding location of a
/// texture_external and the value is a struct containing the desired
/// locations for new bindings expanded from the texture_external instance.
using BindingsMap = std::unordered_map<BindingPoint, BindingPoints>;

}  // namespace tint::sem::external_texture

#endif  // SRC_TINT_SEM_EXTERNAL_TEXTURE_H_
