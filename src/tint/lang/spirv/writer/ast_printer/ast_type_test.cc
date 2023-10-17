// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest_Type = TestHelper;

TEST_F(SpirvASTPrinterTest_Type, GenerateRuntimeArray) {
    auto ary = ty.array(ty.i32());
    auto* str = Structure("S", Vector{Member("x", ary)});
    GlobalVar("a", ty.Of(str), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(0_a));
    ast::Type type = str->members[0]->type;

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(type));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedRuntimeArray) {
    auto ary = ty.array(ty.i32());
    auto* str = Structure("S", Vector{Member("x", ary)});
    GlobalVar("a", ty.Of(str), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(0_a));
    ast::Type type = str->members[0]->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(type)), 1u);
    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(type)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(SpirvASTPrinterTest_Type, GenerateArray) {
    auto ary = ty.array<i32, 4>();
    ast::Type type = GlobalVar("a", ary, core::AddressSpace::kPrivate)->type;

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(program->TypeOf(type));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(SpirvASTPrinterTest_Type, GenerateArray_WithStride) {
    auto ary = ty.array<i32, 4>(Vector{Stride(16)});
    ast::Type ty = GlobalVar("a", ary, core::AddressSpace::kPrivate)->type;

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedArray) {
    auto ary = ty.array<i32, 4>();
    ast::Type ty = GlobalVar("a", ary, core::AddressSpace::kPrivate)->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(SpirvASTPrinterTest_Type, GenerateBool) {
    auto* bool_ = create<core::type::Bool>();

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(bool_);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeBool
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedBool) {
    auto* bool_ = create<core::type::Bool>();
    auto* i32 = create<core::type::I32>();

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(bool_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(bool_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GenerateF32) {
    auto* f32 = create<core::type::F32>();

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(f32);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedF32) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GenerateF16) {
    auto* f16 = create<core::type::F16>();

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(f16);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeFloat 16
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedF16) {
    auto* f16 = create<core::type::F16>();
    auto* i32 = create<core::type::I32>();

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(f16), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f16), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GenerateI32) {
    auto* i32 = create<core::type::I32>();

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(i32);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeInt 32 1
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedI32) {
    auto* f32 = create<core::type::F32>();
    auto* i32 = create<core::type::I32>();

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GenerateMatrix) {
    auto* f32 = create<core::type::F32>();
    auto* vec3 = create<core::type::Vector>(f32, 3u);
    auto* mat2x3 = create<core::type::Matrix>(vec3, 2u);

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(mat2x3);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(b.Module().Types().size(), 3u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedMatrix) {
    auto* i32 = create<core::type::I32>();
    auto* col = create<core::type::Vector>(i32, 4u);
    auto* mat = create<core::type::Matrix>(col, 3u);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 3u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GenerateF16Matrix) {
    auto* f16 = create<core::type::F16>();
    auto* vec3 = create<core::type::Vector>(f16, 3u);
    auto* mat2x3 = create<core::type::Matrix>(vec3, 2u);

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(mat2x3);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(b.Module().Types().size(), 3u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 16
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedF16Matrix) {
    auto* f16 = create<core::type::F16>();
    auto* col = create<core::type::Vector>(f16, 4u);
    auto* mat = create<core::type::Matrix>(col, 3u);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f16), 3u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(mat), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GeneratePtr) {
    auto* i32 = create<core::type::I32>();
    auto* ptr =
        create<core::type::Pointer>(core::AddressSpace::kOut, i32, core::Access::kReadWrite);

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(ptr);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypePointer Output %2
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedPtr) {
    auto* i32 = create<core::type::I32>();
    auto* ptr =
        create<core::type::Pointer>(core::AddressSpace::kOut, i32, core::Access::kReadWrite);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(ptr), 1u);
    EXPECT_EQ(b.GenerateTypeIfNeeded(ptr), 1u);
}

TEST_F(SpirvASTPrinterTest_Type, GenerateStruct) {
    Enable(wgsl::Extension::kF16);

    auto* s = Structure("my_struct", Vector{Member("a", ty.f32()), Member("b", ty.f16())});

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, GenerateStruct_DecoratedMembers) {
    Enable(wgsl::Extension::kF16);

    auto* s = Structure("S", Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.f32(), Vector{MemberAlign(8_i)}),
                                 Member("c", ty.f16(), Vector{MemberAlign(8_u)}),
                                 Member("d", ty.f16()),
                             });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, GenerateStruct_DecoratedMembers_Matrix) {
    Enable(wgsl::Extension::kF16);

    auto* s = Structure("S", Vector{
                                 Member("mat2x2_f32", ty.mat2x2<f32>()),
                                 Member("mat2x3_f32", ty.mat2x3<f32>(), Vector{MemberAlign(64_i)}),
                                 Member("mat4x4_f32", ty.mat4x4<f32>()),
                                 Member("mat2x2_f16", ty.mat2x2<f16>(), Vector{MemberAlign(32_i)}),
                                 Member("mat2x3_f16", ty.mat2x3<f16>()),
                                 Member("mat4x4_f16", ty.mat4x4<f16>(), Vector{MemberAlign(64_i)}),
                             });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, GenerateStruct_DecoratedMembers_ArraysOfMatrix) {
    Enable(wgsl::Extension::kF16);

    auto arr_mat2x2_f32 = ty.array(ty.mat2x2<f32>(), 1_u);  // Singly nested array
    auto arr_mat2x2_f16 = ty.array(ty.mat2x2<f16>(), 1_u);  // Singly nested array
    ast::Type arr_arr_mat2x3_f32 =
        ty.array(ty.array(ty.mat2x3<f32>(), 1_u), 2_u);  // Doubly nested array
    ast::Type arr_arr_mat2x3_f16 =
        ty.array(ty.array(ty.mat2x3<f16>(), 1_u), 2_u);  // Doubly nested array
    auto rtarr_mat4x4 = ty.array(ty.mat4x4<f32>());      // Runtime array

    auto* s = Structure(
        "S", Vector{
                 Member("arr_mat2x2_f32", arr_mat2x2_f32),
                 Member("arr_mat2x2_f16", arr_mat2x2_f16, Vector{MemberAlign(64_i)}),
                 Member("arr_arr_mat2x3_f32", arr_arr_mat2x3_f32, Vector{MemberAlign(64_i)}),
                 Member("arr_arr_mat2x3_f16", arr_arr_mat2x3_f16),
                 Member("rtarr_mat4x4", rtarr_mat4x4),
             });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, GenerateU32) {
    auto* u32 = create<core::type::U32>();

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(u32);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeInt 32 0
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedU32) {
    auto* u32 = create<core::type::U32>();
    auto* f32 = create<core::type::F32>();

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(u32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(f32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(u32), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GenerateVector) {
    auto* vec = create<core::type::Vector>(create<core::type::F32>(), 3u);

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(vec);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    EXPECT_EQ(b.Module().Types().size(), 2u);
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedVector) {
    auto* i32 = create<core::type::I32>();
    auto* vec = create<core::type::Vector>(i32, 3u);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(vec), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(vec), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

TEST_F(SpirvASTPrinterTest_Type, GenerateVoid) {
    auto* void_ = create<core::type::Void>();

    Builder& b = Build();

    auto id = b.GenerateTypeIfNeeded(void_);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(id, 1u);

    ASSERT_EQ(b.Module().Types().size(), 1u);
    EXPECT_EQ(DumpInstruction(b.Module().Types()[0]), R"(%1 = OpTypeVoid
)");
}

TEST_F(SpirvASTPrinterTest_Type, ReturnsGeneratedVoid) {
    auto* void_ = create<core::type::Void>();
    auto* i32 = create<core::type::I32>();

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(void_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(i32), 2u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(b.GenerateTypeIfNeeded(void_), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
}

struct PtrData {
    core::AddressSpace ast_class;
    SpvStorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, PtrData data) {
    StringStream str;
    str << data.ast_class;
    out << str.str();
    return out;
}
using PtrDataTest = TestParamHelper<PtrData>;
TEST_P(PtrDataTest, ConvertAddressSpace) {
    auto params = GetParam();

    Builder& b = Build();

    EXPECT_EQ(b.ConvertAddressSpace(params.ast_class), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvASTPrinterTest_Type,
    PtrDataTest,
    testing::Values(PtrData{core::AddressSpace::kIn, SpvStorageClassInput},
                    PtrData{core::AddressSpace::kOut, SpvStorageClassOutput},
                    PtrData{core::AddressSpace::kUniform, SpvStorageClassUniform},
                    PtrData{core::AddressSpace::kWorkgroup, SpvStorageClassWorkgroup},
                    PtrData{core::AddressSpace::kHandle, SpvStorageClassUniformConstant},
                    PtrData{core::AddressSpace::kStorage, SpvStorageClassStorageBuffer},
                    PtrData{core::AddressSpace::kPrivate, SpvStorageClassPrivate},
                    PtrData{core::AddressSpace::kFunction, SpvStorageClassFunction}));

TEST_F(SpirvASTPrinterTest_Type, DepthTexture_Generate_2d) {
    auto* two_d = create<core::type::DepthTexture>(core::type::TextureDimension::k2d);

    Builder& b = Build();

    auto id_two_d = b.GenerateTypeIfNeeded(two_d);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id_two_d);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, DepthTexture_Generate_2dArray) {
    auto* two_d_array = create<core::type::DepthTexture>(core::type::TextureDimension::k2dArray);

    Builder& b = Build();

    auto id_two_d_array = b.GenerateTypeIfNeeded(two_d_array);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id_two_d_array);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, DepthTexture_Generate_Cube) {
    auto* cube = create<core::type::DepthTexture>(core::type::TextureDimension::kCube);

    Builder& b = Build();

    auto id_cube = b.GenerateTypeIfNeeded(cube);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(1u, id_cube);

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 0 0 1 Unknown
)");
    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()), "");
}

TEST_F(SpirvASTPrinterTest_Type, DepthTexture_Generate_CubeArray) {
    auto* cube_array = create<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, MultisampledTexture_Generate_2d_i32) {
    auto* i32 = create<core::type::I32>();
    auto* ms = create<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, i32);

    Builder& b = Build();

    EXPECT_EQ(1u, b.GenerateTypeIfNeeded(ms));
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, MultisampledTexture_Generate_2d_u32) {
    auto* u32 = create<core::type::U32>();
    auto* ms = create<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, u32);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(ms), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, MultisampledTexture_Generate_2d_f32) {
    auto* f32 = create<core::type::F32>();
    auto* ms = create<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, f32);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(ms), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_1d_i32) {
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::k1d,
                                                 create<core::type::I32>());

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_1d_u32) {
    auto* u32 = create<core::type::U32>();
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::k1d, u32);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_1d_f32) {
    auto* f32 = create<core::type::F32>();
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::k1d, f32);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_2d) {
    auto* f32 = create<core::type::F32>();
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::k2d, f32);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_2d_array) {
    auto* f32 = create<core::type::F32>();
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, f32);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_3d) {
    auto* f32 = create<core::type::F32>();
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::k3d, f32);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 1 Unknown
)");
}

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_Cube) {
    auto* f32 = create<core::type::F32>();
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::kCube, f32);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(s), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()),
              R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 0 0 1 Unknown
)");
    EXPECT_EQ(DumpInstructions(b.Module().Capabilities()), "");
}

