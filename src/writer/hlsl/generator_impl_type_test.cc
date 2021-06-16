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
                           ast::Access::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArray) {
  auto* arr = ty.array(ty.array<bool, 4>(), 5);
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::Access::kReadWrite, "ary"))
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
                           ast::Access::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[5][4][1]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayOfArrayOfArray) {
  auto* arr = ty.array(ty.array(ty.array<bool, 4>(), 5), 6);
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::Access::kReadWrite, "ary"))
      << gen.error();
  EXPECT_EQ(result(), "bool ary[6][5][4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Array_WithoutName) {
  auto* arr = ty.array<bool, 4>();
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, program->TypeOf(arr), ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "bool[4]");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_ArrayWithStride) {
  auto* s = Structure("s", {Member("arr", ty.array<f32, 4>(64))},
                      {create<ast::StructBlockDecoration>()});
  auto* ubo = Global("ubo", ty.Of(s), ast::StorageClass::kUniform,
                     ast::DecorationList{
                         create<ast::GroupDecoration>(1),
                         create<ast::BindingDecoration>(1),
                     });
  WrapInFunction(MemberAccessor(ubo, "arr"));

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(R"(struct tint_padded_array_element {
  /* 0x0000 */ float el;
  /* 0x0004 */ int tint_pad_0[15];
};)"));
  EXPECT_THAT(result(), HasSubstr(R"(struct tint_array_wrapper {
  /* 0x0000 */ tint_padded_array_element arr[4];
};)"));
  EXPECT_THAT(result(), HasSubstr(R"(struct s {
  /* 0x0000 */ tint_array_wrapper arr;
};)"));
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Bool) {
  auto* bool_ = create<sem::Bool>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, bool_, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "bool");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_F32) {
  auto* f32 = create<sem::F32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, f32, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "float");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_I32) {
  auto* i32 = create<sem::I32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, i32, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "int");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Matrix) {
  auto* f32 = create<sem::F32>();
  auto* vec3 = create<sem::Vector>(f32, 3);
  auto* mat2x3 = create<sem::Matrix>(vec3, 2);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, mat2x3, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "float2x3");
}

// TODO(dsinclair): How to annotate as workgroup?
TEST_F(HlslGeneratorImplTest_Type, DISABLED_EmitType_Pointer) {
  auto* f32 = create<sem::F32>();
  auto* p = create<sem::Pointer>(f32, ast::StorageClass::kWorkgroup,
                                 ast::Access::kReadWrite);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, p, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "float*");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_StructDecl) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });
  Global("g", ty.Of(s), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s)) << gen.error();
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
  Global("g", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s)) << gen.error();
  EXPECT_EQ(result(), "");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });
  Global("g", ty.Of(s), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitType(out, sem_s, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "S");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_Layout_NonComposites) {
  auto* s =
      Structure("S",
                {
                    Member("a", ty.i32(), {MemberSize(32)}),
                    Member("b", ty.f32(), {MemberAlign(128), MemberSize(128)}),
                    Member("c", ty.vec2<f32>()),
                    Member("d", ty.u32()),
                    Member("e", ty.vec3<f32>()),
                    Member("f", ty.u32()),
                    Member("g", ty.vec4<f32>()),
                    Member("h", ty.u32()),
                    Member("i", ty.mat2x2<f32>()),
                    Member("j", ty.u32()),
                    Member("k", ty.mat2x3<f32>()),
                    Member("l", ty.u32()),
                    Member("m", ty.mat2x4<f32>()),
                    Member("n", ty.u32()),
                    Member("o", ty.mat3x2<f32>()),
                    Member("p", ty.u32()),
                    Member("q", ty.mat3x3<f32>()),
                    Member("r", ty.u32()),
                    Member("s", ty.mat3x4<f32>()),
                    Member("t", ty.u32()),
                    Member("u", ty.mat4x2<f32>()),
                    Member("v", ty.u32()),
                    Member("w", ty.mat4x3<f32>()),
                    Member("x", ty.u32()),
                    Member("y", ty.mat4x4<f32>()),
                    Member("z", ty.f32()),
                },
                {create<ast::StructBlockDecoration>()});

  Global("G", ty.Of(s), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s)) << gen.error();

  auto* expect = R"(struct S {
  /* 0x0000 */ int a;
  /* 0x0004 */ int tint_pad_0[31];
  /* 0x0080 */ float b;
  /* 0x0084 */ int tint_pad_1[31];
  /* 0x0100 */ float2 c;
  /* 0x0108 */ uint d;
  /* 0x010c */ int tint_pad_2[1];
  /* 0x0110 */ float3 e;
  /* 0x011c */ uint f;
  /* 0x0120 */ float4 g;
  /* 0x0130 */ uint h;
  /* 0x0134 */ int tint_pad_3[1];
  /* 0x0138 */ float2x2 i;
  /* 0x0148 */ uint j;
  /* 0x014c */ int tint_pad_4[1];
  /* 0x0150 */ float2x3 k;
  /* 0x0170 */ uint l;
  /* 0x0174 */ int tint_pad_5[3];
  /* 0x0180 */ float2x4 m;
  /* 0x01a0 */ uint n;
  /* 0x01a4 */ int tint_pad_6[1];
  /* 0x01a8 */ float3x2 o;
  /* 0x01c0 */ uint p;
  /* 0x01c4 */ int tint_pad_7[3];
  /* 0x01d0 */ float3x3 q;
  /* 0x0200 */ uint r;
  /* 0x0204 */ int tint_pad_8[3];
  /* 0x0210 */ float3x4 s;
  /* 0x0240 */ uint t;
  /* 0x0244 */ int tint_pad_9[1];
  /* 0x0248 */ float4x2 u;
  /* 0x0268 */ uint v;
  /* 0x026c */ int tint_pad_10[1];
  /* 0x0270 */ float4x3 w;
  /* 0x02b0 */ uint x;
  /* 0x02b4 */ int tint_pad_11[3];
  /* 0x02c0 */ float4x4 y;
  /* 0x0300 */ float z;
  /* 0x0304 */ int tint_pad_12[31];
};

