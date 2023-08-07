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

#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

using GlslASTPrinterTest_Type = TestHelper;

TEST_F(GlslASTPrinterTest_Type, EmitType_Array) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined, core::Access::kReadWrite,
                 "ary");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool ary[4]");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_ArrayOfArray) {
    auto arr = ty.array(ty.array<bool, 4>(), 5_u);
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined, core::Access::kReadWrite,
                 "ary");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool ary[5][4]");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_ArrayOfArrayOfArray) {
    auto arr = ty.array(ty.array(ty.array<bool, 4>(), 5_u), 6_u);
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined, core::Access::kReadWrite,
                 "ary");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool ary[6][5][4]");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Array_WithoutName) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, core::AddressSpace::kPrivate)->type;

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, program->TypeOf(ty), core::AddressSpace::kUndefined, core::Access::kReadWrite,
                 "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool[4]");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Bool) {
    auto* bool_ = create<type::Bool>();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, bool_, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_F32) {
    auto* f32 = create<type::F32>();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, f32, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "float");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_F16) {
    Enable(core::Extension::kF16);

    auto* f16 = create<type::F16>();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, f16, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "float16_t");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_I32) {
    auto* i32 = create<type::I32>();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, i32, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "int");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Matrix_F32) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, mat2x3, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mat2x3");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Matrix_F16) {
    Enable(core::Extension::kF16);

    auto* f16 = create<type::F16>();
    auto* vec3 = create<type::Vector>(f16, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, mat2x3, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16mat2x3");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_StructDecl) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    tint::TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<type::Struct>();
    gen.EmitStructType(&buf, str);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};

)");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Struct) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    auto* str = program->TypeOf(s)->As<type::Struct>();
    StringStream out;
    gen.EmitType(out, str, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "S");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Struct_NameCollision) {
    auto* s = Structure("S", Vector{
                                 Member("double", ty.i32()),
                                 Member("float", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(R"(struct S {
  int tint_symbol;
  float tint_symbol_1;
};
)"));
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Struct_WithOffsetAttributes) {
    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32(), Vector{MemberOffset(0_a)}),
                                 Member("b", ty.f32(), Vector{MemberOffset(8_a)}),
                             });
    GlobalVar("g", ty.Of(s), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    tint::TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<type::Struct>();
    gen.EmitStructType(&buf, str);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};

)");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_U32) {
    auto* u32 = create<type::U32>();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, u32, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "uint");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Vector_F32) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, vec3, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Vector_F16) {
    Enable(core::Extension::kF16);

    auto* f16 = create<type::F16>();
    auto* vec3 = create<type::Vector>(f16, 3u);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, vec3, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16vec3");
}

TEST_F(GlslASTPrinterTest_Type, EmitType_Void) {
    auto* void_ = create<type::Void>();

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, void_, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "void");
}

TEST_F(GlslASTPrinterTest_Type, EmitSampler) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, sampler, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
}

TEST_F(GlslASTPrinterTest_Type, EmitSamplerComparison) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kComparisonSampler);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, sampler, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
}

struct GlslDepthTextureData {
    type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslDepthTextureData data) {
    StringStream s;
    s << data.dim;
    out << s.str();
    return out;
}
using GlslDepthTexturesTest = TestParamHelper<GlslDepthTextureData>;
TEST_P(GlslDepthTexturesTest, Emit) {
    auto params = GetParam();

    auto t = ty.depth_texture(params.dim);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    GlslASTPrinterTest_Type,
    GlslDepthTexturesTest,
    testing::Values(
        GlslDepthTextureData{type::TextureDimension::k2d, "sampler2DShadow tex;"},
        GlslDepthTextureData{type::TextureDimension::k2dArray, "sampler2DArrayShadow tex;"},
        GlslDepthTextureData{type::TextureDimension::kCube, "samplerCubeShadow tex;"},
        GlslDepthTextureData{type::TextureDimension::kCubeArray, "samplerCubeArrayShadow tex;"}));

using GlslDepthMultisampledTexturesTest = TestHelper;
TEST_F(GlslDepthMultisampledTexturesTest, Emit) {
    auto t = ty.depth_multisampled_texture(type::TextureDimension::k2d);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("sampler2DMS tex;"));
}

