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
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/program.h"
#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/pointer_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/sampler_type.h"
#include "src/type/storage_texture_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitType_Alias) {
  auto* alias = ty.alias("alias", ty.f32());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(alias, "")) << gen.error();
  EXPECT_EQ(gen.result(), "alias");
}

TEST_F(MslGeneratorImplTest, EmitType_Alias_NameCollision) {
  auto* alias = ty.alias("bool", ty.f32());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(alias, "")) << gen.error();
  EXPECT_EQ(gen.result(), "bool_tint_0");
}

TEST_F(MslGeneratorImplTest, EmitType_Array) {
  auto* arr = ty.array<bool, 4>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(arr, "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArray) {
  auto* a = ty.array<bool, 4>();
  auto* b = ty.array(a, 5);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(b, "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[5][4]");
}

// TODO(dsinclair): Is this possible? What order should it output in?
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_ArrayOfArrayOfRuntimeArray) {
  auto* a = ty.array<bool, 4>();
  auto* b = ty.array(a, 5);
  auto* c = ty.array(b, 0);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(c, "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[5][4][1]");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArrayOfArray) {
  auto* a = ty.array<bool, 4>();
  auto* b = ty.array(a, 5);
  auto* c = ty.array(b, 6);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(c, "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[6][5][4]");
}

TEST_F(MslGeneratorImplTest, EmitType_Array_NameCollision) {
  auto* arr = ty.array<bool, 4>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(arr, "bool")) << gen.error();
  EXPECT_EQ(gen.result(), "bool bool_tint_0[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_Array_WithoutName) {
  auto* arr = ty.array<bool, 4>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(arr, "")) << gen.error();
  EXPECT_EQ(gen.result(), "bool[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_RuntimeArray) {
  auto* arr = ty.array<bool, 1>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(arr, "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[1]");
}

TEST_F(MslGeneratorImplTest, EmitType_RuntimeArray_NameCollision) {
  auto* arr = ty.array<bool, 1>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(arr, "discard_fragment")) << gen.error();
  EXPECT_EQ(gen.result(), "bool discard_fragment_tint_0[1]");
}

TEST_F(MslGeneratorImplTest, EmitType_Bool) {
  auto* bool_ = ty.bool_();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(bool_, "")) << gen.error();
  EXPECT_EQ(gen.result(), "bool");
}

TEST_F(MslGeneratorImplTest, EmitType_F32) {
  auto* f32 = ty.f32();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(f32, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float");
}

TEST_F(MslGeneratorImplTest, EmitType_I32) {
  auto* i32 = ty.i32();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(i32, "")) << gen.error();
  EXPECT_EQ(gen.result(), "int");
}

TEST_F(MslGeneratorImplTest, EmitType_Matrix) {
  auto* mat2x3 = ty.mat2x3<f32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(mat2x3, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float2x3");
}

// TODO(dsinclair): How to annotate as workgroup?
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_Pointer) {
  type::Pointer p(ty.f32(), ast::StorageClass::kWorkgroup);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&p, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float*");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32()),
                            Member("b", ty.f32(), {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(s, "")) << gen.error();
  EXPECT_EQ(gen.result(), "S");
}

TEST_F(MslGeneratorImplTest, EmitType_StructDecl) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32()),
                            Member("b", ty.f32(), {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_InjectPadding) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{
          Member("a", ty.i32(), {MemberOffset(4)}),
          Member("b", ty.f32(), {MemberOffset(32)}),
          Member("c", ty.f32(), {MemberOffset(128)}),
      },
      ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  int8_t pad_0[4];
  int a;
  int8_t pad_1[24];
  float b;
  int8_t pad_2[92];
  float c;
};
)");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_NameCollision) {
  auto* str =
      create<ast::Struct>(ast::StructMemberList{Member("main", ty.i32()),
                                                Member("float", ty.f32())},
                          ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  int main_tint_0;
  float float_tint_0;
};
)");
}

// TODO(dsinclair): How to translate [[block]]
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_Struct_WithDecoration) {
  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32()),
                            Member("b", ty.f32(), {MemberOffset(4)})},
      decos);

  auto* s = ty.struct_("S", str);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(s, "")) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct {
  int a;
  float b;
})");
}

TEST_F(MslGeneratorImplTest, EmitType_U32) {
  auto* u32 = ty.u32();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(u32, "")) << gen.error();
  EXPECT_EQ(gen.result(), "uint");
}

TEST_F(MslGeneratorImplTest, EmitType_Vector) {
  auto* vec3 = ty.vec3<f32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(vec3, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float3");
}

TEST_F(MslGeneratorImplTest, EmitType_Void) {
  auto* void_ = ty.void_();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(void_, "")) << gen.error();
  EXPECT_EQ(gen.result(), "void");
}

