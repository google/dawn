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

#include "gmock/gmock.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/sampler.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/hlsl/test_helper.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Type = TestHelper;

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, "ary"))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool ary[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArray) {
    auto arr = ty.array(ty.array<bool, 4>(), 5_u);
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, "ary"))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool ary[5][4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArrayOfArray) {
    auto arr = ty.array(ty.array(ty.array<bool, 4>(), 5_u), 6_u);
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, "ary"))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool ary[6][5][4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array_WithoutName) {
    auto arr = ty.array<bool, 4>();
    ast::Type ty = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(ty), builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Bool) {
    auto* bool_ = create<type::Bool>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, bool_, builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_F16) {
    auto* f16 = create<type::F16>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, f16, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float16_t");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_F32) {
    auto* f32 = create<type::F32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, f32, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_I32) {
    auto* i32 = create<type::I32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, i32, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "int");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Matrix_F16) {
    auto* f16 = create<type::F16>();
    auto* vec3 = create<type::Vector>(f16, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, mat2x3, builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "matrix<float16_t, 2, 3>");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Matrix_F32) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, mat2x3, builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float2x3");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_StructDecl) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_StructDecl_OmittedIfStorageBuffer) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(0_a), Group(0_a));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), "RWByteAddressBuffer g : register(u0);\n");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    auto* str = program->TypeOf(s)->As<type::Struct>();
    utils::StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, str, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "S");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_NameCollision) {
    auto* s = Structure("S", utils::Vector{
                                 Member("double", ty.i32()),
                                 Member("float", ty.f32()),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(R"(struct S {
  int tint_symbol;
  float tint_symbol_1;
};
)"));
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_WithOffsetAttributes) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32(), utils::Vector{MemberOffset(0_a)}),
                                 Member("b", ty.f32(), utils::Vector{MemberOffset(8_a)}),
                             });
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(s)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();
    EXPECT_EQ(buf.String(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_U32) {
    auto* u32 = create<type::U32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, u32, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "uint");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Vector) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, vec3, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float3");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Void) {
    auto* void_ = create<type::Void>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, void_, builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "void");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSampler) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, sampler, builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "SamplerState");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSamplerComparison) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kComparisonSampler);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, sampler, builtin::AddressSpace::kUndefined,
                             builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "SamplerComparisonState");
}

struct HlslDepthTextureData {
    type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, HlslDepthTextureData data) {
    utils::StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using HlslDepthTexturesTest = TestParamHelper<HlslDepthTextureData>;
TEST_P(HlslDepthTexturesTest, Emit) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Type,
    HlslDepthTexturesTest,
    testing::Values(HlslDepthTextureData{type::TextureDimension::k2d,
                                         "Texture2D tex : register(t1, space2);"},
                    HlslDepthTextureData{type::TextureDimension::k2dArray,
                                         "Texture2DArray tex : register(t1, space2);"},
                    HlslDepthTextureData{type::TextureDimension::kCube,
                                         "TextureCube tex : register(t1, space2);"},
                    HlslDepthTextureData{type::TextureDimension::kCubeArray,
                                         "TextureCubeArray tex : register(t1, space2);"}));

using HlslDepthMultisampledTexturesTest = TestHelper;
TEST_F(HlslDepthMultisampledTexturesTest, Emit) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("Texture2DMS<float4> tex : register(t1, space2);"));
}

