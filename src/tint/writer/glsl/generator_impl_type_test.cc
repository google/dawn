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

#include "gmock/gmock.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/sampler.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/writer/glsl/test_helper.h"

namespace tint::writer::glsl {
namespace {

using ::testing::HasSubstr;

using GlslGeneratorImplTest_Type = TestHelper;

TEST_F(GlslGeneratorImplTest_Type, EmitType_Array) {
    auto* arr = ty.array<bool, 4>();
    Global("G", arr, ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                             ast::Access::kReadWrite, "ary"))
        << gen.error();
    EXPECT_EQ(out.str(), "bool ary[4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_ArrayOfArray) {
    auto* arr = ty.array(ty.array<bool, 4>(), 5);
    Global("G", arr, ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                             ast::Access::kReadWrite, "ary"))
        << gen.error();
    EXPECT_EQ(out.str(), "bool ary[5][4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_ArrayOfArrayOfArray) {
    auto* arr = ty.array(ty.array(ty.array<bool, 4>(), 5), 6);
    Global("G", arr, ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                             ast::Access::kReadWrite, "ary"))
        << gen.error();
    EXPECT_EQ(out.str(), "bool ary[6][5][4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Array_WithoutName) {
    auto* arr = ty.array<bool, 4>();
    Global("G", arr, ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                             ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "bool[4]");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Bool) {
    auto* bool_ = create<sem::Bool>();

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, bool_, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_F32) {
    auto* f32 = create<sem::F32>();

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, f32, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "float");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_I32) {
    auto* i32 = create<sem::I32>();

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, i32, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "int");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Matrix) {
    auto* f32 = create<sem::F32>();
    auto* vec3 = create<sem::Vector>(f32, 3u);
    auto* mat2x3 = create<sem::Matrix>(vec3, 2u);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, mat2x3, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "mat2x3");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_StructDecl) {
    auto* s = Structure("S", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    Global("g", ty.Of(s), ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, sem_s)) << gen.error();
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};

)");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Struct) {
    auto* s = Structure("S", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    Global("g", ty.Of(s), ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, sem_s, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "S");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Struct_NameCollision) {
    auto* s = Structure("S", {
                                 Member("double", ty.i32()),
                                 Member("float", ty.f32()),
                             });
    Global("g", ty.Of(s), ast::StorageClass::kPrivate);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(R"(struct S {
  int tint_symbol;
  float tint_symbol_1;
};
)"));
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Struct_WithOffsetAttributes) {
    auto* s = Structure("S", {
                                 Member("a", ty.i32(), {MemberOffset(0)}),
                                 Member("b", ty.f32(), {MemberOffset(8)}),
                             });
    Global("g", ty.Of(s), ast::StorageClass::kPrivate);

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, sem_s)) << gen.error();
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};

)");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_U32) {
    auto* u32 = create<sem::U32>();

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, u32, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "uint");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Vector) {
    auto* f32 = create<sem::F32>();
    auto* vec3 = create<sem::Vector>(f32, 3u);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, vec3, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "vec3");
}

TEST_F(GlslGeneratorImplTest_Type, EmitType_Void) {
    auto* void_ = create<sem::Void>();

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, void_, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "void");
}

