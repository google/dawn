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

#include "src/tint/type/texture.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Texture);

namespace tint::type {

Texture::Texture(size_t hash, TextureDimension dim) : Base(hash, type::Flags{}), dim_(dim) {}

Texture::~Texture() = default;

bool IsTextureArray(type::TextureDimension dim) {
    switch (dim) {
        case type::TextureDimension::k2dArray:
        case type::TextureDimension::kCubeArray:
            return true;
        case type::TextureDimension::k2d:
        case type::TextureDimension::kNone:
        case type::TextureDimension::k1d:
        case type::TextureDimension::k3d:
        case type::TextureDimension::kCube:
            return false;
    }
    return false;
}

int NumCoordinateAxes(type::TextureDimension dim) {
    switch (dim) {
        case type::TextureDimension::kNone:
            return 0;
        case type::TextureDimension::k1d:
            return 1;
        case type::TextureDimension::k2d:
        case type::TextureDimension::k2dArray:
            return 2;
        case type::TextureDimension::k3d:
        case type::TextureDimension::kCube:
        case type::TextureDimension::kCubeArray:
            return 3;
    }
    return 0;
}

}  // namespace tint::type
