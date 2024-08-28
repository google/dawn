// Copyright 2024 The Dawn & Tint Authors
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

#include "gmock/gmock.h"

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/glsl/writer/helper_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::glsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(GlslWriterTest, EmitType_Array) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.array<bool, 4>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  bool a[4] = bool[4](false, false, false, false);
}
)");
}

TEST_F(GlslWriterTest, EmitType_ArrayOfArray) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.array(ty.array<bool, 4>(), 5)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  bool a[5][4] = bool[5][4](bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false));
}
)");
}

TEST_F(GlslWriterTest, EmitType_ArrayOfArrayOfArray) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a",
              ty.ptr(core::AddressSpace::kPrivate, ty.array(ty.array(ty.array<bool, 4>(), 5), 6)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  bool a[6][5][4] = bool[6][5][4](bool[5][4](bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false)), bool[5][4](bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false)), bool[5][4](bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false)), bool[5][4](bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false)), bool[5][4](bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false)), bool[5][4](bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false), bool[4](false, false, false, false)));
}
)");
}

TEST_F(GlslWriterTest, EmitType_Bool) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.bool_()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  bool a = false;
}
)");
}

TEST_F(GlslWriterTest, EmitType_F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.f32()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  float a = 0.0f;
}
)");
}

TEST_F(GlslWriterTest, EmitType_F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.f16()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  float16_t a = 0.0hf;
}
)");
}

TEST_F(GlslWriterTest, EmitType_I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.i32()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  int a = 0;
}
)");
}

// TODO(dsinclair): Add matrix support
TEST_F(GlslWriterTest, DISABLED_EmitType_Matrix_F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.mat2x3<f32>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  mat2x3 a = mat2x3(0.0f);
}
)");
}

// TODO(dsinclair): Add matrix support
TEST_F(GlslWriterTest, DISABLED_EmitType_Matrix_F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.mat2x3<f16>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  f16mat2x3 a = f16mat2x3(0.0h);
}
)");
}

TEST_F(GlslWriterTest, EmitType_U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.u32()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  uint a = 0u;
}
)");
}

// TODO(dsinclair): Add atomic support
TEST_F(GlslWriterTest, DISABLED_EmitType_Atomic_U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", ty.ptr(core::AddressSpace::kWorkgroup, ty.atomic<u32>()));
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo(inout uint a) {
}
)");
}

// TODO(dsinclair): Add atomic support
TEST_F(GlslWriterTest, DISABLED_EmitType_Atomic_I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", ty.ptr(core::AddressSpace::kWorkgroup, ty.atomic<i32>()));
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo(inout int a) {
}
)");
}

TEST_F(GlslWriterTest, EmitType_Vector_F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.vec3<f32>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  vec3 a = vec3(0.0f);
}
)");
}

TEST_F(GlslWriterTest, EmitType_Vector_F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.vec3<f16>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  f16vec3 a = f16vec3(0.0hf);
}
)");
}

TEST_F(GlslWriterTest, EmitType_Vector_I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.vec2<i32>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  ivec2 a = ivec2(0);
}
)");
}

TEST_F(GlslWriterTest, EmitType_Vector_U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.vec4<u32>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  uvec4 a = uvec4(0u);
}
)");
}

TEST_F(GlslWriterTest, EmitType_Vector_bool) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, ty.vec3<bool>()));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  bvec3 a = bvec3(false);
}
)");
}

TEST_F(GlslWriterTest, EmitType_Void) {
    // Tested via the function return type.
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
}
)");
}

// TODO(dsinclair): Add struct support
TEST_F(GlslWriterTest, DISABLED_EmitType_Struct) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.i32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, s));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
struct S {
  int a;
  float b;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  S a = {};
}
)");
}

// TODO(dsinclair): Add struct support
TEST_F(GlslWriterTest, DISABLED_EmitType_Struct_NameCollision) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("double"), ty.i32()},
                                                  {mod.symbols.Register("float"), ty.f32()},
                                              });
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, s));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
struct S {
  int tint_symbol_1;
  float tint_symbol_2;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  S a = {};
}
)");
}

// TODO(dsinclair): Add struct support
TEST_F(GlslWriterTest, DISABLED_EmitType_Struct_Dedup) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.i32()},
                                                  {mod.symbols.Register("b"), ty.f32()},
                                              });
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    b.Append(func->Block(), [&] {
        func->SetWorkgroupSize(1, 1, 1);
        b.Var("a", ty.ptr(core::AddressSpace::kPrivate, s));
        b.Var("b", ty.ptr(core::AddressSpace::kPrivate, s));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
struct S {
  int a;
  float b;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
  S a = {};
  S b = {};
}
)");
}

// TODO(dsinclair): Add sampler support
TEST_F(GlslWriterTest, DISABLED_EmitType_Sampler) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", ty.sampler());
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
}
)");
}

// TODO(dsinclair): Add sampler comparison support
TEST_F(GlslWriterTest, DISABLED_EmitType_SamplerComparison) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", ty.comparison_sampler());
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo() {
}
)");
}

