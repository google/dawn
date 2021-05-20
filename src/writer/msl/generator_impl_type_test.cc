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

#include <array>

#include "src/ast/struct_block_decoration.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

#define CHECK_TYPE_SIZE_AND_ALIGN(TYPE, SIZE, ALIGN)    \
  static_assert(sizeof(TYPE) == SIZE, "Bad type size"); \
  static_assert(alignof(TYPE) == ALIGN, "Bad type alignment")

// Declare C++ types that match the size and alignment of the types of the same
// name in MSL.
#define DECLARE_TYPE(NAME, SIZE, ALIGN) \
  struct alignas(ALIGN) NAME {          \
    uint8_t _[SIZE];                    \
  };                                    \
  CHECK_TYPE_SIZE_AND_ALIGN(NAME, SIZE, ALIGN)

// Size and alignments taken from the MSL spec:
// https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
DECLARE_TYPE(packed_float2, 8, 4);
DECLARE_TYPE(packed_float3, 12, 4);
DECLARE_TYPE(packed_float4, 16, 4);
DECLARE_TYPE(float2x2, 16, 8);
DECLARE_TYPE(float2x3, 32, 16);
DECLARE_TYPE(float2x4, 32, 16);
DECLARE_TYPE(float3x2, 24, 8);
DECLARE_TYPE(float3x3, 48, 16);
DECLARE_TYPE(float3x4, 48, 16);
DECLARE_TYPE(float4x2, 32, 8);
DECLARE_TYPE(float4x3, 64, 16);
DECLARE_TYPE(float4x4, 64, 16);
using uint = unsigned int;

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitType_Array) {
  auto* arr = ty.array<bool, 4>();
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(arr), "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArray) {
  auto* a = ty.array<bool, 4>();
  auto* b = ty.array(a, 5);
  Global("G", b, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(b), "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[5][4]");
}

// TODO(dsinclair): Is this possible? What order should it output in?
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_ArrayOfArrayOfRuntimeArray) {
  auto* a = ty.array<bool, 4>();
  auto* b = ty.array(a, 5);
  auto* c = ty.array(b, 0);
  Global("G", c, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(c), "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[5][4][1]");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArrayOfArray) {
  auto* a = ty.array<bool, 4>();
  auto* b = ty.array(a, 5);
  auto* c = ty.array(b, 6);
  Global("G", c, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(c), "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[6][5][4]");
}

TEST_F(MslGeneratorImplTest, EmitType_Array_WithoutName) {
  auto* arr = ty.array<bool, 4>();
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(arr), "")) << gen.error();
  EXPECT_EQ(gen.result(), "bool[4]");
}

TEST_F(MslGeneratorImplTest, EmitType_RuntimeArray) {
  auto* arr = ty.array<bool, 1>();
  Global("G", arr, ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(arr), "ary")) << gen.error();
  EXPECT_EQ(gen.result(), "bool ary[1]");
}

TEST_F(MslGeneratorImplTest, EmitType_Bool) {
  auto* bool_ = create<sem::Bool>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(bool_, "")) << gen.error();
  EXPECT_EQ(gen.result(), "bool");
}

TEST_F(MslGeneratorImplTest, EmitType_F32) {
  auto* f32 = create<sem::F32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(f32, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float");
}

TEST_F(MslGeneratorImplTest, EmitType_I32) {
  auto* i32 = create<sem::I32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(i32, "")) << gen.error();
  EXPECT_EQ(gen.result(), "int");
}

TEST_F(MslGeneratorImplTest, EmitType_Matrix) {
  auto* f32 = create<sem::F32>();
  auto* vec3 = create<sem::Vector>(f32, 3);
  auto* mat2x3 = create<sem::Matrix>(vec3, 2);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(mat2x3, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float2x3");
}

// TODO(dsinclair): How to annotate as workgroup?
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_Pointer) {
  auto* f32 = create<sem::F32>();
  auto* p = create<sem::Pointer>(f32, ast::StorageClass::kWorkgroup);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(p, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float*");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(s), "")) << gen.error();
  EXPECT_EQ(gen.result(), "S");
}

TEST_F(MslGeneratorImplTest, EmitType_StructDecl) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(sem_s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  int a;
  float b;
};
)");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_Layout_NonComposites) {
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

  Global("G", ty.access(ast::AccessControl::kReadOnly, s),
         ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(sem_s)) << gen.error();

  // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, NAME, SUFFIX)
  // for each field of the structure s.