enum class TextureDataType { F32, U32, I32 };
struct HlslSampledTextureData {
    type::TextureDimension dim;
    TextureDataType datatype;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, HlslSampledTextureData data) {
    utils::StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using HlslSampledTexturesTest = TestParamHelper<HlslSampledTextureData>;
TEST_P(HlslSampledTexturesTest, Emit) {
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

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_Type,
                         HlslSampledTexturesTest,
                         testing::Values(
                             HlslSampledTextureData{
                                 type::TextureDimension::k1d,
                                 TextureDataType::F32,
                                 "Texture1D<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k2d,
                                 TextureDataType::F32,
                                 "Texture2D<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k2dArray,
                                 TextureDataType::F32,
                                 "Texture2DArray<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k3d,
                                 TextureDataType::F32,
                                 "Texture3D<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::kCube,
                                 TextureDataType::F32,
                                 "TextureCube<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::kCubeArray,
                                 TextureDataType::F32,
                                 "TextureCubeArray<float4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k1d,
                                 TextureDataType::U32,
                                 "Texture1D<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k2d,
                                 TextureDataType::U32,
                                 "Texture2D<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k2dArray,
                                 TextureDataType::U32,
                                 "Texture2DArray<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k3d,
                                 TextureDataType::U32,
                                 "Texture3D<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::kCube,
                                 TextureDataType::U32,
                                 "TextureCube<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::kCubeArray,
                                 TextureDataType::U32,
                                 "TextureCubeArray<uint4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k1d,
                                 TextureDataType::I32,
                                 "Texture1D<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k2d,
                                 TextureDataType::I32,
                                 "Texture2D<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k2dArray,
                                 TextureDataType::I32,
                                 "Texture2DArray<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::k3d,
                                 TextureDataType::I32,
                                 "Texture3D<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::kCube,
                                 TextureDataType::I32,
                                 "TextureCube<int4> tex : register(t1, space2);",
                             },
                             HlslSampledTextureData{
                                 type::TextureDimension::kCubeArray,
                                 TextureDataType::I32,
                                 "TextureCubeArray<int4> tex : register(t1, space2);",
                             }));

TEST_F(HlslGeneratorImplTest_Type, EmitMultisampledTexture) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::MultisampledTexture>(type::TextureDimension::k2d, f32);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(
        gen.EmitType(out, s, builtin::AddressSpace::kUndefined, builtin::Access::kReadWrite, ""))
        << gen.Diagnostics();
    EXPECT_EQ(out.str(), "Texture2DMS<float4>");
}

struct HlslStorageTextureData {
    type::TextureDimension dim;
    builtin::TexelFormat imgfmt;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, HlslStorageTextureData data) {
    utils::StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using HlslStorageTexturesTest = TestParamHelper<HlslStorageTextureData>;
TEST_P(HlslStorageTexturesTest, Emit) {
    auto params = GetParam();

    auto t = ty.storage_texture(params.dim, params.imgfmt, builtin::Access::kWrite);

    GlobalVar("tex", t,
              utils::Vector{
                  Group(2_a),
                  Binding(1_a),
              });

    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Call("textureDimensions", "tex"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(params.result));
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Type,
    HlslStorageTexturesTest,
    testing::Values(
        HlslStorageTextureData{type::TextureDimension::k1d, builtin::TexelFormat::kRgba8Unorm,
                               "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2d, builtin::TexelFormat::kRgba16Float,
                               "RWTexture2D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2dArray, builtin::TexelFormat::kR32Float,
                               "RWTexture2DArray<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k3d, builtin::TexelFormat::kRg32Float,
                               "RWTexture3D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k1d, builtin::TexelFormat::kRgba32Float,
                               "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2d, builtin::TexelFormat::kRgba16Uint,
                               "RWTexture2D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2dArray, builtin::TexelFormat::kR32Uint,
                               "RWTexture2DArray<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k3d, builtin::TexelFormat::kRg32Uint,
                               "RWTexture3D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k1d, builtin::TexelFormat::kRgba32Uint,
                               "RWTexture1D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2d, builtin::TexelFormat::kRgba16Sint,
                               "RWTexture2D<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2dArray, builtin::TexelFormat::kR32Sint,
                               "RWTexture2DArray<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k3d, builtin::TexelFormat::kRg32Sint,
                               "RWTexture3D<int4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k1d, builtin::TexelFormat::kRgba32Sint,
                               "RWTexture1D<int4> tex : register(u1, space2);"}));

}  // namespace
}  // namespace tint::writer::hlsl
