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

#include "src/tint/lang/core/builtin/builtin_value.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitType_Alias) {
    auto* alias = Alias("alias", ty.f32());
    auto type = Alias("make_type_reachable", ty.Of(alias))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "alias");
}

TEST_F(WgslASTPrinterTest, EmitType_Array) {
    auto type = Alias("make_type_reachable", ty.array<bool, 4u>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "array<bool, 4u>");
}

TEST_F(WgslASTPrinterTest, EmitType_Array_Attribute) {
    auto type = Alias("make_type_reachable", ty.array(ty.bool_(), 4_u, Vector{Stride(16)}))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "@stride(16) array<bool, 4u>");
}

TEST_F(WgslASTPrinterTest, EmitType_RuntimeArray) {
    auto type = Alias("make_type_reachable", ty.array(ty.bool_()))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "array<bool>");
}

TEST_F(WgslASTPrinterTest, EmitType_Bool) {
    auto type = Alias("make_type_reachable", ty.bool_())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(WgslASTPrinterTest, EmitType_F32) {
    auto type = Alias("make_type_reachable", ty.f32())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f32");
}

TEST_F(WgslASTPrinterTest, EmitType_F16) {
    Enable(builtin::Extension::kF16);

    auto type = Alias("make_type_reachable", ty.f16())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16");
}

TEST_F(WgslASTPrinterTest, EmitType_I32) {
    auto type = Alias("make_type_reachable", ty.i32())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "i32");
}

TEST_F(WgslASTPrinterTest, EmitType_Matrix_F32) {
    auto type = Alias("make_type_reachable", ty.mat2x3<f32>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mat2x3<f32>");
}

TEST_F(WgslASTPrinterTest, EmitType_Matrix_F16) {
    Enable(builtin::Extension::kF16);

    auto type = Alias("make_type_reachable", ty.mat2x3<f16>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mat2x3<f16>");
}

TEST_F(WgslASTPrinterTest, EmitType_Pointer) {
    auto type = Alias("make_type_reachable", ty.ptr<workgroup, f32>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "ptr<workgroup, f32>");
}

TEST_F(WgslASTPrinterTest, EmitType_PointerAccessMode) {
    auto type = Alias("make_type_reachable", ty.ptr<storage, f32, read_write>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "ptr<storage, f32, read_write>");
}

TEST_F(WgslASTPrinterTest, EmitType_Struct) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    auto type = Alias("make_reachable", ty.Of(s))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "S");
}