#define ALL_FIELDS()                             \
  FIELD(0x0000, int, a, /*NO SUFFIX*/)           \
  FIELD(0x0004, int8_t, tint_pad_0, [124])       \
  FIELD(0x0080, float, b, /*NO SUFFIX*/)         \
  FIELD(0x0084, int8_t, tint_pad_1, [124])       \
  FIELD(0x0100, packed_float2, c, /*NO SUFFIX*/) \
  FIELD(0x0108, uint, d, /*NO SUFFIX*/)          \
  FIELD(0x010c, int8_t, tint_pad_2, [4])         \
  FIELD(0x0110, packed_float3, e, /*NO SUFFIX*/) \
  FIELD(0x011c, uint, f, /*NO SUFFIX*/)          \
  FIELD(0x0120, packed_float4, g, /*NO SUFFIX*/) \
  FIELD(0x0130, uint, h, /*NO SUFFIX*/)          \
  FIELD(0x0134, int8_t, tint_pad_3, [4])         \
  FIELD(0x0138, float2x2, i, /*NO SUFFIX*/)      \
  FIELD(0x0148, uint, j, /*NO SUFFIX*/)          \
  FIELD(0x014c, int8_t, tint_pad_4, [4])         \
  FIELD(0x0150, float2x3, k, /*NO SUFFIX*/)      \
  FIELD(0x0170, uint, l, /*NO SUFFIX*/)          \
  FIELD(0x0174, int8_t, tint_pad_5, [12])        \
  FIELD(0x0180, float2x4, m, /*NO SUFFIX*/)      \
  FIELD(0x01a0, uint, n, /*NO SUFFIX*/)          \
  FIELD(0x01a4, int8_t, tint_pad_6, [4])         \
  FIELD(0x01a8, float3x2, o, /*NO SUFFIX*/)      \
  FIELD(0x01c0, uint, p, /*NO SUFFIX*/)          \
  FIELD(0x01c4, int8_t, tint_pad_7, [12])        \
  FIELD(0x01d0, float3x3, q, /*NO SUFFIX*/)      \
  FIELD(0x0200, uint, r, /*NO SUFFIX*/)          \
  FIELD(0x0204, int8_t, tint_pad_8, [12])        \
  FIELD(0x0210, float3x4, s, /*NO SUFFIX*/)      \
  FIELD(0x0240, uint, t, /*NO SUFFIX*/)          \
  FIELD(0x0244, int8_t, tint_pad_9, [4])         \
  FIELD(0x0248, float4x2, u, /*NO SUFFIX*/)      \
  FIELD(0x0268, uint, v, /*NO SUFFIX*/)          \
  FIELD(0x026c, int8_t, tint_pad_10, [4])        \
  FIELD(0x0270, float4x3, w, /*NO SUFFIX*/)      \
  FIELD(0x02b0, uint, x, /*NO SUFFIX*/)          \
  FIELD(0x02b4, int8_t, tint_pad_11, [12])       \
  FIELD(0x02c0, float4x4, y, /*NO SUFFIX*/)      \
  FIELD(0x0300, float, z, /*NO SUFFIX*/)         \
  FIELD(0x0304, int8_t, tint_pad_12, [124])

  // Check that the generated string is as expected.
#define FIELD(ADDR, TYPE, NAME, SUFFIX) \
  "  /* " #ADDR " */ " #TYPE " " #NAME #SUFFIX ";\n"
  auto* expect = "struct S {\n" ALL_FIELDS() "};\n";
#undef FIELD
  EXPECT_EQ(gen.result(), expect);

  // 1.4 Metal and C++14
  // The Metal programming language is a C++14-based Specification with
  // extensions and restrictions. Refer to the C++14 Specification (also known
  // as the ISO/IEC JTC1/SC22/WG21 N4431 Language Specification) for a detailed
  // description of the language grammar.
  //
  // Tint is written in C++14, so use the compiler to verify the generated
  // layout is as expected for C++14 / MSL.
  {
    struct S {
#define FIELD(ADDR, TYPE, NAME, SUFFIX) TYPE NAME SUFFIX;
      ALL_FIELDS()
#undef FIELD
    };

#define FIELD(ADDR, TYPE, NAME, SUFFIX) \
  EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
    ALL_FIELDS()
#undef FIELD
  }
#undef ALL_FIELDS
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_Layout_Structures) {
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
                          Member("b", inner_x),
                          Member("c", ty.f32()),
                          Member("d", inner_y),
                          Member("e", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});

  Global("G", ty.access(ast::AccessControl::kReadOnly, s),
         ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(sem_s)) << gen.error();

  // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, NAME, SUFFIX)
  // for each field of the structure s.
