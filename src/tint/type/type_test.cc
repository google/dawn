// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/array_count.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/test_helper.h"

namespace tint::type {
namespace {

struct TypeTest : public TestHelper {
    const AbstractFloat* af = create<AbstractFloat>();
    const AbstractInt* ai = create<AbstractInt>();
    const F32* f32 = create<F32>();
    const F16* f16 = create<F16>();
    const I32* i32 = create<I32>();
    const U32* u32 = create<U32>();
    const Vector* vec2_f32 = create<Vector>(f32, 2u);
    const Vector* vec3_f32 = create<Vector>(f32, 3u);
    const Vector* vec3_f16 = create<Vector>(f16, 3u);
    const Vector* vec4_f32 = create<Vector>(f32, 4u);
    const Vector* vec3_u32 = create<Vector>(u32, 3u);
    const Vector* vec3_i32 = create<Vector>(i32, 3u);
    const Vector* vec3_af = create<Vector>(af, 3u);
    const Vector* vec3_ai = create<Vector>(ai, 3u);
    const Matrix* mat2x4_f32 = create<Matrix>(vec4_f32, 2u);
    const Matrix* mat3x4_f32 = create<Matrix>(vec4_f32, 3u);
    const Matrix* mat4x2_f32 = create<Matrix>(vec2_f32, 4u);
    const Matrix* mat4x3_f32 = create<Matrix>(vec3_f32, 4u);
    const Matrix* mat4x3_f16 = create<Matrix>(vec3_f16, 4u);
    const Matrix* mat4x3_af = create<Matrix>(vec3_af, 4u);
    const Reference* ref_u32 =
        create<Reference>(u32, builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite);
    const Struct* str_f32 = create<Struct>(Sym("str_f32"),
                                           utils::Vector{
                                               create<StructMember>(
                                                   /* name */ Sym("x"),
                                                   /* type */ f32,
                                                   /* index */ 0u,
                                                   /* offset */ 0u,
                                                   /* align */ 4u,
                                                   /* size */ 4u,
                                                   /* attributes */ type::StructMemberAttributes{}),
                                           },
                                           /* align*/ 4u,
                                           /* size*/ 4u,
                                           /* size_no_padding*/ 4u);
    const Struct* str_f16 = create<Struct>(Sym("str_f16"),
                                           utils::Vector{
                                               create<StructMember>(
                                                   /* name */ Sym("x"),
                                                   /* type */ f16,
                                                   /* index */ 0u,
                                                   /* offset */ 0u,
                                                   /* align */ 4u,
                                                   /* size */ 4u,
                                                   /* attributes */ type::StructMemberAttributes{}),
                                           },
                                           /* align*/ 4u,
                                           /* size*/ 4u,
                                           /* size_no_padding*/ 4u);
    Struct* str_af = create<Struct>(Sym("str_af"),
                                    utils::Vector{
                                        create<StructMember>(
                                            /* name */ Sym("x"),
                                            /* type */ af,
                                            /* index */ 0u,
                                            /* offset */ 0u,
                                            /* align */ 4u,
                                            /* size */ 4u,
                                            /* attributes */ type::StructMemberAttributes{}),
                                    },
                                    /* align*/ 4u,
                                    /* size*/ 4u,
                                    /* size_no_padding*/ 4u);
    const Array* arr_i32 = create<Array>(
        /* element */ i32,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);
    const Array* arr_ai = create<Array>(
        /* element */ ai,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);
    const Array* arr_vec3_i32 = create<Array>(
        /* element */ vec3_i32,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 16u,
        /* size */ 5u * 16u,
        /* stride */ 5u * 16u,
        /* implicit_stride */ 5u * 16u);
    const Array* arr_vec3_ai = create<Array>(
        /* element */ vec3_ai,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 16u,
        /* size */ 5u * 16u,
        /* stride */ 5u * 16u,
        /* implicit_stride */ 5u * 16u);
    const Array* arr_mat4x3_f16 = create<Array>(
        /* element */ mat4x3_f16,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 32u,
        /* size */ 5u * 32u,
        /* stride */ 5u * 32u,
        /* implicit_stride */ 5u * 32u);
    const Array* arr_mat4x3_f32 = create<Array>(
        /* element */ mat4x3_f32,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 64u,
        /* size */ 5u * 64u,
        /* stride */ 5u * 64u,
        /* implicit_stride */ 5u * 64u);
    const Array* arr_mat4x3_af = create<Array>(
        /* element */ mat4x3_af,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 64u,
        /* size */ 5u * 64u,
        /* stride */ 5u * 64u,
        /* implicit_stride */ 5u * 64u);
    const Array* arr_str_f16 = create<Array>(
        /* element */ str_f16,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);
    const Array* arr_str_af = create<Array>(
        /* element */ str_af,
        /* count */ create<ConstantArrayCount>(5u),
        /* align */ 4u,
        /* size */ 5u * 4u,
        /* stride */ 5u * 4u,
        /* implicit_stride */ 5u * 4u);

