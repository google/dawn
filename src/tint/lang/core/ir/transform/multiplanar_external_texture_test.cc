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

#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/external_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_MultiplanarExternalTextureTest = TransformTest;

TEST_F(IR_MultiplanarExternalTextureTest, NoRootBlock) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";

    ExternalTextureOptions options;
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, DeclWithNoUses) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func():void -> %b2 {
  %b2 = block {
    ret
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func():void -> %b2 {
  %b2 = block {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, LoadWithNoUses) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:texture_external = load %texture
    ret
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %5:texture_2d<f32> = load %texture_plane0
    %6:texture_2d<f32> = load %texture_plane1
    %7:tint_ExternalTextureParams = load %texture_params
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureDimensions) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, load);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func():vec2<u32> -> %b2 {
  %b2 = block {
    %3:texture_external = load %texture
    %result:vec2<u32> = textureDimensions %3
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func():vec2<u32> -> %b2 {
  %b2 = block {
    %5:texture_2d<f32> = load %texture_plane0
    %6:texture_2d<f32> = load %texture_plane1
    %7:tint_ExternalTextureParams = load %texture_params
    %result:vec2<u32> = textureDimensions %5
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureLoad) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load, coords);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func(%coords:vec2<u32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_external = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func(%coords:vec2<u32>):vec4<f32> -> %b2 {
  %b2 = block {
    %6:texture_2d<f32> = load %texture_plane0
    %7:texture_2d<f32> = load %texture_plane1
    %8:tint_ExternalTextureParams = load %texture_params
    %9:vec4<f32> = call %tint_TextureLoadExternal, %6, %7, %8, %coords
    ret %9
  }
}
%tint_TextureLoadExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %15:u32 = access %params, 1u
    %16:mat3x4<f32> = access %params, 2u
    %17:u32 = access %params, 0u
    %18:bool = eq %17, 1u
    %19:vec3<f32>, %20:f32 = if %18 [t: %b4, f: %b5] {  # if_1
      %b4 = block {  # true
        %21:vec4<f32> = textureLoad %plane_0, %coords_1, 0u
        %22:vec3<f32> = swizzle %21, xyz
        %23:f32 = access %21, 3u
        exit_if %22, %23  # if_1
      }
      %b5 = block {  # false
        %24:vec4<f32> = textureLoad %plane_0, %coords_1, 0u
        %25:f32 = access %24, 0u
        %26:vec2<u32> = shiftr %coords_1, vec2<u32>(1u)
        %27:vec4<f32> = textureLoad %plane_1, %26, 0u
        %28:vec2<f32> = swizzle %27, xy
        %29:vec4<f32> = construct %25, %28, 1.0f
        %30:vec3<f32> = mul %29, %16
        exit_if %30, 1.0f  # if_1
      }
    }
    %31:bool = eq %15, 0u
    %32:vec3<f32> = if %31 [t: %b6, f: %b7] {  # if_2
      %b6 = block {  # true
        %33:tint_GammaTransferParams = access %params, 3u
        %34:tint_GammaTransferParams = access %params, 4u
        %35:mat3x3<f32> = access %params, 5u
        %36:vec3<f32> = call %tint_GammaCorrection, %19, %33
        %38:vec3<f32> = mul %35, %36
        %39:vec3<f32> = call %tint_GammaCorrection, %38, %34
        exit_if %39  # if_2
      }
      %b7 = block {  # false
        exit_if %19  # if_2
      }
    }
    %40:vec4<f32> = construct %32, %20
    ret %40
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> -> %b8 {  # %params_1: 'params'
  %b8 = block {
    %43:f32 = access %params_1, 0u
    %44:f32 = access %params_1, 1u
    %45:f32 = access %params_1, 2u
    %46:f32 = access %params_1, 3u
    %47:f32 = access %params_1, 4u
    %48:f32 = access %params_1, 5u
    %49:f32 = access %params_1, 6u
    %50:vec3<f32> = construct %43
    %51:vec3<f32> = construct %47
    %52:vec3<f32> = abs %v
    %53:vec3<f32> = sign %v
    %54:vec3<bool> = lt %52, %51
    %55:vec3<f32> = mul %46, %52
    %56:vec3<f32> = add %55, %49
    %57:vec3<f32> = mul %53, %56
    %58:vec3<f32> = mul %44, %52
    %59:vec3<f32> = add %58, %45
    %60:vec3<f32> = pow %59, %50
    %61:vec3<f32> = add %60, %48
    %62:vec3<f32> = mul %53, %61
    %63:vec3<f32> = select %62, %57, %54
    ret %63
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureLoad_SignedCoords) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    func->SetParams({coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load, coords);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func(%coords:vec2<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_external = load %texture
    %result:vec4<f32> = textureLoad %4, %coords
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func(%coords:vec2<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %6:texture_2d<f32> = load %texture_plane0
    %7:texture_2d<f32> = load %texture_plane1
    %8:tint_ExternalTextureParams = load %texture_params
    %9:vec2<u32> = convert %coords
    %10:vec4<f32> = call %tint_TextureLoadExternal, %6, %7, %8, %9
    ret %10
  }
}
%tint_TextureLoadExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %16:u32 = access %params, 1u
    %17:mat3x4<f32> = access %params, 2u
    %18:u32 = access %params, 0u
    %19:bool = eq %18, 1u
    %20:vec3<f32>, %21:f32 = if %19 [t: %b4, f: %b5] {  # if_1
      %b4 = block {  # true
        %22:vec4<f32> = textureLoad %plane_0, %coords_1, 0u
        %23:vec3<f32> = swizzle %22, xyz
        %24:f32 = access %22, 3u
        exit_if %23, %24  # if_1
      }
      %b5 = block {  # false
        %25:vec4<f32> = textureLoad %plane_0, %coords_1, 0u
        %26:f32 = access %25, 0u
        %27:vec2<u32> = shiftr %coords_1, vec2<u32>(1u)
        %28:vec4<f32> = textureLoad %plane_1, %27, 0u
        %29:vec2<f32> = swizzle %28, xy
        %30:vec4<f32> = construct %26, %29, 1.0f
        %31:vec3<f32> = mul %30, %17
        exit_if %31, 1.0f  # if_1
      }
    }
    %32:bool = eq %16, 0u
    %33:vec3<f32> = if %32 [t: %b6, f: %b7] {  # if_2
      %b6 = block {  # true
        %34:tint_GammaTransferParams = access %params, 3u
        %35:tint_GammaTransferParams = access %params, 4u
        %36:mat3x3<f32> = access %params, 5u
        %37:vec3<f32> = call %tint_GammaCorrection, %20, %34
        %39:vec3<f32> = mul %36, %37
        %40:vec3<f32> = call %tint_GammaCorrection, %39, %35
        exit_if %40  # if_2
      }
      %b7 = block {  # false
        exit_if %20  # if_2
      }
    }
    %41:vec4<f32> = construct %33, %21
    ret %41
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> -> %b8 {  # %params_1: 'params'
  %b8 = block {
    %44:f32 = access %params_1, 0u
    %45:f32 = access %params_1, 1u
    %46:f32 = access %params_1, 2u
    %47:f32 = access %params_1, 3u
    %48:f32 = access %params_1, 4u
    %49:f32 = access %params_1, 5u
    %50:f32 = access %params_1, 6u
    %51:vec3<f32> = construct %44
    %52:vec3<f32> = construct %48
    %53:vec3<f32> = abs %v
    %54:vec3<f32> = sign %v
    %55:vec3<bool> = lt %53, %52
    %56:vec3<f32> = mul %47, %53
    %57:vec3<f32> = add %56, %50
    %58:vec3<f32> = mul %54, %57
    %59:vec3<f32> = mul %45, %53
    %60:vec3<f32> = add %59, %46
    %61:vec3<f32> = pow %60, %51
    %62:vec3<f32> = add %61, %49
    %63:vec3<f32> = mul %54, %62
    %64:vec3<f32> = select %63, %58, %55
    ret %64
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, TextureSampleBaseClampToEdge) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* sampler = b.FunctionParam("sampler", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    func->SetParams({sampler, coords});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(var->Result());
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load,
                              sampler, coords);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func(%sampler:sampler, %coords:vec2<f32>):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_external = load %texture
    %result:vec4<f32> = textureSampleBaseClampToEdge %5, %sampler, %coords
    ret %result
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func(%sampler:sampler, %coords:vec2<f32>):vec4<f32> -> %b2 {
  %b2 = block {
    %7:texture_2d<f32> = load %texture_plane0
    %8:texture_2d<f32> = load %texture_plane1
    %9:tint_ExternalTextureParams = load %texture_params
    %10:vec4<f32> = call %tint_TextureSampleExternal, %7, %8, %9, %sampler, %coords
    ret %10
  }
}
%tint_TextureSampleExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> -> %b3 {  # %sampler_1: 'sampler', %coords_1: 'coords'
  %b3 = block {
    %17:u32 = access %params, 1u
    %18:mat3x4<f32> = access %params, 2u
    %19:mat3x2<f32> = access %params, 6u
    %20:vec3<f32> = construct %coords_1, 1.0f
    %21:vec2<f32> = mul %19, %20
    %22:vec2<u32> = textureDimensions %plane_0
    %23:vec2<f32> = convert %22
    %24:vec2<f32> = div vec2<f32>(0.5f), %23
    %25:vec2<f32> = sub 1.0f, %24
    %26:vec2<f32> = clamp %21, %24, %25
    %27:vec2<u32> = textureDimensions %plane_1
    %28:vec2<f32> = convert %27
    %29:vec2<f32> = div vec2<f32>(0.5f), %28
    %30:vec2<f32> = sub 1.0f, %29
    %31:vec2<f32> = clamp %21, %29, %30
    %32:u32 = access %params, 0u
    %33:bool = eq %32, 1u
    %34:vec3<f32>, %35:f32 = if %33 [t: %b4, f: %b5] {  # if_1
      %b4 = block {  # true
        %36:vec4<f32> = textureSampleLevel %plane_0, %sampler_1, %26, 0.0f
        %37:vec3<f32> = swizzle %36, xyz
        %38:f32 = access %36, 3u
        exit_if %37, %38  # if_1
      }
      %b5 = block {  # false
        %39:vec4<f32> = textureSampleLevel %plane_0, %sampler_1, %26, 0.0f
        %40:f32 = access %39, 0u
        %41:vec4<f32> = textureSampleLevel %plane_1, %sampler_1, %31, 0.0f
        %42:vec2<f32> = swizzle %41, xy
        %43:vec4<f32> = construct %40, %42, 1.0f
        %44:vec3<f32> = mul %43, %18
        exit_if %44, 1.0f  # if_1
      }
    }
    %45:bool = eq %17, 0u
    %46:vec3<f32> = if %45 [t: %b6, f: %b7] {  # if_2
      %b6 = block {  # true
        %47:tint_GammaTransferParams = access %params, 3u
        %48:tint_GammaTransferParams = access %params, 4u
        %49:mat3x3<f32> = access %params, 5u
        %50:vec3<f32> = call %tint_GammaCorrection, %34, %47
        %52:vec3<f32> = mul %49, %50
        %53:vec3<f32> = call %tint_GammaCorrection, %52, %48
        exit_if %53  # if_2
      }
      %b7 = block {  # false
        exit_if %34  # if_2
      }
    }
    %54:vec4<f32> = construct %46, %35
    ret %54
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> -> %b8 {  # %params_1: 'params'
  %b8 = block {
    %57:f32 = access %params_1, 0u
    %58:f32 = access %params_1, 1u
    %59:f32 = access %params_1, 2u
    %60:f32 = access %params_1, 3u
    %61:f32 = access %params_1, 4u
    %62:f32 = access %params_1, 5u
    %63:f32 = access %params_1, 6u
    %64:vec3<f32> = construct %57
    %65:vec3<f32> = construct %61
    %66:vec3<f32> = abs %v
    %67:vec3<f32> = sign %v
    %68:vec3<bool> = lt %66, %65
    %69:vec3<f32> = mul %60, %66
    %70:vec3<f32> = add %69, %63
    %71:vec3<f32> = mul %67, %70
    %72:vec3<f32> = mul %58, %66
    %73:vec3<f32> = add %72, %59
    %74:vec3<f32> = pow %73, %64
    %75:vec3<f32> = add %74, %62
    %76:vec3<f32> = mul %67, %75
    %77:vec3<f32> = select %76, %71, %68
    ret %77
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, ViaUserFunctionParameter) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.vec4<f32>());
    {
        auto* texture = b.FunctionParam("texture", ty.Get<core::type::ExternalTexture>());
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
        foo->SetParams({texture, sampler, coords});
        b.Append(foo->Block(), [&] {
            auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge,
                                  texture, sampler, coords);
            b.Return(foo, result);
            mod.SetName(result, "result");
        });
    }

    auto* bar = b.Function("bar", ty.vec4<f32>());
    {
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
        bar->SetParams({sampler, coords});
        b.Append(bar->Block(), [&] {
            auto* load = b.Load(var->Result());
            auto* result = b.Call(ty.vec4<f32>(), foo, load, sampler, coords);
            b.Return(bar, result);
            mod.SetName(result, "result");
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func(%texture_1:texture_external, %sampler:sampler, %coords:vec2<f32>):vec4<f32> -> %b2 {  # %texture_1: 'texture'
  %b2 = block {
    %result:vec4<f32> = textureSampleBaseClampToEdge %texture_1, %sampler, %coords
    ret %result
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> -> %b3 {  # %sampler_1: 'sampler', %coords_1: 'coords'
  %b3 = block {
    %10:texture_external = load %texture
    %result_1:vec4<f32> = call %foo, %10, %sampler_1, %coords_1  # %result_1: 'result'
    ret %result_1
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func(%texture_plane0_1:texture_2d<f32>, %texture_plane1_1:texture_2d<f32>, %texture_params_1:tint_ExternalTextureParams, %sampler:sampler, %coords:vec2<f32>):vec4<f32> -> %b2 {  # %texture_plane0_1: 'texture_plane0', %texture_plane1_1: 'texture_plane1', %texture_params_1: 'texture_params'
  %b2 = block {
    %10:vec4<f32> = call %tint_TextureSampleExternal, %texture_plane0_1, %texture_plane1_1, %texture_params_1, %sampler, %coords
    ret %10
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> -> %b3 {  # %sampler_1: 'sampler', %coords_1: 'coords'
  %b3 = block {
    %15:texture_2d<f32> = load %texture_plane0
    %16:texture_2d<f32> = load %texture_plane1
    %17:tint_ExternalTextureParams = load %texture_params
    %result:vec4<f32> = call %foo, %15, %16, %17, %sampler_1, %coords_1
    ret %result
  }
}
%tint_TextureSampleExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %sampler_2:sampler, %coords_2:vec2<f32>):vec4<f32> -> %b4 {  # %sampler_2: 'sampler', %coords_2: 'coords'
  %b4 = block {
    %24:u32 = access %params, 1u
    %25:mat3x4<f32> = access %params, 2u
    %26:mat3x2<f32> = access %params, 6u
    %27:vec3<f32> = construct %coords_2, 1.0f
    %28:vec2<f32> = mul %26, %27
    %29:vec2<u32> = textureDimensions %plane_0
    %30:vec2<f32> = convert %29
    %31:vec2<f32> = div vec2<f32>(0.5f), %30
    %32:vec2<f32> = sub 1.0f, %31
    %33:vec2<f32> = clamp %28, %31, %32
    %34:vec2<u32> = textureDimensions %plane_1
    %35:vec2<f32> = convert %34
    %36:vec2<f32> = div vec2<f32>(0.5f), %35
    %37:vec2<f32> = sub 1.0f, %36
    %38:vec2<f32> = clamp %28, %36, %37
    %39:u32 = access %params, 0u
    %40:bool = eq %39, 1u
    %41:vec3<f32>, %42:f32 = if %40 [t: %b5, f: %b6] {  # if_1
      %b5 = block {  # true
        %43:vec4<f32> = textureSampleLevel %plane_0, %sampler_2, %33, 0.0f
        %44:vec3<f32> = swizzle %43, xyz
        %45:f32 = access %43, 3u
        exit_if %44, %45  # if_1
      }
      %b6 = block {  # false
        %46:vec4<f32> = textureSampleLevel %plane_0, %sampler_2, %33, 0.0f
        %47:f32 = access %46, 0u
        %48:vec4<f32> = textureSampleLevel %plane_1, %sampler_2, %38, 0.0f
        %49:vec2<f32> = swizzle %48, xy
        %50:vec4<f32> = construct %47, %49, 1.0f
        %51:vec3<f32> = mul %50, %25
        exit_if %51, 1.0f  # if_1
      }
    }
    %52:bool = eq %24, 0u
    %53:vec3<f32> = if %52 [t: %b7, f: %b8] {  # if_2
      %b7 = block {  # true
        %54:tint_GammaTransferParams = access %params, 3u
        %55:tint_GammaTransferParams = access %params, 4u
        %56:mat3x3<f32> = access %params, 5u
        %57:vec3<f32> = call %tint_GammaCorrection, %41, %54
        %59:vec3<f32> = mul %56, %57
        %60:vec3<f32> = call %tint_GammaCorrection, %59, %55
        exit_if %60  # if_2
      }
      %b8 = block {  # false
        exit_if %41  # if_2
      }
    }
    %61:vec4<f32> = construct %53, %42
    ret %61
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> -> %b9 {  # %params_1: 'params'
  %b9 = block {
    %64:f32 = access %params_1, 0u
    %65:f32 = access %params_1, 1u
    %66:f32 = access %params_1, 2u
    %67:f32 = access %params_1, 3u
    %68:f32 = access %params_1, 4u
    %69:f32 = access %params_1, 5u
    %70:f32 = access %params_1, 6u
    %71:vec3<f32> = construct %64
    %72:vec3<f32> = construct %68
    %73:vec3<f32> = abs %v
    %74:vec3<f32> = sign %v
    %75:vec3<bool> = lt %73, %72
    %76:vec3<f32> = mul %67, %73
    %77:vec3<f32> = add %76, %70
    %78:vec3<f32> = mul %74, %77
    %79:vec3<f32> = mul %65, %73
    %80:vec3<f32> = add %79, %66
    %81:vec3<f32> = pow %80, %71
    %82:vec3<f32> = add %81, %69
    %83:vec3<f32> = mul %74, %82
    %84:vec3<f32> = select %83, %78, %75
    ret %84
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultipleUses) {
    auto* var = b.Var("texture", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var->SetBindingPoint(1, 2);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.vec4<f32>());
    {
        auto* texture = b.FunctionParam("texture", ty.Get<core::type::ExternalTexture>());
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
        foo->SetParams({texture, sampler, coords});
        b.Append(foo->Block(), [&] {
            auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge,
                                  texture, sampler, coords);
            b.Return(foo, result);
            mod.SetName(result, "result");
        });
    }

    auto* bar = b.Function("bar", ty.vec4<f32>());
    {
        auto* sampler = b.FunctionParam("sampler", ty.sampler());
        auto* coords_f = b.FunctionParam("coords", ty.vec2<f32>());
        bar->SetParams({sampler, coords_f});
        b.Append(bar->Block(), [&] {
            auto* load_a = b.Load(var->Result());
            b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, load_a);
            auto* load_b = b.Load(var->Result());
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load_b, sampler,
                   coords_f);
            auto* load_c = b.Load(var->Result());
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBaseClampToEdge, load_c, sampler,
                   coords_f);
            auto* load_d = b.Load(var->Result());
            auto* result_a = b.Call(ty.vec4<f32>(), foo, load_d, sampler, coords_f);
            auto* result_b = b.Call(ty.vec4<f32>(), foo, load_d, sampler, coords_f);
            b.Return(bar, b.Add(ty.vec4<f32>(), result_a, result_b));
            mod.SetName(result_a, "result_a");
            mod.SetName(result_b, "result_b");
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
}

%foo = func(%texture_1:texture_external, %sampler:sampler, %coords:vec2<f32>):vec4<f32> -> %b2 {  # %texture_1: 'texture'
  %b2 = block {
    %result:vec4<f32> = textureSampleBaseClampToEdge %texture_1, %sampler, %coords
    ret %result
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> -> %b3 {  # %sampler_1: 'sampler', %coords_1: 'coords'
  %b3 = block {
    %10:texture_external = load %texture
    %11:vec2<u32> = textureDimensions %10
    %12:texture_external = load %texture
    %13:vec4<f32> = textureSampleBaseClampToEdge %12, %sampler_1, %coords_1
    %14:texture_external = load %texture
    %15:vec4<f32> = textureSampleBaseClampToEdge %14, %sampler_1, %coords_1
    %16:texture_external = load %texture
    %result_a:vec4<f32> = call %foo, %16, %sampler_1, %coords_1
    %result_b:vec4<f32> = call %foo, %16, %sampler_1, %coords_1
    %19:vec4<f32> = add %result_a, %result_b
    ret %19
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
}

%foo = func(%texture_plane0_1:texture_2d<f32>, %texture_plane1_1:texture_2d<f32>, %texture_params_1:tint_ExternalTextureParams, %sampler:sampler, %coords:vec2<f32>):vec4<f32> -> %b2 {  # %texture_plane0_1: 'texture_plane0', %texture_plane1_1: 'texture_plane1', %texture_params_1: 'texture_params'
  %b2 = block {
    %10:vec4<f32> = call %tint_TextureSampleExternal, %texture_plane0_1, %texture_plane1_1, %texture_params_1, %sampler, %coords
    ret %10
  }
}
%bar = func(%sampler_1:sampler, %coords_1:vec2<f32>):vec4<f32> -> %b3 {  # %sampler_1: 'sampler', %coords_1: 'coords'
  %b3 = block {
    %15:texture_2d<f32> = load %texture_plane0
    %16:texture_2d<f32> = load %texture_plane1
    %17:tint_ExternalTextureParams = load %texture_params
    %18:vec2<u32> = textureDimensions %15
    %19:texture_2d<f32> = load %texture_plane0
    %20:texture_2d<f32> = load %texture_plane1
    %21:tint_ExternalTextureParams = load %texture_params
    %22:vec4<f32> = call %tint_TextureSampleExternal, %19, %20, %21, %sampler_1, %coords_1
    %23:texture_2d<f32> = load %texture_plane0
    %24:texture_2d<f32> = load %texture_plane1
    %25:tint_ExternalTextureParams = load %texture_params
    %26:vec4<f32> = call %tint_TextureSampleExternal, %23, %24, %25, %sampler_1, %coords_1
    %27:texture_2d<f32> = load %texture_plane0
    %28:texture_2d<f32> = load %texture_plane1
    %29:tint_ExternalTextureParams = load %texture_params
    %result_a:vec4<f32> = call %foo, %27, %28, %29, %sampler_1, %coords_1
    %result_b:vec4<f32> = call %foo, %27, %28, %29, %sampler_1, %coords_1
    %32:vec4<f32> = add %result_a, %result_b
    ret %32
  }
}
%tint_TextureSampleExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %sampler_2:sampler, %coords_2:vec2<f32>):vec4<f32> -> %b4 {  # %sampler_2: 'sampler', %coords_2: 'coords'
  %b4 = block {
    %38:u32 = access %params, 1u
    %39:mat3x4<f32> = access %params, 2u
    %40:mat3x2<f32> = access %params, 6u
    %41:vec3<f32> = construct %coords_2, 1.0f
    %42:vec2<f32> = mul %40, %41
    %43:vec2<u32> = textureDimensions %plane_0
    %44:vec2<f32> = convert %43
    %45:vec2<f32> = div vec2<f32>(0.5f), %44
    %46:vec2<f32> = sub 1.0f, %45
    %47:vec2<f32> = clamp %42, %45, %46
    %48:vec2<u32> = textureDimensions %plane_1
    %49:vec2<f32> = convert %48
    %50:vec2<f32> = div vec2<f32>(0.5f), %49
    %51:vec2<f32> = sub 1.0f, %50
    %52:vec2<f32> = clamp %42, %50, %51
    %53:u32 = access %params, 0u
    %54:bool = eq %53, 1u
    %55:vec3<f32>, %56:f32 = if %54 [t: %b5, f: %b6] {  # if_1
      %b5 = block {  # true
        %57:vec4<f32> = textureSampleLevel %plane_0, %sampler_2, %47, 0.0f
        %58:vec3<f32> = swizzle %57, xyz
        %59:f32 = access %57, 3u
        exit_if %58, %59  # if_1
      }
      %b6 = block {  # false
        %60:vec4<f32> = textureSampleLevel %plane_0, %sampler_2, %47, 0.0f
        %61:f32 = access %60, 0u
        %62:vec4<f32> = textureSampleLevel %plane_1, %sampler_2, %52, 0.0f
        %63:vec2<f32> = swizzle %62, xy
        %64:vec4<f32> = construct %61, %63, 1.0f
        %65:vec3<f32> = mul %64, %39
        exit_if %65, 1.0f  # if_1
      }
    }
    %66:bool = eq %38, 0u
    %67:vec3<f32> = if %66 [t: %b7, f: %b8] {  # if_2
      %b7 = block {  # true
        %68:tint_GammaTransferParams = access %params, 3u
        %69:tint_GammaTransferParams = access %params, 4u
        %70:mat3x3<f32> = access %params, 5u
        %71:vec3<f32> = call %tint_GammaCorrection, %55, %68
        %73:vec3<f32> = mul %70, %71
        %74:vec3<f32> = call %tint_GammaCorrection, %73, %69
        exit_if %74  # if_2
      }
      %b8 = block {  # false
        exit_if %55  # if_2
      }
    }
    %75:vec4<f32> = construct %67, %56
    ret %75
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> -> %b9 {  # %params_1: 'params'
  %b9 = block {
    %78:f32 = access %params_1, 0u
    %79:f32 = access %params_1, 1u
    %80:f32 = access %params_1, 2u
    %81:f32 = access %params_1, 3u
    %82:f32 = access %params_1, 4u
    %83:f32 = access %params_1, 5u
    %84:f32 = access %params_1, 6u
    %85:vec3<f32> = construct %78
    %86:vec3<f32> = construct %82
    %87:vec3<f32> = abs %v
    %88:vec3<f32> = sign %v
    %89:vec3<bool> = lt %87, %86
    %90:vec3<f32> = mul %81, %87
    %91:vec3<f32> = add %90, %84
    %92:vec3<f32> = mul %88, %91
    %93:vec3<f32> = mul %79, %87
    %94:vec3<f32> = add %93, %80
    %95:vec3<f32> = pow %94, %85
    %96:vec3<f32> = add %95, %83
    %97:vec3<f32> = mul %88, %96
    %98:vec3<f32> = select %97, %92, %89
    ret %98
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_MultiplanarExternalTextureTest, MultipleTextures) {
    auto* var_a = b.Var("texture_a", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* var_b = b.Var("texture_b", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var_b->SetBindingPoint(2, 2);
    mod.root_block->Append(var_b);

    auto* var_c = b.Var("texture_c", ty.ptr(handle, ty.Get<core::type::ExternalTexture>()));
    var_c->SetBindingPoint(3, 2);
    mod.root_block->Append(var_c);

    auto* foo = b.Function("foo", ty.void_());
    auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
    foo->SetParams({coords});
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a->Result());
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load_a, coords);
        auto* load_b = b.Load(var_b->Result());
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load_b, coords);
        auto* load_c = b.Load(var_c->Result());
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, load_c, coords);
        b.Return(foo);
    });

    auto* src = R"(
%b1 = block {  # root
  %texture_a:ptr<handle, texture_external, read_write> = var @binding_point(1, 2)
  %texture_b:ptr<handle, texture_external, read_write> = var @binding_point(2, 2)
  %texture_c:ptr<handle, texture_external, read_write> = var @binding_point(3, 2)
}

%foo = func(%coords:vec2<u32>):void -> %b2 {
  %b2 = block {
    %6:texture_external = load %texture_a
    %7:vec4<f32> = textureLoad %6, %coords
    %8:texture_external = load %texture_b
    %9:vec4<f32> = textureLoad %8, %coords
    %10:texture_external = load %texture_c
    %11:vec4<f32> = textureLoad %10, %coords
    ret
  }
}
)";
    auto* expect = R"(
tint_GammaTransferParams = struct @align(4) {
  G:f32 @offset(0)
  A:f32 @offset(4)
  B:f32 @offset(8)
  C:f32 @offset(12)
  D:f32 @offset(16)
  E:f32 @offset(20)
  F:f32 @offset(24)
  padding:u32 @offset(28)
}

tint_ExternalTextureParams = struct @align(16) {
  numPlanes:u32 @offset(0)
  doYuvToRgbConversionOnly:u32 @offset(4)
  yuvToRgbConversionMatrix:mat3x4<f32> @offset(16)
  gammaDecodeParams:tint_GammaTransferParams @offset(64)
  gammaEncodeParams:tint_GammaTransferParams @offset(96)
  gamutConversionMatrix:mat3x3<f32> @offset(128)
  coordTransformationMatrix:mat3x2<f32> @offset(176)
}

%b1 = block {  # root
  %texture_a_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 2)
  %texture_a_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(1, 3)
  %texture_a_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(1, 4)
  %texture_b_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(2, 2)
  %texture_b_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(2, 3)
  %texture_b_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(2, 4)
  %texture_c_plane0:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(3, 2)
  %texture_c_plane1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(3, 3)
  %texture_c_params:ptr<uniform, tint_ExternalTextureParams, read_write> = var @binding_point(3, 4)
}

%foo = func(%coords:vec2<u32>):void -> %b2 {
  %b2 = block {
    %12:texture_2d<f32> = load %texture_a_plane0
    %13:texture_2d<f32> = load %texture_a_plane1
    %14:tint_ExternalTextureParams = load %texture_a_params
    %15:vec4<f32> = call %tint_TextureLoadExternal, %12, %13, %14, %coords
    %17:texture_2d<f32> = load %texture_b_plane0
    %18:texture_2d<f32> = load %texture_b_plane1
    %19:tint_ExternalTextureParams = load %texture_b_params
    %20:vec4<f32> = call %tint_TextureLoadExternal, %17, %18, %19, %coords
    %21:texture_2d<f32> = load %texture_c_plane0
    %22:texture_2d<f32> = load %texture_c_plane1
    %23:tint_ExternalTextureParams = load %texture_c_params
    %24:vec4<f32> = call %tint_TextureLoadExternal, %21, %22, %23, %coords
    ret
  }
}
%tint_TextureLoadExternal = func(%plane_0:texture_2d<f32>, %plane_1:texture_2d<f32>, %params:tint_ExternalTextureParams, %coords_1:vec2<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %29:u32 = access %params, 1u
    %30:mat3x4<f32> = access %params, 2u
    %31:u32 = access %params, 0u
    %32:bool = eq %31, 1u
    %33:vec3<f32>, %34:f32 = if %32 [t: %b4, f: %b5] {  # if_1
      %b4 = block {  # true
        %35:vec4<f32> = textureLoad %plane_0, %coords_1, 0u
        %36:vec3<f32> = swizzle %35, xyz
        %37:f32 = access %35, 3u
        exit_if %36, %37  # if_1
      }
      %b5 = block {  # false
        %38:vec4<f32> = textureLoad %plane_0, %coords_1, 0u
        %39:f32 = access %38, 0u
        %40:vec2<u32> = shiftr %coords_1, vec2<u32>(1u)
        %41:vec4<f32> = textureLoad %plane_1, %40, 0u
        %42:vec2<f32> = swizzle %41, xy
        %43:vec4<f32> = construct %39, %42, 1.0f
        %44:vec3<f32> = mul %43, %30
        exit_if %44, 1.0f  # if_1
      }
    }
    %45:bool = eq %29, 0u
    %46:vec3<f32> = if %45 [t: %b6, f: %b7] {  # if_2
      %b6 = block {  # true
        %47:tint_GammaTransferParams = access %params, 3u
        %48:tint_GammaTransferParams = access %params, 4u
        %49:mat3x3<f32> = access %params, 5u
        %50:vec3<f32> = call %tint_GammaCorrection, %33, %47
        %52:vec3<f32> = mul %49, %50
        %53:vec3<f32> = call %tint_GammaCorrection, %52, %48
        exit_if %53  # if_2
      }
      %b7 = block {  # false
        exit_if %33  # if_2
      }
    }
    %54:vec4<f32> = construct %46, %34
    ret %54
  }
}
%tint_GammaCorrection = func(%v:vec3<f32>, %params_1:tint_GammaTransferParams):vec3<f32> -> %b8 {  # %params_1: 'params'
  %b8 = block {
    %57:f32 = access %params_1, 0u
    %58:f32 = access %params_1, 1u
    %59:f32 = access %params_1, 2u
    %60:f32 = access %params_1, 3u
    %61:f32 = access %params_1, 4u
    %62:f32 = access %params_1, 5u
    %63:f32 = access %params_1, 6u
    %64:vec3<f32> = construct %57
    %65:vec3<f32> = construct %61
    %66:vec3<f32> = abs %v
    %67:vec3<f32> = sign %v
    %68:vec3<bool> = lt %66, %65
    %69:vec3<f32> = mul %60, %66
    %70:vec3<f32> = add %69, %63
    %71:vec3<f32> = mul %67, %70
    %72:vec3<f32> = mul %58, %66
    %73:vec3<f32> = add %72, %59
    %74:vec3<f32> = pow %73, %64
    %75:vec3<f32> = add %74, %62
    %76:vec3<f32> = mul %67, %75
    %77:vec3<f32> = select %76, %71, %68
    ret %77
  }
}
)";

    EXPECT_EQ(src, str());

    ExternalTextureOptions options;
    options.bindings_map[{1u, 2u}] = {{1u, 3u}, {1u, 4u}};
    options.bindings_map[{2u, 2u}] = {{2u, 3u}, {2u, 4u}};
    options.bindings_map[{3u, 2u}] = {{3u, 3u}, {3u, 4u}};
    Run(MultiplanarExternalTexture, options);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