#define ALL_FIELDS()                       \
  FIELD(0x0000, int, a, /*NO SUFFIX*/)     \
  FIELD(0x0004, int8_t, tint_pad_0, [508]) \
  FIELD(0x0200, inner_x, b, /*NO SUFFIX*/) \
  FIELD(0x0600, float, c, /*NO SUFFIX*/)   \
  FIELD(0x0604, inner_y, d, /*NO SUFFIX*/) \
  FIELD(0x0808, float, e, /*NO SUFFIX*/)   \
  FIELD(0x080c, int8_t, tint_pad_1, [500])

  // Check that the generated string is as expected.
#define FIELD(ADDR, TYPE, NAME, SUFFIX) \
  "  /* " #ADDR " */ " #TYPE " " #NAME #SUFFIX ";\n"
  auto* expect = "struct S {\n" ALL_FIELDS() "};\n";
#undef FIELD
  EXPECT_EQ(gen.result(), expect);

  // 1.4 Metal and C++14
  // The Metal programming language is a C++14-based Specification with
  // extensions and restrictions. Refer to the C++14 Specification (also known
  // as the ISO/IEC JTC1/SC22/WG21 N4431 Language Specification) for a detailed
  // description of the language grammar.
  //
  // Tint is written in C++14, so use the compiler to verify the generated
  // layout is as expected for C++14 / MSL.
  {
    struct inner_x {
      uint32_t a;
      alignas(512) float b;
    };
    CHECK_TYPE_SIZE_AND_ALIGN(inner_x, 1024, 512);

    struct inner_y {
      uint32_t a[128];
      float b;
    };
    CHECK_TYPE_SIZE_AND_ALIGN(inner_y, 516, 4);

    struct S {
#define FIELD(ADDR, TYPE, NAME, SUFFIX) TYPE NAME SUFFIX;
      ALL_FIELDS()
#undef FIELD
    };

#define FIELD(ADDR, TYPE, NAME, SUFFIX) \
  EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
    ALL_FIELDS()
#undef FIELD
  }

#undef ALL_FIELDS
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_Layout_ArrayDefaultStride) {
  // inner: size(1024), align(512)
  auto* inner =
      Structure("inner", {
                             Member("a", ty.i32()),
                             Member("b", ty.f32(), {MemberAlign(512)}),
                         });

  // array_x: size(28), align(4)
  auto* array_x = ty.array<f32, 7>();

  // array_y: size(4096), align(512)
  auto* array_y = ty.array(inner, 4);

  // array_z: size(4), align(4)
  auto* array_z = ty.array<f32>();

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

  Global("G", ty.access(ast::AccessControl::kReadOnly, s),
         ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(sem_s)) << gen.error();

  // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, NAME, SUFFIX)
  // for each field of the structure s.
#define ALL_FIELDS()                       \
  FIELD(0x0000, int, a, /*NO SUFFIX*/)     \
  FIELD(0x0004, float, b, [7])             \
  FIELD(0x0020, float, c, /*NO SUFFIX*/)   \
  FIELD(0x0024, int8_t, tint_pad_0, [476]) \
  FIELD(0x0200, inner, d, [4])             \
  FIELD(0x1200, float, e, /*NO SUFFIX*/)   \
  FIELD(0x1204, float, f, [1])             \
  FIELD(0x1208, int8_t, tint_pad_1, [504])

  // Check that the generated string is as expected.
#define FIELD(ADDR, TYPE, NAME, SUFFIX) \
  "  /* " #ADDR " */ " #TYPE " " #NAME #SUFFIX ";\n"
  auto* expect = "struct S {\n" ALL_FIELDS() "};\n";
#undef FIELD
  EXPECT_EQ(gen.result(), expect);

  // 1.4 Metal and C++14
  // The Metal programming language is a C++14-based Specification with
  // extensions and restrictions. Refer to the C++14 Specification (also known
  // as the ISO/IEC JTC1/SC22/WG21 N4431 Language Specification) for a detailed
  // description of the language grammar.
  //
  // Tint is written in C++14, so use the compiler to verify the generated
  // layout is as expected for C++14 / MSL.
  {
    struct inner {
      uint32_t a;
      alignas(512) float b;
    };
    CHECK_TYPE_SIZE_AND_ALIGN(inner, 1024, 512);

    // array_x: size(28), align(4)
    using array_x = std::array<float, 7>;
    CHECK_TYPE_SIZE_AND_ALIGN(array_x, 28, 4);

    // array_y: size(4096), align(512)
    using array_y = std::array<inner, 4>;
    CHECK_TYPE_SIZE_AND_ALIGN(array_y, 4096, 512);

    // array_z: size(4), align(4)
    using array_z = std::array<float, 1>;
    CHECK_TYPE_SIZE_AND_ALIGN(array_z, 4, 4);

    struct S {
#define FIELD(ADDR, TYPE, NAME, SUFFIX) TYPE NAME SUFFIX;
      ALL_FIELDS()
#undef FIELD
    };

#define FIELD(ADDR, TYPE, NAME, SUFFIX) \
  EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
    ALL_FIELDS()
#undef FIELD
  }

#undef ALL_FIELDS
}

