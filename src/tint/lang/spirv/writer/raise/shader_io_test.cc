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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/spirv/writer/raise/shader_io.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using SpirvWriter_ShaderIOTest = ir::transform::TransformTest;

TEST_F(SpirvWriter_ShaderIOTest, NoInputsOrOutputs) {
    auto* ep = b.Function("foo", ty.void_());
    ep->SetStage(ir::Function::PipelineStage::kCompute);

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

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_NonStruct_Spirv) {
    auto* ep = b.Function("foo", ty.void_());
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(ir::FunctionParam::Builtin::kFrontFacing);
    auto* position = b.FunctionParam("position", ty.vec4<f32>());
    position->SetBuiltin(ir::FunctionParam::Builtin::kPosition);
    position->SetInvariant(true);
    auto* color1 = b.FunctionParam("color1", ty.f32());
    color1->SetLocation(0, {});
    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1, builtin::Interpolation{builtin::InterpolationType::kLinear,
                                                  builtin::InterpolationSampling::kSample});

    ep->SetParams({front_facing, position, color1, color2});
    ep->SetStage(ir::Function::PipelineStage::kFragment);

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
foo_BuiltinInputsStruct = struct @align(16), @block {
  front_facing:bool @offset(0), @builtin(front_facing)
  position:vec4<f32> @offset(16), @invariant, @builtin(position)
}

foo_LocationInputsStruct = struct @align(4), @block {
  color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
}

%b1 = block {  # root
  %foo_BuiltinInputs:ptr<__in, foo_BuiltinInputsStruct, read> = var
  %foo_LocationInputs:ptr<__in, foo_LocationInputsStruct, read> = var
}

%foo_inner = func(%front_facing:bool, %position:vec4<f32>, %color1:f32, %color2:f32):void -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        %8:f32 = add %color1, %color2
        %9:vec4<f32> = mul %position, %8
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void -> %b4 {
  %b4 = block {
    %11:ptr<__in, bool, read> = access %foo_BuiltinInputs, 0u
    %12:bool = load %11
    %13:ptr<__in, vec4<f32>, read> = access %foo_BuiltinInputs, 1u
    %14:vec4<f32> = load %13
    %15:ptr<__in, f32, read> = access %foo_LocationInputs, 0u
    %16:f32 = load %15
    %17:ptr<__in, f32, read> = access %foo_LocationInputs, 1u
    %18:f32 = load %17
    %19:void = call %foo_inner, %12, %14, %16, %18
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_Struct_Spirv) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Inputs"),
                  {
                      {
                          mod.symbols.New("front_facing"),
                          ty.bool_(),
                          {{}, {}, builtin::BuiltinValue::kFrontFacing, {}, false},
                      },
                      {
                          mod.symbols.New("position"),
                          ty.vec4<f32>(),
                          {{}, {}, builtin::BuiltinValue::kPosition, {}, true},
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
                           builtin::Interpolation{builtin::InterpolationType::kLinear,
                                                  builtin::InterpolationSampling::kSample},
                           false},
                      },
                  });

    auto* ep = b.Function("foo", ty.void_());
    auto* str_param = b.FunctionParam("inputs", str_ty);
    ep->SetParams({str_param});
    ep->SetStage(ir::Function::PipelineStage::kFragment);

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

foo_BuiltinInputsStruct = struct @align(16), @block {
  Inputs_front_facing:bool @offset(0), @builtin(front_facing)
  Inputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
}

foo_LocationInputsStruct = struct @align(4), @block {
  Inputs_color1:f32 @offset(0), @location(0)
  Inputs_color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
}

%b1 = block {  # root
  %foo_BuiltinInputs:ptr<__in, foo_BuiltinInputsStruct, read> = var
  %foo_LocationInputs:ptr<__in, foo_LocationInputsStruct, read> = var
}

