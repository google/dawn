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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/spirv/writer/raise/shader_io.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_ShaderIOTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_ShaderIOTest, NoInputsOrOutputs) {
    auto* ep = b.Function("foo", ty.void_());
    ep->SetStage(core::ir::Function::PipelineStage::kCompute);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_NonStruct) {
    auto* ep = b.Function("foo", ty.void_());
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::ir::FunctionParam::Builtin::kFrontFacing);
    auto* position = b.FunctionParam("position", ty.vec4<f32>());
    position->SetBuiltin(core::ir::FunctionParam::Builtin::kPosition);
    position->SetInvariant(true);
    auto* color1 = b.FunctionParam("color1", ty.f32());
    color1->SetLocation(0, {});
    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1, core::Interpolation{core::InterpolationType::kLinear,
                                               core::InterpolationSampling::kSample});

    ep->SetParams({front_facing, position, color1, color2});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {
            b.Multiply(ty.vec4<f32>(), position, b.Add(ty.f32(), color1, color2));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
%foo = @fragment func(%front_facing:bool [@front_facing], %position:vec4<f32> [@invariant, @position], %color1:f32 [@location(0)], %color2:f32 [@location(1), @interpolate(linear, sample)]):void -> %b1 {
  %b1 = block {
    if %front_facing [t: %b2] {  # if_1
      %b2 = block {  # true
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
%b1 = block {  # root
  %foo_front_facing_Input:ptr<__in, bool, read> = var @builtin(front_facing)
  %foo_position_Input:ptr<__in, vec4<f32>, read> = var @invariant @builtin(position)
  %foo_loc0_Input:ptr<__in, f32, read> = var @location(0)
  %foo_loc1_Input:ptr<__in, f32, read> = var @location(1) @interpolate(linear, sample)
}

%foo_inner = func(%front_facing:bool, %position:vec4<f32>, %color1:f32, %color2:f32):void -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        %10:f32 = add %color1, %color2
        %11:vec4<f32> = mul %position, %10
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void -> %b4 {
  %b4 = block {
    %13:bool = load %foo_front_facing_Input
    %14:vec4<f32> = load %foo_position_Input
    %15:f32 = load %foo_loc0_Input
    %16:f32 = load %foo_loc1_Input
    %17:void = call %foo_inner, %13, %14, %15, %16
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("front_facing"),
                                     ty.bool_(),
                                     {{}, {}, core::BuiltinValue::kFrontFacing, {}, false},
                                 },
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     {{}, {}, core::BuiltinValue::kPosition, {}, true},
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     {0u, {}, {}, {}, false},
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     {1u,
                                      {},
                                      {},
                                      core::Interpolation{core::InterpolationType::kLinear,
                                                          core::InterpolationSampling::kSample},
                                      false},
                                 },
                             });

    auto* ep = b.Function("foo", ty.void_());
    auto* str_param = b.FunctionParam("inputs", str_ty);
    ep->SetParams({str_param});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

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

%foo = @fragment func(%inputs:Inputs):void -> %b1 {
  %b1 = block {
    %3:bool = access %inputs, 0i
    if %3 [t: %b2] {  # if_1
      %b2 = block {  # true
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

%b1 = block {  # root
  %foo_front_facing_Input:ptr<__in, bool, read> = var @builtin(front_facing)
  %foo_position_Input:ptr<__in, vec4<f32>, read> = var @invariant @builtin(position)
  %foo_loc0_Input:ptr<__in, f32, read> = var @location(0)
  %foo_loc1_Input:ptr<__in, f32, read> = var @location(1) @interpolate(linear, sample)
}

%foo_inner = func(%inputs:Inputs):void -> %b2 {
  %b2 = block {
    %7:bool = access %inputs, 0i
    if %7 [t: %b3] {  # if_1
      %b3 = block {  # true
        %8:vec4<f32> = access %inputs, 1i
        %9:f32 = access %inputs, 2i
        %10:f32 = access %inputs, 3i
        %11:f32 = add %9, %10
        %12:vec4<f32> = mul %8, %11
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void -> %b4 {
  %b4 = block {
    %14:bool = load %foo_front_facing_Input
    %15:vec4<f32> = load %foo_position_Input
    %16:f32 = load %foo_loc0_Input
    %17:f32 = load %foo_loc1_Input
    %18:Inputs = construct %14, %15, %16, %17
    %19:void = call %foo_inner, %18
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_Mixed) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     {{}, {}, core::BuiltinValue::kPosition, {}, true},
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     {0u, {}, {}, {}, false},
                                 },
                             });

    auto* ep = b.Function("foo", ty.void_());
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::ir::FunctionParam::Builtin::kFrontFacing);
    auto* str_param = b.FunctionParam("inputs", str_ty);
    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1, core::Interpolation{core::InterpolationType::kLinear,
                                               core::InterpolationSampling::kSample});

    ep->SetParams({front_facing, str_param, color2});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

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

%foo = @fragment func(%front_facing:bool [@front_facing], %inputs:Inputs, %color2:f32 [@location(1), @interpolate(linear, sample)]):void -> %b1 {
  %b1 = block {
    if %front_facing [t: %b2] {  # if_1
      %b2 = block {  # true
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

%b1 = block {  # root
  %foo_front_facing_Input:ptr<__in, bool, read> = var @builtin(front_facing)
  %foo_position_Input:ptr<__in, vec4<f32>, read> = var @invariant @builtin(position)
  %foo_loc0_Input:ptr<__in, f32, read> = var @location(0)
  %foo_loc1_Input:ptr<__in, f32, read> = var @location(1) @interpolate(linear, sample)
}

%foo_inner = func(%front_facing:bool, %inputs:Inputs, %color2:f32):void -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        %9:vec4<f32> = access %inputs, 0i
        %10:f32 = access %inputs, 1i
        %11:f32 = add %10, %color2
        %12:vec4<f32> = mul %9, %11
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void -> %b4 {
  %b4 = block {
    %14:bool = load %foo_front_facing_Input
    %15:vec4<f32> = load %foo_position_Input
    %16:f32 = load %foo_loc0_Input
    %17:Inputs = construct %15, %16
    %18:f32 = load %foo_loc1_Input
    %19:void = call %foo_inner, %14, %17, %18
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_NonStructBuiltin) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetReturnBuiltin(core::ir::Function::ReturnBuiltin::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func():vec4<f32> [@invariant, @position] -> %b1 {
  %b1 = block {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %foo_position_Output:ptr<__out, vec4<f32>, write> = var @invariant @builtin(position)
}

%foo_inner = func():vec4<f32> -> %b2 {
  %b2 = block {
    %3:vec4<f32> = construct 0.5f
    ret %3
  }
}
%foo = @vertex func():void -> %b3 {
  %b3 = block {
    %5:vec4<f32> = call %foo_inner
    store %foo_position_Output, %5
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_NonStructLocation) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetReturnLocation(1u, {});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

    auto* src = R"(
%foo = @fragment func():vec4<f32> [@location(1)] -> %b1 {
  %b1 = block {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %foo_loc1_Output:ptr<__out, vec4<f32>, write> = var @location(1)
}

%foo_inner = func():vec4<f32> -> %b2 {
  %b2 = block {
    %3:vec4<f32> = construct 0.5f
    ret %3
  }
}
%foo = @fragment func():void -> %b3 {
  %b3 = block {
    %5:vec4<f32> = call %foo_inner
    store %foo_loc1_Output, %5
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     {{}, {}, core::BuiltinValue::kPosition, {}, true},
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     {0u, {}, {}, {}, false},
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     {1u,
                                      {},
                                      {},
                                      core::Interpolation{core::InterpolationType::kLinear,
                                                          core::InterpolationSampling::kSample},
                                      false},
                                 },
                             });

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, b.Construct(ty.vec4<f32>(), 0_f), 0.25_f, 0.75_f));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color1:f32 @offset(16), @location(0)
  color2:f32 @offset(20), @location(1), @interpolate(linear, sample)
}

%foo = @vertex func():Outputs -> %b1 {
  %b1 = block {
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

%b1 = block {  # root
  %foo_position_Output:ptr<__out, vec4<f32>, write> = var @invariant @builtin(position)
  %foo_loc0_Output:ptr<__out, f32, write> = var @location(0)
  %foo_loc1_Output:ptr<__out, f32, write> = var @location(1) @interpolate(linear, sample)
}

%foo_inner = func():Outputs -> %b2 {
  %b2 = block {
    %5:vec4<f32> = construct 0.0f
    %6:Outputs = construct %5, 0.25f, 0.75f
    ret %6
  }
}
%foo = @vertex func():void -> %b3 {
  %b3 = block {
    %8:Outputs = call %foo_inner
    %9:vec4<f32> = access %8, 0u
    store %foo_position_Output, %9
    %10:f32 = access %8, 1u
    store %foo_loc0_Output, %10
    %11:f32 = access %8, 2u
    store %foo_loc1_Output, %11
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_DualSourceBlending) {
    auto* str_ty = ty.Struct(mod.symbols.New("Output"), {
                                                            {
                                                                mod.symbols.New("color1"),
                                                                ty.f32(),
                                                                {0u, 0u, {}, {}, false},
                                                            },
                                                            {
                                                                mod.symbols.New("color2"),
                                                                ty.f32(),
                                                                {0u, 1u, {}, {}, false},
                                                            },
                                                        });

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, 0.25_f, 0.75_f));
    });

    auto* src = R"(
Output = struct @align(4) {
  color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(0)
}

%foo = @fragment func():Output -> %b1 {
  %b1 = block {
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

%b1 = block {  # root
  %foo_loc0_idx0_Output:ptr<__out, f32, write> = var @location(0) @index(0)
  %foo_loc0_idx1_Output:ptr<__out, f32, write> = var @location(0) @index(1)
}

%foo_inner = func():Output -> %b2 {
  %b2 = block {
    %4:Output = construct 0.25f, 0.75f
    ret %4
  }
}
%foo = @fragment func():void -> %b3 {
  %b3 = block {
    %6:Output = call %foo_inner
    %7:f32 = access %6, 0u
    store %foo_loc0_idx0_Output, %7
    %8:f32 = access %6, 1u
    store %foo_loc0_idx1_Output, %8
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Struct_SharedByVertexAndFragment) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Interface"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     {{}, {}, core::BuiltinValue::kPosition, {}, false},
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     vec4f,
                                     {0u, {}, {}, {}, false},
                                 },
                             });

    // Vertex shader.
    {
        auto* ep = b.Function("vert", str_ty);
        ep->SetStage(core::ir::Function::PipelineStage::kVertex);

        b.Append(ep->Block(), [&] {  //
            auto* position = b.Construct(vec4f, 0_f);
            auto* color = b.Construct(vec4f, 1_f);
            b.Return(ep, b.Construct(str_ty, position, color));
        });
    }

    // Fragment shader.
    {
        auto* ep = b.Function("frag", vec4f);
        auto* inputs = b.FunctionParam("inputs", str_ty);
        ep->SetStage(core::ir::Function::PipelineStage::kFragment);
        ep->SetParams({inputs});
        ep->SetReturnLocation(0u, {});

        b.Append(ep->Block(), [&] {  //
            auto* position = b.Access(vec4f, inputs, 0_u);
            auto* color = b.Access(vec4f, inputs, 1_u);
            b.Return(ep, b.Add(vec4f, position, color));
        });
    }

    auto* src = R"(
Interface = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec4<f32> @offset(16), @location(0)
}

%vert = @vertex func():Interface -> %b1 {
  %b1 = block {
    %2:vec4<f32> = construct 0.0f
    %3:vec4<f32> = construct 1.0f
    %4:Interface = construct %2, %3
    ret %4
  }
}
%frag = @fragment func(%inputs:Interface):vec4<f32> [@location(0)] -> %b2 {
  %b2 = block {
    %7:vec4<f32> = access %inputs, 0u
    %8:vec4<f32> = access %inputs, 1u
    %9:vec4<f32> = add %7, %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Interface = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec4<f32> @offset(16)
}

%b1 = block {  # root
  %vert_position_Output:ptr<__out, vec4<f32>, write> = var @builtin(position)
  %vert_loc0_Output:ptr<__out, vec4<f32>, write> = var @location(0)
  %frag_position_Input:ptr<__in, vec4<f32>, read> = var @builtin(position)
  %frag_loc0_Input:ptr<__in, vec4<f32>, read> = var @location(0)
  %frag_loc0_Output:ptr<__out, vec4<f32>, write> = var @location(0)
}

%vert_inner = func():Interface -> %b2 {
  %b2 = block {
    %7:vec4<f32> = construct 0.0f
    %8:vec4<f32> = construct 1.0f
    %9:Interface = construct %7, %8
    ret %9
  }
}
%frag_inner = func(%inputs:Interface):vec4<f32> -> %b3 {
  %b3 = block {
    %12:vec4<f32> = access %inputs, 0u
    %13:vec4<f32> = access %inputs, 1u
    %14:vec4<f32> = add %12, %13
    ret %14
  }
}
%vert = @vertex func():void -> %b4 {
  %b4 = block {
    %16:Interface = call %vert_inner
    %17:vec4<f32> = access %16, 0u
    store %vert_position_Output, %17
    %18:vec4<f32> = access %16, 1u
    store %vert_loc0_Output, %18
    ret
  }
}
%frag = @fragment func():void -> %b5 {
  %b5 = block {
    %20:vec4<f32> = load %frag_position_Input
    %21:vec4<f32> = load %frag_loc0_Input
    %22:Interface = construct %20, %21
    %23:vec4<f32> = call %frag_inner, %22
    store %frag_loc0_Output, %23
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Struct_SharedWithBuffer) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     {{}, {}, core::BuiltinValue::kPosition, {}, false},
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     vec4f,
                                     {0u, {}, {}, {}, false},
                                 },
                             });

    auto* buffer = mod.root_block->Append(b.Var(ty.ptr(storage, str_ty, read)));

    auto* ep = b.Function("vert", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Load(buffer));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec4<f32> @offset(16), @location(0)
}

%b1 = block {  # root
  %1:ptr<storage, Outputs, read> = var
}

%vert = @vertex func():Outputs -> %b2 {
  %b2 = block {
    %3:Outputs = load %1
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec4<f32> @offset(16)
}

%b1 = block {  # root
  %1:ptr<storage, Outputs, read> = var
  %vert_position_Output:ptr<__out, vec4<f32>, write> = var @builtin(position)
  %vert_loc0_Output:ptr<__out, vec4<f32>, write> = var @location(0)
}

%vert_inner = func():Outputs -> %b2 {
  %b2 = block {
    %5:Outputs = load %1
    ret %5
  }
}
%vert = @vertex func():void -> %b3 {
  %b3 = block {
    %7:Outputs = call %vert_inner
    %8:vec4<f32> = access %7, 0u
    store %vert_position_Output, %8
    %9:vec4<f32> = access %7, 1u
    store %vert_loc0_Output, %9
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that we change the type of the sample mask builtin to an array for SPIR-V.
TEST_F(SpirvWriter_ShaderIOTest, SampleMask) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("color"),
                                     ty.f32(),
                                     {0u, {}, {}, {}, false},
                                 },
                                 {
                                     mod.symbols.New("mask"),
                                     ty.u32(),
                                     {{}, {}, core::BuiltinValue::kSampleMask, {}, false},
                                 },
                             });

    auto* mask_in = b.FunctionParam("mask_in", ty.u32());
    mask_in->SetBuiltin(core::ir::FunctionParam::Builtin::kSampleMask);

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({mask_in});

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, 0.5_f, mask_in));
    });

    auto* src = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0), @location(0)
  mask:u32 @offset(4), @builtin(sample_mask)
}

%foo = @fragment func(%mask_in:u32 [@sample_mask]):Outputs -> %b1 {
  %b1 = block {
    %3:Outputs = construct 0.5f, %mask_in
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0)
  mask:u32 @offset(4)
}

%b1 = block {  # root
  %foo_sample_mask_Input:ptr<__in, array<u32, 1>, read> = var @builtin(sample_mask)
  %foo_loc0_Output:ptr<__out, f32, write> = var @location(0)
  %foo_sample_mask_Output:ptr<__out, array<u32, 1>, write> = var @builtin(sample_mask)
}

%foo_inner = func(%mask_in:u32):Outputs -> %b2 {
  %b2 = block {
    %6:Outputs = construct 0.5f, %mask_in
    ret %6
  }
}
%foo = @fragment func():void -> %b3 {
  %b3 = block {
    %8:ptr<__in, u32, read> = access %foo_sample_mask_Input, 0u
    %9:u32 = load %8
    %10:Outputs = call %foo_inner, %9
    %11:f32 = access %10, 0u
    store %foo_loc0_Output, %11
    %12:u32 = access %10, 1u
    %13:ptr<__out, u32, write> = access %foo_sample_mask_Output, 0u
    store %13, %12
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that interpolation attributes are stripped from vertex inputs and fragment outputs.
TEST_F(SpirvWriter_ShaderIOTest, InterpolationOnVertexInputOrFragmentOutput) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"),
                             {
                                 {
                                     mod.symbols.New("color"),
                                     ty.f32(),
                                     {1u,
                                      {},
                                      {},
                                      core::Interpolation{core::InterpolationType::kLinear,
                                                          core::InterpolationSampling::kSample},
                                      false},
                                 },
                             });

    // Vertex shader.
    {
        auto* ep = b.Function("vert", ty.vec4<f32>());
        ep->SetReturnBuiltin(core::ir::Function::ReturnBuiltin::kPosition);
        ep->SetReturnInvariant(true);
        ep->SetStage(core::ir::Function::PipelineStage::kVertex);

        auto* str_param = b.FunctionParam("input", str_ty);
        auto* ival = b.FunctionParam("ival", ty.i32());
        ival->SetLocation(1, core::Interpolation{core::InterpolationType::kFlat});
        ep->SetParams({str_param, ival});

        b.Append(ep->Block(), [&] {  //
            b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
        });
    }

    // Fragment shader with struct output.
    {
        auto* ep = b.Function("frag1", str_ty);
        ep->SetStage(core::ir::Function::PipelineStage::kFragment);

        b.Append(ep->Block(), [&] {  //
            b.Return(ep, b.Construct(str_ty, 0.5_f));
        });
    }

    // Fragment shader with non-struct output.
    {
        auto* ep = b.Function("frag2", ty.i32());
        ep->SetStage(core::ir::Function::PipelineStage::kFragment);
        ep->SetReturnLocation(0, core::Interpolation{core::InterpolationType::kFlat});

        b.Append(ep->Block(), [&] {  //
            b.Return(ep, b.Constant(42_i));
        });
    }

    auto* src = R"(
MyStruct = struct @align(4) {
  color:f32 @offset(0), @location(1), @interpolate(linear, sample)
}

%vert = @vertex func(%input:MyStruct, %ival:i32 [@location(1), @interpolate(flat)]):vec4<f32> [@invariant, @position] -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
%frag1 = @fragment func():MyStruct -> %b2 {
  %b2 = block {
    %6:MyStruct = construct 0.5f
    ret %6
  }
}
%frag2 = @fragment func():i32 [@location(0), @interpolate(flat)] -> %b3 {
  %b3 = block {
    ret 42i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  color:f32 @offset(0)
}

%b1 = block {  # root
  %vert_loc1_Input:ptr<__in, f32, read> = var @location(1)
  %vert_loc1_Input_1:ptr<__in, i32, read> = var @location(1)  # %vert_loc1_Input_1: 'vert_loc1_Input'
  %vert_position_Output:ptr<__out, vec4<f32>, write> = var @invariant @builtin(position)
  %frag1_loc1_Output:ptr<__out, f32, write> = var @location(1)
  %frag2_loc0_Output:ptr<__out, i32, write> = var @location(0)
}

%vert_inner = func(%input:MyStruct, %ival:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %9:vec4<f32> = construct 0.5f
    ret %9
  }
}
%frag1_inner = func():MyStruct -> %b3 {
  %b3 = block {
    %11:MyStruct = construct 0.5f
    ret %11
  }
}
%frag2_inner = func():i32 -> %b4 {
  %b4 = block {
    ret 42i
  }
}
%vert = @vertex func():void -> %b5 {
  %b5 = block {
    %14:f32 = load %vert_loc1_Input
    %15:MyStruct = construct %14
    %16:i32 = load %vert_loc1_Input_1
    %17:vec4<f32> = call %vert_inner, %15, %16
    store %vert_position_Output, %17
    ret
  }
}
%frag1 = @fragment func():void -> %b6 {
  %b6 = block {
    %19:MyStruct = call %frag1_inner
    %20:f32 = access %19, 0u
    store %frag1_loc1_Output, %20
    ret
  }
}
%frag2 = @fragment func():void -> %b7 {
  %b7 = block {
    %22:i32 = call %frag2_inner
    store %frag2_loc0_Output, %22
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ClampFragDepth) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("color"),
                                     ty.f32(),
                                     {0u, {}, {}, {}, false},
                                 },
                                 {
                                     mod.symbols.New("depth"),
                                     ty.f32(),
                                     {{}, {}, core::BuiltinValue::kFragDepth, {}, false},
                                 },
                             });

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, 0.5_f, 2_f));
    });

    auto* src = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0), @location(0)
  depth:f32 @offset(4), @builtin(frag_depth)
}