S make_S(int param_0,
         float param_1,
         float2 param_2,
         uint param_3,
         float3 param_4,
         uint param_5,
         float4 param_6,
         uint param_7,
         float2x2 param_8,
         uint param_9,
         float2x3 param_10,
         uint param_11,
         float2x4 param_12,
         uint param_13,
         float3x2 param_14,
         uint param_15,
         float3x3 param_16,
         uint param_17,
         float3x4 param_18,
         uint param_19,
         float4x2 param_20,
         uint param_21,
         float4x3 param_22,
         uint param_23,
         float4x4 param_24,
         float param_25) {
  S output;
  output.a = param_0;
  output.b = param_1;
  output.c = param_2;
  output.d = param_3;
  output.e = param_4;
  output.f = param_5;
  output.g = param_6;
  output.h = param_7;
  output.i = param_8;
  output.j = param_9;
  output.k = param_10;
  output.l = param_11;
  output.m = param_12;
  output.n = param_13;
  output.o = param_14;
  output.p = param_15;
  output.q = param_16;
  output.r = param_17;
  output.s = param_18;
  output.t = param_19;
  output.u = param_20;
  output.v = param_21;
  output.w = param_22;
  output.x = param_23;
  output.y = param_24;
  output.z = param_25;
  return output;
}
)";

  EXPECT_EQ(result(), expect);
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_Layout_Structures) {
  // inner_x: size(1024), align(512)
  auto* inner_x =
      Structure("inner_x", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32(), {MemberAlign(512)}),
                           });

  // inner_y: size(516), align(4)
  auto* inner_y =
      Structure("inner_y", {
                               Member("a", ty.i32(), {MemberSize(512)}),
                               Member("b", ty.f32()),
                           });

  auto* s = Structure("S",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.Of(inner_x)),
                          Member("c", ty.f32()),
                          Member("d", ty.Of(inner_y)),
                          Member("e", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});

  Global("G", ty.Of(s), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s)) << gen.error();

  auto* expect = R"(struct S {
  /* 0x0000 */ int a;
  /* 0x0004 */ int tint_pad_0[127];
  /* 0x0200 */ inner_x b;
  /* 0x0600 */ float c;
  /* 0x0604 */ inner_y d;
  /* 0x0808 */ float e;
  /* 0x080c */ int tint_pad_1[125];
};

