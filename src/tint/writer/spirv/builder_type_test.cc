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

#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest_Type = TestHelper;

TEST_F(BuilderTest_Type, GenerateRuntimeArray) {
    auto ary = ty.array(ty.i32());
    auto* str = Structure("S", utils::Vector{Member("x", ary)});
    GlobalVar("a", ty.Of(str), builtin::AddressSpace::kStorage, builtin::Access::kRead,
              Binding(0_a), Group(0_a));
    ast::Type type = str->members[0]->type;

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(type));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedRuntimeArray) {
    auto ary = ty.array(ty.i32());
    auto* str = Structure("S", utils::Vector{Member("x", ary)});
    GlobalVar("a", ty.Of(str), builtin::AddressSpace::kStorage, builtin::Access::kRead,
              Binding(0_a), Group(0_a));
    ast::Type type = str->members[0]->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(type)), 1u);
    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(type)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, GenerateArray) {
    auto ary = ty.array<i32, 4>();
    ast::Type type = GlobalVar("a", ary, builtin::AddressSpace::kPrivate)->type;

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(type));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateArray_WithStride) {
    auto ary = ty.array<i32, 4>(utils::Vector{Stride(16)});
    ast::Type ty = GlobalVar("a", ary, builtin::AddressSpace::kPrivate)->type;

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(ty));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpDecorate %1 ArrayStride 16
)");

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedArray) {
    auto ary = ty.array<i32, 4>();
    ast::Type ty = GlobalVar("a", ary, builtin::AddressSpace::kPrivate)->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateBool) {
    auto* bool_ = create<type::Bool>();

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(bool_);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeBool
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedBool) {
    auto* bool_ = create<type::Bool>();
    auto* i32 = create<type::I32>();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(bool_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(bool_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GenerateF32) {
    auto* f32 = create<type::F32>();

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(f32);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedF32) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GenerateF16) {
    auto* f16 = create<type::F16>();

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(f16);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeFloat 16
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedF16) {
    auto* f16 = create<type::F16>();
    auto* i32 = create<type::I32>();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(f16), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f16), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GenerateI32) {
    auto* i32 = create<type::I32>();

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(i32);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeInt 32 1
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedI32) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GenerateMatrix) {
    auto* f32 = create<type::F32>();
    auto* vec3 = create<type::Vector>(f32, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(mat2x3);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(b.Module().Types().size(), 3u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedMatrix) {
    auto* i32 = create<type::I32>();
    auto* col = create<type::Vector>(i32, 4u);
    auto* mat = create<type::Matrix>(col, 3u);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 3u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GenerateF16Matrix) {
    auto* f16 = create<type::F16>();
    auto* vec3 = create<type::Vector>(f16, 3u);
    auto* mat2x3 = create<type::Matrix>(vec3, 2u);

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(mat2x3);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(b.Module().Types().size(), 3u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedF16Matrix) {
    auto* f16 = create<type::F16>();
    auto* col = create<type::Vector>(f16, 4u);
    auto* mat = create<type::Matrix>(col, 3u);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f16), 3u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GeneratePtr) {
    auto* i32 = create<type::I32>();
    auto* ptr =
        create<type::Pointer>(i32, builtin::AddressSpace::kOut, builtin::Access::kReadWrite);

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(ptr);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypePointer Output %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedPtr) {
    auto* i32 = create<type::I32>();
    auto* ptr =
        create<type::Pointer>(i32, builtin::AddressSpace::kOut, builtin::Access::kReadWrite);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(ptr), 1u);
    EXPECT_EQ(b.GenerateTypeIfNeeded(ptr), 1u);
}

TEST_F(BuilderTest_Type, GenerateStruct) {
    Enable(builtin::Extension::kF16);

    auto* s = Structure("my_struct", utils::Vector{Member("a", ty.f32()), Member("b", ty.f16())});

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(s));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeFloat 16
%1 = OpTypeStruct %2 %3
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "my_struct"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers) {
    Enable(builtin::Extension::kF16);

    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.f32(), utils::Vector{MemberAlign(8_i)}),
                                 Member("c", ty.f16(), utils::Vector{MemberAlign(8_u)}),
                                 Member("d", ty.f16()),
                             });

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(s));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeFloat 16
%1 = OpTypeStruct %2 %2 %3 %3
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
OpMemberName %1 3 "d"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 1 Offset 8
OpMemberDecorate %1 2 Offset 16
OpMemberDecorate %1 3 Offset 18
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers_Matrix) {
    Enable(builtin::Extension::kF16);

    auto* s =
        Structure("S", utils::Vector{
                           Member("mat2x2_f32", ty.mat2x2<f32>()),
                           Member("mat2x3_f32", ty.mat2x3<f32>(), utils::Vector{MemberAlign(64_i)}),
                           Member("mat4x4_f32", ty.mat4x4<f32>()),
                           Member("mat2x2_f16", ty.mat2x2<f16>(), utils::Vector{MemberAlign(32_i)}),
                           Member("mat2x3_f16", ty.mat2x3<f16>()),
                           Member("mat4x4_f16", ty.mat4x4<f16>(), utils::Vector{MemberAlign(64_i)}),
                       });

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(s));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypeMatrix %3 2
%6 = OpTypeVector %4 3
%5 = OpTypeMatrix %6 2
%8 = OpTypeVector %4 4
%7 = OpTypeMatrix %8 4
%11 = OpTypeFloat 16
%10 = OpTypeVector %11 2
%9 = OpTypeMatrix %10 2
%13 = OpTypeVector %11 3
%12 = OpTypeMatrix %13 2
%15 = OpTypeVector %11 4
%14 = OpTypeMatrix %15 4
%1 = OpTypeStruct %2 %5 %7 %9 %12 %14
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "S"
OpMemberName %1 0 "mat2x2_f32"
OpMemberName %1 1 "mat2x3_f32"
OpMemberName %1 2 "mat4x4_f32"
OpMemberName %1 3 "mat2x2_f16"
OpMemberName %1 4 "mat2x3_f16"
OpMemberName %1 5 "mat4x4_f16"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 8
OpMemberDecorate %1 1 Offset 64
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 16
OpMemberDecorate %1 2 Offset 96
OpMemberDecorate %1 2 ColMajor
OpMemberDecorate %1 2 MatrixStride 16
OpMemberDecorate %1 3 Offset 160
OpMemberDecorate %1 3 ColMajor
OpMemberDecorate %1 3 MatrixStride 4
OpMemberDecorate %1 4 Offset 168
OpMemberDecorate %1 4 ColMajor
OpMemberDecorate %1 4 MatrixStride 8
OpMemberDecorate %1 5 Offset 192
OpMemberDecorate %1 5 ColMajor
OpMemberDecorate %1 5 MatrixStride 8
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers_ArraysOfMatrix) {
    Enable(builtin::Extension::kF16);

    auto arr_mat2x2_f32 = ty.array(ty.mat2x2<f32>(), 1_u);  // Singly nested array
    auto arr_mat2x2_f16 = ty.array(ty.mat2x2<f16>(), 1_u);  // Singly nested array
    ast::Type arr_arr_mat2x3_f32 =
        ty.array(ty.array(ty.mat2x3<f32>(), 1_u), 2_u);  // Doubly nested array
    ast::Type arr_arr_mat2x3_f16 =
        ty.array(ty.array(ty.mat2x3<f16>(), 1_u), 2_u);  // Doubly nested array
    auto rtarr_mat4x4 = ty.array(ty.mat4x4<f32>());      // Runtime array

    auto* s = Structure(
        "S", utils::Vector{
                 Member("arr_mat2x2_f32", arr_mat2x2_f32),
                 Member("arr_mat2x2_f16", arr_mat2x2_f16, utils::Vector{MemberAlign(64_i)}),
                 Member("arr_arr_mat2x3_f32", arr_arr_mat2x3_f32, utils::Vector{MemberAlign(64_i)}),
                 Member("arr_arr_mat2x3_f16", arr_arr_mat2x3_f16),
                 Member("rtarr_mat4x4", rtarr_mat4x4),
             });

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(s));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 2
%3 = OpTypeMatrix %4 2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%2 = OpTypeArray %3 %7
%11 = OpTypeFloat 16
%10 = OpTypeVector %11 2
%9 = OpTypeMatrix %10 2
%8 = OpTypeArray %9 %7
%15 = OpTypeVector %5 3
%14 = OpTypeMatrix %15 2
%13 = OpTypeArray %14 %7
%16 = OpConstant %6 2
%12 = OpTypeArray %13 %16
%20 = OpTypeVector %11 3
%19 = OpTypeMatrix %20 2
%18 = OpTypeArray %19 %7
%17 = OpTypeArray %18 %16
%23 = OpTypeVector %5 4
%22 = OpTypeMatrix %23 4
%21 = OpTypeRuntimeArray %22
%1 = OpTypeStruct %2 %8 %12 %17 %21
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "S"
OpMemberName %1 0 "arr_mat2x2_f32"
OpMemberName %1 1 "arr_mat2x2_f16"
OpMemberName %1 2 "arr_arr_mat2x3_f32"
OpMemberName %1 3 "arr_arr_mat2x3_f16"
OpMemberName %1 4 "rtarr_mat4x4"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 8
OpDecorate %2 ArrayStride 16
OpMemberDecorate %1 1 Offset 64
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 4
OpDecorate %8 ArrayStride 8
OpMemberDecorate %1 2 Offset 128
OpMemberDecorate %1 2 ColMajor
OpMemberDecorate %1 2 MatrixStride 16
OpDecorate %13 ArrayStride 32
OpDecorate %12 ArrayStride 32
OpMemberDecorate %1 3 Offset 192
OpMemberDecorate %1 3 ColMajor
OpMemberDecorate %1 3 MatrixStride 8
OpDecorate %18 ArrayStride 16
OpDecorate %17 ArrayStride 16
OpMemberDecorate %1 4 Offset 224
OpMemberDecorate %1 4 ColMajor
OpMemberDecorate %1 4 MatrixStride 16
OpDecorate %21 ArrayStride 64
)");
}

