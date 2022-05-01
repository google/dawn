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

// Note, these names match the names in the WGSL spec. This behaviour is used
// in the WGSL writer to emit the texture format names.
std::ostream& operator<<(std::ostream& out, TexelFormat format) {
    switch (format) {
        case TexelFormat::kNone:
            out << "none";
            break;
        case TexelFormat::kR32Uint:
            out << "r32uint";
            break;
        case TexelFormat::kR32Sint:
            out << "r32sint";
            break;
        case TexelFormat::kR32Float:
            out << "r32float";
            break;
        case TexelFormat::kRgba8Unorm:
            out << "rgba8unorm";
            break;
        case TexelFormat::kRgba8Snorm:
            out << "rgba8snorm";
            break;
        case TexelFormat::kRgba8Uint:
            out << "rgba8uint";
            break;
        case TexelFormat::kRgba8Sint:
            out << "rgba8sint";
            break;
        case TexelFormat::kRg32Uint:
            out << "rg32uint";
            break;
        case TexelFormat::kRg32Sint:
            out << "rg32sint";
            break;
        case TexelFormat::kRg32Float:
            out << "rg32float";
            break;
        case TexelFormat::kRgba16Uint:
            out << "rgba16uint";
            break;
        case TexelFormat::kRgba16Sint:
            out << "rgba16sint";
            break;
        case TexelFormat::kRgba16Float:
            out << "rgba16float";
            break;
        case TexelFormat::kRgba32Uint:
            out << "rgba32uint";
            break;
        case TexelFormat::kRgba32Sint:
            out << "rgba32sint";
            break;
        case TexelFormat::kRgba32Float:
            out << "rgba32float";
            break;
    }
    return out;
}

StorageTexture::StorageTexture(ProgramID pid,
                               const Source& src,
                               TextureDimension d,
                               TexelFormat fmt,
                               const Type* subtype,
                               Access ac)
    : Base(pid, src, d), format(fmt), type(subtype), access(ac) {}

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

        case TexelFormat::kNone:
            break;
    }

    return nullptr;
}

}  // namespace tint::ast
