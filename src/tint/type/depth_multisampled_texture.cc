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

#include "src/tint/type/depth_multisampled_texture.h"

#include "src/tint/debug.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/type/manager.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::DepthMultisampledTexture);

namespace tint::type {
namespace {

bool IsValidDepthDimension(TextureDimension dim) {
    return dim == TextureDimension::k2d;
}

}  // namespace

DepthMultisampledTexture::DepthMultisampledTexture(TextureDimension dim)
    : Base(utils::Hash(utils::TypeInfo::Of<DepthMultisampledTexture>().full_hashcode, dim), dim) {
    TINT_ASSERT(Type, IsValidDepthDimension(dim));
}

DepthMultisampledTexture::~DepthMultisampledTexture() = default;

bool DepthMultisampledTexture::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<DepthMultisampledTexture>()) {
        return o->dim() == dim();
    }
    return false;
}

std::string DepthMultisampledTexture::FriendlyName() const {
    utils::StringStream out;
    out << "texture_depth_multisampled_" << dim();
    return out.str();
}

DepthMultisampledTexture* DepthMultisampledTexture::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<DepthMultisampledTexture>(dim());
}

}  // namespace tint::type