TEST_F(BuilderTest_Type, GenerateU32) {
    auto* u32 = create<type::U32>();

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(u32);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeInt 32 0
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedU32) {
    auto* u32 = create<type::U32>();
    auto* f32 = create<type::F32>();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(u32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(u32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GenerateVector) {
    auto* vec = create<type::Vector>(create<type::F32>(), 3u);

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(vec);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(b.Module().Types().size(), 2u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVector) {
    auto* i32 = create<type::I32>();
    auto* vec = create<type::Vector>(i32, 3u);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(vec), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(vec), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(BuilderTest_Type, GenerateVoid) {
    auto* void_ = create<type::Void>();

    spirv::Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(void_);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeVoid
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVoid) {
    auto* void_ = create<type::Void>();
    auto* i32 = create<type::I32>();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(void_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(void_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

struct PtrData {
    builtin::AddressSpace ast_class;
    SpvStorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, PtrData data) {
    utils::StringStream str;
    str << data.ast_class;
    out << str.str();
    return out;
}
using PtrDataTest = TestParamHelper<PtrData>;
TEST_P(PtrDataTest, ConvertAddressSpace) {
    auto params = GetParam();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.ConvertAddressSpace(params.ast_class), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest_Type,
    PtrDataTest,
    testing::Values(PtrData{builtin::AddressSpace::kUndefined, SpvStorageClassMax},
                    PtrData{builtin::AddressSpace::kIn, SpvStorageClassInput},
                    PtrData{builtin::AddressSpace::kOut, SpvStorageClassOutput},
                    PtrData{builtin::AddressSpace::kUniform, SpvStorageClassUniform},
                    PtrData{builtin::AddressSpace::kWorkgroup, SpvStorageClassWorkgroup},
                    PtrData{builtin::AddressSpace::kHandle, SpvStorageClassUniformConstant},
                    PtrData{builtin::AddressSpace::kStorage, SpvStorageClassStorageBuffer},
                    PtrData{builtin::AddressSpace::kPrivate, SpvStorageClassPrivate},
                    PtrData{builtin::AddressSpace::kFunction, SpvStorageClassFunction}));

TEST_F(BuilderTest_Type, DepthTexture_Generate_2d) {
    auto* two_d = create<type::DepthTexture>(type::TextureDimension::k2d);

    spirv::Builder& b = Build();

    auto id_two_d = b.GenerateTypeIfNeeded(two_d);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id_two_d);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_2dArray) {
    auto* two_d_array = create<type::DepthTexture>(type::TextureDimension::k2dArray);

    spirv::Builder& b = Build();

    auto id_two_d_array = b.GenerateTypeIfNeeded(two_d_array);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id_two_d_array);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_Cube) {
    auto* cube = create<type::DepthTexture>(type::TextureDimension::kCube);

    spirv::Builder& b = Build();

    auto id_cube = b.GenerateTypeIfNeeded(cube);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id_cube);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 0 0 1 Unknown
)");
    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()), "");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_CubeArray) {
    auto* cube_array = create<type::DepthTexture>(type::TextureDimension::kCubeArray);

    spirv::Builder& b = Build();

    auto id_cube_array = b.GenerateTypeIfNeeded(cube_array);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id_cube_array);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 1 0 1 Unknown
)");
    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
              R"(OpCapability SampledCubeArray
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_i32) {
    auto* i32 = create<type::I32>();
    auto* ms = create<type::MultisampledTexture>(type::TextureDimension::k2d, i32);

    spirv::Builder& b = Build();

    EXPECT_EQ(1u, b.GenerateTypeIfNeeded(ms));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_u32) {
    auto* u32 = create<type::U32>();
    auto* ms = create<type::MultisampledTexture>(type::TextureDimension::k2d, u32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(ms), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_f32) {
    auto* f32 = create<type::F32>();
    auto* ms = create<type::MultisampledTexture>(type::TextureDimension::k2d, f32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(ms), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_i32) {
    auto* s = create<type::SampledTexture>(type::TextureDimension::k1d, create<type::I32>());

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
              R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_u32) {
    auto* u32 = create<type::U32>();
    auto* s = create<type::SampledTexture>(type::TextureDimension::k1d, u32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
              R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_f32) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::SampledTexture>(type::TextureDimension::k1d, f32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
              R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_2d) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::SampledTexture>(type::TextureDimension::k2d, f32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_2d_array) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::SampledTexture>(type::TextureDimension::k2dArray, f32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_3d) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::SampledTexture>(type::TextureDimension::k3d, f32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_Cube) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::SampledTexture>(type::TextureDimension::kCube, f32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 0 0 1 Unknown
)");
    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()), "");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_CubeArray) {
    auto* f32 = create<type::F32>();
    auto* s = create<type::SampledTexture>(type::TextureDimension::kCubeArray, f32);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 1 0 1 Unknown
)");
    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
              R"(OpCapability SampledCubeArray
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_1d) {
    auto s = ty.storage_texture(type::TextureDimension::k1d, builtin::TexelFormat::kR32Float,
                                builtin::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R32f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_2d) {
    auto s = ty.storage_texture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Float,
                                builtin::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 2 R32f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_2dArray) {
    auto s = ty.storage_texture(type::TextureDimension::k2dArray, builtin::TexelFormat::kR32Float,
                                builtin::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 2 R32f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_3d) {
    auto s = ty.storage_texture(type::TextureDimension::k3d, builtin::TexelFormat::kR32Float,
                                builtin::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 2 R32f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_SampledTypeFloat_Format_r32float) {
    auto s = ty.storage_texture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Float,
                                builtin::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 2 R32f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_SampledTypeSint_Format_r32sint) {
    auto s = ty.storage_texture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Sint,
                                builtin::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 2D 0 0 0 2 R32i
)");
}

TEST_F(BuilderTest_Type, StorageTexture_Generate_SampledTypeUint_Format_r32uint) {
    auto s = ty.storage_texture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                builtin::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 2D 0 0 0 2 R32ui
)");
}

TEST_F(BuilderTest_Type, Sampler) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(sampler), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), "%1 = OpTypeSampler\n");
}

TEST_F(BuilderTest_Type, ComparisonSampler) {
    auto* sampler = create<type::Sampler>(type::SamplerKind::kComparisonSampler);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(sampler), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), "%1 = OpTypeSampler\n");
}

TEST_F(BuilderTest_Type, Dedup_Sampler_And_ComparisonSampler) {
    auto* comp_sampler = create<type::Sampler>(type::SamplerKind::kComparisonSampler);
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);

    spirv::Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(comp_sampler), 1u);

    EXPECT_EQ(b.GenerateTypeIfNeeded(sampler), 1u);

    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), "%1 = OpTypeSampler\n");
}

}  // namespace
}  // namespace tint::writer::spirv
