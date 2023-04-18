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

#include "gtest/gtest.h"

#include "src/tint/reader/spirv/parser_type.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::reader::spirv {
namespace {

TEST(SpvParserTypeTest, SameArgumentsGivesSamePointer) {
    Symbol sym(Symbol(1, {}, "1"));

    TypeManager ty;
    EXPECT_EQ(ty.Void(), ty.Void());
    EXPECT_EQ(ty.Bool(), ty.Bool());
    EXPECT_EQ(ty.U32(), ty.U32());
    EXPECT_EQ(ty.F32(), ty.F32());
    EXPECT_EQ(ty.I32(), ty.I32());
    EXPECT_EQ(ty.Pointer(ty.I32(), builtin::AddressSpace::kUndefined),
              ty.Pointer(ty.I32(), builtin::AddressSpace::kUndefined));
    EXPECT_EQ(ty.Vector(ty.I32(), 3), ty.Vector(ty.I32(), 3));
    EXPECT_EQ(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 3, 2));
    EXPECT_EQ(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 3, 2));
    EXPECT_EQ(ty.Alias(sym, ty.I32()), ty.Alias(sym, ty.I32()));
    EXPECT_EQ(ty.Struct(sym, {ty.I32()}), ty.Struct(sym, {ty.I32()}));
    EXPECT_EQ(ty.Sampler(type::SamplerKind::kSampler), ty.Sampler(type::SamplerKind::kSampler));
    EXPECT_EQ(ty.DepthTexture(type::TextureDimension::k2d),
              ty.DepthTexture(type::TextureDimension::k2d));
    EXPECT_EQ(ty.MultisampledTexture(type::TextureDimension::k2d, ty.I32()),
              ty.MultisampledTexture(type::TextureDimension::k2d, ty.I32()));
    EXPECT_EQ(ty.SampledTexture(type::TextureDimension::k2d, ty.I32()),
              ty.SampledTexture(type::TextureDimension::k2d, ty.I32()));
    EXPECT_EQ(ty.StorageTexture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kRead),
              ty.StorageTexture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kRead));
}

TEST(SpvParserTypeTest, DifferentArgumentsGivesDifferentPointer) {
    Symbol sym_a(Symbol(1, {}, "1"));
    Symbol sym_b(Symbol(2, {}, "2"));

    TypeManager ty;
    EXPECT_NE(ty.Pointer(ty.I32(), builtin::AddressSpace::kUndefined),
              ty.Pointer(ty.U32(), builtin::AddressSpace::kUndefined));
    EXPECT_NE(ty.Pointer(ty.I32(), builtin::AddressSpace::kUndefined),
              ty.Pointer(ty.I32(), builtin::AddressSpace::kIn));
    EXPECT_NE(ty.Vector(ty.I32(), 3), ty.Vector(ty.U32(), 3));
    EXPECT_NE(ty.Vector(ty.I32(), 3), ty.Vector(ty.I32(), 2));
    EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.U32(), 3, 2));
    EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 2, 2));
    EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 3, 3));
    EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.U32(), 3, 2));
    EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 2, 2));
    EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 3, 3));
    EXPECT_NE(ty.Alias(sym_a, ty.I32()), ty.Alias(sym_b, ty.I32()));
    EXPECT_NE(ty.Struct(sym_a, {ty.I32()}), ty.Struct(sym_b, {ty.I32()}));
    EXPECT_NE(ty.Sampler(type::SamplerKind::kSampler),
              ty.Sampler(type::SamplerKind::kComparisonSampler));
    EXPECT_NE(ty.DepthTexture(type::TextureDimension::k2d),
              ty.DepthTexture(type::TextureDimension::k1d));
    EXPECT_NE(ty.MultisampledTexture(type::TextureDimension::k2d, ty.I32()),
              ty.MultisampledTexture(type::TextureDimension::k3d, ty.I32()));
    EXPECT_NE(ty.MultisampledTexture(type::TextureDimension::k2d, ty.I32()),
              ty.MultisampledTexture(type::TextureDimension::k2d, ty.U32()));
    EXPECT_NE(ty.SampledTexture(type::TextureDimension::k2d, ty.I32()),
              ty.SampledTexture(type::TextureDimension::k3d, ty.I32()));
    EXPECT_NE(ty.SampledTexture(type::TextureDimension::k2d, ty.I32()),
              ty.SampledTexture(type::TextureDimension::k2d, ty.U32()));
    EXPECT_NE(ty.StorageTexture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kRead),
              ty.StorageTexture(type::TextureDimension::k3d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kRead));
    EXPECT_NE(ty.StorageTexture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kRead),
              ty.StorageTexture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Sint,
                                builtin::Access::kRead));
    EXPECT_NE(ty.StorageTexture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kRead),
              ty.StorageTexture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kWrite));
}

}  // namespace
}  // namespace tint::reader::spirv
