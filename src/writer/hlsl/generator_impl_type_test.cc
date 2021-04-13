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
#include "src/ast/call_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/type/access_control_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/storage_texture_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using ::testing::HasSubstr;

using HlslGeneratorImplTest_Type = TestHelper;

TEST_F(HlslGeneratorImplTest_Type, EmitType_Alias) {
  auto* alias = ty.alias("alias", ty.f32());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, alias, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "alias");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array) {
  auto* arr = ty.array<bool, 4>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, arr, ast::StorageClass::kNone, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArray) {
  auto* arr = ty.array(ty.array<bool, 4>(), 5);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, arr, ast::StorageClass::kNone, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[5][4]");
}

// TODO(dsinclair): Is this possible? What order should it output in?
TEST_F(HlslGeneratorImplTest_Type,
       DISABLED_EmitType_ArrayOfArrayOfRuntimeArray) {
  auto* arr = ty.array(ty.array(ty.array<bool, 4>(), 5), 0);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, arr, ast::StorageClass::kNone, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[5][4][1]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArrayOfArray) {
  auto* arr = ty.array(ty.array(ty.array<bool, 4>(), 5), 6);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, arr, ast::StorageClass::kNone, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[6][5][4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array_WithoutName) {
  auto* arr = ty.array<bool, 4>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, arr, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "bool[4]");
}

TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_RuntimeArray) {
  auto* arr = ty.array<bool>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, arr, ast::StorageClass::kNone, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Bool) {
  auto* bool_ = ty.bool_();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, bool_, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "bool");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_F32) {
  auto* f32 = ty.f32();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, f32, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "float");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_I32) {
  auto* i32 = ty.i32();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, i32, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "int");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Matrix) {
  auto* mat2x3 = ty.mat2x3<f32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, mat2x3, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "float2x3");
}

// TODO(dsinclair): How to annotate as workgroup?
TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Pointer) {
  type::Pointer p(ty.f32(), ast::StorageClass::kWorkgroup);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, &p, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "float*");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_StructDecl) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });
  Global("g", s, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(out, s, "S")) << gen.error();
  EXPECT_EQ(result(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_StructDecl_OmittedIfStorageBuffer) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });
  Global("g", s, ast::StorageClass::kStorage);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(out, s, "S")) << gen.error();
  EXPECT_EQ(result(), "");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });
  Global("g", s, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, s, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "S");
}

/// TODO(bclayton): Enable this, fix it, add tests for vector, matrix, array and
/// nested structures.
TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Struct_InjectPadding) {
  auto* s = Structure(
      "S", {
               Member("a", ty.i32(), {MemberSize(32)}),
               Member("b", ty.f32()),
               Member("c", ty.f32(), {MemberAlign(128), MemberSize(128)}),
           });
  Global("g", s, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, s, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  int a;
  int8_t pad_0[28];
  float b;
  int8_t pad_1[92];
  float c;
  int8_t pad_2[124];
};
)");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_NameCollision) {
  auto* s = Structure("S", {
                               Member("double", ty.i32()),
                               Member("float", ty.f32()),
                           });
  Global("g", s, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(R"(struct S {
  int tint_double;
  float tint_float;
};
)"));
}

// TODO(dsinclair): How to translate [[block]]
TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Struct_WithDecoration) {
  auto* s = Structure("S",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("g", s, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(out, s, "B")) << gen.error();
  EXPECT_EQ(result(), R"(struct B {
  int a;
  float b;
})");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_U32) {
  auto* u32 = ty.u32();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, u32, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "uint");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Vector) {
  auto* vec3 = ty.vec3<f32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, vec3, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "float3");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Void) {
  auto* void_ = ty.void_();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, void_, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "void");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSampler) {
  type::Sampler sampler(type::SamplerKind::kSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, &sampler, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "SamplerState");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSamplerComparison) {
  type::Sampler sampler(type::SamplerKind::kComparisonSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, &sampler, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "SamplerComparisonState");
}

