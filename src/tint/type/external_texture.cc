// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/external_texture.h"

#include "src/tint/type/manager.h"
#include "src/tint/type/texture_dimension.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::ExternalTexture);

namespace tint::type {

ExternalTexture::ExternalTexture()
    : Base(static_cast<size_t>(utils::TypeInfo::Of<ExternalTexture>().full_hashcode),
           TextureDimension::k2d) {}

ExternalTexture::~ExternalTexture() = default;

bool ExternalTexture::Equals(const UniqueNode& other) const {
    return other.Is<ExternalTexture>();
}

std::string ExternalTexture::FriendlyName() const {
    return "texture_external";
}

ExternalTexture* ExternalTexture::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<ExternalTexture>();
}

}  // namespace tint::type
