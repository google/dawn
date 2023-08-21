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

#ifndef SRC_TINT_API_OPTIONS_TEXTURE_BUILTINS_FROM_UNIFORM_H_
#define SRC_TINT_API_OPTIONS_TEXTURE_BUILTINS_FROM_UNIFORM_H_

#include <unordered_map>
#include <utility>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/utils/reflection/reflection.h"

namespace tint {

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer sizes.
struct TextureBuiltinsFromUniformOptions {
    /// Indicate the type of field for each entry to push.
    enum class Field {
        /// The number of mip levels of the bonnd texture view.
        TextureNumLevels,
        /// The number of samples per texel of the bound multipsampled texture.
        TextureNumSamples,
    };

    /// Records the field and the byte offset of the data to push in the internal uniform buffer.
    using FieldAndOffset = std::pair<Field, uint32_t>;
    /// Maps from binding point to data entry with the information to populate the data.
    using BindingPointToFieldAndOffset = std::unordered_map<BindingPoint, FieldAndOffset>;

    /// The binding point to use to generate a uniform buffer from which to read
    /// buffer sizes.
    BindingPoint ubo_binding = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ubo_binding);
};

}  // namespace tint

#endif  // SRC_TINT_API_OPTIONS_TEXTURE_BUILTINS_FROM_UNIFORM_H_