S make_S(int param_0,
         inner_x param_1,
         float param_2,
         inner_y param_3,
         float param_4) {
  S output;
  output.a = param_0;
  output.b = param_1;
  output.c = param_2;
  output.d = param_3;
  output.e = param_4;
  return output;
}
)";
  EXPECT_EQ(result(), expect);
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_Layout_ArrayDefaultStride) {
  // inner: size(1024), align(512)
  auto* inner =
      Structure("inner", {
                             Member("a", ty.i32()),
                             Member("b", ty.f32(), {MemberAlign(512)}),
                         });

  // array_x: size(28), align(4)
  auto* array_x = ty.array<f32, 7>();

  // array_y: size(4096), align(512)
  auto* array_y = ty.array(ty.Of(inner), 4);

  // array_z: size(4), align(4)
  auto* array_z = ty.array<f32, 1>();

  auto* s =
      Structure("S",
                {
                    Member("a", ty.i32()),
                    Member("b", array_x),
                    Member("c", ty.f32()),
                    Member("d", array_y),
                    Member("e", ty.f32()),
                    Member("f", array_z),
                },
                ast::DecorationList{create<ast::StructBlockDecoration>()});

  Global("G", ty.Of(s), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s)) << gen.error();

  auto* expect = R"(struct S {
  /* 0x0000 */ int a;
  /* 0x0004 */ float b[7];
  /* 0x0020 */ float c;
  /* 0x0024 */ int tint_pad_0[119];
  /* 0x0200 */ inner d[4];
  /* 0x1200 */ float e;
  /* 0x1204 */ float f[1];
  /* 0x1208 */ int tint_pad_1[126];
};

S make_S(int param_0,
         float param_1[7],
         float param_2,
         inner param_3[4],
         float param_4,
         float param_5[1]) {
  S output;
  output.a = param_0;
  output.b = param_1;
  output.c = param_2;
  output.d = param_3;
  output.e = param_4;
  output.f = param_5;
  return output;
}
)";

  EXPECT_EQ(result(), expect);
}