struct GlslDepthTextureData {
    core::type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslDepthTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using GlslWriterDepthTexturesTest = GlslWriterTestWithParam<GlslDepthTextureData>;
// TODO(dsinclair): Add depth texture support
TEST_P(GlslWriterDepthTexturesTest, DISABLED_Emit) {
    auto params = GetParam();

    auto* t = ty.Get<core::type::DepthTexture>(params.dim);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", t);
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo()" + params.result +
                                R"( a) {
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    GlslWriterTest,
    GlslWriterDepthTexturesTest,
    testing::Values(
        GlslDepthTextureData{core::type::TextureDimension::k2d, "sampler2DShadow"},
        GlslDepthTextureData{core::type::TextureDimension::k2dArray, "sampler2DArrayShadow"},
        GlslDepthTextureData{core::type::TextureDimension::kCube, "samplerCubeShadow"},
        GlslDepthTextureData{core::type::TextureDimension::kCubeArray, "samplerCubeArrayShadow"}));

// TODO(dsinclair): Add depth multisampled support
TEST_F(GlslWriterTest, DISABLED_EmitType_DepthMultisampledTexture) {
    auto* t = ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", t);
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo(sampler2DMS a) {
}
)");
}

enum class TextureDataType : uint8_t {
    kF32,
    kU32,
    kI32,
};

struct GlslTextureData {
    core::type::TextureDimension dim;
    TextureDataType datatype;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslTextureData data) {
    StringStream str;
    str << data.dim;
    out << str.str();
    return out;
}
using GlslWriterSampledTexturesTest = GlslWriterTestWithParam<GlslTextureData>;
// TODO(dsinclair): Add sampled texture support
TEST_P(GlslWriterSampledTexturesTest, DISABLED_Emit) {
    auto params = GetParam();

    const core::type::Type* subtype = nullptr;
    switch (params.datatype) {
        case TextureDataType::kF32:
            subtype = ty.f32();
            break;
        case TextureDataType::kI32:
            subtype = ty.u32();
            break;
        case TextureDataType::kU32:
            subtype = ty.u32();
            break;
    }

    auto* t = ty.Get<core::type::SampledTexture>(params.dim, subtype);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", t);
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo()" + params.result +
                                R"( a) {
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    GlslWriterTest,
    GlslWriterSampledTexturesTest,
    testing::Values(
        GlslTextureData{core::type::TextureDimension::k1d, TextureDataType::kF32, "sampler1D"},
        GlslTextureData{core::type::TextureDimension::k2d, TextureDataType::kF32, "sampler2D"},
        GlslTextureData{core::type::TextureDimension::k2dArray, TextureDataType::kF32,
                        "sampler2DArray"},
        GlslTextureData{core::type::TextureDimension::k3d, TextureDataType::kF32, "sampler3D"},
        GlslTextureData{core::type::TextureDimension::kCube, TextureDataType::kF32, "samplerCube"},
        GlslTextureData{core::type::TextureDimension::kCubeArray, TextureDataType::kF32,
                        "samplerCubeArray"},

        GlslTextureData{core::type::TextureDimension::k1d, TextureDataType::kI32, "isampler1D"},
        GlslTextureData{core::type::TextureDimension::k2d, TextureDataType::kI32, "isampler2D"},
        GlslTextureData{core::type::TextureDimension::k2dArray, TextureDataType::kI32,
                        "isampler2DArray"},
        GlslTextureData{core::type::TextureDimension::k3d, TextureDataType::kI32, "isampler3D"},
        GlslTextureData{core::type::TextureDimension::kCube, TextureDataType::kI32, "samplerCube"},
        GlslTextureData{core::type::TextureDimension::kCubeArray, TextureDataType::kI32,
                        "isamplerCubeArray"},

        GlslTextureData{core::type::TextureDimension::k1d, TextureDataType::kU32, "usampler1D"},
        GlslTextureData{core::type::TextureDimension::k2d, TextureDataType::kU32, "usampler2D"},
        GlslTextureData{core::type::TextureDimension::k2dArray, TextureDataType::kU32,
                        "usampler2DArray"},
        GlslTextureData{core::type::TextureDimension::k3d, TextureDataType::kU32, "usampler3D"},
        GlslTextureData{core::type::TextureDimension::kCube, TextureDataType::kU32, "usamplerCube"},
        GlslTextureData{core::type::TextureDimension::kCubeArray, TextureDataType::kU32,
                        "usamplerCubeArray"}));

// TODO(dsinclair): Add multisampled texture support
TEST_F(GlslWriterTest, DISABLED_EmitType_MultisampledTexture) {
    auto* ms = ty.Get<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, ty.u32());
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", ms);
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo(usampler2DMS a) {
}
)");
}

struct GlslStorageTextureData {
    core::type::TextureDimension dim;
    std::string result;
};
inline std::ostream& operator<<(std::ostream& out, GlslStorageTextureData data) {
    StringStream str;
    str << data.dim;
    return out << str.str();
}
using GlslWriterStorageTexturesTest = GlslWriterTestWithParam<GlslStorageTextureData>;
// TODO(dsinclair): Add storage texture support
TEST_P(GlslWriterStorageTexturesTest, DISABLED_Emit) {
    auto params = GetParam();

    auto* f32 = ty.f32();
    auto s = ty.Get<core::type::StorageTexture>(params.dim, core::TexelFormat::kR32Float,
                                                core::Access::kWrite, f32);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    auto* param = b.FunctionParam("a", s);
    func->SetParams({param});
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void foo()" + params.result +
                                R"( a) {
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    GlslWriterTest,
    GlslWriterStorageTexturesTest,
    testing::Values(GlslStorageTextureData{core::type::TextureDimension::k1d, "image1D"},
                    GlslStorageTextureData{core::type::TextureDimension::k2d, "image2D"},
                    GlslStorageTextureData{core::type::TextureDimension::k2dArray, "image2DArray"},
                    GlslStorageTextureData{core::type::TextureDimension::k3d, "image3D"}));

}  // namespace
}  // namespace tint::glsl::writer
