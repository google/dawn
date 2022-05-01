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

#include "src/tint/sem/multisampled_texture.h"

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::MultisampledTexture);

namespace tint::sem {

MultisampledTexture::MultisampledTexture(ast::TextureDimension dim, const Type* type)
    : Base(dim), type_(type) {
    TINT_ASSERT(Semantic, type_);
}

MultisampledTexture::MultisampledTexture(MultisampledTexture&&) = default;

MultisampledTexture::~MultisampledTexture() = default;

size_t MultisampledTexture::Hash() const {
    return utils::Hash(TypeInfo::Of<MultisampledTexture>().full_hashcode, dim(), type_);
}

bool MultisampledTexture::Equals(const sem::Type& other) const {
    if (auto* o = other.As<MultisampledTexture>()) {
        return o->dim() == dim() && o->type_ == type_;
    }
    return false;
}

std::string MultisampledTexture::FriendlyName(const SymbolTable& symbols) const {
    std::ostringstream out;
    out << "texture_multisampled_" << dim() << "<" << type_->FriendlyName(symbols) << ">";
    return out.str();
}

}  // namespace tint::sem
