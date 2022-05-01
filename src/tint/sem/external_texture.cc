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

#include "src/tint/sem/external_texture.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::ExternalTexture);

namespace tint::sem {

ExternalTexture::ExternalTexture() : Base(ast::TextureDimension::k2d) {}

ExternalTexture::ExternalTexture(ExternalTexture&&) = default;

ExternalTexture::~ExternalTexture() = default;

size_t ExternalTexture::Hash() const {
    return static_cast<size_t>(TypeInfo::Of<ExternalTexture>().full_hashcode);
}

bool ExternalTexture::Equals(const sem::Type& other) const {
    return other.Is<ExternalTexture>();
}

std::string ExternalTexture::FriendlyName(const SymbolTable&) const {
    return "texture_external";
}

}  // namespace tint::sem
