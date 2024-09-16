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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/sampler_kind.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/glsl/writer/helper_test.h"

#include "gtest/gtest.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, BuiltinGeneric) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kAbs, x);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int x = 1;
  int w = abs(x);
}
)");
}

TEST_F(GlslWriterTest, BuiltinSelectScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kSelect, x, y, true);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int x = 1;
  int y = 2;
  int w = ((true) ? (y) : (x));
}
)");
}

TEST_F(GlslWriterTest, BuiltinSelectVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Construct<vec2<i32>>(1_i, 2_i));
        auto* y = b.Let("y", b.Construct<vec2<i32>>(3_i, 4_i));
        auto* cmp = b.Construct<vec2<bool>>(true, false);

        auto* c = b.Call(ty.vec2<i32>(), core::BuiltinFn::kSelect, x, y, cmp);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  ivec2 x = ivec2(1, 2);
  ivec2 y = ivec2(3, 4);
  bvec2 v = bvec2(true, false);
  int v_1 = ((v.x) ? (y.x) : (x.x));
  ivec2 w = ivec2(v_1, ((v.y) ? (y.y) : (x.y)));
}
)");
}

TEST_F(GlslWriterTest, BuiltinStorageBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
  memoryBarrierBuffer();
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
  memoryBarrierImage();
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