    TypeTest() { str_af->SetConcreteTypes(utils::Vector{str_f32, str_f16}); }
};

TEST_F(TypeTest, ConversionRank) {
    EXPECT_EQ(Type::ConversionRank(i32, i32), 0u);
    EXPECT_EQ(Type::ConversionRank(f32, f32), 0u);
    EXPECT_EQ(Type::ConversionRank(u32, u32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_f32, vec3_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_f16, vec3_f16), 0u);
    EXPECT_EQ(Type::ConversionRank(vec4_f32, vec4_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_u32, vec3_u32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_i32, vec3_i32), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_af, vec3_af), 0u);
    EXPECT_EQ(Type::ConversionRank(vec3_ai, vec3_ai), 0u);
    EXPECT_EQ(Type::ConversionRank(mat3x4_f32, mat3x4_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f32, mat4x3_f32), 0u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f16, mat4x3_f16), 0u);
    EXPECT_EQ(Type::ConversionRank(arr_vec3_ai, arr_vec3_ai), 0u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_f16, arr_mat4x3_f16), 0u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_af, mat4x3_af), 0u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_af, arr_mat4x3_af), 0u);
    EXPECT_EQ(Type::ConversionRank(ref_u32, u32), 0u);

    EXPECT_EQ(Type::ConversionRank(af, f32), 1u);
    EXPECT_EQ(Type::ConversionRank(vec3_af, vec3_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_af, mat4x3_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_af, arr_mat4x3_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(af, f16), 2u);
    EXPECT_EQ(Type::ConversionRank(vec3_af, vec3_f16), 2u);
    EXPECT_EQ(Type::ConversionRank(mat4x3_af, mat4x3_f16), 2u);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_af, arr_mat4x3_f16), 2u);
    EXPECT_EQ(Type::ConversionRank(ai, i32), 3u);
    EXPECT_EQ(Type::ConversionRank(vec3_ai, vec3_i32), 3u);
    EXPECT_EQ(Type::ConversionRank(arr_ai, arr_i32), 3u);
    EXPECT_EQ(Type::ConversionRank(arr_vec3_ai, arr_vec3_i32), 3u);
    EXPECT_EQ(Type::ConversionRank(ai, u32), 4u);
    EXPECT_EQ(Type::ConversionRank(vec3_ai, vec3_u32), 4u);
    EXPECT_EQ(Type::ConversionRank(ai, af), 5u);
    EXPECT_EQ(Type::ConversionRank(ai, f32), 6u);
    EXPECT_EQ(Type::ConversionRank(ai, f16), 7u);
    EXPECT_EQ(Type::ConversionRank(str_af, str_f32), 1u);
    EXPECT_EQ(Type::ConversionRank(str_af, str_f16), 2u);

    EXPECT_EQ(Type::ConversionRank(i32, f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f32, u32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(u32, i32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(vec3_u32, vec3_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(vec3_f32, vec4_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat3x4_f32, mat4x3_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f32, mat3x4_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f32, mat4x3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(arr_vec3_i32, arr_vec3_ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_f32, arr_mat4x3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(arr_mat4x3_f16, arr_mat4x3_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f32, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f16, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(vec3_f16, vec3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(mat4x3_f16, mat4x3_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(i32, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(u32, af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(af, ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f32, ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(f16, ai), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f32, str_f16), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f16, str_f32), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f32, str_af), Type::kNoConversion);
    EXPECT_EQ(Type::ConversionRank(str_f16, str_af), Type::kNoConversion);
}

TEST_F(TypeTest, ElementOf) {
    // No count
    EXPECT_TYPE(Type::ElementOf(f32), f32);
    EXPECT_TYPE(Type::ElementOf(f16), f16);
    EXPECT_TYPE(Type::ElementOf(i32), i32);
    EXPECT_TYPE(Type::ElementOf(u32), u32);
    EXPECT_TYPE(Type::ElementOf(vec2_f32), f32);
    EXPECT_TYPE(Type::ElementOf(vec3_f16), f16);
    EXPECT_TYPE(Type::ElementOf(vec4_f32), f32);
    EXPECT_TYPE(Type::ElementOf(vec3_u32), u32);
    EXPECT_TYPE(Type::ElementOf(vec3_i32), i32);
    EXPECT_TYPE(Type::ElementOf(mat2x4_f32), vec4_f32);
    EXPECT_TYPE(Type::ElementOf(mat4x2_f32), vec2_f32);
    EXPECT_TYPE(Type::ElementOf(mat4x3_f16), vec3_f16);
    EXPECT_TYPE(Type::ElementOf(str_f16), str_f16);
    EXPECT_TYPE(Type::ElementOf(arr_i32), i32);
    EXPECT_TYPE(Type::ElementOf(arr_vec3_i32), vec3_i32);
    EXPECT_TYPE(Type::ElementOf(arr_mat4x3_f16), mat4x3_f16);
    EXPECT_TYPE(Type::ElementOf(arr_mat4x3_af), mat4x3_af);
    EXPECT_TYPE(Type::ElementOf(arr_str_f16), str_f16);

    // With count
    uint32_t count = 42;
    EXPECT_TYPE(Type::ElementOf(f32, &count), f32);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(f16, &count), f16);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(i32, &count), i32);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(u32, &count), u32);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(vec2_f32, &count), f32);
    EXPECT_EQ(count, 2u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(vec3_f16, &count), f16);
    EXPECT_EQ(count, 3u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(vec4_f32, &count), f32);
    EXPECT_EQ(count, 4u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(vec3_u32, &count), u32);
    EXPECT_EQ(count, 3u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(vec3_i32, &count), i32);
    EXPECT_EQ(count, 3u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(mat2x4_f32, &count), vec4_f32);
    EXPECT_EQ(count, 2u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(mat4x2_f32, &count), vec2_f32);
    EXPECT_EQ(count, 4u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(mat4x3_f16, &count), vec3_f16);
    EXPECT_EQ(count, 4u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(str_f16, &count), str_f16);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(arr_i32, &count), i32);
    EXPECT_EQ(count, 5u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(arr_vec3_i32, &count), vec3_i32);
    EXPECT_EQ(count, 5u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(arr_mat4x3_f16, &count), mat4x3_f16);
    EXPECT_EQ(count, 5u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(arr_mat4x3_af, &count), mat4x3_af);
    EXPECT_EQ(count, 5u);
    count = 42;
    EXPECT_TYPE(Type::ElementOf(arr_str_f16, &count), str_f16);
    EXPECT_EQ(count, 5u);
}

TEST_F(TypeTest, DeepestElementOf) {
    // No count
    EXPECT_TYPE(Type::DeepestElementOf(f32), f32);
    EXPECT_TYPE(Type::DeepestElementOf(f16), f16);
    EXPECT_TYPE(Type::DeepestElementOf(i32), i32);
    EXPECT_TYPE(Type::DeepestElementOf(u32), u32);
    EXPECT_TYPE(Type::DeepestElementOf(vec2_f32), f32);
    EXPECT_TYPE(Type::DeepestElementOf(vec3_f16), f16);
    EXPECT_TYPE(Type::DeepestElementOf(vec4_f32), f32);
    EXPECT_TYPE(Type::DeepestElementOf(vec3_u32), u32);
    EXPECT_TYPE(Type::DeepestElementOf(vec3_i32), i32);
    EXPECT_TYPE(Type::DeepestElementOf(mat2x4_f32), f32);
    EXPECT_TYPE(Type::DeepestElementOf(mat4x2_f32), f32);
    EXPECT_TYPE(Type::DeepestElementOf(mat4x3_f16), f16);
    EXPECT_TYPE(Type::DeepestElementOf(str_f16), str_f16);
    EXPECT_TYPE(Type::DeepestElementOf(arr_i32), i32);
    EXPECT_TYPE(Type::DeepestElementOf(arr_vec3_i32), i32);
    EXPECT_TYPE(Type::DeepestElementOf(arr_mat4x3_f16), f16);
    EXPECT_TYPE(Type::DeepestElementOf(arr_mat4x3_af), af);
    EXPECT_TYPE(Type::DeepestElementOf(arr_str_f16), str_f16);

    // With count
    uint32_t count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(f32, &count), f32);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(f16, &count), f16);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(i32, &count), i32);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(u32, &count), u32);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(vec2_f32, &count), f32);
    EXPECT_EQ(count, 2u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(vec3_f16, &count), f16);
    EXPECT_EQ(count, 3u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(vec4_f32, &count), f32);
    EXPECT_EQ(count, 4u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(vec3_u32, &count), u32);
    EXPECT_EQ(count, 3u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(vec3_i32, &count), i32);
    EXPECT_EQ(count, 3u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(mat2x4_f32, &count), f32);
    EXPECT_EQ(count, 8u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(mat4x2_f32, &count), f32);
    EXPECT_EQ(count, 8u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(mat4x3_f16, &count), f16);
    EXPECT_EQ(count, 12u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(str_f16, &count), str_f16);
    EXPECT_EQ(count, 1u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(arr_i32, &count), i32);
    EXPECT_EQ(count, 5u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(arr_vec3_i32, &count), i32);
    EXPECT_EQ(count, 15u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(arr_mat4x3_f16, &count), f16);
    EXPECT_EQ(count, 60u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(arr_mat4x3_af, &count), af);
    EXPECT_EQ(count, 60u);
    count = 42;
    EXPECT_TYPE(Type::DeepestElementOf(arr_str_f16, &count), str_f16);
    EXPECT_EQ(count, 5u);
}

