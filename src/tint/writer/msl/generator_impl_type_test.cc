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

#include "gmock/gmock.h"

#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/sampler.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/msl/test_helper.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

void FormatMSLField(utils::StringStream& out,
                    const char* addr,
                    const char* type,
                    size_t array_count,
                    const char* name) {
    out << "  /* " << std::string(addr) << " */ ";
    if (array_count == 0) {
        out << type << " ";
    } else {
        out << "tint_array<" << type << ", " << std::to_string(array_count) << "> ";
    }
    out << name << ";\n";
}

#define CHECK_TYPE_SIZE_AND_ALIGN(TYPE, SIZE, ALIGN)      \
    static_assert(sizeof(TYPE) == SIZE, "Bad type size"); \
    static_assert(alignof(TYPE) == ALIGN, "Bad type alignment")

// Declare C++ types that match the size and alignment of the types of the same
// name in MSL.
#define DECLARE_TYPE(NAME, SIZE, ALIGN) \
    struct alignas(ALIGN) NAME {        \
        uint8_t _[SIZE];                \
    };                                  \
    CHECK_TYPE_SIZE_AND_ALIGN(NAME, SIZE, ALIGN)

// Size and alignments taken from the MSL spec:
// https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
DECLARE_TYPE(float2, 8, 8);
DECLARE_TYPE(float3, 12, 4);
DECLARE_TYPE(float4, 16, 16);
DECLARE_TYPE(float2x2, 16, 8);
DECLARE_TYPE(float2x3, 32, 16);
DECLARE_TYPE(float2x4, 32, 16);
DECLARE_TYPE(float3x2, 24, 8);
DECLARE_TYPE(float3x3, 48, 16);
DECLARE_TYPE(float3x4, 48, 16);
DECLARE_TYPE(float4x2, 32, 8);
DECLARE_TYPE(float4x3, 64, 16);
DECLARE_TYPE(float4x4, 64, 16);
DECLARE_TYPE(half2, 4, 4);
DECLARE_TYPE(packed_half3, 6, 2);
DECLARE_TYPE(half4, 8, 8);
DECLARE_TYPE(half2x2, 8, 4);
DECLARE_TYPE(half2x3, 16, 8);
DECLARE_TYPE(half2x4, 16, 8);
DECLARE_TYPE(half3x2, 12, 4);
DECLARE_TYPE(half3x3, 24, 8);
DECLARE_TYPE(half3x4, 24, 8);
DECLARE_TYPE(half4x2, 16, 4);
DECLARE_TYPE(half4x3, 32, 8);
DECLARE_TYPE(half4x4, 32, 8);
using uint = unsigned int;

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitType_Array) {
    auto arr = ty.array<bool, 4>();
    ast::Type type = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(type), "ary")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "tint_array<bool, 4>");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArray) {
    auto a = ty.array<bool, 4>();
    auto b = ty.array(a, 5_u);
    ast::Type type = GlobalVar("G", b, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(type), "ary")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "tint_array<tint_array<bool, 4>, 5>");
}

TEST_F(MslGeneratorImplTest, EmitType_ArrayOfArrayOfArray) {
    auto a = ty.array<bool, 4>();
    auto b = ty.array(a, 5_u);
    auto c = ty.array(b, 6_u);
    ast::Type type = GlobalVar("G", c, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(type), "ary")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "tint_array<tint_array<tint_array<bool, 4>, 5>, 6>");
}

TEST_F(MslGeneratorImplTest, EmitType_Array_WithoutName) {
    auto arr = ty.array<bool, 4>();
    ast::Type type = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(type), "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "tint_array<bool, 4>");
}

TEST_F(MslGeneratorImplTest, EmitType_RuntimeArray) {
    auto arr = ty.array<bool, 1>();
    ast::Type type = GlobalVar("G", arr, builtin::AddressSpace::kPrivate)->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(type), "ary")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "tint_array<bool, 1>");
}

