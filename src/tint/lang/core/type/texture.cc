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

#include "src/tint/lang/core/type/texture.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Texture);

namespace tint::core::type {

Texture::Texture(size_t hash, TextureDimension dim) : Base(hash, core::type::Flags{}), dim_(dim) {}

Texture::~Texture() = default;

bool IsTextureArray(core::type::TextureDimension dim) {
    switch (dim) {
        case core::type::TextureDimension::k2dArray:
        case core::type::TextureDimension::kCubeArray:
            return true;
        case core::type::TextureDimension::k2d:
        case core::type::TextureDimension::kNone:
        case core::type::TextureDimension::k1d:
        case core::type::TextureDimension::k3d:
        case core::type::TextureDimension::kCube:
            return false;
    }
    return false;
}

int NumCoordinateAxes(core::type::TextureDimension dim) {
    switch (dim) {
        case core::type::TextureDimension::kNone:
            return 0;
        case core::type::TextureDimension::k1d:
            return 1;
        case core::type::TextureDimension::k2d:
        case core::type::TextureDimension::k2dArray:
            return 2;
        case core::type::TextureDimension::k3d:
        case core::type::TextureDimension::kCube:
        case core::type::TextureDimension::kCubeArray:
            return 3;
    }
    return 0;
}

}  // namespace tint::core::type