TEST_F(TypeTest, Common2) {
    EXPECT_TYPE(Type::Common(utils::Vector{ai, ai}), ai);
    EXPECT_TYPE(Type::Common(utils::Vector{af, af}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{f32, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{i32, i32}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, u32}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{i32, u32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{f32, f16}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, i32}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{ai, af}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, i32}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, u32}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{af, ai}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{f32, ai}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, ai}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{i32, ai}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, ai}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{ai, af}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{f32, af}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, af}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{i32, af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, af}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{af, ai}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{af, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{af, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{af, i32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{af, u32}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_ai}), vec3_ai);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_af}), vec3_af);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f32, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f16, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec4_f32, vec4_f32}), vec4_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_u32, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_i32, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f32, vec3_ai}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f16, vec3_ai}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec4_f32, vec3_ai}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_u32, vec3_ai}), vec3_u32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_i32, vec3_ai}), vec3_i32);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_u32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_i32}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f32, vec3_af}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f16, vec3_af}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec4_f32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_u32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_i32, vec3_af}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat4x3_af}), mat4x3_af);
    EXPECT_TYPE(Type::Common(utils::Vector{mat3x4_f32, mat3x4_f32}), mat3x4_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f32, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f16, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat3x4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(utils::Vector{mat3x4_f32, mat4x3_af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f32, mat4x3_af}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f16, mat4x3_af}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(utils::Vector{arr_mat4x3_f32, arr_mat4x3_f16}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{arr_mat4x3_f32, arr_mat4x3_af}), arr_mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{arr_mat4x3_f16, arr_mat4x3_af}), arr_mat4x3_f16);
}

