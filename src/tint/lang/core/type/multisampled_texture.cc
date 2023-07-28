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

#include "src/tint/lang/core/type/multisampled_texture.h"

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::MultisampledTexture);

namespace tint::type {

MultisampledTexture::MultisampledTexture(TextureDimension dim, const Type* type)
    : Base(Hash(tint::TypeInfo::Of<MultisampledTexture>().full_hashcode, dim, type), dim),
      type_(type) {
    TINT_ASSERT(type_);
}

MultisampledTexture::~MultisampledTexture() = default;

bool MultisampledTexture::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<MultisampledTexture>()) {
        return o->dim() == dim() && o->type_ == type_;
    }
    return false;
}

std::string MultisampledTexture::FriendlyName() const {
    StringStream out;
    out << "texture_multisampled_" << dim() << "<" << type_->FriendlyName() << ">";
    return out.str();
}

MultisampledTexture* MultisampledTexture::Clone(CloneContext& ctx) const {
    auto* ty = type_->Clone(ctx);
    return ctx.dst.mgr->Get<MultisampledTexture>(dim(), ty);
}

}  // namespace tint::type