%foo = @fragment func():Outputs -> %b1 {
  %b1 = block {
    %2:Outputs = construct 0.5f, 2.0f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0)
  depth:f32 @offset(4)
}

FragDepthClampArgs = struct @align(4), @block {
  min:f32 @offset(0)
  max:f32 @offset(4)
}

%b1 = block {  # root
  %foo_loc0_Output:ptr<__out, f32, write> = var @location(0)
  %foo_frag_depth_Output:ptr<__out, f32, write> = var @builtin(frag_depth)
  %tint_frag_depth_clamp_args:ptr<push_constant, FragDepthClampArgs, read_write> = var
}

%foo_inner = func():Outputs -> %b2 {
  %b2 = block {
    %5:Outputs = construct 0.5f, 2.0f
    ret %5
  }
}
%foo = @fragment func():void -> %b3 {
  %b3 = block {
    %7:Outputs = call %foo_inner
    %8:f32 = access %7, 0u
    store %foo_loc0_Output, %8
    %9:f32 = access %7, 1u
    %10:FragDepthClampArgs = load %tint_frag_depth_clamp_args
    %11:f32 = access %10, 0u
    %12:f32 = access %10, 1u
    %13:f32 = clamp %9, %11, %12
    store %foo_frag_depth_Output, %13
    ret
  }
}
)";

    ShaderIOConfig config;
    config.clamp_frag_depth = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, EmitVertexPointSize) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnBuiltin(core::ir::Function::ReturnBuiltin::kPosition);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func():vec4<f32> [@position] -> %b1 {
  %b1 = block {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %foo_position_Output:ptr<__out, vec4<f32>, write> = var @builtin(position)
  %foo___point_size_Output:ptr<__out, f32, write> = var @builtin(__point_size)
}

%foo_inner = func():vec4<f32> -> %b2 {
  %b2 = block {
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
%foo = @vertex func():void -> %b3 {
  %b3 = block {
    %6:vec4<f32> = call %foo_inner
    store %foo_position_Output, %6
    store %foo___point_size_Output, 1.0f
    ret
  }
}
)";

    ShaderIOConfig config;
    config.emit_vertex_point_size = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