TEST_F(WgslASTPrinterTest, EmitType_StructOffsetDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberOffset(8_a)}),
                                 Member("b", ty.f32(), Vector{MemberOffset(16_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  a : i32,
  @size(4)
  padding_1 : u32,
  /* @offset(16) */
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_StructOffsetDecl_WithSymbolCollisions) {
    auto* s = Structure("S", Vector{
                                 Member("tint_0_padding", ty.i32(), Vector{MemberOffset(8_a)}),
                                 Member("tint_2_padding", ty.f32(), Vector{MemberOffset(16_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  tint_0_padding : i32,
  @size(4)
  padding_1 : u32,
  /* @offset(16) */
  tint_2_padding : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_StructAlignDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberAlign(8_a)}),
                                 Member("b", ty.f32(), Vector{MemberAlign(16_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @align(8)
  a : i32,
  @align(16)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_StructSizeDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberSize(16_a)}),
                                 Member("b", ty.f32(), Vector{MemberSize(32_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @size(16)
  a : i32,
  @size(32)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_Struct_WithAttribute) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32(), Vector{MemberAlign(8_a)}),
                             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  a : i32,
  @align(8)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_Struct_WithEntryPointAttributes) {
    auto* s = Structure(
        "S", Vector{
                 Member("a", ty.u32(), Vector{Builtin(builtin::BuiltinValue::kVertexIndex)}),
                 Member("b", ty.f32(), Vector{Location(2_a)}),
             });

    ASTPrinter& gen = Build();

    gen.EmitStructType(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct S {
  @builtin(vertex_index)
  a : u32,
  @location(2)
  b : f32,
}
)");
}

TEST_F(WgslASTPrinterTest, EmitType_U32) {
    auto type = Alias("make_type_reachable", ty.u32())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "u32");
}

TEST_F(WgslASTPrinterTest, EmitType_Vector_F32) {
    auto type = Alias("make_type_reachable", ty.vec3<f32>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f32>");
}

TEST_F(WgslASTPrinterTest, EmitType_Vector_F16) {
    Enable(builtin::Extension::kF16);

    auto type = Alias("make_type_reachable", ty.vec3<f16>())->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f16>");
}

struct TextureData {
    type::TextureDimension dim;
    const char* name;
};
inline std::ostream& operator<<(std::ostream& out, TextureData data) {
    out << data.name;
    return out;
}
using WgslGenerator_DepthTextureTest = TestParamHelper<TextureData>;

TEST_P(WgslGenerator_DepthTextureTest, EmitType_DepthTexture) {
    auto param = GetParam();

    auto type = Alias("make_type_reachable", ty.depth_texture(param.dim))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_DepthTextureTest,
    testing::Values(TextureData{type::TextureDimension::k2d, "texture_depth_2d"},
                    TextureData{type::TextureDimension::k2dArray, "texture_depth_2d_array"},
                    TextureData{type::TextureDimension::kCube, "texture_depth_cube"},
                    TextureData{type::TextureDimension::kCubeArray, "texture_depth_cube_array"}));

using WgslGenerator_SampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_F32) {
    auto param = GetParam();

    auto t = ty.sampled_texture(param.dim, ty.f32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_I32) {
    auto param = GetParam();

    auto t = ty.sampled_texture(param.dim, ty.i32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_U32) {
    auto param = GetParam();

    auto t = ty.sampled_texture(param.dim, ty.u32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_SampledTextureTest,
    testing::Values(TextureData{type::TextureDimension::k1d, "texture_1d"},
                    TextureData{type::TextureDimension::k2d, "texture_2d"},
                    TextureData{type::TextureDimension::k2dArray, "texture_2d_array"},
                    TextureData{type::TextureDimension::k3d, "texture_3d"},
                    TextureData{type::TextureDimension::kCube, "texture_cube"},
                    TextureData{type::TextureDimension::kCubeArray, "texture_cube_array"}));

using WgslGenerator_MultiampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_F32) {
    auto param = GetParam();

    auto t = ty.multisampled_texture(param.dim, ty.f32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_I32) {
    auto param = GetParam();

    auto t = ty.multisampled_texture(param.dim, ty.i32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_U32) {
    auto param = GetParam();

    auto t = ty.multisampled_texture(param.dim, ty.u32());
    auto type = Alias("make_type_reachable", t)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(WgslASTPrinterTest,
                         WgslGenerator_MultiampledTextureTest,
                         testing::Values(TextureData{type::TextureDimension::k2d,
                                                     "texture_multisampled_2d"}));

struct StorageTextureData {
    builtin::TexelFormat fmt;
    type::TextureDimension dim;
    builtin::Access access;
    const char* name;
};
inline std::ostream& operator<<(std::ostream& out, StorageTextureData data) {
    out << data.name;
    return out;
}
using WgslGenerator_StorageTextureTest = TestParamHelper<StorageTextureData>;
TEST_P(WgslGenerator_StorageTextureTest, EmitType_StorageTexture) {
    auto param = GetParam();

    auto s = ty.storage_texture(param.dim, param.fmt, param.access);
    auto type = GlobalVar("g", s, Binding(1_a), Group(2_a))->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_StorageTextureTest,
    testing::Values(
        StorageTextureData{builtin::TexelFormat::kRgba8Sint, type::TextureDimension::k1d,
                           builtin::Access::kWrite, "texture_storage_1d<rgba8sint, write>"},
        StorageTextureData{builtin::TexelFormat::kRgba8Sint, type::TextureDimension::k2d,
                           builtin::Access::kWrite, "texture_storage_2d<rgba8sint, write>"},
        StorageTextureData{builtin::TexelFormat::kRgba8Sint, type::TextureDimension::k2dArray,
                           builtin::Access::kWrite, "texture_storage_2d_array<rgba8sint, write>"},
        StorageTextureData{builtin::TexelFormat::kRgba8Sint, type::TextureDimension::k3d,
                           builtin::Access::kWrite, "texture_storage_3d<rgba8sint, write>"}));

struct ImageFormatData {
    builtin::TexelFormat fmt;
    const char* name;
};
inline std::ostream& operator<<(std::ostream& out, ImageFormatData data) {
    out << data.name;
    return out;
}
using WgslGenerator_ImageFormatTest = TestParamHelper<ImageFormatData>;
TEST_P(WgslGenerator_ImageFormatTest, EmitType_StorageTexture_ImageFormat) {
    auto param = GetParam();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitImageFormat(out, param.fmt);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), param.name);
}

INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslGenerator_ImageFormatTest,
    testing::Values(ImageFormatData{builtin::TexelFormat::kR32Uint, "r32uint"},
                    ImageFormatData{builtin::TexelFormat::kR32Sint, "r32sint"},
                    ImageFormatData{builtin::TexelFormat::kR32Float, "r32float"},
                    ImageFormatData{builtin::TexelFormat::kRgba8Unorm, "rgba8unorm"},
                    ImageFormatData{builtin::TexelFormat::kRgba8Snorm, "rgba8snorm"},
                    ImageFormatData{builtin::TexelFormat::kRgba8Uint, "rgba8uint"},
                    ImageFormatData{builtin::TexelFormat::kRgba8Sint, "rgba8sint"},
                    ImageFormatData{builtin::TexelFormat::kRg32Uint, "rg32uint"},
                    ImageFormatData{builtin::TexelFormat::kRg32Sint, "rg32sint"},
                    ImageFormatData{builtin::TexelFormat::kRg32Float, "rg32float"},
                    ImageFormatData{builtin::TexelFormat::kRgba16Uint, "rgba16uint"},
                    ImageFormatData{builtin::TexelFormat::kRgba16Sint, "rgba16sint"},
                    ImageFormatData{builtin::TexelFormat::kRgba16Float, "rgba16float"},
                    ImageFormatData{builtin::TexelFormat::kRgba32Uint, "rgba32uint"},
                    ImageFormatData{builtin::TexelFormat::kRgba32Sint, "rgba32sint"},
                    ImageFormatData{builtin::TexelFormat::kRgba32Float, "rgba32float"}));

TEST_F(WgslASTPrinterTest, EmitType_Sampler) {
    auto sampler = ty.sampler(type::SamplerKind::kSampler);
    auto type = Alias("make_type_reachable", sampler)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "sampler");
}

TEST_F(WgslASTPrinterTest, EmitType_SamplerComparison) {
    auto sampler = ty.sampler(type::SamplerKind::kComparisonSampler);
    auto type = Alias("make_type_reachable", sampler)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, type);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "sampler_comparison");
}

}  // namespace
}  // namespace tint::wgsl::writer
