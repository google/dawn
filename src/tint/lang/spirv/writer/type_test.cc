// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::writer {
namespace {

TEST_F(SpirvWriterTest, Type_Void) {
    writer_.Type(ty.void_());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%void = OpTypeVoid");
}

TEST_F(SpirvWriterTest, Type_Bool) {
    writer_.Type(ty.bool_());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%bool = OpTypeBool");
}

TEST_F(SpirvWriterTest, Type_I32) {
    writer_.Type(ty.i32());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int = OpTypeInt 32 1");
}

TEST_F(SpirvWriterTest, Type_U32) {
    writer_.Type(ty.u32());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%uint = OpTypeInt 32 0");
}

TEST_F(SpirvWriterTest, Type_F32) {
    writer_.Type(ty.f32());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%float = OpTypeFloat 32");
}

TEST_F(SpirvWriterTest, Type_F16) {
    writer_.Type(ty.f16());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability Float16");
    EXPECT_INST("OpCapability UniformAndStorageBuffer16BitAccess");
    EXPECT_INST("OpCapability StorageBuffer16BitAccess");
    EXPECT_INST("OpCapability StorageInputOutput16");
    EXPECT_INST("%half = OpTypeFloat 16");
}

TEST_F(SpirvWriterTest, Type_Vec2i) {
    writer_.Type(ty.vec2<i32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v2int = OpTypeVector %int 2");
}

TEST_F(SpirvWriterTest, Type_Vec3u) {
    writer_.Type(ty.vec3<u32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v3uint = OpTypeVector %uint 3");
}

TEST_F(SpirvWriterTest, Type_Vec4f) {
    writer_.Type(ty.vec4<f32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v4float = OpTypeVector %float 4");
}

TEST_F(SpirvWriterTest, Type_Vec2h) {
    writer_.Type(ty.vec2<f16>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v2half = OpTypeVector %half 2");
}

TEST_F(SpirvWriterTest, Type_Vec4Bool) {
    writer_.Type(ty.vec4<bool>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v4bool = OpTypeVector %bool 4");
}

TEST_F(SpirvWriterTest, Type_Mat2x3f) {
    writer_.Type(ty.mat2x3(ty.f32()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%mat2v3float = OpTypeMatrix %v3float 2");
}

TEST_F(SpirvWriterTest, Type_Mat4x2h) {
    writer_.Type(ty.mat4x2(ty.f16()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%mat4v2half = OpTypeMatrix %v2half 4");
}

TEST_F(SpirvWriterTest, Type_Array_DefaultStride) {
    writer_.Type(ty.array<f32, 4>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_4 ArrayStride 4");
    EXPECT_INST("%_arr_float_uint_4 = OpTypeArray %float %uint_4");
}

TEST_F(SpirvWriterTest, Type_Array_ExplicitStride) {
    writer_.Type(ty.array<f32, 4>(16));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_4 ArrayStride 16");
    EXPECT_INST("%_arr_float_uint_4 = OpTypeArray %float %uint_4");
}

TEST_F(SpirvWriterTest, Type_Array_NestedArray) {
    writer_.Type(ty.array(ty.array<f32, 64u>(), 4u));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_arr_float_uint_64 ArrayStride 4");
    EXPECT_INST("OpDecorate %_arr__arr_float_uint_64_uint_4 ArrayStride 256");
    EXPECT_INST("%_arr_float_uint_64 = OpTypeArray %float %uint_64");
    EXPECT_INST("%_arr__arr_float_uint_64_uint_4 = OpTypeArray %_arr_float_uint_64 %uint_4");
}

TEST_F(SpirvWriterTest, Type_RuntimeArray_DefaultStride) {
    writer_.Type(ty.array<f32>());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_runtimearr_float ArrayStride 4");
    EXPECT_INST("%_runtimearr_float = OpTypeRuntimeArray %float");
}

TEST_F(SpirvWriterTest, Type_RuntimeArray_ExplicitStride) {
    writer_.Type(ty.array<f32>(16));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpDecorate %_runtimearr_float ArrayStride 16");
    EXPECT_INST("%_runtimearr_float = OpTypeRuntimeArray %float");
}

TEST_F(SpirvWriterTest, Type_Struct) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.f32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    writer_.Type(str);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpMemberName %MyStruct 0 \"a\"");
    EXPECT_INST("OpMemberName %MyStruct 1 \"b\"");
    EXPECT_INST("OpName %MyStruct \"MyStruct\"");
    EXPECT_INST("OpMemberDecorate %MyStruct 0 Offset 0");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 Offset 16");
    EXPECT_INST("%MyStruct = OpTypeStruct %float %v4int");
}

TEST_F(SpirvWriterTest, Type_Struct_MatrixLayout) {
    auto* str = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.Register("m"), ty.mat3x3<f32>()},
            // Matrices nested inside arrays need layout decorations on the struct member too.
            {mod.symbols.Register("arr"), ty.array(ty.array(ty.mat2x4<f16>(), 4), 4)},
        });
    writer_.Type(str);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpMemberDecorate %MyStruct 0 ColMajor");
    EXPECT_INST("OpMemberDecorate %MyStruct 0 MatrixStride 16");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 ColMajor");
    EXPECT_INST("OpMemberDecorate %MyStruct 1 MatrixStride 8");
    EXPECT_INST("%MyStruct = OpTypeStruct %mat3v3float %_arr__arr_mat2v4half_uint_4_uint_4");
}

TEST_F(SpirvWriterTest, Type_Atomic) {
    writer_.Type(ty.atomic(ty.i32()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int = OpTypeInt 32 1");
}

TEST_F(SpirvWriterTest, Type_Sampler) {
    writer_.Type(ty.sampler());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpTypeSampler");
}

TEST_F(SpirvWriterTest, Type_SamplerComparison) {
    writer_.Type(ty.comparison_sampler());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpTypeSampler");
}

TEST_F(SpirvWriterTest, Type_Samplers_Dedup) {
    auto id = writer_.Type(ty.sampler());
    EXPECT_EQ(writer_.Type(ty.comparison_sampler()), id);

    ASSERT_TRUE(Generate()) << Error() << output_;
}

using Dim = core::type::TextureDimension;
struct TextureCase {
    std::string result;
    Dim dim;
    TestElementType format = kF32;
};

using Type_SampledTexture = SpirvWriterTestWithParam<TextureCase>;
TEST_P(Type_SampledTexture, Emit) {
    auto params = GetParam();
    writer_.Type(ty.Get<core::type::SampledTexture>(params.dim, MakeScalarType(params.format)));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    Type_SampledTexture,
    testing::Values(
        TextureCase{"%1 = OpTypeImage %float 1D 0 0 0 1 Unknown", Dim::k1d, kF32},
        TextureCase{"%1 = OpTypeImage %float 2D 0 0 0 1 Unknown", Dim::k2d, kF32},
        TextureCase{"%1 = OpTypeImage %float 2D 0 1 0 1 Unknown", Dim::k2dArray, kF32},
        TextureCase{"%1 = OpTypeImage %float 3D 0 0 0 1 Unknown", Dim::k3d, kF32},
        TextureCase{"%1 = OpTypeImage %float Cube 0 0 0 1 Unknown", Dim::kCube, kF32},
        TextureCase{"%1 = OpTypeImage %float Cube 0 1 0 1 Unknown", Dim::kCubeArray, kF32},
        TextureCase{"%1 = OpTypeImage %int 1D 0 0 0 1 Unknown", Dim::k1d, kI32},
        TextureCase{"%1 = OpTypeImage %int 2D 0 0 0 1 Unknown", Dim::k2d, kI32},
        TextureCase{"%1 = OpTypeImage %int 2D 0 1 0 1 Unknown", Dim::k2dArray, kI32},
        TextureCase{"%1 = OpTypeImage %int 3D 0 0 0 1 Unknown", Dim::k3d, kI32},
        TextureCase{"%1 = OpTypeImage %int Cube 0 0 0 1 Unknown", Dim::kCube, kI32},
        TextureCase{"%1 = OpTypeImage %int Cube 0 1 0 1 Unknown", Dim::kCubeArray, kI32},
        TextureCase{"%1 = OpTypeImage %uint 1D 0 0 0 1 Unknown", Dim::k1d, kU32},
        TextureCase{"%1 = OpTypeImage %uint 2D 0 0 0 1 Unknown", Dim::k2d, kU32},
        TextureCase{"%1 = OpTypeImage %uint 2D 0 1 0 1 Unknown", Dim::k2dArray, kU32},
        TextureCase{"%1 = OpTypeImage %uint 3D 0 0 0 1 Unknown", Dim::k3d, kU32},
        TextureCase{"%1 = OpTypeImage %uint Cube 0 0 0 1 Unknown", Dim::kCube, kU32},
        TextureCase{"%1 = OpTypeImage %uint Cube 0 1 0 1 Unknown", Dim::kCubeArray, kU32}));

using Type_MultisampledTexture = SpirvWriterTestWithParam<TextureCase>;
TEST_P(Type_MultisampledTexture, Emit) {
    auto params = GetParam();
    writer_.Type(
        ty.Get<core::type::MultisampledTexture>(params.dim, MakeScalarType(params.format)));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    Type_MultisampledTexture,
    testing::Values(TextureCase{"%1 = OpTypeImage %float 2D 0 0 1 1 Unknown", Dim::k2d, kF32},
                    TextureCase{"%1 = OpTypeImage %int 2D 0 0 1 1 Unknown", Dim::k2d, kI32},
                    TextureCase{"%1 = OpTypeImage %uint 2D 0 0 1 1 Unknown", Dim::k2d, kU32}));

using Type_DepthTexture = SpirvWriterTestWithParam<TextureCase>;
TEST_P(Type_DepthTexture, Emit) {
    auto params = GetParam();
    writer_.Type(ty.Get<core::type::DepthTexture>(params.dim));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    Type_DepthTexture,
    testing::Values(TextureCase{"%1 = OpTypeImage %float 2D 0 0 0 1 Unknown", Dim::k2d},
                    TextureCase{"%1 = OpTypeImage %float 2D 0 1 0 1 Unknown", Dim::k2dArray},
                    TextureCase{"%1 = OpTypeImage %float Cube 0 0 0 1 Unknown", Dim::kCube},
                    TextureCase{"%1 = OpTypeImage %float Cube 0 1 0 1 Unknown", Dim::kCubeArray}));

TEST_F(SpirvWriterTest, Type_DepthTexture_DedupWithSampledTexture) {
    writer_.Type(ty.Get<core::type::SampledTexture>(Dim::k2d, ty.f32()));
    writer_.Type(ty.Get<core::type::DepthTexture>(Dim::k2d));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 1 Unknown
%4 = OpTypeVoid
%5 = OpTypeFunction %4
)");
}

TEST_F(SpirvWriterTest, Type_DepthMultiSampledTexture) {
    writer_.Type(ty.Get<core::type::DepthMultisampledTexture>(Dim::k2d));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%1 = OpTypeImage %float 2D 0 0 1 1 Unknown");
}

TEST_F(SpirvWriterTest, Type_DepthMultisampledTexture_DedupWithMultisampledTexture) {
    writer_.Type(ty.Get<core::type::MultisampledTexture>(Dim::k2d, ty.f32()));
    writer_.Type(ty.Get<core::type::DepthMultisampledTexture>(Dim::k2d));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
%4 = OpTypeVoid
%5 = OpTypeFunction %4
)");
}

using Format = core::TexelFormat;
struct StorageTextureCase {
    std::string result;
    Dim dim;
    Format format;
};
using Type_StorageTexture = SpirvWriterTestWithParam<StorageTextureCase>;
TEST_P(Type_StorageTexture, Emit) {
    auto params = GetParam();
    writer_.Type(ty.Get<core::type::StorageTexture>(
        params.dim, params.format, core::Access::kWrite,
        core::type::StorageTexture::SubtypeFor(params.format, mod.Types())));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(params.result);
    if (params.format == core::TexelFormat::kRg32Uint ||
        params.format == core::TexelFormat::kRg32Sint ||
        params.format == core::TexelFormat::kRg32Float) {
        EXPECT_INST("OpCapability StorageImageExtendedFormats");
    }
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Type_StorageTexture,
                         testing::Values(
                             // Test all the dimensions with a single format.
                             StorageTextureCase{"%1 = OpTypeImage %float 1D 0 0 0 2 R32f",  //
                                                Dim::k1d, Format::kR32Float},
                             StorageTextureCase{"%1 = OpTypeImage %float 2D 0 0 0 2 R32f",  //
                                                Dim::k2d, Format::kR32Float},
                             StorageTextureCase{"%1 = OpTypeImage %float 2D 0 1 0 2 R32f",  //
                                                Dim::k2dArray, Format::kR32Float},
                             StorageTextureCase{"%1 = OpTypeImage %float 3D 0 0 0 2 R32f",  //
                                                Dim::k3d, Format::kR32Float},

                             // Test all the formats with 2D.
                             StorageTextureCase{"%1 = OpTypeImage %int 2D 0 0 0 2 R32i",  //
                                                Dim::k2d, Format::kR32Sint},
                             StorageTextureCase{"%1 = OpTypeImage %uint 2D 0 0 0 2 R32u",  //
                                                Dim::k2d, Format::kR32Uint},
                             StorageTextureCase{"%1 = OpTypeImage %float 2D 0 0 0 2 Rg32f",  //
                                                Dim::k2d, Format::kRg32Float},
                             StorageTextureCase{"%1 = OpTypeImage %int 2D 0 0 0 2 Rg32i",  //
                                                Dim::k2d, Format::kRg32Sint},
                             StorageTextureCase{"%1 = OpTypeImage %uint 2D 0 0 0 2 Rg32ui",  //
                                                Dim::k2d, Format::kRg32Uint},
                             StorageTextureCase{"%1 = OpTypeImage %float 2D 0 0 0 2 Rgba16f",  //
                                                Dim::k2d, Format::kRgba16Float},
                             StorageTextureCase{"%1 = OpTypeImage %int 2D 0 0 0 2 Rgba16i",  //
                                                Dim::k2d, Format::kRgba16Sint},
                             StorageTextureCase{"%1 = OpTypeImage %uint 2D 0 0 0 2 Rgba16ui",  //
                                                Dim::k2d, Format::kRgba16Uint},
                             StorageTextureCase{"%1 = OpTypeImage %float 2D 0 0 0 2 Rgba32f",  //
                                                Dim::k2d, Format::kRgba32Float},
                             StorageTextureCase{"%1 = OpTypeImage %int 2D 0 0 0 2 Rgba32i",  //
                                                Dim::k2d, Format::kRgba32Sint},
                             StorageTextureCase{"%1 = OpTypeImage %uint 2D 0 0 0 2 Rgba32ui",  //
                                                Dim::k2d, Format::kRgba32Uint},
                             StorageTextureCase{"%1 = OpTypeImage %int 2D 0 0 0 2 Rgba8i",  //
                                                Dim::k2d, Format::kRgba8Sint},
                             StorageTextureCase{"%1 = OpTypeImage %float 2D 0 0 0 2 Rgba8Snorm",  //
                                                Dim::k2d, Format::kRgba8Snorm},
                             StorageTextureCase{"%1 = OpTypeImage %uint 2D 0 0 0 2 Rgba8ui",  //
                                                Dim::k2d, Format::kRgba8Uint},
                             StorageTextureCase{"%1 = OpTypeImage %float 2D 0 0 0 2 Rgba8",  //
                                                Dim::k2d, Format::kRgba8Unorm}));

// Test that we can emit multiple types.
// Includes types with the same opcode but different parameters.
TEST_F(SpirvWriterTest, Type_Multiple) {
    writer_.Type(ty.i32());
    writer_.Type(ty.u32());
    writer_.Type(ty.f32());
    writer_.Type(ty.f16());

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
       %half = OpTypeFloat 16
)");
}

// Test that we do not emit the same type more than once.
TEST_F(SpirvWriterTest, Type_Deduplicate) {
    auto id = writer_.Type(ty.i32());
    EXPECT_EQ(writer_.Type(ty.i32()), id);
    EXPECT_EQ(writer_.Type(ty.i32()), id);

    ASSERT_TRUE(Generate()) << Error() << output_;
}

}  // namespace
}  // namespace tint::spirv::writer
