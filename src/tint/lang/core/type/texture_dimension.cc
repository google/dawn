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

#include "src/tint/lang/core/type/texture_dimension.h"

namespace tint::core::type {

std::string_view ToString(core::type::TextureDimension dim) {
    switch (dim) {
        case core::type::TextureDimension::kNone:
            return "None";
        case core::type::TextureDimension::k1d:
            return "1d";
        case core::type::TextureDimension::k2d:
            return "2d";
        case core::type::TextureDimension::k2dArray:
            return "2d_array";
        case core::type::TextureDimension::k3d:
            return "3d";
        case core::type::TextureDimension::kCube:
            return "cube";
        case core::type::TextureDimension::kCubeArray:
            return "cube_array";
    }
    return "<unknown>";
}

}  // namespace tint::core::type
