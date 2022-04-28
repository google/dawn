// Copyright 2021 The Tint Authors.
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

#include "src/tint/sem/depth_multisampled_texture.h"

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::DepthMultisampledTexture);

namespace tint::sem {
namespace {

bool IsValidDepthDimension(ast::TextureDimension dim) {
  return dim == ast::TextureDimension::k2d;
}

}  // namespace

DepthMultisampledTexture::DepthMultisampledTexture(ast::TextureDimension dim)
    : Base(dim) {
  TINT_ASSERT(Semantic, IsValidDepthDimension(dim));
}

DepthMultisampledTexture::DepthMultisampledTexture(DepthMultisampledTexture&&) =
    default;

DepthMultisampledTexture::~DepthMultisampledTexture() = default;

size_t DepthMultisampledTexture::Hash() const {
  return utils::Hash(TypeInfo::Of<DepthMultisampledTexture>().full_hashcode,
                     dim());
}

bool DepthMultisampledTexture::Equals(const sem::Type& other) const {
  if (auto* o = other.As<DepthMultisampledTexture>()) {
    return o->dim() == dim();
  }
  return false;
}

std::string DepthMultisampledTexture::FriendlyName(const SymbolTable&) const {
  std::ostringstream out;
  out << "texture_depth_multisampled_" << dim();
  return out.str();
}

}  // namespace tint::sem
