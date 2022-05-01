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

#include "src/tint/ast/depth_multisampled_texture.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DepthMultisampledTexture);

namespace tint::ast {
namespace {

bool IsValidDepthDimension(TextureDimension dim) {
    return dim == TextureDimension::k2d;
}

}  // namespace

DepthMultisampledTexture::DepthMultisampledTexture(ProgramID pid,
                                                   const Source& src,
                                                   TextureDimension d)
    : Base(pid, src, d) {
    TINT_ASSERT(AST, IsValidDepthDimension(dim));
}

DepthMultisampledTexture::DepthMultisampledTexture(DepthMultisampledTexture&&) = default;

DepthMultisampledTexture::~DepthMultisampledTexture() = default;

std::string DepthMultisampledTexture::FriendlyName(const SymbolTable&) const {
    std::ostringstream out;
    out << "texture_depth_multisampled_" << dim;
    return out.str();
}

const DepthMultisampledTexture* DepthMultisampledTexture::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    return ctx->dst->create<DepthMultisampledTexture>(src, dim);
}

}  // namespace tint::ast
