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
                               ast::ImageFormat format,
                               ast::AccessControl::Access access_control,
                               sem::Type* subtype)
    : Base(dim),
      image_format_(format),
      access_control_(access_control),
      subtype_(subtype) {}

StorageTexture::StorageTexture(StorageTexture&&) = default;

StorageTexture::~StorageTexture() = default;

std::string StorageTexture::type_name() const {
  std::ostringstream out;
  out << "__storage_texture_" << dim() << "_" << image_format_ << "_"
      << access_control_;
  return out.str();
}

std::string StorageTexture::FriendlyName(const SymbolTable&) const {
  std::ostringstream out;
  out << "texture_storage_" << dim() << "<" << image_format_ << ", "
      << access_control_ << ">";
  return out.str();
}

sem::Type* StorageTexture::SubtypeFor(ast::ImageFormat format,
                                      sem::Manager& type_mgr) {
  switch (format) {
    case ast::ImageFormat::kR8Uint:
    case ast::ImageFormat::kR16Uint:
    case ast::ImageFormat::kRg8Uint:
    case ast::ImageFormat::kR32Uint:
    case ast::ImageFormat::kRg16Uint:
    case ast::ImageFormat::kRgba8Uint:
    case ast::ImageFormat::kRg32Uint:
    case ast::ImageFormat::kRgba16Uint:
    case ast::ImageFormat::kRgba32Uint: {
      return type_mgr.Get<sem::U32>();
    }

    case ast::ImageFormat::kR8Sint:
    case ast::ImageFormat::kR16Sint:
    case ast::ImageFormat::kRg8Sint:
    case ast::ImageFormat::kR32Sint:
    case ast::ImageFormat::kRg16Sint:
    case ast::ImageFormat::kRgba8Sint:
    case ast::ImageFormat::kRg32Sint:
    case ast::ImageFormat::kRgba16Sint:
    case ast::ImageFormat::kRgba32Sint: {
      return type_mgr.Get<sem::I32>();
    }

    case ast::ImageFormat::kR8Unorm:
    case ast::ImageFormat::kRg8Unorm:
    case ast::ImageFormat::kRgba8Unorm:
    case ast::ImageFormat::kRgba8UnormSrgb:
    case ast::ImageFormat::kBgra8Unorm:
    case ast::ImageFormat::kBgra8UnormSrgb:
    case ast::ImageFormat::kRgb10A2Unorm:
    case ast::ImageFormat::kR8Snorm:
    case ast::ImageFormat::kRg8Snorm:
    case ast::ImageFormat::kRgba8Snorm:
    case ast::ImageFormat::kR16Float:
    case ast::ImageFormat::kR32Float:
    case ast::ImageFormat::kRg16Float:
    case ast::ImageFormat::kRg11B10Float:
    case ast::ImageFormat::kRg32Float:
    case ast::ImageFormat::kRgba16Float:
    case ast::ImageFormat::kRgba32Float: {
      return type_mgr.Get<sem::F32>();
    }

    case ast::ImageFormat::kNone:
      break;
  }

  return nullptr;
}

}  // namespace sem
}  // namespace tint
