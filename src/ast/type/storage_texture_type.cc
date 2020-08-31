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

#include "src/ast/type/storage_texture_type.h"

#include <cassert>
#include <sstream>

namespace tint {
namespace ast {
namespace type {
namespace {

#ifndef NDEBUG

bool IsValidStorageDimension(TextureDimension dim) {
  return dim == TextureDimension::k1d || dim == TextureDimension::k1dArray ||
         dim == TextureDimension::k2d || dim == TextureDimension::k2dArray ||
         dim == TextureDimension::k3d;
}

#endif  // NDEBUG

}  // namespace

std::ostream& operator<<(std::ostream& out, StorageAccess access) {
  switch (access) {
    case StorageAccess::kRead:
      out << "read";
      break;
    case StorageAccess::kWrite:
      out << "write";
      break;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, ImageFormat format) {
  switch (format) {
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
      out << "rgba8unorm-srgb";
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
      out << "rbgra8unorm-srgb";
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

StorageTextureType::StorageTextureType(TextureDimension dim,
                                       StorageAccess access,
                                       ImageFormat format)
    : TextureType(dim), storage_access_(access), image_format_(format) {
  assert(IsValidStorageDimension(dim));
}

void StorageTextureType::set_type(Type* const type) {
  type_ = type;
}

Type* StorageTextureType::type() const {
  return type_;
}

StorageTextureType::StorageTextureType(StorageTextureType&&) = default;

StorageTextureType::~StorageTextureType() = default;

bool StorageTextureType::IsStorage() const {
  return true;
}

std::string StorageTextureType::type_name() const {
  std::ostringstream out;
  out << "__storage_texture_" << storage_access_ << "_" << dim() << "_"
      << image_format_;
  return out.str();
}

}  // namespace type
}  // namespace ast
}  // namespace tint
