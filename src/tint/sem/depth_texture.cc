// Copyright 2020 The Tint Authors.
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

#include "src/tint/sem/depth_texture.h"

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::DepthTexture);

namespace tint::sem {
namespace {

bool IsValidDepthDimension(ast::TextureDimension dim) {
    return dim == ast::TextureDimension::k2d || dim == ast::TextureDimension::k2dArray ||
           dim == ast::TextureDimension::kCube || dim == ast::TextureDimension::kCubeArray;
}

}  // namespace

DepthTexture::DepthTexture(ast::TextureDimension dim) : Base(dim) {
    TINT_ASSERT(Semantic, IsValidDepthDimension(dim));
}

DepthTexture::DepthTexture(DepthTexture&&) = default;

DepthTexture::~DepthTexture() = default;

size_t DepthTexture::Hash() const {
    return utils::Hash(TypeInfo::Of<DepthTexture>().full_hashcode, dim());
}

bool DepthTexture::Equals(const sem::Type& other) const {
    if (auto* o = other.As<DepthTexture>()) {
        return o->dim() == dim();
    }
    return false;
}

std::string DepthTexture::FriendlyName(const SymbolTable&) const {
    std::ostringstream out;
    out << "texture_depth_" << dim();
    return out.str();
}

}  // namespace tint::sem