TEST_F(HlslGeneratorImplTest_Type, AttemptTintPadSymbolCollision) {
  auto* s = Structure(
      "S",
      {
          // uses symbols tint_pad_[0..9] and tint_pad_[20..35]
          Member("tint_pad_2", ty.i32(), {MemberSize(32)}),
          Member("tint_pad_20", ty.f32(), {MemberAlign(128), MemberSize(128)}),
          Member("tint_pad_33", ty.vec2<f32>()),
          Member("tint_pad_1", ty.u32()),
          Member("tint_pad_3", ty.vec3<f32>()),
          Member("tint_pad_7", ty.u32()),
          Member("tint_pad_25", ty.vec4<f32>()),
          Member("tint_pad_5", ty.u32()),
          Member("tint_pad_27", ty.mat2x2<f32>()),
          Member("tint_pad_24", ty.u32()),
          Member("tint_pad_23", ty.mat2x3<f32>()),
          Member("tint_pad_0", ty.u32()),
          Member("tint_pad_8", ty.mat2x4<f32>()),
          Member("tint_pad_26", ty.u32()),
          Member("tint_pad_29", ty.mat3x2<f32>()),
          Member("tint_pad_6", ty.u32()),
          Member("tint_pad_22", ty.mat3x3<f32>()),
          Member("tint_pad_32", ty.u32()),
          Member("tint_pad_34", ty.mat3x4<f32>()),
          Member("tint_pad_35", ty.u32()),
          Member("tint_pad_30", ty.mat4x2<f32>()),
          Member("tint_pad_9", ty.u32()),
          Member("tint_pad_31", ty.mat4x3<f32>()),
          Member("tint_pad_28", ty.u32()),
          Member("tint_pad_4", ty.mat4x4<f32>()),
          Member("tint_pad_21", ty.f32()),
      },
      {create<ast::StructBlockDecoration>()});

  Global("G", ty.Of(s), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(out, sem_s)) << gen.error();
  EXPECT_EQ(result(), R"(struct S {
  /* 0x0000 */ int tint_pad_2;
  /* 0x0004 */ int tint_pad_10[31];
  /* 0x0080 */ float tint_pad_20;
  /* 0x0084 */ int tint_pad_11[31];
  /* 0x0100 */ float2 tint_pad_33;
  /* 0x0108 */ uint tint_pad_1;
  /* 0x010c */ int tint_pad_12[1];
  /* 0x0110 */ float3 tint_pad_3;
  /* 0x011c */ uint tint_pad_7;
  /* 0x0120 */ float4 tint_pad_25;
  /* 0x0130 */ uint tint_pad_5;
  /* 0x0134 */ int tint_pad_13[1];
  /* 0x0138 */ float2x2 tint_pad_27;
  /* 0x0148 */ uint tint_pad_24;
  /* 0x014c */ int tint_pad_14[1];
  /* 0x0150 */ float2x3 tint_pad_23;
  /* 0x0170 */ uint tint_pad_0;
  /* 0x0174 */ int tint_pad_15[3];
  /* 0x0180 */ float2x4 tint_pad_8;
  /* 0x01a0 */ uint tint_pad_26;
  /* 0x01a4 */ int tint_pad_16[1];
  /* 0x01a8 */ float3x2 tint_pad_29;
  /* 0x01c0 */ uint tint_pad_6;
  /* 0x01c4 */ int tint_pad_17[3];
  /* 0x01d0 */ float3x3 tint_pad_22;
  /* 0x0200 */ uint tint_pad_32;
  /* 0x0204 */ int tint_pad_18[3];
  /* 0x0210 */ float3x4 tint_pad_34;
  /* 0x0240 */ uint tint_pad_35;
  /* 0x0244 */ int tint_pad_19[1];
  /* 0x0248 */ float4x2 tint_pad_30;
  /* 0x0268 */ uint tint_pad_9;
  /* 0x026c */ int tint_pad_36[1];
  /* 0x0270 */ float4x3 tint_pad_31;
  /* 0x02b0 */ uint tint_pad_28;
  /* 0x02b4 */ int tint_pad_37[3];
  /* 0x02c0 */ float4x4 tint_pad_4;
  /* 0x0300 */ float tint_pad_21;
  /* 0x0304 */ int tint_pad_38[31];
};

S make_S(int param_0,
         float param_1,
         float2 param_2,
         uint param_3,
         float3 param_4,
         uint param_5,
         float4 param_6,
         uint param_7,
         float2x2 param_8,
         uint param_9,
         float2x3 param_10,
         uint param_11,
         float2x4 param_12,
         uint param_13,
         float3x2 param_14,
         uint param_15,
         float3x3 param_16,
         uint param_17,
         float3x4 param_18,
         uint param_19,
         float4x2 param_20,
         uint param_21,
         float4x3 param_22,
         uint param_23,
         float4x4 param_24,
         float param_25) {
  S output;
  output.tint_pad_2 = param_0;
  output.tint_pad_20 = param_1;
  output.tint_pad_33 = param_2;
  output.tint_pad_1 = param_3;
  output.tint_pad_3 = param_4;
  output.tint_pad_7 = param_5;
  output.tint_pad_25 = param_6;
  output.tint_pad_5 = param_7;
  output.tint_pad_27 = param_8;
  output.tint_pad_24 = param_9;
  output.tint_pad_23 = param_10;
  output.tint_pad_0 = param_11;
  output.tint_pad_8 = param_12;
  output.tint_pad_26 = param_13;
  output.tint_pad_29 = param_14;
  output.tint_pad_6 = param_15;
  output.tint_pad_22 = param_16;
  output.tint_pad_32 = param_17;
  output.tint_pad_34 = param_18;
  output.tint_pad_35 = param_19;
  output.tint_pad_30 = param_20;
  output.tint_pad_9 = param_21;
  output.tint_pad_31 = param_22;
  output.tint_pad_28 = param_23;
  output.tint_pad_4 = param_24;
  output.tint_pad_21 = param_25;
  return output;
}
)");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Struct_NameCollision) {
  auto* s = Structure("S", {
                               Member("double", ty.i32()),
                               Member("float", ty.f32()),
                           });
  Global("g", ty.Of(s), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(R"(struct S {
  int tint_symbol;
  float tint_symbol_1;
};
)"));
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_U32) {
  auto* u32 = create<sem::U32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, u32, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "uint");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Vector) {
  auto* f32 = create<sem::F32>();
  auto* vec3 = create<sem::Vector>(f32, 3);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, vec3, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "float3");
}

TEST_F(HlslGeneratorImplTest_Type, EmitType_Void) {
  auto* void_ = create<sem::Void>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, void_, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "void");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSampler) {
  auto* sampler = create<sem::Sampler>(ast::SamplerKind::kSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, sampler, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
      << gen.error();
  EXPECT_EQ(result(), "SamplerState");
}

TEST_F(HlslGeneratorImplTest_Type, EmitSamplerComparison) {
  auto* sampler = create<sem::Sampler>(ast::SamplerKind::kComparisonSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(out, sampler, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, ""))
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

  Global("tex", t,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(), {Ignore(Call("textureDimensions", "tex"))},
       {Stage(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));
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

  Global("tex", t,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(), {Ignore(Call("textureDimensions", "tex"))},
       {Stage(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));
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
                           ast::Access::kReadWrite, ""))
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

  auto* t =
      ty.storage_texture(params.dim, params.imgfmt,
                         params.ro ? ast::Access::kRead : ast::Access::kWrite);

  Global("tex", t,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("main", {}, ty.void_(), {Ignore(Call("textureDimensions", "tex"))},
       {Stage(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(params.result));
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