TEST_F(MslGeneratorImplTest, EmitType_Bool) {
    auto* bool_ = create<type::Bool>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, bool_, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "bool");
}

TEST_F(MslGeneratorImplTest, EmitType_F32) {
    auto* f32 = create<type::F32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, f32, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float");
}

TEST_F(MslGeneratorImplTest, EmitType_F16) {
    auto* f16 = create<type::F16>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, f16, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "half");
}

TEST_F(MslGeneratorImplTest, EmitType_I32) {
    auto* i32 = create<type::I32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, i32, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "int");
}

TEST_F(MslGeneratorImplTest, EmitType_Matrix_F32) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, mat2x3, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float2x3");
}

TEST_F(MslGeneratorImplTest, EmitType_Matrix_F16) {
    auto* f16 = create<type::F16>();
    auto* vec3 = create<type::Vector>(f16, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, mat2x3, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "half2x3");
}

TEST_F(MslGeneratorImplTest, EmitType_Pointer) {
    auto* f32 = create<type::F32>();
    auto* p =
        create<type::Pointer>(f32, builtin::AddressSpace::kWorkgroup, builtin::Access::kReadWrite);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, p, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "threadgroup float* ");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(s), "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "S");
}

TEST_F(MslGeneratorImplTest, EmitType_StructDecl) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });

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

TEST_F(MslGeneratorImplTest, EmitType_Struct_Layout_NonComposites) {
    auto* s = Structure(
        "S", utils::Vector{
                 Member("a", ty.i32(), utils::Vector{MemberSize(32_a)}),
                 Member("b", ty.f32(), utils::Vector{MemberAlign(128_i), MemberSize(128_a)}),
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
             });

    ast::Type type = GlobalVar("G", ty.Of(s), builtin::AddressSpace::kStorage,
                               builtin::Access::kRead, Binding(0_a), Group(0_a))
                         ->type;

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(type)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();

    // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, ARRAY_COUNT, NAME)
    // for each field of the structure s.
#define ALL_FIELDS()                       \
    FIELD(0x0000, int, 0, a)               \
    FIELD(0x0004, int8_t, 124, tint_pad)   \
    FIELD(0x0080, float, 0, b)             \
    FIELD(0x0084, int8_t, 124, tint_pad_1) \
    FIELD(0x0100, float2, 0, c)            \
    FIELD(0x0108, uint, 0, d)              \
    FIELD(0x010c, int8_t, 4, tint_pad_2)   \
    FIELD(0x0110, float3, 0, e)            \
    FIELD(0x011c, uint, 0, f)              \
    FIELD(0x0120, float4, 0, g)            \
    FIELD(0x0130, uint, 0, h)              \
    FIELD(0x0134, int8_t, 4, tint_pad_3)   \
    FIELD(0x0138, float2x2, 0, i)          \
    FIELD(0x0148, uint, 0, j)              \
    FIELD(0x014c, int8_t, 4, tint_pad_4)   \
    FIELD(0x0150, float2x3, 0, k)          \
    FIELD(0x0170, uint, 0, l)              \
    FIELD(0x0174, int8_t, 12, tint_pad_5)  \
    FIELD(0x0180, float2x4, 0, m)          \
    FIELD(0x01a0, uint, 0, n)              \
    FIELD(0x01a4, int8_t, 4, tint_pad_6)   \
    FIELD(0x01a8, float3x2, 0, o)          \
    FIELD(0x01c0, uint, 0, p)              \
    FIELD(0x01c4, int8_t, 12, tint_pad_7)  \
    FIELD(0x01d0, float3x3, 0, q)          \
    FIELD(0x0200, uint, 0, r)              \
    FIELD(0x0204, int8_t, 12, tint_pad_8)  \
    FIELD(0x0210, float3x4, 0, s)          \
    FIELD(0x0240, uint, 0, t)              \
    FIELD(0x0244, int8_t, 4, tint_pad_9)   \
    FIELD(0x0248, float4x2, 0, u)          \
    FIELD(0x0268, uint, 0, v)              \
    FIELD(0x026c, int8_t, 4, tint_pad_10)  \
    FIELD(0x0270, float4x3, 0, w)          \
    FIELD(0x02b0, uint, 0, x)              \
    FIELD(0x02b4, int8_t, 12, tint_pad_11) \
    FIELD(0x02c0, float4x4, 0, y)          \
    FIELD(0x0300, float, 0, z)             \
    FIELD(0x0304, int8_t, 124, tint_pad_12)

    // Check that the generated string is as expected.
    utils::StringStream expect;
    expect << "struct S {\n";
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    FormatMSLField(expect, #ADDR, #TYPE, ARRAY_COUNT, #NAME);
    ALL_FIELDS()
#undef FIELD
    expect << "};\n";
    EXPECT_EQ(buf.String(), expect.str());

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
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) std::array<TYPE, ARRAY_COUNT ? ARRAY_COUNT : 1> NAME;
            ALL_FIELDS()
#undef FIELD
        };

