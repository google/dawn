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

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/sampler.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Type = TestHelper;

TEST_F(GlslGeneratorImplTest_Type, EmitType_Array) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                 builtin::Access::kReadWrite, "ary");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool ary[4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_ArrayOfArray) {
    auto arr = ty.array(ty.array<bool, 4>(), 5_u);
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                 builtin::Access::kReadWrite, "ary");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool ary[5][4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_ArrayOfArrayOfArray) {
    auto arr = ty.array(ty.array(ty.array<bool, 4>(), 5_u), 6_u);
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                 builtin::Access::kReadWrite, "ary");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool ary[6][5][4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Array_WithoutName) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                 builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool[4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Bool) {
    auto* bool_ = create<type::Bool>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, bool_, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_F32) {
    auto* f32 = create<type::F32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, f32, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "float");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_F16) {
    Enable(builtin::Extension::kF16);

    auto* f16 = create<type::F16>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, f16, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "float16_t");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_I32) {
    auto* i32 = create<type::I32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, i32, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "int");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Matrix_F32) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, mat2x3, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mat2x3");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Matrix_F16) {
    Enable(builtin::Extension::kF16);

    auto* f16 = create<type::F16>();
    auto* vec3 = create<type::Vector>(f16, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, mat2x3, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16mat2x3");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_StructDecl) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<type::Struct>();
    gen.EmitStructType(&buf, str);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};

)");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Struct) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    auto* str = program->TypeOf(s)->As<type::Struct>();
    utils::StringStream out;
    gen.EmitType(out, str, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "S");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Struct_NameCollision) {
    auto* s = Structure("S", utils::Vector{
                                 Member("double", ty.i32()),
                                 Member("float", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr(R"(struct S {
  int tint_symbol;
  float tint_symbol_1;
};
)"));
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Struct_WithOffsetAttributes) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32(), utils::Vector{MemberOffset(0_a)}),
                                 Member("b", ty.f32(), utils::Vector{MemberOffset(8_a)}),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<type::Struct>();
    gen.EmitStructType(&buf, str);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};

)");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_U32) {
    auto* u32 = create<type::U32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, u32, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "uint");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Vector_F32) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, vec3, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Vector_F16) {
    Enable(builtin::Extension::kF16);

    auto* f16 = create<type::F16>();
    auto* vec3 = create<type::Vector>(f16, 3u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, vec3, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16vec3");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Void) {
    auto* void_ = create<type::Void>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, void_, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "void");
}

TEST_F(GlslGeneratorImplTest_Type, EmitSampler) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, sampler, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
}

TEST_F(GlslGeneratorImplTest_Type, EmitSamplerComparison) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kComparisonSampler);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, sampler, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
}

struct GlslDepthTextureData {
    type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslDepthTextureData data) {
    utils::StringStream s;
    s << data.dim;
    out << s.str();
    return out;
}
using GlslDepthTexturesTest = TestParamHelper<GlslDepthTextureData>;
TEST_P(GlslDepthTexturesTest, Emit) {
    auto params = GetParam();

    auto t = ty.depth_texture(params.dim);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest_Type,
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

    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("sampler2DMS tex;"));
}

enum class TextureDataType { F32, U32, I32 };
struct GlslSampledTextureData {
    type::TextureDimension dim;
    TextureDataType datatype;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslSampledTextureData data) {
    utils::StringStream str;
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

    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(GlslGeneratorImplTest_Type,
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

TEST_F(GlslGeneratorImplTest_Type, EmitMultisampledTexture) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::MultisampledTexture>(type::TextureDimension::k2d, f32);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitType(out, s, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, "");
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "highp sampler2DMS");
}

struct GlslStorageTextureData {
    type::TextureDimension dim;
    builtin::TexelFormat imgfmt;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslStorageTextureData data) {
    utils::StringStream str;
    str << data.dim;
    return out << str.str();
}
using GlslStorageTexturesTest = TestParamHelper<GlslStorageTextureData>;
TEST_P(GlslStorageTexturesTest, Emit) {
    auto params = GetParam();

    auto t = ty.storage_texture(params.dim, params.imgfmt, builtin::Access::kWrite);

    GlobalVar("tex", t, Binding(1_a), Group(2_a));

    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest_Type,
    GlslStorageTexturesTest,
    testing::Values(GlslStorageTextureData{type::TextureDimension::k1d,
                                           builtin::TexelFormat::kRgba8Unorm, "image1D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2d,
                                           builtin::TexelFormat::kRgba16Float, "image2D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2dArray,
                                           builtin::TexelFormat::kR32Float, "image2DArray tex;"},
                    GlslStorageTextureData{type::TextureDimension::k3d,
                                           builtin::TexelFormat::kRg32Float, "image3D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k1d,
                                           builtin::TexelFormat::kRgba32Float, "image1D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2d,
                                           builtin::TexelFormat::kRgba16Uint, "image2D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2dArray,
                                           builtin::TexelFormat::kR32Uint, "image2DArray tex;"},
                    GlslStorageTextureData{type::TextureDimension::k3d,
                                           builtin::TexelFormat::kRg32Uint, "image3D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k1d,
                                           builtin::TexelFormat::kRgba32Uint, "image1D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2d,
                                           builtin::TexelFormat::kRgba16Sint, "image2D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k2dArray,
                                           builtin::TexelFormat::kR32Sint, "image2DArray tex;"},
                    GlslStorageTextureData{type::TextureDimension::k3d,
                                           builtin::TexelFormat::kRg32Sint, "image3D tex;"},
                    GlslStorageTextureData{type::TextureDimension::k1d,
                                           builtin::TexelFormat::kRgba32Sint, "image1D tex;"}));

}  // namespace
}  // namespace tint::writer::glsl
