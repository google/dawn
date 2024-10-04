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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/hlsl/writer/raise/shader_io.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using HlslWriterTransformTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterTransformTest, ShaderIONoInputsOrOutputs) {
    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetWorkgroupSize(1, 1, 1);
    b.Append(ep->Block(), [&] { b.Return(ep); });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NonStruct) {
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);

    auto* position = b.FunctionParam("position", ty.vec4<f32>());
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    auto* color1 = b.FunctionParam("color1", ty.f32());
    color1->SetLocation(0);

    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({front_facing, position, color1, color2});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {
            b.Multiply(ty.vec4<f32>(), position, b.Add(ty.f32(), color1, color2));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
%foo = @fragment func(%front_facing:bool [@front_facing], %position:vec4<f32> [@invariant, @position], %color1:f32 [@location(0)], %color2:f32 [@location(1), @interpolate(linear, sample)]):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %6:f32 = add %color1, %color2
        %7:vec4<f32> = mul %position, %6
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  position:vec4<f32> @offset(16), @invariant, @builtin(position)
  front_facing:bool @offset(32), @builtin(front_facing)
}

%foo_inner = func(%front_facing:bool, %position:vec4<f32>, %color1:f32, %color2:f32):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %6:f32 = add %color1, %color2
        %7:vec4<f32> = mul %position, %6
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func(%inputs:foo_inputs):void {
  $B3: {
    %10:bool = access %inputs, 3u
    %11:vec4<f32> = access %inputs, 2u
    %12:f32 = access %11, 3u
    %13:f32 = div 1.0f, %12
    %14:vec3<f32> = swizzle %11, xyz
    %15:vec4<f32> = construct %14, %13
    %16:f32 = access %inputs, 0u
    %17:f32 = access %inputs, 1u
    %18:void = call %foo_inner, %10, %15, %16, %17
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("front_facing"),
                                     ty.bool_(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kFrontFacing,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 1u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */
                                         core::Interpolation{
                                             core::InterpolationType::kLinear,
                                             core::InterpolationSampling::kSample,
                                         },
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({str_param});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(b.Access(ty.bool_(), str_param, 0_i));
        b.Append(ifelse->True(), [&] {
            auto* position = b.Access(ty.vec4<f32>(), str_param, 1_i);
            auto* color1 = b.Access(ty.f32(), str_param, 2_i);
            auto* color2 = b.Access(ty.f32(), str_param, 3_i);
            b.Multiply(ty.vec4<f32>(), position, b.Add(ty.f32(), color1, color2));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  front_facing:bool @offset(0), @builtin(front_facing)
  position:vec4<f32> @offset(16), @invariant, @builtin(position)
  color1:f32 @offset(32), @location(0)
  color2:f32 @offset(36), @location(1), @interpolate(linear, sample)
}

%foo = @fragment func(%inputs:Inputs):void {
  $B1: {
    %3:bool = access %inputs, 0i
    if %3 [t: $B2] {  # if_1
      $B2: {  # true
        %4:vec4<f32> = access %inputs, 1i
        %5:f32 = access %inputs, 2i
        %6:f32 = access %inputs, 3i
        %7:f32 = add %5, %6
        %8:vec4<f32> = mul %4, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  front_facing:bool @offset(0)
  position:vec4<f32> @offset(16)
  color1:f32 @offset(32)
  color2:f32 @offset(36)
}

foo_inputs = struct @align(16) {
  Inputs_color1:f32 @offset(0), @location(0)
  Inputs_color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  Inputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
  Inputs_front_facing:bool @offset(32), @builtin(front_facing)
}

%foo_inner = func(%inputs:Inputs):void {
  $B1: {
    %3:bool = access %inputs, 0i
    if %3 [t: $B2] {  # if_1
      $B2: {  # true
        %4:vec4<f32> = access %inputs, 1i
        %5:f32 = access %inputs, 2i
        %6:f32 = access %inputs, 3i
        %7:f32 = add %5, %6
        %8:vec4<f32> = mul %4, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func(%inputs_1:foo_inputs):void {  # %inputs_1: 'inputs'
  $B3: {
    %11:bool = access %inputs_1, 3u
    %12:vec4<f32> = access %inputs_1, 2u
    %13:f32 = access %12, 3u
    %14:f32 = div 1.0f, %13
    %15:vec3<f32> = swizzle %12, xyz
    %16:vec4<f32> = construct %15, %14
    %17:f32 = access %inputs_1, 0u
    %18:f32 = access %inputs_1, 1u
    %19:Inputs = construct %11, %16, %17, %18
    %20:void = call %foo_inner, %19
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Mixed) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({front_facing, str_param, color2});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {
            auto* position = b.Access(ty.vec4<f32>(), str_param, 0_i);
            auto* color1 = b.Access(ty.f32(), str_param, 1_i);
            b.Multiply(ty.vec4<f32>(), position, b.Add(ty.f32(), color1, color2));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color1:f32 @offset(16), @location(0)
}

%foo = @fragment func(%front_facing:bool [@front_facing], %inputs:Inputs, %color2:f32 [@location(1), @interpolate(linear, sample)]):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %5:vec4<f32> = access %inputs, 0i
        %6:f32 = access %inputs, 1i
        %7:f32 = add %6, %color2
        %8:vec4<f32> = mul %5, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color1:f32 @offset(16)
}

foo_inputs = struct @align(16) {
  Inputs_color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  Inputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
  front_facing:bool @offset(32), @builtin(front_facing)
}

%foo_inner = func(%front_facing:bool, %inputs:Inputs, %color2:f32):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %5:vec4<f32> = access %inputs, 0i
        %6:f32 = access %inputs, 1i
        %7:f32 = add %6, %color2
        %8:vec4<f32> = mul %5, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func(%inputs_1:foo_inputs):void {  # %inputs_1: 'inputs'
  $B3: {
    %11:bool = access %inputs_1, 3u
    %12:vec4<f32> = access %inputs_1, 2u
    %13:f32 = access %12, 3u
    %14:f32 = div 1.0f, %13
    %15:vec3<f32> = swizzle %12, xyz
    %16:vec4<f32> = construct %15, %14
    %17:f32 = access %inputs_1, 0u
    %18:Inputs = construct %16, %17
    %19:f32 = access %inputs_1, 1u
    %20:void = call %foo_inner, %11, %18, %19
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_NonStructBuiltin) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);

    b.Append(ep->Block(), [&] { b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f)); });

    auto* src = R"(
%foo = @vertex func():vec4<f32> [@invariant, @position] {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @invariant, @builtin(position)
}

%foo_inner = func():vec4<f32> {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %4:vec4<f32> = call %foo_inner
    %5:foo_outputs = construct %4
    ret %5
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_NonStructLocation) {
    auto* ep = b.Function("foo", ty.vec4<f32>(), core::ir::Function::PipelineStage::kFragment);
    ep->SetReturnLocation(1u);

    b.Append(ep->Block(), [&] { b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f)); });

    auto* src = R"(
%foo = @fragment func():vec4<f32> [@location(1)] {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @location(1)
}

%foo_inner = func():vec4<f32> {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
%foo = @fragment func():foo_outputs {
  $B2: {
    %4:vec4<f32> = call %foo_inner
    %5:foo_outputs = construct %4
    ret %5
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 1u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */
                                         core::Interpolation{
                                             core::InterpolationType::kLinear,
                                             core::InterpolationSampling::kSample,
                                         },
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {
        b.Return(ep, b.Construct(str_ty, b.Construct(ty.vec4<f32>(), 0_f), 0.25_f, 0.75_f));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color1:f32 @offset(16), @location(0)
  color2:f32 @offset(20), @location(1), @interpolate(linear, sample)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:Outputs = construct %2, 0.25f, 0.75f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color1:f32 @offset(16)
  color2:f32 @offset(20)
}

foo_outputs = struct @align(16) {
  Outputs_color1:f32 @offset(0), @location(0)
  Outputs_color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  Outputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:Outputs = construct %2, 0.25f, 0.75f
    ret %3
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %5:Outputs = call %foo_inner
    %6:vec4<f32> = access %5, 0u
    %7:f32 = access %5, 1u
    %8:f32 = access %5, 2u
    %9:foo_outputs = construct %7, %8, %6
    ret %9
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_DualSourceBlending) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Output"), {
                                                 {
                                                     mod.symbols.New("color1"),
                                                     ty.f32(),
                                                     core::IOAttributes{
                                                         /* location */ 0u,
                                                         /* blend_src */ 0u,
                                                         /* color */ std::nullopt,
                                                         /* builtin */ std::nullopt,
                                                         /* interpolation */ std::nullopt,
                                                         /* invariant */ false,
                                                     },
                                                 },
                                                 {
                                                     mod.symbols.New("color2"),
                                                     ty.f32(),
                                                     core::IOAttributes{
                                                         /* location */ 0u,
                                                         /* blend_src */ 1u,
                                                         /* color */ std::nullopt,
                                                         /* builtin */ std::nullopt,
                                                         /* interpolation */ std::nullopt,
                                                         /* invariant */ false,
                                                     },
                                                 },
                                             });

    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] { b.Return(ep, b.Construct(str_ty, 0.25_f, 0.75_f)); });

    auto* src = R"(
Output = struct @align(4) {
  color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(0)
}

%foo = @fragment func():Output {
  $B1: {
    %2:Output = construct 0.25f, 0.75f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Output = struct @align(4) {
  color1:f32 @offset(0)
  color2:f32 @offset(4)
}

foo_outputs = struct @align(4) {
  Output_color1:f32 @offset(0), @location(0)
  Output_color2:f32 @offset(4), @location(0)
}

%foo_inner = func():Output {
  $B1: {
    %2:Output = construct 0.25f, 0.75f
    ret %2
  }
}
%foo = @fragment func():foo_outputs {
  $B2: {
    %4:Output = call %foo_inner
    %5:f32 = access %4, 0u
    %6:f32 = access %4, 1u
    %7:foo_outputs = construct %5, %6
    ret %7
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOStruct_SharedByVertexAndFragment) {
    auto* str_ty = ty.Struct(mod.symbols.New("Interface"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     ty.vec3<f32>(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    // Vertex shader.
    {
        auto* ep = b.Function("vert", str_ty, core::ir::Function::PipelineStage::kVertex);

        b.Append(ep->Block(), [&] {
            auto* position = b.Construct(ty.vec4<f32>(), 0_f);
            auto* color = b.Construct(ty.vec3<f32>(), 1_f);
            b.Return(ep, b.Construct(str_ty, position, color));
        });
    }

    // Fragment shader.
    {
        auto* inputs = b.FunctionParam("inputs", str_ty);

        auto* ep = b.Function("frag", ty.vec4<f32>(), core::ir::Function::PipelineStage::kFragment);
        ep->SetParams({inputs});
        ep->SetReturnLocation(0u);

        b.Append(ep->Block(), [&] {
            auto* position = b.Access(ty.vec4<f32>(), inputs, 0_u);
            auto* color = b.Access(ty.vec3<f32>(), inputs, 1_u);
            b.Return(ep, b.Add(ty.vec4<f32>(), position, b.Construct(ty.vec4<f32>(), color, 1_f)));
        });
    }

    auto* src = R"(
Interface = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec3<f32> @offset(16), @location(0)
}

%vert = @vertex func():Interface {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:vec3<f32> = construct 1.0f
    %4:Interface = construct %2, %3
    ret %4
  }
}
%frag = @fragment func(%inputs:Interface):vec4<f32> [@location(0)] {
  $B2: {
    %7:vec4<f32> = access %inputs, 0u
    %8:vec3<f32> = access %inputs, 1u
    %9:vec4<f32> = construct %8, 1.0f
    %10:vec4<f32> = add %7, %9
    ret %10
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Interface = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec3<f32> @offset(16)
}

vert_outputs = struct @align(16) {
  Interface_color:vec3<f32> @offset(0), @location(0)
  Interface_position:vec4<f32> @offset(16), @builtin(position)
}

frag_inputs = struct @align(16) {
  Interface_color:vec3<f32> @offset(0), @location(0)
  Interface_position:vec4<f32> @offset(16), @builtin(position)
}

frag_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @location(0)
}

%vert_inner = func():Interface {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:vec3<f32> = construct 1.0f
    %4:Interface = construct %2, %3
    ret %4
  }
}
%frag_inner = func(%inputs:Interface):vec4<f32> {
  $B2: {
    %7:vec4<f32> = access %inputs, 0u
    %8:vec3<f32> = access %inputs, 1u
    %9:vec4<f32> = construct %8, 1.0f
    %10:vec4<f32> = add %7, %9
    ret %10
  }
}
%vert = @vertex func():vert_outputs {
  $B3: {
    %12:Interface = call %vert_inner
    %13:vec4<f32> = access %12, 0u
    %14:vec3<f32> = access %12, 1u
    %15:vert_outputs = construct %14, %13
    ret %15
  }
}
%frag = @fragment func(%inputs_1:frag_inputs):frag_outputs {  # %inputs_1: 'inputs'
  $B4: {
    %18:vec4<f32> = access %inputs_1, 1u
    %19:f32 = access %18, 3u
    %20:f32 = div 1.0f, %19
    %21:vec3<f32> = swizzle %18, xyz
    %22:vec4<f32> = construct %21, %20
    %23:vec3<f32> = access %inputs_1, 0u
    %24:Interface = construct %22, %23
    %25:vec4<f32> = call %frag_inner, %24
    %26:frag_outputs = construct %25
    ret %26
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOStruct_SharedWithBuffer) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     ty.vec3<f32>(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* var = b.Var(ty.ptr(storage, str_ty, read));
    var->SetBindingPoint(0, 0);

    auto* buffer = mod.root_block->Append(var);

    auto* ep = b.Function("vert", str_ty, core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] { b.Return(ep, b.Load(buffer)); });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec3<f32> @offset(16), @location(0)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var @binding_point(0, 0)
}

%vert = @vertex func():Outputs {
  $B2: {
    %3:Outputs = load %1
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec3<f32> @offset(16)
}

vert_outputs = struct @align(16) {
  Outputs_color:vec3<f32> @offset(0), @location(0)
  Outputs_position:vec4<f32> @offset(16), @builtin(position)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var @binding_point(0, 0)
}

%vert_inner = func():Outputs {
  $B2: {
    %3:Outputs = load %1
    ret %3
  }
}
%vert = @vertex func():vert_outputs {
  $B3: {
    %5:Outputs = call %vert_inner
    %6:vec4<f32> = access %5, 0u
    %7:vec3<f32> = access %5, 1u
    %8:vert_outputs = construct %7, %6
    ret %8
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

// Test that IO attributes are stripped from structures that are not used for the shader interface.
TEST_F(HlslWriterTransformTest, ShaderIOStructWithAttributes_NotUsedForInterface) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* var = b.Var(ty.ptr(storage, str_ty, read));
    var->SetBindingPoint(0, 0);

    auto* buffer = mod.root_block->Append(var);

    auto* ep = b.Function("frag", ty.void_(), core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        b.Store(buffer, b.Construct(str_ty));
        b.Return(ep);
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec4<f32> @offset(16), @location(0)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var @binding_point(0, 0)
}

%frag = @fragment func():void {
  $B2: {
    %3:Outputs = construct
    store %1, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec4<f32> @offset(16)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var @binding_point(0, 0)
}

%frag = @fragment func():void {
  $B2: {
    %3:Outputs = construct
    store %1, %3
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOCompute) {
    auto* invoc = b.FunctionParam("invoc_id", ty.vec3<u32>());
    invoc->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* ep = b.Function("cmp", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetParams({invoc});
    ep->SetWorkgroupSize(1, 1, 1);

    b.Append(ep->Block(), [&] {
        b.Let("a", invoc);
        b.Return(ep);
    });

    auto* src = R"(
%cmp = @compute @workgroup_size(1, 1, 1) func(%invoc_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %a:vec3<u32> = let %invoc_id
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
cmp_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

%cmp_inner = func(%invoc_id:vec3<u32>):void {
  $B1: {
    %a:vec3<u32> = let %invoc_id
    ret
  }
}
%cmp = @compute @workgroup_size(1, 1, 1) func(%inputs:cmp_inputs):void {
  $B2: {
    %6:vec3<u32> = access %inputs, 0u
    %7:void = call %cmp_inner, %6
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Subgroup_NonStruct) {
    auto* subgroup_invocation_id = b.FunctionParam("id", ty.u32());
    subgroup_invocation_id->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);

    auto* subgroup_size = b.FunctionParam("size", ty.u32());
    subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({subgroup_invocation_id, subgroup_size});

    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(ty.u32(), subgroup_invocation_id, subgroup_size));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @fragment func(%id:u32 [@subgroup_invocation_id], %size:u32 [@subgroup_size]):void {
  $B1: {
    %4:u32 = mul %id, %size
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo_inner = func(%id:u32, %size:u32):void {
  $B1: {
    %4:u32 = mul %id, %size
    %x:u32 = let %4
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %7:u32 = hlsl.WaveGetLaneIndex
    %8:u32 = hlsl.WaveGetLaneCount
    %9:void = call %foo_inner, %7, %8
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Subgroup_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("id"),
                                     ty.u32(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kSubgroupInvocationId,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("size"),
                                     ty.u32(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kSubgroupSize,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({str_param});

    b.Append(ep->Block(), [&] {
        auto* subgroup_invocation_id = b.Access(ty.u32(), str_param, 0_i);
        auto* subgroup_size = b.Access(ty.u32(), str_param, 1_i);
        b.Let("x", b.Multiply(ty.u32(), subgroup_invocation_id, subgroup_size));
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(4) {
  id:u32 @offset(0), @builtin(subgroup_invocation_id)
  size:u32 @offset(4), @builtin(subgroup_size)
}

%foo = @fragment func(%inputs:Inputs):void {
  $B1: {
    %3:u32 = access %inputs, 0i
    %4:u32 = access %inputs, 1i
    %5:u32 = mul %3, %4
    %x:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  id:u32 @offset(0)
  size:u32 @offset(4)
}

%foo_inner = func(%inputs:Inputs):void {
  $B1: {
    %3:u32 = access %inputs, 0i
    %4:u32 = access %inputs, 1i
    %5:u32 = mul %3, %4
    %x:u32 = let %5
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %8:u32 = hlsl.WaveGetLaneIndex
    %9:u32 = hlsl.WaveGetLaneCount
    %10:Inputs = construct %8, %9
    %11:void = call %foo_inner, %10
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_NonStruct) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3<u32>());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetParams({num_workgroups});
    ep->SetWorkgroupSize(1, 1, 1);

    b.Append(ep->Block(), [&] {
        b.Multiply(ty.vec3<u32>(), num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func(%num_wgs:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %3:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var @binding_point(0, 0)
}

%foo_inner = func(%num_wgs:vec3<u32>):void {
  $B2: {
    %4:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %6:vec3<u32> = load %tint_num_workgroups
    %7:void = call %foo_inner, %6
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("num_wgs"),
                                     ty.vec3<u32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kNumWorkgroups,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetParams({str_param});
    ep->SetWorkgroupSize(1, 1, 1);

    b.Append(ep->Block(), [&] {
        auto* num_workgroups = b.Access(ty.vec3<u32>(), str_param, 0_i);
        b.Multiply(ty.vec3<u32>(), num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  num_wgs:vec3<u32> @offset(0), @builtin(num_workgroups)
}

%foo = @compute @workgroup_size(1, 1, 1) func(%inputs:Inputs):void {
  $B1: {
    %3:vec3<u32> = access %inputs, 0i
    %4:vec3<u32> = mul %3, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  num_wgs:vec3<u32> @offset(0)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var @binding_point(0, 0)
}

%foo_inner = func(%inputs:Inputs):void {
  $B2: {
    %4:vec3<u32> = access %inputs, 0i
    %5:vec3<u32> = mul %4, %4
    ret
  }
}
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %7:vec3<u32> = load %tint_num_workgroups
    %8:Inputs = construct %7
    %9:void = call %foo_inner, %8
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_ExplicitBinding) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3<u32>());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetParams({num_workgroups});
    ep->SetWorkgroupSize(1, 1, 1);

    b.Append(ep->Block(), [&] {
        b.Multiply(ty.vec3<u32>(), num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func(%num_wgs:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %3:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var @binding_point(1, 23)
}

%foo_inner = func(%num_wgs:vec3<u32>):void {
  $B2: {
    %4:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %6:vec3<u32> = load %tint_num_workgroups
    %7:void = call %foo_inner, %6
    ret
  }
}
)";

    ShaderIOConfig config;
    config.num_workgroups_binding = {1u, 23u};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_AutoBinding) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3<u32>());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    ep->SetParams({num_workgroups});
    ep->SetWorkgroupSize(1, 1, 1);

    b.Append(ep->Block(), [&] {
        b.Multiply(ty.vec3<u32>(), num_workgroups, num_workgroups);
        b.Return(ep);
    });

    b.Append(mod.root_block, [&] {
        for (uint32_t group = 0; group < 10; ++group) {
            auto* v = b.Var<core::AddressSpace::kStorage, i32>();
            v->SetBindingPoint(group, group + 1u);
        }
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<storage, i32, read_write> = var @binding_point(0, 1)
  %2:ptr<storage, i32, read_write> = var @binding_point(1, 2)
  %3:ptr<storage, i32, read_write> = var @binding_point(2, 3)
  %4:ptr<storage, i32, read_write> = var @binding_point(3, 4)
  %5:ptr<storage, i32, read_write> = var @binding_point(4, 5)
  %6:ptr<storage, i32, read_write> = var @binding_point(5, 6)
  %7:ptr<storage, i32, read_write> = var @binding_point(6, 7)
  %8:ptr<storage, i32, read_write> = var @binding_point(7, 8)
  %9:ptr<storage, i32, read_write> = var @binding_point(8, 9)
  %10:ptr<storage, i32, read_write> = var @binding_point(9, 10)
}

%foo = @compute @workgroup_size(1, 1, 1) func(%num_wgs:vec3<u32> [@num_workgroups]):void {
  $B2: {
    %13:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<storage, i32, read_write> = var @binding_point(0, 1)
  %2:ptr<storage, i32, read_write> = var @binding_point(1, 2)
  %3:ptr<storage, i32, read_write> = var @binding_point(2, 3)
  %4:ptr<storage, i32, read_write> = var @binding_point(3, 4)
  %5:ptr<storage, i32, read_write> = var @binding_point(4, 5)
  %6:ptr<storage, i32, read_write> = var @binding_point(5, 6)
  %7:ptr<storage, i32, read_write> = var @binding_point(6, 7)
  %8:ptr<storage, i32, read_write> = var @binding_point(7, 8)
  %9:ptr<storage, i32, read_write> = var @binding_point(8, 9)
  %10:ptr<storage, i32, read_write> = var @binding_point(9, 10)
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var @binding_point(10, 0)
}

%foo_inner = func(%num_wgs:vec3<u32>):void {
  $B2: {
    %14:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %16:vec3<u32> = load %tint_num_workgroups
    %17:void = call %foo_inner, %16
    ret
  }
}
)";

    Run(ShaderIO, ShaderIOConfig{});

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