#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
        ALL_FIELDS()
#undef FIELD
    }
#undef ALL_FIELDS
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_Layout_Structures) {
    // inner_x: size(1024), align(512)
    auto* inner_x =
        Structure("inner_x", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32(), utils::Vector{MemberAlign(512_i)}),
                             });

    // inner_y: size(516), align(4)
    auto* inner_y =
        Structure("inner_y", utils::Vector{
                                 Member("a", ty.i32(), utils::Vector{MemberSize(512_a)}),
                                 Member("b", ty.f32()),
                             });

    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.Of(inner_x)),
                                 Member("c", ty.f32()),
                                 Member("d", ty.Of(inner_y)),
                                 Member("e", ty.f32()),
                             });

    ast::Type type = GlobalVar("G", ty.Of(s), builtin::AddressSpace::kStorage,
                               builtin::Access::kRead, Binding(0_a), Group(0_a))
                         ->type;

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(type)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();

    // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, ARRAY_COUNT, NAME)
    // for each field of the structure s.
#define ALL_FIELDS()                     \
    FIELD(0x0000, int, 0, a)             \
    FIELD(0x0004, int8_t, 508, tint_pad) \
    FIELD(0x0200, inner_x, 0, b)         \
    FIELD(0x0600, float, 0, c)           \
    FIELD(0x0604, inner_y, 0, d)         \
    FIELD(0x0808, float, 0, e)           \
    FIELD(0x080c, int8_t, 500, tint_pad_1)

    // Check that the generated string is as expected.
    utils::StringStream expect;
    expect << "struct S {\n";
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    FormatMSLField(expect, #ADDR, #TYPE, ARRAY_COUNT, #NAME);
    ALL_FIELDS()
#undef FIELD
    expect << "};\n";
    EXPECT_EQ(buf.String(), expect.str());

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
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) std::array<TYPE, ARRAY_COUNT ? ARRAY_COUNT : 1> NAME;
            ALL_FIELDS()
#undef FIELD
        };

#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
        ALL_FIELDS()
#undef FIELD
    }

#undef ALL_FIELDS
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_Layout_ArrayDefaultStride) {
    // inner: size(1024), align(512)
    auto* inner = Structure("inner", utils::Vector{
                                         Member("a", ty.i32()),
                                         Member("b", ty.f32(), utils::Vector{MemberAlign(512_i)}),
                                     });

    // array_x: size(28), align(4)
    auto array_x = ty.array<f32, 7>();

    // array_y: size(4096), align(512)
    auto array_y = ty.array(ty.Of(inner), 4_u);

    // array_z: size(4), align(4)
    auto array_z = ty.array<f32>();

    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", array_x),
                                 Member("c", ty.f32()),
                                 Member("d", array_y),
                                 Member("e", ty.f32()),
                                 Member("f", array_z),
                             });

    ast::Type type = GlobalVar("G", ty.Of(s), builtin::AddressSpace::kStorage,
                               builtin::Access::kRead, Binding(0_a), Group(0_a))
                         ->type;

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(type)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();

    // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, ARRAY_COUNT, NAME)
    // for each field of the structure s.
