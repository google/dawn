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

#include "src/tint/sem/storage_texture.h"

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::StorageTexture);

namespace tint::sem {

StorageTexture::StorageTexture(ast::TextureDimension dim,
                               ast::TexelFormat format,
                               ast::Access access,
                               sem::Type* subtype)
    : Base(dim), texel_format_(format), access_(access), subtype_(subtype) {}

StorageTexture::StorageTexture(StorageTexture&&) = default;

StorageTexture::~StorageTexture() = default;

size_t StorageTexture::Hash() const {
  return utils::Hash(TypeInfo::Of<StorageTexture>().full_hashcode, dim(),
                     texel_format_, access_);
}

bool StorageTexture::Equals(const sem::Type& other) const {
  if (auto* o = other.As<StorageTexture>()) {
    return o->dim() == dim() && o->texel_format_ == texel_format_ &&
           o->access_ == access_;
  }
  return false;
}

std::string StorageTexture::FriendlyName(const SymbolTable&) const {
  std::ostringstream out;
  out << "texture_storage_" << dim() << "<" << texel_format_ << ", " << access_
      << ">";
  return out.str();
}

sem::Type* StorageTexture::SubtypeFor(ast::TexelFormat format,
                                      sem::Manager& type_mgr) {
  switch (format) {
    case ast::TexelFormat::kR32Uint:
    case ast::TexelFormat::kRgba8Uint:
    case ast::TexelFormat::kRg32Uint:
    case ast::TexelFormat::kRgba16Uint:
    case ast::TexelFormat::kRgba32Uint: {
      return type_mgr.Get<sem::U32>();
    }

    case ast::TexelFormat::kR32Sint:
    case ast::TexelFormat::kRgba8Sint:
    case ast::TexelFormat::kRg32Sint:
    case ast::TexelFormat::kRgba16Sint:
    case ast::TexelFormat::kRgba32Sint: {
      return type_mgr.Get<sem::I32>();
    }

    case ast::TexelFormat::kRgba8Unorm:
    case ast::TexelFormat::kRgba8Snorm:
    case ast::TexelFormat::kR32Float:
    case ast::TexelFormat::kRg32Float:
    case ast::TexelFormat::kRgba16Float:
    case ast::TexelFormat::kRgba32Float: {
      return type_mgr.Get<sem::F32>();
    }

    case ast::TexelFormat::kNone:
      break;
  }

  return nullptr;
}

}  // namespace tint::sem
