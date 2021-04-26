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

#include "src/ast/external_texture.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::ExternalTexture);

namespace tint {
namespace ast {

// ExternalTexture::ExternalTexture() : Base(ast::TextureDimension::k2d) {}
ExternalTexture::ExternalTexture(ProgramID program_id, const Source& source)
    : Base(program_id, source, ast::TextureDimension::k2d) {}

ExternalTexture::ExternalTexture(ExternalTexture&&) = default;

ExternalTexture::~ExternalTexture() = default;

std::string ExternalTexture::type_name() const {
  return "__external_texture";
}

std::string ExternalTexture::FriendlyName(const SymbolTable&) const {
  return "texture_external";
}

ExternalTexture* ExternalTexture::Clone(CloneContext* ctx) const {
  return ctx->dst->create<ExternalTexture>();
}

}  // namespace ast
}  // namespace tint
