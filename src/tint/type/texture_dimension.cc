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

#include "src/tint/type/texture_dimension.h"

namespace tint::type {

utils::StringStream& operator<<(utils::StringStream& out, type::TextureDimension dim) {
    switch (dim) {
        case type::TextureDimension::kNone:
            out << "None";
            break;
        case type::TextureDimension::k1d:
            out << "1d";
            break;
        case type::TextureDimension::k2d:
            out << "2d";
            break;
        case type::TextureDimension::k2dArray:
            out << "2d_array";
            break;
        case type::TextureDimension::k3d:
            out << "3d";
            break;
        case type::TextureDimension::kCube:
            out << "cube";
            break;
        case type::TextureDimension::kCubeArray:
            out << "cube_array";
            break;
    }
    return out;
}

}  // namespace tint::type