#define ALL_FIELDS()                     \
    FIELD(0x0000, int, 0, a)             \
    FIELD(0x0004, float, 7, b)           \
    FIELD(0x0020, float, 0, c)           \
    FIELD(0x0024, int8_t, 476, tint_pad) \
    FIELD(0x0200, inner, 4, d)           \
    FIELD(0x1200, float, 0, e)           \
    FIELD(0x1204, float, 1, f)           \
    FIELD(0x1208, int8_t, 504, tint_pad_1)

    // Check that the generated string is as expected.
    utils::StringStream expect;
    expect << "struct S {\n";
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    FormatMSLField(expect, #ADDR, #TYPE, ARRAY_COUNT, #NAME);
    ALL_FIELDS()
#undef FIELD
    expect << "};\n";
    EXPECT_EQ(buf.String(), expect.str());

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
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) std::array<TYPE, ARRAY_COUNT ? ARRAY_COUNT : 1> NAME;
            ALL_FIELDS()
#undef FIELD
        };

#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    EXPECT_EQ(ADDR, static_cast<int>(offsetof(S, NAME))) << "Field " << #NAME;
        ALL_FIELDS()
#undef FIELD
    }

#undef ALL_FIELDS
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_Layout_ArrayVec3DefaultStride) {
    // array: size(64), align(16)
    auto array = ty.array(ty.vec3<f32>(), 4_u);

    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", array),
                                 Member("c", ty.i32()),
                             });

    ast::Type type = GlobalVar("G", ty.Of(s), builtin::AddressSpace::kStorage,
                               builtin::Access::kRead, Binding(0_a), Group(0_a))
                         ->type;

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(type)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();

    // ALL_FIELDS() calls the macro FIELD(ADDR, TYPE, ARRAY_COUNT, NAME)
    // for each field of the structure s.
#define ALL_FIELDS()                    \
    FIELD(0x0000, int, 0, a)            \
    FIELD(0x0004, int8_t, 12, tint_pad) \
    FIELD(0x0010, float3, 4, b)         \
    FIELD(0x0050, int, 0, c)            \
    FIELD(0x0054, int8_t, 12, tint_pad_1)

    // Check that the generated string is as expected.
    utils::StringStream expect;
    expect << "struct S {\n";
#define FIELD(ADDR, TYPE, ARRAY_COUNT, NAME) \
    FormatMSLField(expect, #ADDR, #TYPE, ARRAY_COUNT, #NAME);
    ALL_FIELDS()
#undef FIELD
    expect << "};\n";
    EXPECT_EQ(buf.String(), expect.str());
}