TEST_F(TypeTest, Common3) {
    EXPECT_TYPE(Type::Common(utils::Vector{ai, ai, ai}), ai);
    EXPECT_TYPE(Type::Common(utils::Vector{af, af, af}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{f32, f32, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, f16, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{i32, i32, i32}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, u32, u32}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{ai, af, ai}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, f32, ai}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, f16, ai}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, i32, ai}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, u32, ai}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{af, ai, af}), af);
    EXPECT_TYPE(Type::Common(utils::Vector{f32, ai, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, ai, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{i32, ai, i32}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, ai, u32}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{ai, f32, ai}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, f16, ai}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, i32, ai}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, u32, ai}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{f32, ai, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, ai, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{i32, ai, i32}), i32);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, ai, u32}), u32);

    EXPECT_TYPE(Type::Common(utils::Vector{af, f32, af}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{af, f16, af}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{af, i32, af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{af, u32, af}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{f32, af, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{f16, af, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{i32, af, i32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{u32, af, u32}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{ai, af, f32}), f32);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, af, f16}), f16);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, af, i32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{ai, af, u32}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_ai, vec3_ai}), vec3_ai);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_af, vec3_af}), vec3_af);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f32, vec3_f32, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f16, vec3_f16, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec4_f32, vec4_f32, vec4_f32}), vec4_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_u32, vec3_u32, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_i32, vec3_i32, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f32, vec3_ai, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f16, vec3_ai, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec4_f32, vec3_ai, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_u32, vec3_ai, vec3_u32}), vec3_u32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_i32, vec3_ai, vec3_i32}), vec3_i32);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_f32, vec3_ai}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_f16, vec3_ai}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec4_f32, vec3_ai}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_u32, vec3_ai}), vec3_u32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_i32, vec3_ai}), vec3_i32);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f32, vec3_af, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_f16, vec3_af, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec4_f32, vec3_af, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_u32, vec3_af, vec3_u32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_i32, vec3_af, vec3_i32}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_f32, vec3_af}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_f16, vec3_af}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec4_f32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_u32, vec3_af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_af, vec3_i32, vec3_af}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_af, vec3_f32}), vec3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_af, vec3_f16}), vec3_f16);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_af, vec4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_af, vec3_u32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{vec3_ai, vec3_af, vec3_i32}), nullptr);

    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat4x3_af, mat4x3_af}), mat4x3_af);
    EXPECT_TYPE(Type::Common(utils::Vector{mat3x4_f32, mat3x4_f32, mat3x4_f32}), mat3x4_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f32, mat4x3_f32, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f16, mat4x3_f16, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(utils::Vector{mat3x4_f32, mat4x3_af, mat3x4_f32}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f32, mat4x3_af, mat4x3_f32}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_f16, mat4x3_af, mat4x3_f16}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat3x4_f32, mat4x3_af}), nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat4x3_f32, mat4x3_af}), mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{mat4x3_af, mat4x3_f16, mat4x3_af}), mat4x3_f16);

    EXPECT_TYPE(Type::Common(utils::Vector{arr_mat4x3_f16, arr_mat4x3_f32, arr_mat4x3_f16}),
                nullptr);
    EXPECT_TYPE(Type::Common(utils::Vector{arr_mat4x3_af, arr_mat4x3_f32, arr_mat4x3_af}),
                arr_mat4x3_f32);
    EXPECT_TYPE(Type::Common(utils::Vector{arr_mat4x3_af, arr_mat4x3_f16, arr_mat4x3_af}),
                arr_mat4x3_f16);
}

