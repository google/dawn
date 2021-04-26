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

#include "src/ast/sampled_texture.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::SampledTexture);

namespace tint {
namespace ast {

SampledTexture::SampledTexture(ProgramID program_id,
                               const Source& source,
                               TextureDimension dim,
                               Type const* type)
    : Base(program_id, source, dim), type_(type) {
  TINT_ASSERT(type_);
}

SampledTexture::SampledTexture(SampledTexture&&) = default;

SampledTexture::~SampledTexture() = default;

std::string SampledTexture::type_name() const {
  std::ostringstream out;
  out << "__sampled_texture_" << dim() << type_->type_name();
  return out.str();
}

std::string SampledTexture::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  out << "texture_" << dim() << "<" << type_->FriendlyName(symbols) << ">";
  return out.str();
}

SampledTexture* SampledTexture::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* ty = ctx->Clone(type());
  return ctx->dst->create<SampledTexture>(src, dim(), ty);
}

}  // namespace ast
}  // namespace tint