TEST_F(MslGeneratorImplTest, AttemptTintPadSymbolCollision) {
    auto* s = Structure("S", utils::Vector{
                                 // uses symbols tint_pad_[0..9] and tint_pad_[20..35]
                                 Member("tint_pad_2", ty.i32(), utils::Vector{MemberSize(32_a)}),
                                 Member("tint_pad_20", ty.f32(),
                                        utils::Vector{MemberAlign(128_i), MemberSize(128_u)}),
                                 Member("tint_pad_33", ty.vec2<f32>()),
                                 Member("tint_pad_1", ty.u32()),
                                 Member("tint_pad_3", ty.vec3<f32>()),
                                 Member("tint_pad_7", ty.u32()),
                                 Member("tint_pad_25", ty.vec4<f32>()),
                                 Member("tint_pad_5", ty.u32()),
                                 Member("tint_pad_27", ty.mat2x2<f32>()),
                                 Member("tint_pad_24", ty.u32()),
                                 Member("tint_pad_23", ty.mat2x3<f32>()),
                                 Member("tint_pad", ty.u32()),
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
                             });

    ast::Type type = GlobalVar("G", ty.Of(s), builtin::AddressSpace::kStorage,
                               builtin::Access::kRead, Binding(0_a), Group(0_a))
                         ->type;

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(type)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();
    EXPECT_EQ(buf.String(), R"(struct S {
  /* 0x0000 */ int tint_pad_2;
  /* 0x0004 */ tint_array<int8_t, 124> tint_pad_10;
  /* 0x0080 */ float tint_pad_20;
  /* 0x0084 */ tint_array<int8_t, 124> tint_pad_11;
  /* 0x0100 */ float2 tint_pad_33;
  /* 0x0108 */ uint tint_pad_1;
  /* 0x010c */ tint_array<int8_t, 4> tint_pad_12;
  /* 0x0110 */ float3 tint_pad_3;
  /* 0x011c */ uint tint_pad_7;
  /* 0x0120 */ float4 tint_pad_25;
  /* 0x0130 */ uint tint_pad_5;
  /* 0x0134 */ tint_array<int8_t, 4> tint_pad_13;
  /* 0x0138 */ float2x2 tint_pad_27;
  /* 0x0148 */ uint tint_pad_24;
  /* 0x014c */ tint_array<int8_t, 4> tint_pad_14;
  /* 0x0150 */ float2x3 tint_pad_23;
  /* 0x0170 */ uint tint_pad;
  /* 0x0174 */ tint_array<int8_t, 12> tint_pad_15;
  /* 0x0180 */ float2x4 tint_pad_8;
  /* 0x01a0 */ uint tint_pad_26;
  /* 0x01a4 */ tint_array<int8_t, 4> tint_pad_16;
  /* 0x01a8 */ float3x2 tint_pad_29;
  /* 0x01c0 */ uint tint_pad_6;
  /* 0x01c4 */ tint_array<int8_t, 12> tint_pad_17;
  /* 0x01d0 */ float3x3 tint_pad_22;
  /* 0x0200 */ uint tint_pad_32;
  /* 0x0204 */ tint_array<int8_t, 12> tint_pad_18;
  /* 0x0210 */ float3x4 tint_pad_34;
  /* 0x0240 */ uint tint_pad_35;
  /* 0x0244 */ tint_array<int8_t, 4> tint_pad_19;
  /* 0x0248 */ float4x2 tint_pad_30;
  /* 0x0268 */ uint tint_pad_9;
  /* 0x026c */ tint_array<int8_t, 4> tint_pad_36;
  /* 0x0270 */ float4x3 tint_pad_31;
  /* 0x02b0 */ uint tint_pad_28;
  /* 0x02b4 */ tint_array<int8_t, 12> tint_pad_37;
  /* 0x02c0 */ float4x4 tint_pad_4;
  /* 0x0300 */ float tint_pad_21;
  /* 0x0304 */ tint_array<int8_t, 124> tint_pad_38;
};
)");
}

TEST_F(MslGeneratorImplTest, EmitType_Struct_WithAttribute) {
    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                             });

    ast::Type type = GlobalVar("G", ty.Of(s), builtin::AddressSpace::kStorage,
                               builtin::Access::kRead, Binding(0_a), Group(0_a))
                         ->type;

    GeneratorImpl& gen = Build();

    TextGenerator::TextBuffer buf;
    auto* str = program->TypeOf(type)->As<type::Struct>();
    ASSERT_TRUE(gen.EmitStructType(&buf, str)) << gen.Diagnostics();
    EXPECT_EQ(buf.String(), R"(struct S {
  /* 0x0000 */ int a;
  /* 0x0004 */ float b;
};
)");
}

TEST_F(MslGeneratorImplTest, EmitType_U32) {
    auto* u32 = create<type::U32>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, u32, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "uint");
}