%foo_inner = func(%inputs:Inputs):void -> %b2 {
  %b2 = block {
    %5:bool = access %inputs, 0i
    if %5 [t: %b3] {  # if_1
      %b3 = block {  # true
        %6:vec4<f32> = access %inputs, 1i
        %7:f32 = access %inputs, 2i
        %8:f32 = access %inputs, 3i
        %9:f32 = add %7, %8
        %10:vec4<f32> = mul %6, %9
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void -> %b4 {
  %b4 = block {
    %12:ptr<__in, bool, read> = access %foo_BuiltinInputs, 0u
    %13:bool = load %12
    %14:ptr<__in, vec4<f32>, read> = access %foo_BuiltinInputs, 1u
    %15:vec4<f32> = load %14
    %16:ptr<__in, f32, read> = access %foo_LocationInputs, 0u
    %17:f32 = load %16
    %18:ptr<__in, f32, read> = access %foo_LocationInputs, 1u
    %19:f32 = load %18
    %20:Inputs = construct %13, %15, %17, %19
    %21:void = call %foo_inner, %20
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_Mixed_Spirv) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     {{}, {}, builtin::BuiltinValue::kPosition, {}, true},
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     {0u, {}, {}, {}, false},
                                 },
                             });

    auto* ep = b.Function("foo", ty.void_());
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(ir::FunctionParam::Builtin::kFrontFacing);
    auto* str_param = b.FunctionParam("inputs", str_ty);
    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1, builtin::Interpolation{builtin::InterpolationType::kLinear,
                                                  builtin::InterpolationSampling::kSample});

    ep->SetParams({front_facing, str_param, color2});
    ep->SetStage(ir::Function::PipelineStage::kFragment);

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

foo_BuiltinInputsStruct = struct @align(16), @block {
  front_facing:bool @offset(0), @builtin(front_facing)
  Inputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
}

foo_LocationInputsStruct = struct @align(4), @block {
  Inputs_color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
}

%b1 = block {  # root
  %foo_BuiltinInputs:ptr<__in, foo_BuiltinInputsStruct, read> = var
  %foo_LocationInputs:ptr<__in, foo_LocationInputsStruct, read> = var
}

%foo_inner = func(%front_facing:bool, %inputs:Inputs, %color2:f32):void -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        %7:vec4<f32> = access %inputs, 0i
        %8:f32 = access %inputs, 1i
        %9:f32 = add %8, %color2
        %10:vec4<f32> = mul %7, %9
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void -> %b4 {
  %b4 = block {
    %12:ptr<__in, bool, read> = access %foo_BuiltinInputs, 0u
    %13:bool = load %12
    %14:ptr<__in, vec4<f32>, read> = access %foo_BuiltinInputs, 1u
    %15:vec4<f32> = load %14
    %16:ptr<__in, f32, read> = access %foo_LocationInputs, 0u
    %17:f32 = load %16
    %18:Inputs = construct %15, %17
    %19:ptr<__in, f32, read> = access %foo_LocationInputs, 1u
    %20:f32 = load %19
    %21:void = call %foo_inner, %13, %18, %20
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_NonStructBuiltin_Spirv) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetReturnBuiltin(ir::Function::ReturnBuiltin::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(ir::Function::PipelineStage::kVertex);

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
foo_BuiltinOutputsStruct = struct @align(16), @block {
  tint_symbol:vec4<f32> @offset(0), @invariant, @builtin(position)
}

%b1 = block {  # root
  %foo_BuiltinOutputs:ptr<__out, foo_BuiltinOutputsStruct, write> = var
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
    %6:ptr<__out, vec4<f32>, write> = access %foo_BuiltinOutputs, 0u
    store %6, %5
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_NonStructLocation_Spirv) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetReturnLocation(1u, {});
    ep->SetStage(ir::Function::PipelineStage::kFragment);

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
foo_LocationOutputsStruct = struct @align(16), @block {
  tint_symbol:vec4<f32> @offset(0), @location(1)
}

%b1 = block {  # root
  %foo_LocationOutputs:ptr<__out, foo_LocationOutputsStruct, write> = var
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
    %6:ptr<__out, vec4<f32>, write> = access %foo_LocationOutputs, 0u
    store %6, %5
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_Struct_Spirv) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {
                          mod.symbols.New("position"),
                          ty.vec4<f32>(),
                          {{}, {}, builtin::BuiltinValue::kPosition, {}, true},
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
                           builtin::Interpolation{builtin::InterpolationType::kLinear,
                                                  builtin::InterpolationSampling::kSample},
                           false},
                      },
                  });

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(ir::Function::PipelineStage::kVertex);

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

