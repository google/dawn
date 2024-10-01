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

#include "src/tint/lang/glsl/writer/raise/texture_builtins_from_uniform.h"

#include <vector>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/writer/common/options.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_TextureBuiltinsFromUniformTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureNumLevels) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:u32 = textureNumLevels %3
    %len:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_2d<f32> = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureNumSamples) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %1
    %4:u32 = textureNumSamples %3
    %len:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_depth_multisampled_2d = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimesTextureNumLevels) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:u32 = textureNumLevels %3
    %len:u32 = let %4
    %6:u32 = textureNumLevels %3
    %len2:u32 = let %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_2d<f32> = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    %8:ptr<uniform, u32, read> = access %2, 0u
    %9:u32 = load %8
    %len2:u32 = let %9
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, SameBuiltinCalledMultipleTimesTextureNumSamples) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %1
    %4:u32 = textureNumSamples %3
    %len:u32 = let %4
    %6:u32 = textureNumSamples %3
    %len2:u32 = let %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_depth_multisampled_2d = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    %8:ptr<uniform, u32, read> = access %2, 0u
    %9:u32 = load %8
    %len2:u32 = let %9
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureAsFunctionParameter) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* p = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f = b.Function("f", ty.u32());
    f->SetParams({p});
    b.Append(f->Block(),
             [&] { b.Return(f, b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p)); });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), f, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%f = func(%t:texture_2d<f32>):u32 {
  $B2: {
    %4:u32 = textureNumLevels %t
    ret %4
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %6:texture_2d<f32> = load %1
    %7:u32 = call %f, %6
    %len:u32 = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f = func(%t:texture_2d<f32>, %tint_tex_value:u32):u32 {
  $B2: {
    ret %tint_tex_value
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %7:texture_2d<f32> = load %1
    %8:ptr<uniform, u32, read> = access %2, 0u
    %9:u32 = load %8
    %10:u32 = call %f, %7, %9
    %len:u32 = let %10
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureAsFunctionParameterUsedTwice) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* p = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f = b.Function("f", ty.u32());
    f->SetParams({p});
    b.Append(f->Block(), [&] {
        auto* val = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p)->Result(0);
        val = b.Add(ty.u32(), val, b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p))
                  ->Result(0);
        b.Return(f, val);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), f, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%f = func(%t:texture_2d<f32>):u32 {
  $B2: {
    %4:u32 = textureNumLevels %t
    %5:u32 = textureNumLevels %t
    %6:u32 = add %4, %5
    ret %6
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:texture_2d<f32> = load %1
    %9:u32 = call %f, %8
    %len:u32 = let %9
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f = func(%t:texture_2d<f32>, %tint_tex_value:u32):u32 {
  $B2: {
    %6:u32 = add %tint_tex_value, %tint_tex_value
    ret %6
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:texture_2d<f32> = load %1
    %9:ptr<uniform, u32, read> = access %2, 0u
    %10:u32 = load %9
    %11:u32 = call %f, %8, %10
    %len:u32 = let %11
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureAsFunctionParameterMultipleParameters) {
    auto* t1 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t1);

    auto* t2 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(t2);

    auto* t3 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t3->SetBindingPoint(0, 2);
    b.ir.root_block->Append(t3);

    auto* p1 = b.FunctionParam(
        "t1", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* p2 = b.FunctionParam(
        "t2", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* p3 = b.FunctionParam(
        "t3", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f = b.Function("f", ty.u32());
    f->SetParams({p1, p2, p3});
    b.Append(f->Block(), [&] {
        auto* v1 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p1)->Result(0);
        auto* v2 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p2)->Result(0);
        auto* v3 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p3)->Result(0);

        auto* val = b.Add(ty.u32(), v1, v2);
        val = b.Add(ty.u32(), val, v3);
        b.Return(f, val);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex1 = b.Load(t1);
        auto* tex2 = b.Load(t2);
        auto* tex3 = b.Load(t3);
        b.Let("len", b.Call(ty.u32(), f, tex1, tex2, tex3));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
}

%f = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>, %t3:texture_2d<f32>):u32 {
  $B2: {
    %8:u32 = textureNumLevels %t1
    %9:u32 = textureNumLevels %t2
    %10:u32 = textureNumLevels %t3
    %11:u32 = add %8, %9
    %12:u32 = add %11, %10
    ret %12
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %14:texture_2d<f32> = load %1
    %15:texture_2d<f32> = load %2
    %16:texture_2d<f32> = load %3
    %17:u32 = call %f, %14, %15, %16
    %len:u32 = let %17
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
  tint_builtin_value_1:u32 @offset(4)
  tint_builtin_value_2:u32 @offset(8)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
  %4:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>, %t3:texture_2d<f32>, %tint_tex_value:u32, %tint_tex_value_1:u32, %tint_tex_value_2:u32):u32 {  # %tint_tex_value_1: 'tint_tex_value', %tint_tex_value_2: 'tint_tex_value'
  $B2: {
    %12:u32 = add %tint_tex_value, %tint_tex_value_1
    %13:u32 = add %12, %tint_tex_value_2
    ret %13
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %15:texture_2d<f32> = load %1
    %16:texture_2d<f32> = load %2
    %17:texture_2d<f32> = load %3
    %18:ptr<uniform, u32, read> = access %4, 0u
    %19:u32 = load %18
    %20:ptr<uniform, u32, read> = access %4, 1u
    %21:u32 = load %20
    %22:ptr<uniform, u32, read> = access %4, 2u
    %23:u32 = load %22
    %24:u32 = call %f, %15, %16, %17, %19, %21, %23
    %len:u32 = let %24
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u},
                                             std::vector<BindingPoint>{{0, 0}, {0, 1}, {0, 2}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureAsFunctionParameterNested) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* p2 = b.FunctionParam(
        "t2", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f2 = b.Function("f2", ty.u32());
    f2->SetParams({p2});
    b.Append(f2->Block(),
             [&] { b.Return(f2, b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p2)); });

    auto* p1 = b.FunctionParam(
        "t1", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f1 = b.Function("f1", ty.u32());
    f1->SetParams({p1});
    b.Append(f1->Block(), [&] { b.Return(f1, b.Call(ty.u32(), f2, p1)); });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), f1, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%f2 = func(%t2:texture_2d<f32>):u32 {
  $B2: {
    %4:u32 = textureNumLevels %t2
    ret %4
  }
}
%f1 = func(%t1:texture_2d<f32>):u32 {
  $B3: {
    %7:u32 = call %f2, %t1
    ret %7
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %9:texture_2d<f32> = load %1
    %10:u32 = call %f1, %9
    %len:u32 = let %10
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f2 = func(%t2:texture_2d<f32>, %tint_tex_value:u32):u32 {
  $B2: {
    ret %tint_tex_value
  }
}
%f1 = func(%t1:texture_2d<f32>, %tint_tex_value_1:u32):u32 {  # %tint_tex_value_1: 'tint_tex_value'
  $B3: {
    %9:u32 = call %f2, %t1, %tint_tex_value_1
    ret %9
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %11:texture_2d<f32> = load %1
    %12:ptr<uniform, u32, read> = access %2, 0u
    %13:u32 = load %12
    %14:u32 = call %f1, %11, %13
    %len:u32 = let %14
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TextureBuiltinsFromUniformTest, TextureAsFunctionParameterMixed) {
    auto* t1 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t1);
    auto* t2 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(t2);
    auto* t3 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t3->SetBindingPoint(0, 2);
    b.ir.root_block->Append(t3);
    auto* t4 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t4->SetBindingPoint(0, 3);
    b.ir.root_block->Append(t4);
    auto* t5 = b.Var(
        ty.ptr(handle,
               ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, ty.f32()),
               read_write));
    t5->SetBindingPoint(0, 4);
    b.ir.root_block->Append(t5);

    auto* p_nested1 = b.FunctionParam(
        "t1", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* p_nested2 = b.FunctionParam(
        "t2", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f_nested = b.Function("f_nested", ty.u32());
    f_nested->SetParams({p_nested1, p_nested2});
    b.Append(f_nested->Block(), [&] {
        auto* v1 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p_nested1);
        auto* v2 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p_nested2);
        b.Return(f_nested, b.Add(ty.u32(), v1, v2));
    });

    auto* a = b.FunctionParam("a", ty.u32());
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f1 = b.Function("f1", ty.u32());
    f1->SetParams({a, t});
    b.Append(f1->Block(), [&] {
        auto* val1 = b.Call(ty.u32(), f_nested, t, b.Load(t1));
        auto* val2 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, b.Load(t3));
        auto* add = b.Add(ty.u32(), a, val1);
        b.Return(f1, b.Add(ty.u32(), add, val2));
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex5 = b.Load(t5);
        b.Let("m", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, tex5));

        auto* tex1 = b.Load(t1);
        b.Let("n", b.Call(ty.u32(), f1, 9_u, tex1));

        auto* tex2 = b.Load(t2);
        b.Let("o", b.Call(ty.u32(), f_nested, tex2, tex2));

        auto* tex4 = b.Load(t4);
        b.Let("p", b.Call(ty.u32(), f_nested, tex1, tex4));

        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
  %4:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 3)
  %5:ptr<handle, texture_2d_array<f32>, read_write> = var @binding_point(0, 4)
}

%f_nested = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>):u32 {
  $B2: {
    %9:u32 = textureNumLevels %t1
    %10:u32 = textureNumLevels %t2
    %11:u32 = add %9, %10
    ret %11
  }
}
%f1 = func(%a:u32, %t:texture_2d<f32>):u32 {
  $B3: {
    %15:texture_2d<f32> = load %1
    %16:u32 = call %f_nested, %t, %15
    %17:texture_2d<f32> = load %3
    %18:u32 = textureNumLevels %17
    %19:u32 = add %a, %16
    %20:u32 = add %19, %18
    ret %20
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %22:texture_2d_array<f32> = load %5
    %23:u32 = textureNumLayers %22
    %m:u32 = let %23
    %25:texture_2d<f32> = load %1
    %26:u32 = call %f1, 9u, %25
    %n:u32 = let %26
    %28:texture_2d<f32> = load %2
    %29:u32 = call %f_nested, %28, %28
    %o:u32 = let %29
    %31:texture_2d<f32> = load %4
    %32:u32 = call %f_nested, %25, %31
    %p:u32 = let %32
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
  tint_builtin_value_1:u32 @offset(4)
  tint_builtin_value_2:u32 @offset(8)
  tint_builtin_value_3:u32 @offset(12)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
  %4:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 3)
  %5:ptr<handle, texture_2d_array<f32>, read_write> = var @binding_point(0, 4)
  %6:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f_nested = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>, %tint_tex_value:u32, %tint_tex_value_1:u32):u32 {  # %tint_tex_value_1: 'tint_tex_value'
  $B2: {
    %12:u32 = add %tint_tex_value, %tint_tex_value_1
    ret %12
  }
}
%f1 = func(%a:u32, %t:texture_2d<f32>, %tint_tex_value_2:u32):u32 {  # %tint_tex_value_2: 'tint_tex_value'
  $B3: {
    %17:texture_2d<f32> = load %1
    %18:ptr<uniform, u32, read> = access %6, 2u
    %19:u32 = load %18
    %20:u32 = call %f_nested, %t, %17, %tint_tex_value_2, %19
    %21:texture_2d<f32> = load %3
    %22:ptr<uniform, u32, read> = access %6, 3u
    %23:u32 = load %22
    %24:u32 = add %a, %20
    %25:u32 = add %24, %23
    ret %25
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %27:texture_2d_array<f32> = load %5
    %28:u32 = textureNumLayers %27
    %m:u32 = let %28
    %30:texture_2d<f32> = load %1
    %31:ptr<uniform, u32, read> = access %6, 2u
    %32:u32 = load %31
    %33:u32 = call %f1, 9u, %30, %32
    %n:u32 = let %33
    %35:texture_2d<f32> = load %2
    %36:ptr<uniform, u32, read> = access %6, 0u
    %37:u32 = load %36
    %38:ptr<uniform, u32, read> = access %6, 0u
    %39:u32 = load %38
    %40:u32 = call %f_nested, %35, %35, %37, %39
    %o:u32 = let %40
    %42:texture_2d<f32> = load %4
    %43:ptr<uniform, u32, read> = access %6, 2u
    %44:u32 = load %43
    %45:ptr<uniform, u32, read> = access %6, 1u
    %46:u32 = load %45
    %47:u32 = call %f_nested, %30, %42, %44, %46
    %p:u32 = let %47
    ret
  }
}
)";

    TextureBuiltinsFromUniformOptions cfg = {
        {0, 30u}, std::vector<BindingPoint>{{0, 1}, {0, 3}, {0, 0}, {0, 2}}};
    Run(TextureBuiltinsFromUniform, cfg);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