TEST_F(MslGeneratorImplTest, EmitType_Vector) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, vec3, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float3");
}

TEST_F(MslGeneratorImplTest, EmitType_Void) {
    auto* void_ = create<type::Void>();

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, void_, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "void");
}

TEST_F(MslGeneratorImplTest, EmitType_Sampler) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, sampler, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "sampler");
}

TEST_F(MslGeneratorImplTest, EmitType_SamplerComparison) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kComparisonSampler);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, sampler, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "sampler");
}

struct MslDepthTextureData {
    type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslDepthTextureData data) {
    utils::StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using MslDepthTexturesTest = TestParamHelper<MslDepthTextureData>;
TEST_P(MslDepthTexturesTest, Emit) {
    auto params = GetParam();

    type::DepthTexture s(params.dim);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, &s, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslDepthTexturesTest,
    testing::Values(
        MslDepthTextureData{type::TextureDimension::k2d, "depth2d<float, access::sample>"},
        MslDepthTextureData{type::TextureDimension::k2dArray,
                            "depth2d_array<float, access::sample>"},
        MslDepthTextureData{type::TextureDimension::kCube, "depthcube<float, access::sample>"},
        MslDepthTextureData{type::TextureDimension::kCubeArray,
                            "depthcube_array<float, access::sample>"}));

using MslDepthMultisampledTexturesTest = TestHelper;
TEST_F(MslDepthMultisampledTexturesTest, Emit) {
    type::DepthMultisampledTexture s(type::TextureDimension::k2d);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, &s, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "depth2d_ms<float, access::read>");
}

struct MslTextureData {
    type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslTextureData data) {
    utils::StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using MslSampledtexturesTest = TestParamHelper<MslTextureData>;
TEST_P(MslSampledtexturesTest, Emit) {
    auto params = GetParam();

    auto* f32 = create<type::F32>();
    auto* s = create<type::SampledTexture>(params.dim, f32);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, s, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslSampledtexturesTest,
    testing::Values(
        MslTextureData{type::TextureDimension::k1d, "texture1d<float, access::sample>"},
        MslTextureData{type::TextureDimension::k2d, "texture2d<float, access::sample>"},
        MslTextureData{type::TextureDimension::k2dArray, "texture2d_array<float, access::sample>"},
        MslTextureData{type::TextureDimension::k3d, "texture3d<float, access::sample>"},
        MslTextureData{type::TextureDimension::kCube, "texturecube<float, access::sample>"},
        MslTextureData{type::TextureDimension::kCubeArray,
                       "texturecube_array<float, access::sample>"}));

TEST_F(MslGeneratorImplTest, Emit_TypeMultisampledTexture) {
    auto* u32 = create<type::U32>();
    auto* ms = create<type::MultisampledTexture>(type::TextureDimension::k2d, u32);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, ms, "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "texture2d_ms<uint, access::read>");
}

struct MslStorageTextureData {
    type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, MslStorageTextureData data) {
    utils::StringStream str;
    str << data.dim;
    return out << str.str();
}
using MslStorageTexturesTest = TestParamHelper<MslStorageTextureData>;
TEST_P(MslStorageTexturesTest, Emit) {
    auto params = GetParam();

    auto s =
        ty.storage_texture(params.dim, builtin::TexelFormat::kR32Float, builtin::Access::kWrite);
    ast::Type type = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitType(out, program->TypeOf(type), "")) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslStorageTexturesTest,
    testing::Values(
        MslStorageTextureData{type::TextureDimension::k1d, "texture1d<float, access::write>"},
        MslStorageTextureData{type::TextureDimension::k2d, "texture2d<float, access::write>"},
        MslStorageTextureData{type::TextureDimension::k2dArray,
                              "texture2d_array<float, access::write>"},
        MslStorageTextureData{type::TextureDimension::k3d, "texture3d<float, access::write>"}));

}  // namespace
}  // namespace tint::writer::msl