TEST_F(TypeTest, HoldsAbstract) {
    EXPECT_TRUE(af->HoldsAbstract());
    EXPECT_TRUE(ai->HoldsAbstract());
    EXPECT_FALSE(f32->HoldsAbstract());
    EXPECT_FALSE(f16->HoldsAbstract());
    EXPECT_FALSE(i32->HoldsAbstract());
    EXPECT_FALSE(u32->HoldsAbstract());
    EXPECT_FALSE(vec2_f32->HoldsAbstract());
    EXPECT_FALSE(vec3_f32->HoldsAbstract());
    EXPECT_FALSE(vec3_f16->HoldsAbstract());
    EXPECT_FALSE(vec4_f32->HoldsAbstract());
    EXPECT_FALSE(vec3_u32->HoldsAbstract());
    EXPECT_FALSE(vec3_i32->HoldsAbstract());
    EXPECT_TRUE(vec3_af->HoldsAbstract());
    EXPECT_TRUE(vec3_ai->HoldsAbstract());
    EXPECT_FALSE(mat2x4_f32->HoldsAbstract());
    EXPECT_FALSE(mat3x4_f32->HoldsAbstract());
    EXPECT_FALSE(mat4x2_f32->HoldsAbstract());
    EXPECT_FALSE(mat4x3_f32->HoldsAbstract());
    EXPECT_FALSE(mat4x3_f16->HoldsAbstract());
    EXPECT_TRUE(mat4x3_af->HoldsAbstract());
    EXPECT_FALSE(str_f16->HoldsAbstract());
    EXPECT_TRUE(str_af->HoldsAbstract());
    EXPECT_FALSE(arr_i32->HoldsAbstract());
    EXPECT_TRUE(arr_ai->HoldsAbstract());
    EXPECT_FALSE(arr_vec3_i32->HoldsAbstract());
    EXPECT_TRUE(arr_vec3_ai->HoldsAbstract());
    EXPECT_FALSE(arr_mat4x3_f16->HoldsAbstract());
    EXPECT_FALSE(arr_mat4x3_f32->HoldsAbstract());
    EXPECT_TRUE(arr_mat4x3_af->HoldsAbstract());
    EXPECT_FALSE(arr_str_f16->HoldsAbstract());
    EXPECT_TRUE(arr_str_af->HoldsAbstract());
}

}  // namespace
}  // namespace tint::type
