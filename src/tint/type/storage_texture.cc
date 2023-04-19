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

#include "src/tint/type/storage_texture.h"

#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/manager.h"
#include "src/tint/type/u32.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::StorageTexture);

namespace tint::type {

StorageTexture::StorageTexture(TextureDimension dim,
                               builtin::TexelFormat format,
                               builtin::Access access,
                               Type* subtype)
    : Base(utils::Hash(utils::TypeInfo::Of<StorageTexture>().full_hashcode, dim, format, access),
           dim),
      texel_format_(format),
      access_(access),
      subtype_(subtype) {}

StorageTexture::~StorageTexture() = default;

bool StorageTexture::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<StorageTexture>()) {
        return o->dim() == dim() && o->texel_format_ == texel_format_ && o->access_ == access_;
    }
    return false;
}

std::string StorageTexture::FriendlyName() const {
    utils::StringStream out;
    out << "texture_storage_" << dim() << "<" << texel_format_ << ", " << access_ << ">";
    return out.str();
}

Type* StorageTexture::SubtypeFor(builtin::TexelFormat format, Manager& type_mgr) {
    switch (format) {
        case builtin::TexelFormat::kR32Uint:
        case builtin::TexelFormat::kRgba8Uint:
        case builtin::TexelFormat::kRg32Uint:
        case builtin::TexelFormat::kRgba16Uint:
        case builtin::TexelFormat::kRgba32Uint: {
            return type_mgr.Get<U32>();
        }

        case builtin::TexelFormat::kR32Sint:
        case builtin::TexelFormat::kRgba8Sint:
        case builtin::TexelFormat::kRg32Sint:
        case builtin::TexelFormat::kRgba16Sint:
        case builtin::TexelFormat::kRgba32Sint: {
            return type_mgr.Get<I32>();
        }

        case builtin::TexelFormat::kBgra8Unorm:
        case builtin::TexelFormat::kRgba8Unorm:
        case builtin::TexelFormat::kRgba8Snorm:
        case builtin::TexelFormat::kR32Float:
        case builtin::TexelFormat::kRg32Float:
        case builtin::TexelFormat::kRgba16Float:
        case builtin::TexelFormat::kRgba32Float: {
            return type_mgr.Get<F32>();
        }

        case builtin::TexelFormat::kUndefined:
            break;
    }

    return nullptr;
}

StorageTexture* StorageTexture::Clone(CloneContext& ctx) const {
    auto* ty = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<StorageTexture>(dim(), texel_format_, access_, ty);
}

}  // namespace tint::type