TEST_F(SpirvASTPrinterTest_Type, SampledTexture_Generate_CubeArray) {
    auto* f32 = create<core::type::F32>();
    auto* s = create<core::type::SampledTexture>(core::type::TextureDimension::kCubeArray, f32);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest_Type, StorageTexture_Generate_1d) {
    auto s = ty.storage_texture(core::type::TextureDimension::k1d, core::TexelFormat::kR32Float,
                                core::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R32f
)");
}

TEST_F(SpirvASTPrinterTest_Type, StorageTexture_Generate_2d) {
    auto s = ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Float,
                                core::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 2 R32f
)");
}

TEST_F(SpirvASTPrinterTest_Type, StorageTexture_Generate_2dArray) {
    auto s = ty.storage_texture(core::type::TextureDimension::k2dArray,
                                core::TexelFormat::kR32Float, core::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 2 R32f
)");
}

TEST_F(SpirvASTPrinterTest_Type, StorageTexture_Generate_3d) {
    auto s = ty.storage_texture(core::type::TextureDimension::k3d, core::TexelFormat::kR32Float,
                                core::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 2 R32f
)");
}

TEST_F(SpirvASTPrinterTest_Type, StorageTexture_Generate_SampledTypeFloat_Format_r32float) {
    auto s = ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Float,
                                core::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 2 R32f
)");
}

