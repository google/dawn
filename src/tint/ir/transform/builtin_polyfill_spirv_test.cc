// Copyright 2023 The Tint Authors.
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

#include "src/tint/ir/transform/builtin_polyfill_spirv.h"

#include <utility>

#include "src/tint/ir/transform/test_helper.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/sampled_texture.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_BuiltinPolyfillSpirvTest = TransformTest;

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec4f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 -> %b1 {
  %b1 = block {
    %4:f32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 -> %b1 {
  %b1 = block {
    %4:f32 = spirv.dot %arg1, %arg2
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec2i) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<i32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 -> %b1 {
  %b1 = block {
    %4:i32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 -> %b1 {
  %b1 = block {
    %4:i32 = access %arg1, 0u
    %5:i32 = access %arg2, 0u
    %6:i32 = mul %4, %5
    %7:i32 = access %arg1, 1u
    %8:i32 = access %arg2, 1u
    %9:i32 = mul %7, %8
    %10:i32 = add %6, %9
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec4u) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<u32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 -> %b1 {
  %b1 = block {
    %4:u32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 -> %b1 {
  %b1 = block {
    %4:u32 = access %arg1, 0u
    %5:u32 = access %arg2, 0u
    %6:u32 = mul %4, %5
    %7:u32 = access %arg1, 1u
    %8:u32 = access %arg2, 1u
    %9:u32 = mul %7, %8
    %10:u32 = add %6, %9
    %11:u32 = access %arg1, 2u
    %12:u32 = access %arg2, 2u
    %13:u32 = mul %11, %12
    %14:u32 = add %10, %13
    %15:u32 = access %arg1, 3u
    %16:u32 = access %arg2, 3u
    %17:u32 = mul %15, %16
    %18:u32 = add %14, %17
    ret %18
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_ScalarCondition_ScalarOperands) {
    auto* argf = b.FunctionParam("argf", ty.i32());
    auto* argt = b.FunctionParam("argt", ty.i32());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 -> %b1 {
  %b1 = block {
    %5:i32 = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 -> %b1 {
  %b1 = block {
    %5:i32 = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_VectorCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_ScalarCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<bool> = construct %cond, %cond, %cond, %cond
    %6:vec4<i32> = spirv.select %5, %argt, %argf
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_1D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k1d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kTextureSample, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>, %s:sampler, %coords:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:f32 = textureSample %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>, %s:sampler, %coords:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:spirv.sampled_image = spirv.sampled_image %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 0u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kTextureSample, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureSample %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:spirv.sampled_image = spirv.sampled_image %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 0u
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSample, t, s, coords,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = textureSample %t, %s, %coords, vec2<i32>(1i)
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %5:spirv.sampled_image = spirv.sampled_image %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 8u, vec2<i32>(1i)
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSample_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSample, t, s, coords, array_idx,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSample %t, %s, %coords, %array_idx, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = convert %array_idx
    %8:vec3<f32> = construct %coords, %7
    %9:vec4<f32> = spirv.image_sample_implicit_lod %6, %8, 8u, vec2<i32>(1i)
    ret %9
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleBias_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, bias});

    b.With(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, bias);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleBias %t, %s, %coords, %bias
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, %coords, 1u, %bias
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleBias_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, bias});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleBias %t, %s, %coords, %bias, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, %coords, 9u, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleBias_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, bias});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, array_idx, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_sample_implicit_lod %7, %9, 9u, %bias, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompare_2D) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.With(func->Block(), [&] {
        auto* result =
            b.Call(ty.f32(), builtin::Function::kTextureSampleCompare, t, s, coords, dref);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompare %t, %s, %coords, %dref
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 0u
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompare_2D_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompare, t, s, coords, dref,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompare %t, %s, %coords, %dref, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 8u, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompare_2DArray_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, array_idx, bias});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompare, t, s, coords, array_idx, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:f32 = textureSampleCompare %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:f32 = spirv.image_sample_dref_implicit_lod %7, %9, %bias, 8u, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompareLevel_2D) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.With(func->Block(), [&] {
        auto* result =
            b.Call(ty.f32(), builtin::Function::kTextureSampleCompareLevel, t, s, coords, dref);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompareLevel %t, %s, %coords, %dref
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 2u, 0.0f
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompareLevel_2D_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompareLevel, t, s, coords, dref,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:f32 = textureSampleCompareLevel %t, %s, %coords, %dref, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>, %dref:f32):f32 -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 10u, 0.0f, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleCompareLevel_2DArray_Offset) {
    auto* t = b.FunctionParam("t", ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, array_idx, bias});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.f32(), builtin::Function::kTextureSampleCompareLevel, t, s, coords, array_idx, bias,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:f32 = textureSampleCompareLevel %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:f32 = spirv.image_sample_dref_implicit_lod %7, %9, %bias, 10u, 0.0f, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleGrad_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});

    b.With(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, ddx, ddy);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %ddx, %ddy
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:vec4<f32> = spirv.image_sample_implicit_lod %7, %coords, 9u, %ddx, %ddy
    ret %8
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleGrad_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, ddx, ddy,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %ddx, %ddy, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:vec4<f32> = spirv.image_sample_implicit_lod %7, %coords, 9u, %ddx, %ddy
    ret %8
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleGrad_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, ddx, ddy});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleBias, t, s, coords, array_idx, ddx,
            ddy,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %8:vec4<f32> = textureSampleBias %t, %s, %coords, %array_idx, %ddx, %ddy, vec2<i32>(1i)
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %8:spirv.sampled_image = spirv.sampled_image %t, %s
    %9:f32 = convert %array_idx
    %10:vec3<f32> = construct %coords, %9
    %11:vec4<f32> = spirv.image_sample_implicit_lod %8, %10, 9u, %ddx, %ddy
    ret %11
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleLevel_2D) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, lod});

    b.With(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), builtin::Function::kTextureSampleLevel, t, s, coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleLevel %t, %s, %coords, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_explicit_lod %6, %coords, 2u, %lod
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleLevel_2D_Offset) {
    auto* t =
        b.FunctionParam("t", ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, lod});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleLevel, t, s, coords, lod,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:vec4<f32> = textureSampleLevel %t, %s, %coords, %lod, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %6:spirv.sampled_image = spirv.sampled_image %t, %s
    %7:vec4<f32> = spirv.image_sample_explicit_lod %6, %coords, 10u, %lod, vec2<i32>(1i)
    ret %7
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, TextureSampleLevel_2DArray_Offset) {
    auto* t = b.FunctionParam(
        "t", ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, lod});

    b.With(func->Block(), [&] {
        auto* result = b.Call(
            ty.vec4<f32>(), builtin::Function::kTextureSampleLevel, t, s, coords, array_idx, lod,
            b.Constant(mod.constant_values.Splat(ty.vec2<i32>(), mod.constant_values.Get(1_i), 2)));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:vec4<f32> = textureSampleLevel %t, %s, %coords, %array_idx, %lod, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %lod:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %7:spirv.sampled_image = spirv.sampled_image %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_sample_explicit_lod %7, %9, 10u, %lod, vec2<i32>(1i)
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
