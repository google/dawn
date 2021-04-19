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

#include "src/type/external_texture_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::ExternalTexture);

namespace tint {
namespace type {

ExternalTexture::ExternalTexture() : Base(TextureDimension::k2d) {}

ExternalTexture::ExternalTexture(ExternalTexture&&) = default;

ExternalTexture::~ExternalTexture() = default;

std::string ExternalTexture::type_name() const {
  std::ostringstream out;
  out << "__external_texture";
  return out.str();
}

std::string ExternalTexture::FriendlyName(const SymbolTable&) const {
  std::ostringstream out;
  out << "texture_external";
  return out.str();
}

ExternalTexture* ExternalTexture::Clone(CloneContext* ctx) const {
  return ctx->dst->create<ExternalTexture>();
}

}  // namespace type
}  // namespace tint
