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

#include "src/ast/depth_texture.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DepthTexture);

namespace tint {
namespace ast {
namespace {

bool IsValidDepthDimension(TextureDimension dim) {
  return dim == TextureDimension::k2d || dim == TextureDimension::k2dArray ||
         dim == TextureDimension::kCube || dim == TextureDimension::kCubeArray;
}

}  // namespace

DepthTexture::DepthTexture(ProgramID program_id,
                           const Source& source,
                           TextureDimension dim)
    : Base(program_id, source, dim) {
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

DepthTexture* DepthTexture::Clone(CloneContext* ctx) const {
  auto src = ctx->Clone(source());
  return ctx->dst->create<DepthTexture>(src, dim());
}

}  // namespace ast
}  // namespace tint