TEST_F(MslGeneratorImplTest, AttemptTintPadSymbolCollision) {
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

  Global("G", ty.access(ast::AccessControl::kReadOnly, s),
         ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  auto* sem_s = program->TypeOf(s)->As<sem::Struct>();
  ASSERT_TRUE(gen.EmitStructType(sem_s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  /* 0x0000 */ int tint_pad_2;
  /* 0x0004 */ int8_t tint_pad_10[124];
  /* 0x0080 */ float tint_pad_20;
  /* 0x0084 */ int8_t tint_pad_11[124];
  /* 0x0100 */ packed_float2 tint_pad_33;
  /* 0x0108 */ uint tint_pad_1;
  /* 0x010c */ int8_t tint_pad_12[4];
  /* 0x0110 */ packed_float3 tint_pad_3;
  /* 0x011c */ uint tint_pad_7;
  /* 0x0120 */ packed_float4 tint_pad_25;
  /* 0x0130 */ uint tint_pad_5;
  /* 0x0134 */ int8_t tint_pad_13[4];
  /* 0x0138 */ float2x2 tint_pad_27;
  /* 0x0148 */ uint tint_pad_24;
  /* 0x014c */ int8_t tint_pad_14[4];
  /* 0x0150 */ float2x3 tint_pad_23;
  /* 0x0170 */ uint tint_pad_0;
  /* 0x0174 */ int8_t tint_pad_15[12];
  /* 0x0180 */ float2x4 tint_pad_8;
  /* 0x01a0 */ uint tint_pad_26;
  /* 0x01a4 */ int8_t tint_pad_16[4];
  /* 0x01a8 */ float3x2 tint_pad_29;
  /* 0x01c0 */ uint tint_pad_6;
  /* 0x01c4 */ int8_t tint_pad_17[12];
  /* 0x01d0 */ float3x3 tint_pad_22;
  /* 0x0200 */ uint tint_pad_32;
  /* 0x0204 */ int8_t tint_pad_18[12];
  /* 0x0210 */ float3x4 tint_pad_34;
  /* 0x0240 */ uint tint_pad_35;
  /* 0x0244 */ int8_t tint_pad_19[4];
  /* 0x0248 */ float4x2 tint_pad_30;
  /* 0x0268 */ uint tint_pad_9;
  /* 0x026c */ int8_t tint_pad_36[4];
  /* 0x0270 */ float4x3 tint_pad_31;
  /* 0x02b0 */ uint tint_pad_28;
  /* 0x02b4 */ int8_t tint_pad_37[12];
  /* 0x02c0 */ float4x4 tint_pad_4;
  /* 0x0300 */ float tint_pad_21;
  /* 0x0304 */ int8_t tint_pad_38[124];
};
)");
}

// TODO(crbug.com/tint/649): Add tests for array with explicit stride.

// TODO(dsinclair): How to translate [[block]]
TEST_F(MslGeneratorImplTest, DISABLED_EmitType_Struct_WithDecoration) {
  auto* s = Structure("S",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.f32()),
                      },
                      {create<ast::StructBlockDecoration>()});

  Global("G", ty.access(ast::AccessControl::kReadOnly, s),
         ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(s), "")) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct {
  /* 0x0000 */ int a;
  /* 0x0004 */ float b;
})");
}

TEST_F(MslGeneratorImplTest, EmitType_U32) {
  auto* u32 = create<sem::U32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(u32, "")) << gen.error();
  EXPECT_EQ(gen.result(), "uint");
}

TEST_F(MslGeneratorImplTest, EmitType_Vector) {
  auto* f32 = create<sem::F32>();
  auto* vec3 = create<sem::Vector>(f32, 3);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(vec3, "")) << gen.error();
  EXPECT_EQ(gen.result(), "float3");
}

TEST_F(MslGeneratorImplTest, EmitType_Void) {
  auto* void_ = create<sem::Void>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(void_, "")) << gen.error();
  EXPECT_EQ(gen.result(), "void");
}

