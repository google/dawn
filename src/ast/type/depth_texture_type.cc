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

#include "src/ast/type/depth_texture_type.h"

#include <cassert>
#include <sstream>

namespace tint {
namespace ast {
namespace type {
namespace {

#ifndef NDEBUG

bool IsValidDepthDimension(TextureDimension dim) {
  return dim == TextureDimension::k2d || dim == TextureDimension::k2dArray ||
         dim == TextureDimension::kCube || dim == TextureDimension::kCubeArray;
}

#endif  // NDEBUG

}  // namespace

DepthTextureType::DepthTextureType(TextureDimension dim) : TextureType(dim) {
  assert(IsValidDepthDimension(dim));
}

DepthTextureType::DepthTextureType(DepthTextureType&&) = default;

DepthTextureType::~DepthTextureType() = default;

bool DepthTextureType::IsDepth() const {
  return true;
}

std::string DepthTextureType::type_name() const {
  std::ostringstream out;
  out << "__depth_texture_" << dim();
  return out.str();
}

}  // namespace type
}  // namespace ast
}  // namespace tint
