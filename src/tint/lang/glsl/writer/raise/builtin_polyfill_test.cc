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

#include "src/tint/lang/glsl/writer/raise/builtin_polyfill.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectScalar) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(),
             [&] { b.Return(func, b.Call<f32>(core::BuiltinFn::kSelect, 2_f, 1_f, false)); });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = select 2.0f, 1.0f, false
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = glsl.ternary 2.0f, 1.0f, false
    ret %2
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectVector) {
    auto* func = b.Function("foo", ty.vec3<f32>());
    b.Append(func->Block(), [&] {
        auto* false_ = b.Splat(ty.vec3<f32>(), 2_f);
        auto* true_ = b.Splat(ty.vec3<f32>(), 1_f);
        auto* cond = b.Splat(ty.vec3<bool>(), false);
        b.Return(func, b.Call<vec3<f32>>(core::BuiltinFn::kSelect, false_, true_, cond));
    });

    auto* src = R"(
%foo = func():vec3<f32> {
  $B1: {
    %2:vec3<f32> = select vec3<f32>(2.0f), vec3<f32>(1.0f), vec3<bool>(false)
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():vec3<f32> {
  $B1: {
    %2:f32 = swizzle vec3<f32>(2.0f), x
    %3:f32 = swizzle vec3<f32>(1.0f), x
    %4:f32 = swizzle vec3<bool>(false), x
    %5:f32 = glsl.ternary %2, %3, %4
    %6:f32 = swizzle vec3<f32>(2.0f), y
    %7:f32 = swizzle vec3<f32>(1.0f), y
    %8:f32 = swizzle vec3<bool>(false), y
    %9:f32 = glsl.ternary %6, %7, %8
    %10:f32 = swizzle vec3<f32>(2.0f), z
    %11:f32 = swizzle vec3<f32>(1.0f), z
    %12:f32 = swizzle vec3<bool>(false), z
    %13:f32 = glsl.ternary %10, %11, %12
    %14:vec3<f32> = construct %5, %9, %13
    ret %14
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, StorageBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = storageBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = glsl.barrier
    %3:void = glsl.memoryBarrierBuffer
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = textureBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = glsl.barrier
    %3:void = glsl.memoryBarrierImage
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, WorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = workgroupBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = glsl.barrier
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