enum class TextureDataType { F32, U32, I32 };
struct GlslSampledTextureData {
    type::TextureDimension dim;
    TextureDataType datatype;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslSampledTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using GlslSampledTexturesTest = TestParamHelper<GlslSampledTextureData>;
TEST_P(GlslSampledTexturesTest, Emit) {
    auto params = GetParam();

    ast::Type datatype;
    switch (params.datatype) {
        case TextureDataType::F32:
            datatype = ty.f32();
            break;
        case TextureDataType::U32:
            datatype = ty.u32();
            break;
        case TextureDataType::I32:
            datatype = ty.i32();
            break;
    }
    ast::Type t = ty.sampled_texture(params.dim, datatype);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Type,
                         GlslSampledTexturesTest,
                         testing::Values(
                             GlslSampledTextureData{
                                 type::TextureDimension::k1d,
                                 TextureDataType::F32,
                                 "sampler1D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k2d,
                                 TextureDataType::F32,
                                 "sampler2D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k2dArray,
                                 TextureDataType::F32,
                                 "sampler2DArray tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k3d,
                                 TextureDataType::F32,
                                 "sampler3D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::kCube,
                                 TextureDataType::F32,
                                 "samplerCube tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::kCubeArray,
                                 TextureDataType::F32,
                                 "samplerCubeArray tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k1d,
                                 TextureDataType::U32,
                                 "usampler1D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k2d,
                                 TextureDataType::U32,
                                 "usampler2D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k2dArray,
                                 TextureDataType::U32,
                                 "usampler2DArray tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k3d,
                                 TextureDataType::U32,
                                 "usampler3D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::kCube,
                                 TextureDataType::U32,
                                 "usamplerCube tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::kCubeArray,
                                 TextureDataType::U32,
                                 "usamplerCubeArray tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k1d,
                                 TextureDataType::I32,
                                 "isampler1D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k2d,
                                 TextureDataType::I32,
                                 "isampler2D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k2dArray,
                                 TextureDataType::I32,
                                 "isampler2DArray tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::k3d,
                                 TextureDataType::I32,
                                 "isampler3D tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::kCube,
                                 TextureDataType::I32,
                                 "isamplerCube tex;",
                             },
                             GlslSampledTextureData{
                                 type::TextureDimension::kCubeArray,
                                 TextureDataType::I32,
                                 "isamplerCubeArray tex;",
                             }));

TEST_F(GlslASTPrinterTest_Type, EmitMultisampledTexture) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::MultisampledTexture>(type::TextureDimension::k2d, f32);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitType(out, s, core::AddressSpace::kUndefined, core::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "highp sampler2DMS");
}

struct GlslStorageTextureData {
    type::TextureDimension dim;
    core::TexelFormat imgfmt;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslStorageTextureData data) {
    StringStream str;
    str << data.dim;
    return out << str.str();
}
using GlslStorageTexturesTest = TestParamHelper<GlslStorageTextureData>;
TEST_P(GlslStorageTexturesTest, Emit) {
    auto params = GetParam();

    auto t = ty.storage_texture(params.dim, params.imgfmt, core::Access::kWrite);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    GlslASTPrinterTest_Type,
    GlslStorageTexturesTest,
    testing::Values(GlslStorageTextureData{type::TextureDimension::k1d,
                                           core::TexelFormat::kRgba8Unorm, "image1D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2d,
                                           core::TexelFormat::kRgba16Float, "image2D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2dArray,
                                           core::TexelFormat::kR32Float, "image2DArray tex;"},
                    GlslStorageTextureData{type::TextureDimension::k3d,
                                           core::TexelFormat::kRg32Float, "image3D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k1d,
                                           core::TexelFormat::kRgba32Float, "image1D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2d,
                                           core::TexelFormat::kRgba16Uint, "image2D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2dArray,
                                           core::TexelFormat::kR32Uint, "image2DArray tex;"},
                    GlslStorageTextureData{type::TextureDimension::k3d,
                                           core::TexelFormat::kRg32Uint, "image3D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k1d,
                                           core::TexelFormat::kRgba32Uint, "image1D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2d,
                                           core::TexelFormat::kRgba16Sint, "image2D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2dArray,
                                           core::TexelFormat::kR32Sint, "image2DArray tex;"},
                    GlslStorageTextureData{type::TextureDimension::k3d,
                                           core::TexelFormat::kRg32Sint, "image3D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k1d,
                                           core::TexelFormat::kRgba32Sint, "image1D tex;"}));

}  // namespace
}  // namespace tint::glsl::writer
