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

#include "src/type/storage_texture_type.h"

#include <cassert>
#include <sstream>

#include "src/ast/module.h"
#include "src/clone_context.h"

TINT_INSTANTIATE_CLASS_ID(tint::type::StorageTexture);

namespace tint {
namespace type {

// Note, these names match the names in the WGSL spec. This behaviour is used
// in the WGSL writer to emit the texture format names.
std::ostream& operator<<(std::ostream& out, ImageFormat format) {
  switch (format) {
    case ImageFormat::kNone:
      out << "none";
      break;
    case ImageFormat::kR8Unorm:
      out << "r8unorm";
      break;
    case ImageFormat::kR8Snorm:
      out << "r8snorm";
      break;
    case ImageFormat::kR8Uint:
      out << "r8uint";
      break;
    case ImageFormat::kR8Sint:
      out << "r8sint";
      break;
    case ImageFormat::kR16Uint:
      out << "r16uint";
      break;
    case ImageFormat::kR16Sint:
      out << "r16sint";
      break;
    case ImageFormat::kR16Float:
      out << "r16float";
      break;
    case ImageFormat::kRg8Unorm:
      out << "rg8unorm";
      break;
    case ImageFormat::kRg8Snorm:
      out << "rg8snorm";
      break;
    case ImageFormat::kRg8Uint:
      out << "rg8uint";
      break;
    case ImageFormat::kRg8Sint:
      out << "rg8sint";
      break;
    case ImageFormat::kR32Uint:
      out << "r32uint";
      break;
    case ImageFormat::kR32Sint:
      out << "r32sint";
      break;
    case ImageFormat::kR32Float:
      out << "r32float";
      break;
    case ImageFormat::kRg16Uint:
      out << "rg16uint";
      break;
    case ImageFormat::kRg16Sint:
      out << "rg16sint";
      break;
    case ImageFormat::kRg16Float:
      out << "rg16float";
      break;
    case ImageFormat::kRgba8Unorm:
      out << "rgba8unorm";
      break;
    case ImageFormat::kRgba8UnormSrgb:
      out << "rgba8unorm_srgb";
      break;
    case ImageFormat::kRgba8Snorm:
      out << "rgba8snorm";
      break;
    case ImageFormat::kRgba8Uint:
      out << "rgba8uint";
      break;
    case ImageFormat::kRgba8Sint:
      out << "rgba8sint";
      break;
    case ImageFormat::kBgra8Unorm:
      out << "bgra8unorm";
      break;
    case ImageFormat::kBgra8UnormSrgb:
      out << "bgra8unorm_srgb";
      break;
    case ImageFormat::kRgb10A2Unorm:
      out << "rgb10a2unorm";
      break;
    case ImageFormat::kRg11B10Float:
      out << "rg11b10float";
      break;
    case ImageFormat::kRg32Uint:
      out << "rg32uint";
      break;
    case ImageFormat::kRg32Sint:
      out << "rg32sint";
      break;
    case ImageFormat::kRg32Float:
      out << "rg32float";
      break;
    case ImageFormat::kRgba16Uint:
      out << "rgba16uint";
      break;
    case ImageFormat::kRgba16Sint:
      out << "rgba16sint";
      break;
    case ImageFormat::kRgba16Float:
      out << "rgba16float";
      break;
    case ImageFormat::kRgba32Uint:
      out << "rgba32uint";
      break;
    case ImageFormat::kRgba32Sint:
      out << "rgba32sint";
      break;
    case ImageFormat::kRgba32Float:
      out << "rgba32float";
      break;
  }
  return out;
}

StorageTexture::StorageTexture(TextureDimension dim, ImageFormat format)
    : Base(dim), image_format_(format) {}

void StorageTexture::set_type(Type* const type) {
  type_ = type;
}

Type* StorageTexture::type() const {
  return type_;
}

StorageTexture::StorageTexture(StorageTexture&&) = default;

StorageTexture::~StorageTexture() = default;

std::string StorageTexture::type_name() const {
  std::ostringstream out;
  out << "__storage_texture_" << dim() << "_" << image_format_;
  return out.str();
}

StorageTexture* StorageTexture::Clone(CloneContext* ctx) const {
  return ctx->mod->create<StorageTexture>(dim(), image_format_);
}

}  // namespace type
}  // namespace tint