TEST_F(GlslGeneratorImplTest_Type, EmitSampler) {
    auto* sampler = create<sem::Sampler>(ast::SamplerKind::kSampler);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_FALSE(gen.EmitType(out, sampler, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
}

TEST_F(GlslGeneratorImplTest_Type, EmitSamplerComparison) {
    auto* sampler = create<sem::Sampler>(ast::SamplerKind::kComparisonSampler);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_FALSE(gen.EmitType(out, sampler, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
}

struct GlslDepthTextureData {
    ast::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslDepthTextureData data) {
    out << data.dim;
    return out;
}
using GlslDepthTexturesTest = TestParamHelper<GlslDepthTextureData>;
TEST_P(GlslDepthTexturesTest, Emit) {
    auto params = GetParam();

    auto* t = ty.depth_texture(params.dim);

    Global("tex", t,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("main", {}, ty.void_(), {CallStmt(Call("textureDimensions", "tex"))},
         {Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest_Type,
    GlslDepthTexturesTest,
    testing::Values(
        GlslDepthTextureData{ast::TextureDimension::k2d, "sampler2DShadow tex;"},
        GlslDepthTextureData{ast::TextureDimension::k2dArray, "sampler2DArrayShadow tex;"},
        GlslDepthTextureData{ast::TextureDimension::kCube, "samplerCubeShadow tex;"},
        GlslDepthTextureData{ast::TextureDimension::kCubeArray, "samplerCubeArrayShadow tex;"}));

using GlslDepthMultisampledTexturesTest = TestHelper;
TEST_F(GlslDepthMultisampledTexturesTest, Emit) {
    auto* t = ty.depth_multisampled_texture(ast::TextureDimension::k2d);

    Global("tex", t,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("main", {}, ty.void_(), {CallStmt(Call("textureDimensions", "tex"))},
         {Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("sampler2DMS tex;"));
}

enum class TextureDataType { F32, U32, I32 };
struct GlslSampledTextureData {
    ast::TextureDimension dim;
    TextureDataType datatype;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslSampledTextureData data) {
    out << data.dim;
    return out;
}
using GlslSampledTexturesTest = TestParamHelper<GlslSampledTextureData>;
TEST_P(GlslSampledTexturesTest, Emit) {
    auto params = GetParam();

    const ast::Type* datatype = nullptr;
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
    auto* t = ty.sampled_texture(params.dim, datatype);

    Global("tex", t,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("main", {}, ty.void_(), {CallStmt(Call("textureDimensions", "tex"))},
         {Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(GlslGeneratorImplTest_Type,
                         GlslSampledTexturesTest,
                         testing::Values(
                             GlslSampledTextureData{
                                 ast::TextureDimension::k1d,
                                 TextureDataType::F32,
                                 "sampler1D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k2d,
                                 TextureDataType::F32,
                                 "sampler2D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k2dArray,
                                 TextureDataType::F32,
                                 "sampler2DArray tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k3d,
                                 TextureDataType::F32,
                                 "sampler3D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::kCube,
                                 TextureDataType::F32,
                                 "samplerCube tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::kCubeArray,
                                 TextureDataType::F32,
                                 "samplerCubeArray tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k1d,
                                 TextureDataType::U32,
                                 "usampler1D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k2d,
                                 TextureDataType::U32,
                                 "usampler2D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k2dArray,
                                 TextureDataType::U32,
                                 "usampler2DArray tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k3d,
                                 TextureDataType::U32,
                                 "usampler3D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::kCube,
                                 TextureDataType::U32,
                                 "usamplerCube tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::kCubeArray,
                                 TextureDataType::U32,
                                 "usamplerCubeArray tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k1d,
                                 TextureDataType::I32,
                                 "isampler1D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k2d,
                                 TextureDataType::I32,
                                 "isampler2D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k2dArray,
                                 TextureDataType::I32,
                                 "isampler2DArray tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::k3d,
                                 TextureDataType::I32,
                                 "isampler3D tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::kCube,
                                 TextureDataType::I32,
                                 "isamplerCube tex;",
                             },
                             GlslSampledTextureData{
                                 ast::TextureDimension::kCubeArray,
                                 TextureDataType::I32,
                                 "isamplerCubeArray tex;",
                             }));

TEST_F(GlslGeneratorImplTest_Type, EmitMultisampledTexture) {
    auto* f32 = create<sem::F32>();
    auto* s = create<sem::MultisampledTexture>(ast::TextureDimension::k2d, f32);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitType(out, s, ast::StorageClass::kNone, ast::Access::kReadWrite, ""))
        << gen.error();
    EXPECT_EQ(out.str(), "highp sampler2DMS");
}

struct GlslStorageTextureData {
    ast::TextureDimension dim;
    ast::TexelFormat imgfmt;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslStorageTextureData data) {
    return out << data.dim;
}
using GlslStorageTexturesTest = TestParamHelper<GlslStorageTextureData>;
TEST_P(GlslStorageTexturesTest, Emit) {
    auto params = GetParam();

    auto* t = ty.storage_texture(params.dim, params.imgfmt, ast::Access::kWrite);

    Global("tex", t,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("main", {}, ty.void_(), {CallStmt(Call("textureDimensions", "tex"))},
         {Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest_Type,
    GlslStorageTexturesTest,
    testing::Values(GlslStorageTextureData{ast::TextureDimension::k1d,
                                           ast::TexelFormat::kRgba8Unorm, "image1D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k2d,
                                           ast::TexelFormat::kRgba16Float, "image2D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k2dArray,
                                           ast::TexelFormat::kR32Float, "image2DArray tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k3d, ast::TexelFormat::kRg32Float,
                                           "image3D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k1d,
                                           ast::TexelFormat::kRgba32Float, "image1D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k2d,
                                           ast::TexelFormat::kRgba16Uint, "image2D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k2dArray,
                                           ast::TexelFormat::kR32Uint, "image2DArray tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k3d, ast::TexelFormat::kRg32Uint,
                                           "image3D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k1d,
                                           ast::TexelFormat::kRgba32Uint, "image1D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k2d,
                                           ast::TexelFormat::kRgba16Sint, "image2D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k2dArray,
                                           ast::TexelFormat::kR32Sint, "image2DArray tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k3d, ast::TexelFormat::kRg32Sint,
                                           "image3D tex;"},
                    GlslStorageTextureData{ast::TextureDimension::k1d,
                                           ast::TexelFormat::kRgba32Sint, "image1D tex;"}));

}  // namespace
}  // namespace tint::writer::glsl
