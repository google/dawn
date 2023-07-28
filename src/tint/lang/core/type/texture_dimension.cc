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

namespace tint::type {

std::string_view ToString(type::TextureDimension dim) {
    switch (dim) {
        case type::TextureDimension::kNone:
            return "None";
        case type::TextureDimension::k1d:
            return "1d";
        case type::TextureDimension::k2d:
            return "2d";
        case type::TextureDimension::k2dArray:
            return "2d_array";
        case type::TextureDimension::k3d:
            return "3d";
        case type::TextureDimension::kCube:
            return "cube";
        case type::TextureDimension::kCubeArray:
            return "cube_array";
    }
    return "<unknown>";
}

}  // namespace tint::type