TEST_F(MslGeneratorImplTest, EmitType_Sampler) {
  type::Sampler sampler(type::SamplerKind::kSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&sampler, "")) << gen.error();
  EXPECT_EQ(gen.result(), "sampler");
}

TEST_F(MslGeneratorImplTest, EmitType_SamplerComparison) {
  type::Sampler sampler(type::SamplerKind::kComparisonSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&sampler, "")) << gen.error();
  EXPECT_EQ(gen.result(), "sampler");
}

struct MslDepthTextureData {
  type::TextureDimension dim;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslDepthTextureData data) {
  out << data.dim;
  return out;
}
using MslDepthTexturesTest = TestParamHelper<MslDepthTextureData>;
TEST_P(MslDepthTexturesTest, Emit) {
  auto params = GetParam();

  type::DepthTexture s(params.dim);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&s, "")) << gen.error();
  EXPECT_EQ(gen.result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslDepthTexturesTest,
    testing::Values(MslDepthTextureData{type::TextureDimension::k2d,
                                        "depth2d<float, access::sample>"},
                    MslDepthTextureData{type::TextureDimension::k2dArray,
                                        "depth2d_array<float, access::sample>"},
                    MslDepthTextureData{type::TextureDimension::kCube,
                                        "depthcube<float, access::sample>"},
                    MslDepthTextureData{
                        type::TextureDimension::kCubeArray,
                        "depthcube_array<float, access::sample>"}));

struct MslTextureData {
  type::TextureDimension dim;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslTextureData data) {
  out << data.dim;
  return out;
}
using MslSampledtexturesTest = TestParamHelper<MslTextureData>;
TEST_P(MslSampledtexturesTest, Emit) {
  auto params = GetParam();

  type::SampledTexture s(params.dim, ty.f32());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&s, "")) << gen.error();
  EXPECT_EQ(gen.result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslSampledtexturesTest,
    testing::Values(MslTextureData{type::TextureDimension::k1d,
                                   "texture1d<float, access::sample>"},
                    MslTextureData{type::TextureDimension::k1dArray,
                                   "texture1d_array<float, access::sample>"},
                    MslTextureData{type::TextureDimension::k2d,
                                   "texture2d<float, access::sample>"},
                    MslTextureData{type::TextureDimension::k2dArray,
                                   "texture2d_array<float, access::sample>"},
                    MslTextureData{type::TextureDimension::k3d,
                                   "texture3d<float, access::sample>"},
                    MslTextureData{type::TextureDimension::kCube,
                                   "texturecube<float, access::sample>"},
                    MslTextureData{
                        type::TextureDimension::kCubeArray,
                        "texturecube_array<float, access::sample>"}));

TEST_F(MslGeneratorImplTest, Emit_TypeMultisampledTexture) {
  type::MultisampledTexture s(type::TextureDimension::k2d, ty.u32());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&s, "")) << gen.error();
  EXPECT_EQ(gen.result(), "texture2d_ms<uint, access::sample>");
}

struct MslStorageTextureData {
  type::TextureDimension dim;
  bool ro;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslStorageTextureData data) {
  out << data.dim << (data.ro ? "ReadOnly" : "WriteOnly");
  return out;
}
using MslStorageTexturesTest = TestParamHelper<MslStorageTextureData>;
TEST_P(MslStorageTexturesTest, Emit) {
  auto params = GetParam();

  type::StorageTexture s(params.dim, type::ImageFormat::kR16Float);
  type::AccessControl ac(params.ro ? ast::AccessControl::kReadOnly
                                   : ast::AccessControl::kWriteOnly,
                         &s);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&ac, "")) << gen.error();
  EXPECT_EQ(gen.result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslStorageTexturesTest,
    testing::Values(
        MslStorageTextureData{type::TextureDimension::k1d, true,
                              "texture1d<float, access::read>"},
        MslStorageTextureData{type::TextureDimension::k1dArray, true,
                              "texture1d_array<float, access::read>"},
        MslStorageTextureData{type::TextureDimension::k2d, true,
                              "texture2d<float, access::read>"},
        MslStorageTextureData{type::TextureDimension::k2dArray, true,
                              "texture2d_array<float, access::read>"},
        MslStorageTextureData{type::TextureDimension::k3d, true,
                              "texture3d<float, access::read>"},
        MslStorageTextureData{type::TextureDimension::k1d, false,
                              "texture1d<float, access::write>"},
        MslStorageTextureData{type::TextureDimension::k1dArray, false,
                              "texture1d_array<float, access::write>"},
        MslStorageTextureData{type::TextureDimension::k2d, false,
                              "texture2d<float, access::write>"},
        MslStorageTextureData{type::TextureDimension::k2dArray, false,
                              "texture2d_array<float, access::write>"},
        MslStorageTextureData{type::TextureDimension::k3d, false,
                              "texture3d<float, access::write>"}));

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