foo_BuiltinOutputsStruct = struct @align(16), @block {
  Outputs_position:vec4<f32> @offset(0), @invariant, @builtin(position)
}

foo_LocationOutputsStruct = struct @align(4), @block {
  Outputs_color1:f32 @offset(0), @location(0)
  Outputs_color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
}

%b1 = block {  # root
  %foo_BuiltinOutputs:ptr<__out, foo_BuiltinOutputsStruct, write> = var
  %foo_LocationOutputs:ptr<__out, foo_LocationOutputsStruct, write> = var
}

%foo_inner = func():Outputs -> %b2 {
  %b2 = block {
    %4:vec4<f32> = construct 0.0f
    %5:Outputs = construct %4, 0.25f, 0.75f
    ret %5
  }
}
%foo = @vertex func():void -> %b3 {
  %b3 = block {
    %7:Outputs = call %foo_inner
    %8:vec4<f32> = access %7, 0u
    %9:ptr<__out, vec4<f32>, write> = access %foo_BuiltinOutputs, 0u
    store %9, %8
    %10:f32 = access %7, 1u
    %11:ptr<__out, f32, write> = access %foo_LocationOutputs, 0u
    store %11, %10
    %12:f32 = access %7, 2u
    %13:ptr<__out, f32, write> = access %foo_LocationOutputs, 1u
    store %13, %12
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Struct_SharedByVertexAndFragment_Spirv) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Interface"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     {{}, {}, builtin::BuiltinValue::kPosition, {}, false},
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
        ep->SetStage(ir::Function::PipelineStage::kVertex);

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
        ep->SetStage(ir::Function::PipelineStage::kFragment);
        ep->SetParams({inputs});

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
%frag = @fragment func(%inputs:Interface):vec4<f32> -> %b2 {
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

vert_BuiltinOutputsStruct = struct @align(16), @block {
  Interface_position:vec4<f32> @offset(0), @builtin(position)
}

vert_LocationOutputsStruct = struct @align(16), @block {
  Interface_color:vec4<f32> @offset(0), @location(0)
}

frag_BuiltinInputsStruct = struct @align(16), @block {
  Interface_position:vec4<f32> @offset(0), @builtin(position)
}

frag_LocationInputsStruct = struct @align(16), @block {
  Interface_color:vec4<f32> @offset(0), @location(0)
}

frag_LocationOutputsStruct = struct @align(16), @block {
  tint_symbol:vec4<f32> @offset(0)
}

%b1 = block {  # root
  %vert_BuiltinOutputs:ptr<__out, vert_BuiltinOutputsStruct, write> = var
  %vert_LocationOutputs:ptr<__out, vert_LocationOutputsStruct, write> = var
  %frag_BuiltinInputs:ptr<__in, frag_BuiltinInputsStruct, read> = var
  %frag_LocationInputs:ptr<__in, frag_LocationInputsStruct, read> = var
  %frag_LocationOutputs:ptr<__out, frag_LocationOutputsStruct, write> = var
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
    %18:ptr<__out, vec4<f32>, write> = access %vert_BuiltinOutputs, 0u
    store %18, %17
    %19:vec4<f32> = access %16, 1u
    %20:ptr<__out, vec4<f32>, write> = access %vert_LocationOutputs, 0u
    store %20, %19
    ret
  }
}
%frag = @fragment func():void -> %b5 {
  %b5 = block {
    %22:ptr<__in, vec4<f32>, read> = access %frag_BuiltinInputs, 0u
    %23:vec4<f32> = load %22
    %24:ptr<__in, vec4<f32>, read> = access %frag_LocationInputs, 0u
    %25:vec4<f32> = load %24
    %26:Interface = construct %23, %25
    %27:vec4<f32> = call %frag_inner, %26
    %28:ptr<__out, vec4<f32>, write> = access %frag_LocationOutputs, 0u
    store %28, %27
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Struct_SharedWithBuffer_Spirv) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     {{}, {}, builtin::BuiltinValue::kPosition, {}, false},
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     vec4f,
                                     {0u, {}, {}, {}, false},
                                 },
                             });

    auto* buffer = b.RootBlock()->Append(b.Var(ty.ptr(storage, str_ty, read)));

    auto* ep = b.Function("vert", str_ty);
    ep->SetStage(ir::Function::PipelineStage::kVertex);

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

