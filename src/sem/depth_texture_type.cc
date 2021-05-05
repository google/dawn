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

#include "src/sem/depth_texture_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::DepthTexture);

namespace tint {
namespace sem {
namespace {

bool IsValidDepthDimension(ast::TextureDimension dim) {
  return dim == ast::TextureDimension::k2d ||
         dim == ast::TextureDimension::k2dArray ||
         dim == ast::TextureDimension::kCube ||
         dim == ast::TextureDimension::kCubeArray;
}

}  // namespace

DepthTexture::DepthTexture(ast::TextureDimension dim) : Base(dim) {
  TINT_ASSERT(IsValidDepthDimension(dim));
}

DepthTexture::DepthTexture(DepthTexture&&) = default;

DepthTexture::~DepthTexture() = default;

std::string DepthTexture::type_name() const {
  std::ostringstream out;
  out << "__depth_texture_" << dim();
  return out.str();
}

std::string DepthTexture::FriendlyName(const SymbolTable&) const {
  std::ostringstream out;
  out << "texture_depth_" << dim();
  return out.str();
}

}  // namespace sem
}  // namespace tint
