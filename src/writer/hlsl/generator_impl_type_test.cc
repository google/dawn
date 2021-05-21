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
#include "src/sem/depth_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using ::testing::HasSubstr;

using HlslGeneratorImplTest_Type = TestHelper;

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array) {
  auto* arr = ty.array<bool, 4>();
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArray) {
  auto* arr = ty.array(ty.array<bool, 4>(), 5);
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[5][4]");
}

// TODO(dsinclair): Is this possible? What order should it output in?
TEST_F(HlslGeneratorImplTest_Type,
       DISABLED_EmitType_ArrayOfArrayOfRuntimeArray) {
  auto* arr = ty.array(ty.array(ty.array<bool, 4>(), 5), 0);
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[5][4][1]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArrayOfArray) {
  auto* arr = ty.array(ty.array(ty.array<bool, 4>(), 5), 6);
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[6][5][4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array_WithoutName) {
  auto* arr = ty.array<bool, 4>();
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "bool[4]");
}

TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_RuntimeArray) {
  auto* arr = ty.array<bool>();
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Bool) {
  auto* bool_ = create<sem::Bool>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, bool_, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "bool");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_F32) {
  auto* f32 = create<sem::F32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, f32, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "float");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_I32) {
  auto* i32 = create<sem::I32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, i32, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "int");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Matrix) {
  auto* f32 = create<sem::F32>();
  auto* vec3 = create<sem::Vector>(f32, 3);
  auto* mat2x3 = create<sem::Matrix>(vec3, 2);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, mat2x3, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "float2x3");
}

// TODO(dsinclair): How to annotate as workgroup?
TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Pointer) {
  auto* f32 = create<sem::F32>();
  auto* p = create<sem::Pointer>(f32, ast::StorageClass::kWorkgroup);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, p, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
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

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s, "S")) << gen.error();
  EXPECT_EQ(result(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_StructDecl_OmittedIfStorageBuffer) {
  auto* s = Structure("S",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("g", ty.access(ast::AccessControl::kReadWrite, s),
         ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s, "S")) << gen.error();
  EXPECT_EQ(result(), "");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });
  Global("g", s, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitType(out, sem_s, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
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

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitType(out, sem_s, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
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
  int tint_symbol;
  float tint_symbol_1;
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

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s, "B")) << gen.error();
  EXPECT_EQ(result(), R"(struct B {
  int a;
  float b;
})");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_U32) {
  auto* u32 = create<sem::U32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, u32, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "uint");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Vector) {
  auto* f32 = create<sem::F32>();
  auto* vec3 = create<sem::Vector>(f32, 3);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, vec3, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "float3");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Void) {
  auto* void_ = create<sem::Void>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, void_, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "void");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSampler) {
  auto* sampler = create<sem::Sampler>(ast::SamplerKind::kSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, sampler, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "SamplerState");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSamplerComparison) {
  auto* sampler = create<sem::Sampler>(ast::SamplerKind::kComparisonSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, sampler, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "SamplerComparisonState");
}

struct HlslDepthTextureData {
  ast::TextureDimension dim;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out, HlslDepthTextureData data) {
  out << data.dim;
  return out;
}
using HlslDepthTexturesTest = TestParamHelper<HlslDepthTextureData>;
TEST_P(HlslDepthTexturesTest, Emit) {
  auto params = GetParam();

  auto* t = ty.depth_texture(params.dim);

  Global("tex", t, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("textureDimensions", "tex"))},
       {Stage(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));

  Validate();
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Type,
    HlslDepthTexturesTest,
    testing::Values(
        HlslDepthTextureData{ast::TextureDimension::k2d,
                             "Texture2D tex : register(t1, space2);"},
        HlslDepthTextureData{ast::TextureDimension::k2dArray,
                             "Texture2DArray tex : register(t1, space2);"},
        HlslDepthTextureData{ast::TextureDimension::kCube,
                             "TextureCube tex : register(t1, space2);"},
        HlslDepthTextureData{ast::TextureDimension::kCubeArray,
                             "TextureCubeArray tex : register(t1, space2);"}));

enum class TextureDataType { F32, U32, I32 };
struct HlslSampledTextureData {
  ast::TextureDimension dim;
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

  ast::Type* datatype = nullptr;
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