vert_BuiltinOutputsStruct = struct @align(16), @block {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
}

vert_LocationOutputsStruct = struct @align(16), @block {
  Outputs_color:vec4<f32> @offset(0), @location(0)
}

%b1 = block {  # root
  %1:ptr<storage, Outputs, read> = var
  %vert_BuiltinOutputs:ptr<__out, vert_BuiltinOutputsStruct, write> = var
  %vert_LocationOutputs:ptr<__out, vert_LocationOutputsStruct, write> = var
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
    %9:ptr<__out, vec4<f32>, write> = access %vert_BuiltinOutputs, 0u
    store %9, %8
    %10:vec4<f32> = access %7, 1u
    %11:ptr<__out, vec4<f32>, write> = access %vert_LocationOutputs, 0u
    store %11, %10
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

// Test that we change the type of the sample mask builtin to an array for SPIR-V.
TEST_F(SpirvWriter_ShaderIOTest, SampleMask_Spirv) {
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
                                     {{}, {}, builtin::BuiltinValue::kSampleMask, {}, false},
                                 },
                             });

    auto* mask_in = b.FunctionParam("mask_in", ty.u32());
    mask_in->SetBuiltin(ir::FunctionParam::Builtin::kSampleMask);

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(ir::Function::PipelineStage::kFragment);
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

foo_BuiltinInputsStruct = struct @align(4), @block {
  mask_in:array<u32, 1> @offset(0), @builtin(sample_mask)
}

foo_BuiltinOutputsStruct = struct @align(4), @block {
  Outputs_mask:array<u32, 1> @offset(0), @builtin(sample_mask)
}

foo_LocationOutputsStruct = struct @align(4), @block {
  Outputs_color:f32 @offset(0), @location(0)
}

%b1 = block {  # root
  %foo_BuiltinInputs:ptr<__in, foo_BuiltinInputsStruct, read> = var
  %foo_BuiltinOutputs:ptr<__out, foo_BuiltinOutputsStruct, write> = var
  %foo_LocationOutputs:ptr<__out, foo_LocationOutputsStruct, write> = var
}

%foo_inner = func(%mask_in:u32):Outputs -> %b2 {
  %b2 = block {
    %6:Outputs = construct 0.5f, %mask_in
    ret %6
  }
}
%foo = @fragment func():void -> %b3 {
  %b3 = block {
    %8:ptr<__in, u32, read> = access %foo_BuiltinInputs, 0u, 0u
    %9:u32 = load %8
    %10:Outputs = call %foo_inner, %9
    %11:f32 = access %10, 0u
    %12:ptr<__out, f32, write> = access %foo_LocationOutputs, 0u
    store %12, %11
    %13:u32 = access %10, 1u
    %14:ptr<__out, u32, write> = access %foo_BuiltinOutputs, 0u, 0u
    store %14, %13
    ret
  }
}
)";

    Run(ShaderIO);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
