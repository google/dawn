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

#include "src/tint/sem/sampled_texture.h"

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::SampledTexture);

namespace tint::sem {

SampledTexture::SampledTexture(ast::TextureDimension dim, const Type* type)
    : Base(dim), type_(type) {
  TINT_ASSERT(Semantic, type_);
}

SampledTexture::SampledTexture(SampledTexture&&) = default;

SampledTexture::~SampledTexture() = default;

size_t SampledTexture::Hash() const {
  return utils::Hash(TypeInfo::Of<SampledTexture>().full_hashcode, dim(),
                     type_);
}

bool SampledTexture::Equals(const sem::Type& other) const {
  if (auto* o = other.As<SampledTexture>()) {
    return o->dim() == dim() && o->type_ == type_;
  }
  return false;
}

std::string SampledTexture::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  out << "texture_" << dim() << "<" << type_->FriendlyName(symbols) << ">";
  return out.str();
}

}  // namespace tint::sem