TEST_F(MslGeneratorImplTest, EmitType_Sampler) {
  auto* sampler = create<sem::Sampler>(ast::SamplerKind::kSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(sampler, "")) << gen.error();
  EXPECT_EQ(gen.result(), "sampler");
}

TEST_F(MslGeneratorImplTest, EmitType_SamplerComparison) {
  auto* sampler = create<sem::Sampler>(ast::SamplerKind::kComparisonSampler);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(sampler, "")) << gen.error();
  EXPECT_EQ(gen.result(), "sampler");
}

struct MslDepthTextureData {
  ast::TextureDimension dim;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslDepthTextureData data) {
  out << data.dim;
  return out;
}
using MslDepthTexturesTest = TestParamHelper<MslDepthTextureData>;
TEST_P(MslDepthTexturesTest, Emit) {
  auto params = GetParam();

  sem::DepthTexture s(params.dim);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(&s, "")) << gen.error();
  EXPECT_EQ(gen.result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslDepthTexturesTest,
    testing::Values(MslDepthTextureData{ast::TextureDimension::k2d,
                                        "depth2d<float, access::sample>"},
                    MslDepthTextureData{ast::TextureDimension::k2dArray,
                                        "depth2d_array<float, access::sample>"},
                    MslDepthTextureData{ast::TextureDimension::kCube,
                                        "depthcube<float, access::sample>"},
                    MslDepthTextureData{
                        ast::TextureDimension::kCubeArray,
                        "depthcube_array<float, access::sample>"}));

struct MslTextureData {
  ast::TextureDimension dim;
  std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslTextureData data) {
  out << data.dim;
  return out;
}
using MslSampledtexturesTest = TestParamHelper<MslTextureData>;
TEST_P(MslSampledtexturesTest, Emit) {
  auto params = GetParam();

  auto* f32 = create<sem::F32>();
  auto* s = create<sem::SampledTexture>(params.dim, f32);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(s, "")) << gen.error();
  EXPECT_EQ(gen.result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslSampledtexturesTest,
    testing::Values(MslTextureData{ast::TextureDimension::k1d,
                                   "texture1d<float, access::sample>"},
                    MslTextureData{ast::TextureDimension::k2d,
                                   "texture2d<float, access::sample>"},
                    MslTextureData{ast::TextureDimension::k2dArray,
                                   "texture2d_array<float, access::sample>"},
                    MslTextureData{ast::TextureDimension::k3d,
                                   "texture3d<float, access::sample>"},
                    MslTextureData{ast::TextureDimension::kCube,
                                   "texturecube<float, access::sample>"},
                    MslTextureData{
                        ast::TextureDimension::kCubeArray,
                        "texturecube_array<float, access::sample>"}));

TEST_F(MslGeneratorImplTest, Emit_TypeMultisampledTexture) {
  auto* u32 = create<sem::U32>();
  auto* ms = create<sem::MultisampledTexture>(ast::TextureDimension::k2d, u32);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(ms, "")) << gen.error();
  EXPECT_EQ(gen.result(), "texture2d_ms<uint, access::sample>");
}

struct MslStorageTextureData {
  ast::TextureDimension dim;
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

  auto* s = ty.storage_texture(params.dim, ast::ImageFormat::kR32Float);
  auto* ac = ty.access(params.ro ? ast::AccessControl::kReadOnly
                                 : ast::AccessControl::kWriteOnly,
                       s);
  Global("test_var", ac, ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitType(program->TypeOf(ac), "")) << gen.error();
  EXPECT_EQ(gen.result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslStorageTexturesTest,
    testing::Values(
        MslStorageTextureData{ast::TextureDimension::k1d, true,
                              "texture1d<float, access::read>"},
        MslStorageTextureData{ast::TextureDimension::k2d, true,
                              "texture2d<float, access::read>"},
        MslStorageTextureData{ast::TextureDimension::k2dArray, true,
                              "texture2d_array<float, access::read>"},
        MslStorageTextureData{ast::TextureDimension::k3d, true,
                              "texture3d<float, access::read>"},
        MslStorageTextureData{ast::TextureDimension::k1d, false,
                              "texture1d<float, access::write>"},
        MslStorageTextureData{ast::TextureDimension::k2d, false,
                              "texture2d<float, access::write>"},
        MslStorageTextureData{ast::TextureDimension::k2dArray, false,
                              "texture2d_array<float, access::write>"},
        MslStorageTextureData{ast::TextureDimension::k3d, false,
                              "texture3d<float, access::write>"}));

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
