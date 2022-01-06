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

#include "src/sem/storage_texture_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::StorageTexture);

namespace tint {
namespace sem {

StorageTexture::StorageTexture(ast::TextureDimension dim,
                               ast::TexelFormat format,
                               ast::Access access,
                               sem::Type* subtype)
    : Base(dim), texel_format_(format), access_(access), subtype_(subtype) {}

StorageTexture::StorageTexture(StorageTexture&&) = default;

StorageTexture::~StorageTexture() = default;

std::string StorageTexture::type_name() const {
  std::ostringstream out;
  out << "__storage_texture_" << dim() << "_" << texel_format_ << "_"
      << access_;
  return out.str();
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

}  // namespace sem
}  // namespace tint