  Global("tex", t, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("textureDimensions", "tex"))},
       {Stage(ast::PipelineStage::kFragment)});

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
            ast::TextureDimension::k1d,
            TextureDataType::F32,
            "Texture1D<float4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k2d,
            TextureDataType::F32,
            "Texture2D<float4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k2dArray,
            TextureDataType::F32,
            "Texture2DArray<float4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k3d,
            TextureDataType::F32,
            "Texture3D<float4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::kCube,
            TextureDataType::F32,
            "TextureCube<float4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::kCubeArray,
            TextureDataType::F32,
            "TextureCubeArray<float4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k1d,
            TextureDataType::U32,
            "Texture1D<uint4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k2d,
            TextureDataType::U32,
            "Texture2D<uint4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k2dArray,
            TextureDataType::U32,
            "Texture2DArray<uint4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k3d,
            TextureDataType::U32,
            "Texture3D<uint4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::kCube,
            TextureDataType::U32,
            "TextureCube<uint4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::kCubeArray,
            TextureDataType::U32,
            "TextureCubeArray<uint4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k1d,
            TextureDataType::I32,
            "Texture1D<int4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k2d,
            TextureDataType::I32,
            "Texture2D<int4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k2dArray,
            TextureDataType::I32,
            "Texture2DArray<int4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::k3d,
            TextureDataType::I32,
            "Texture3D<int4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::kCube,
            TextureDataType::I32,
            "TextureCube<int4> tex : register(t1, space2);",
        },
        HlslSampledTextureData{
            ast::TextureDimension::kCubeArray,
            TextureDataType::I32,
            "TextureCubeArray<int4> tex : register(t1, space2);",
        }));

TEST_F(HlslGeneratorImplTest_Type, EmitMultisampledTexture) {
  auto* f32 = create<sem::F32>();
  auto* s = create<sem::MultisampledTexture>(ast::TextureDimension::k2d, f32);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, s, ast::StorageClass::kNone,
                           ast::AccessControl::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "Texture2DMS<float4>");
}

struct HlslStorageTextureData {
  ast::TextureDimension dim;
  ast::ImageFormat imgfmt;
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

  auto* t = ty.storage_texture(params.dim, params.imgfmt);
  auto* ac = ty.access(params.ro ? ast::AccessControl::kReadOnly
                                 : ast::AccessControl::kWriteOnly,
                       t);

  Global("tex", ac, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("textureDimensions", "tex"))},
       {Stage(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));

  Validate();
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Type,
    HlslStorageTexturesTest,
    testing::Values(
        HlslStorageTextureData{ast::TextureDimension::k1d,
                               ast::ImageFormat::kRgba8Unorm, true,
                               "Texture1D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{ast::TextureDimension::k2d,
                               ast::ImageFormat::kRgba16Float, true,
                               "Texture2D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            ast::TextureDimension::k2dArray, ast::ImageFormat::kR32Float, true,
            "Texture2DArray<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{ast::TextureDimension::k3d,
                               ast::ImageFormat::kRg32Float, true,
                               "Texture3D<float4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            ast::TextureDimension::k1d, ast::ImageFormat::kRgba32Float, false,
            "RWTexture1D<float4> tex : register(u1, space2);"},
        HlslStorageTextureData{
            ast::TextureDimension::k2d, ast::ImageFormat::kRgba16Uint, false,
            "RWTexture2D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{
            ast::TextureDimension::k2dArray, ast::ImageFormat::kR32Uint, false,
            "RWTexture2DArray<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{
            ast::TextureDimension::k3d, ast::ImageFormat::kRg32Uint, false,
            "RWTexture3D<uint4> tex : register(u1, space2);"},
        HlslStorageTextureData{ast::TextureDimension::k1d,
                               ast::ImageFormat::kRgba32Uint, true,
                               "Texture1D<uint4> tex : register(t1, space2);"},
        HlslStorageTextureData{ast::TextureDimension::k2d,
                               ast::ImageFormat::kRgba16Sint, true,
                               "Texture2D<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            ast::TextureDimension::k2dArray, ast::ImageFormat::kR32Sint, true,
            "Texture2DArray<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{ast::TextureDimension::k3d,
                               ast::ImageFormat::kRg32Sint, true,
                               "Texture3D<int4> tex : register(t1, space2);"},
        HlslStorageTextureData{
            ast::TextureDimension::k1d, ast::ImageFormat::kRgba32Sint, false,
            "RWTexture1D<int4> tex : register(u1, space2);"}));

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
