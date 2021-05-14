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

#include "src/reader/spirv/parser_type.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

TEST(SpvParserTypeTest, SameArgumentsGivesSamePointer) {
  Symbol sym(Symbol(1, {}));

  TypeManager ty;
  EXPECT_EQ(ty.Void(), ty.Void());
  EXPECT_EQ(ty.Bool(), ty.Bool());
  EXPECT_EQ(ty.U32(), ty.U32());
  EXPECT_EQ(ty.F32(), ty.F32());
  EXPECT_EQ(ty.I32(), ty.I32());
  EXPECT_EQ(ty.Pointer(ty.I32(), ast::StorageClass::kNone),
            ty.Pointer(ty.I32(), ast::StorageClass::kNone));
  EXPECT_EQ(ty.Vector(ty.I32(), 3), ty.Vector(ty.I32(), 3));
  EXPECT_EQ(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 3, 2));
  EXPECT_EQ(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 3, 2));
  EXPECT_EQ(ty.AccessControl(ty.I32(), ast::AccessControl::kReadOnly),
            ty.AccessControl(ty.I32(), ast::AccessControl::kReadOnly));
  EXPECT_EQ(ty.Alias(sym, ty.I32()), ty.Alias(sym, ty.I32()));
  EXPECT_EQ(ty.Struct(sym, {ty.I32()}), ty.Struct(sym, {ty.I32()}));
  EXPECT_EQ(ty.Sampler(ast::SamplerKind::kSampler),
            ty.Sampler(ast::SamplerKind::kSampler));
  EXPECT_EQ(ty.DepthTexture(ast::TextureDimension::k2d),
            ty.DepthTexture(ast::TextureDimension::k2d));
  EXPECT_EQ(ty.MultisampledTexture(ast::TextureDimension::k2d, ty.I32()),
            ty.MultisampledTexture(ast::TextureDimension::k2d, ty.I32()));
  EXPECT_EQ(ty.SampledTexture(ast::TextureDimension::k2d, ty.I32()),
            ty.SampledTexture(ast::TextureDimension::k2d, ty.I32()));
  EXPECT_EQ(
      ty.StorageTexture(ast::TextureDimension::k2d, ast::ImageFormat::kR16Sint),
      ty.StorageTexture(ast::TextureDimension::k2d,
                        ast::ImageFormat::kR16Sint));
}

TEST(SpvParserTypeTest, DifferentArgumentsGivesDifferentPointer) {
  Symbol sym_a(Symbol(1, {}));
  Symbol sym_b(Symbol(2, {}));

  TypeManager ty;
  EXPECT_NE(ty.Pointer(ty.I32(), ast::StorageClass::kNone),
            ty.Pointer(ty.U32(), ast::StorageClass::kNone));
  EXPECT_NE(ty.Pointer(ty.I32(), ast::StorageClass::kNone),
            ty.Pointer(ty.I32(), ast::StorageClass::kInput));
  EXPECT_NE(ty.Vector(ty.I32(), 3), ty.Vector(ty.U32(), 3));
  EXPECT_NE(ty.Vector(ty.I32(), 3), ty.Vector(ty.I32(), 2));
  EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.U32(), 3, 2));
  EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 2, 2));
  EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 3, 3));
  EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.U32(), 3, 2));
  EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 2, 2));
  EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 3, 3));
  EXPECT_NE(ty.AccessControl(ty.I32(), ast::AccessControl::kReadOnly),
            ty.AccessControl(ty.U32(), ast::AccessControl::kReadOnly));
  EXPECT_NE(ty.AccessControl(ty.I32(), ast::AccessControl::kReadOnly),
            ty.AccessControl(ty.I32(), ast::AccessControl::kWriteOnly));
  EXPECT_NE(ty.Alias(sym_a, ty.I32()), ty.Alias(sym_b, ty.I32()));
  EXPECT_NE(ty.Struct(sym_a, {ty.I32()}), ty.Struct(sym_b, {ty.I32()}));
  EXPECT_NE(ty.Sampler(ast::SamplerKind::kSampler),
            ty.Sampler(ast::SamplerKind::kComparisonSampler));
  EXPECT_NE(ty.DepthTexture(ast::TextureDimension::k2d),
            ty.DepthTexture(ast::TextureDimension::k1d));
  EXPECT_NE(ty.MultisampledTexture(ast::TextureDimension::k2d, ty.I32()),
            ty.MultisampledTexture(ast::TextureDimension::k3d, ty.I32()));
  EXPECT_NE(ty.MultisampledTexture(ast::TextureDimension::k2d, ty.I32()),
            ty.MultisampledTexture(ast::TextureDimension::k2d, ty.U32()));
  EXPECT_NE(ty.SampledTexture(ast::TextureDimension::k2d, ty.I32()),
            ty.SampledTexture(ast::TextureDimension::k3d, ty.I32()));
  EXPECT_NE(ty.SampledTexture(ast::TextureDimension::k2d, ty.I32()),
            ty.SampledTexture(ast::TextureDimension::k2d, ty.U32()));
  EXPECT_NE(
      ty.StorageTexture(ast::TextureDimension::k2d, ast::ImageFormat::kR16Sint),
      ty.StorageTexture(ast::TextureDimension::k3d,
                        ast::ImageFormat::kR16Sint));
  EXPECT_NE(
      ty.StorageTexture(ast::TextureDimension::k2d, ast::ImageFormat::kR16Sint),
      ty.StorageTexture(ast::TextureDimension::k2d,
                        ast::ImageFormat::kR32Sint));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