struct HlslDepthTextureData {
  type::TextureDimension dim;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out, HlslDepthTextureData data) {
  out << data.dim;
  return out;
}
using HlslDepthTexturesTest = TestParamHelper<HlslDepthTextureData>;
TEST_P(HlslDepthTexturesTest, Emit) {
  auto params = GetParam();

  auto* t = create<type::DepthTexture>(params.dim);

  Global("tex", t, ast::StorageClass::kUniformConstant, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("textureDimensions", "tex"))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));

  Validate();
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Type,
    HlslDepthTexturesTest,
    testing::Values(
        HlslDepthTextureData{type::TextureDimension::k2d,
                             "Texture2D tex : register(t1, space2);"},
        HlslDepthTextureData{type::TextureDimension::k2dArray,
                             "Texture2DArray tex : register(t1, space2);"},
        HlslDepthTextureData{type::TextureDimension::kCube,
                             "TextureCube tex : register(t1, space2);"},
        HlslDepthTextureData{type::TextureDimension::kCubeArray,
                             "TextureCubeArray tex : register(t1, space2);"}));

enum class TextureDataType { F32, U32, I32 };
struct HlslSampledTextureData {
  type::TextureDimension dim;
  TextureDataType datatype;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out,
                                HlslSampledTextureData data) {
  out << data.dim;
  return out;
}
using HlslSampledTexturesTest = TestParamHelper<HlslSampledTextureData>;
TEST_P(HlslSampledTexturesTest, Emit) {
  auto params = GetParam();

  type::Type* datatype = nullptr;
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
  auto* t = create<type::SampledTexture>(params.dim, datatype);

  Global("tex", t, ast::StorageClass::kUniformConstant, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("textureDimensions", "tex"))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));

  Validate();
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Type,
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
  type::MultisampledTexture s(type::TextureDimension::k2d, ty.f32());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, &s, ast::StorageClass::kNone, ""))
      << gen.error();
  EXPECT_EQ(result(), "Texture2DMS<float4>");
}

struct HlslStorageTextureData {
  type::TextureDimension dim;
  type::ImageFormat imgfmt;
  bool ro;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out,
                                HlslStorageTextureData data) {
  out << data.dim << (data.ro ? "ReadOnly" : "WriteOnly");
  return out;
}
using HlslStorageTexturesTest = TestParamHelper<HlslStorageTextureData>;
TEST_P(HlslStorageTexturesTest, Emit) {
  auto params = GetParam();

  auto* subtype = type::StorageTexture::SubtypeFor(params.imgfmt, Types());
  auto* t = create<type::StorageTexture>(params.dim, params.imgfmt, subtype);
  auto* ac =
      create<type::AccessControl>(params.ro ? ast::AccessControl::kReadOnly
                                            : ast::AccessControl::kWriteOnly,
                                  t);

  Global("tex", ac, ast::StorageClass::kUniformConstant, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("textureDimensions", "tex"))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));

  Validate();
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Type,
    HlslStorageTexturesTest,
    testing::Values(
        HlslStorageTextureData{type::TextureDimension::k1d,
                               type::ImageFormat::kRgba8Unorm, true,
                               "Texture1D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2d,
                               type::ImageFormat::kRgba16Float, true,
                               "Texture2D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            type::TextureDimension::k2dArray, type::ImageFormat::kR32Float,
            true, "Texture2DArray<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k3d,
                               type::ImageFormat::kRg32Float, true,
                               "Texture3D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            type::TextureDimension::k1d, type::ImageFormat::kRgba32Float, false,
            "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{
            type::TextureDimension::k2d, type::ImageFormat::kRgba16Uint, false,
            "RWTexture2D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{
            type::TextureDimension::k2dArray, type::ImageFormat::kR32Uint,
            false, "RWTexture2DArray<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{
            type::TextureDimension::k3d, type::ImageFormat::kRg32Uint, false,
            "RWTexture3D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k1d,
                               type::ImageFormat::kRgba32Uint, true,
                               "Texture1D<uint4> tex : register(t1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k2d,
                               type::ImageFormat::kRgba16Sint, true,
                               "Texture2D<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            type::TextureDimension::k2dArray, type::ImageFormat::kR32Sint, true,
            "Texture2DArray<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{type::TextureDimension::k3d,
                               type::ImageFormat::kRg32Sint, true,
                               "Texture3D<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            type::TextureDimension::k1d, type::ImageFormat::kRgba32Sint, false,
            "RWTexture1D<int4> tex : register(u1, space2);"}));

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