TEST_F(SpirvASTPrinterTest_Type, StorageTexture_Generate_SampledTypeSint_Format_r32sint) {
    auto s = ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Sint,
                                core::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 2D 0 0 0 2 R32i
)");
}

TEST_F(SpirvASTPrinterTest_Type, StorageTexture_Generate_SampledTypeUint_Format_r32uint) {
    auto s = ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                core::Access::kWrite);

    ast::Type ty = GlobalVar("test_var", s, Binding(0_a), Group(0_a))->type;

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(program->TypeOf(ty)), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 2D 0 0 0 2 R32ui
)");
}

TEST_F(SpirvASTPrinterTest_Type, Sampler) {
    auto* sampler = create<core::type::Sampler>(core::type::SamplerKind::kSampler);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(sampler), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), "%1 = OpTypeSampler\n");
}

TEST_F(SpirvASTPrinterTest_Type, ComparisonSampler) {
    auto* sampler = create<core::type::Sampler>(core::type::SamplerKind::kComparisonSampler);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(sampler), 1u);
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), "%1 = OpTypeSampler\n");
}

TEST_F(SpirvASTPrinterTest_Type, Dedup_Sampler_And_ComparisonSampler) {
    auto* comp_sampler = create<core::type::Sampler>(core::type::SamplerKind::kComparisonSampler);
    auto* sampler = create<core::type::Sampler>(core::type::SamplerKind::kSampler);

    Builder& b = Build();

    EXPECT_EQ(b.GenerateTypeIfNeeded(comp_sampler), 1u);

    EXPECT_EQ(b.GenerateTypeIfNeeded(sampler), 1u);

    ASSERT_FALSE(b.has_error()) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), "%1 = OpTypeSampler\n");
}

}  // namespace
}  // namespace tint::spirv::writer
