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

#include "src/tint/ast/storage_texture.h"

#include "src/tint/ast/f32.h"
#include "src/tint/ast/i32.h"
#include "src/tint/ast/u32.h"
#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::StorageTexture);

namespace tint::ast {

StorageTexture::StorageTexture(ProgramID pid,
                               NodeID nid,
                               const Source& src,
                               TextureDimension d,
                               TexelFormat fmt,
                               const Type* subtype,
                               Access ac)
    : Base(pid, nid, src, d), format(fmt), type(subtype), access(ac) {}

StorageTexture::StorageTexture(StorageTexture&&) = default;

StorageTexture::~StorageTexture() = default;

std::string StorageTexture::FriendlyName(const SymbolTable&) const {
    std::ostringstream out;
    out << "texture_storage_" << dim << "<" << format << ", " << access << ">";
    return out.str();
}

const StorageTexture* StorageTexture::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* ty = ctx->Clone(type);
    return ctx->dst->create<StorageTexture>(src, dim, format, ty, access);
}

Type* StorageTexture::SubtypeFor(TexelFormat format, ProgramBuilder& builder) {
    switch (format) {
        case TexelFormat::kR32Uint:
        case TexelFormat::kRgba8Uint:
        case TexelFormat::kRg32Uint:
        case TexelFormat::kRgba16Uint:
        case TexelFormat::kRgba32Uint: {
            return builder.create<U32>();
        }

        case TexelFormat::kR32Sint:
        case TexelFormat::kRgba8Sint:
        case TexelFormat::kRg32Sint:
        case TexelFormat::kRgba16Sint:
        case TexelFormat::kRgba32Sint: {
            return builder.create<I32>();
        }

        case TexelFormat::kRgba8Unorm:
        case TexelFormat::kRgba8Snorm:
        case TexelFormat::kR32Float:
        case TexelFormat::kRg32Float:
        case TexelFormat::kRgba16Float:
        case TexelFormat::kRgba32Float: {
            return builder.create<F32>();
        }

        case TexelFormat::kUndefined:
            break;
    }

    return nullptr;
}

}  // namespace tint::ast
