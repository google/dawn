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

#include "gmock/gmock.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/helpers/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

using ProgramToIRFunctionTest = helpers::IRProgramTest;

TEST_F(ProgramToIRFunctionTest, EmitFunction_Vertex) {
    Func("test", tint::Empty, ty.vec4<f32>(), Vector{Return(Call<vec4<f32>>(0_f, 0_f, 0_f, 0_f))},
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @vertex func():vec4<f32> [@position] -> %b1 {
  %b1 = block {
    ret vec4<f32>(0.0f)
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_Fragment) {
    Func("test", tint::Empty, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kFragment)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @fragment func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_Compute) {
    Func("test", tint::Empty, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(8_i, 4_i, 2_i)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test = @compute @workgroup_size(8, 4, 2) func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_Return) {
    Func("test", tint::Empty, ty.vec3<f32>(), Vector{Return(Call<vec3<f32>>(0_f, 0_f, 0_f))},
         tint::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = func():vec3<f32> -> %b1 {
  %b1 = block {
    ret vec3<f32>(0.0f)
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_UnreachableEnd_ReturnValue) {
    Func("test", tint::Empty, ty.f32(),
         Vector{If(true, Block(Return(0_f)), Else(Block(Return(1_f))))}, tint::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = func():f32 -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret 0.0f
      }
      %b3 = block {  # false
        ret 1.0f
      }
    }
    unreachable
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_ReturnPosition) {
    Func("test", tint::Empty, ty.vec4<f32>(), Vector{Return(Call<vec4<f32>>(1_f, 2_f, 3_f, 4_f))},
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @vertex func():vec4<f32> [@position] -> %b1 {
  %b1 = block {
    ret vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_ReturnPositionInvariant) {
    Func("test", tint::Empty, ty.vec4<f32>(), Vector{Return(Call<vec4<f32>>(1_f, 2_f, 3_f, 4_f))},
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition), Invariant()});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test = @vertex func():vec4<f32> [@invariant, @position] -> %b1 {
  %b1 = block {
    ret vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_ReturnLocation) {
    Func("test", tint::Empty, ty.vec4<f32>(), Vector{Return(Call<vec4<f32>>(1_f, 2_f, 3_f, 4_f))},
         Vector{Stage(ast::PipelineStage::kFragment)}, Vector{Location(1_i)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test = @fragment func():vec4<f32> [@location(1)] -> %b1 {
  %b1 = block {
    ret vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_ReturnLocation_Interpolate) {
    Func("test", tint::Empty, ty.vec4<f32>(), Vector{Return(Call<vec4<f32>>(1_f, 2_f, 3_f, 4_f))},
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(1_i), Interpolate(core::InterpolationType::kLinear,
                                           core::InterpolationSampling::kCentroid)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(
        Disassemble(m.Get()),
        R"(%test = @fragment func():vec4<f32> [@location(1), @interpolate(linear, centroid)] -> %b1 {
  %b1 = block {
    ret vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_ReturnFragDepth) {
    Func("test", tint::Empty, ty.f32(), Vector{Return(1_f)},
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Builtin(core::BuiltinValue::kFragDepth)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @fragment func():f32 [@frag_depth] -> %b1 {
  %b1 = block {
    ret 1.0f
  }
}
)");
}

TEST_F(ProgramToIRFunctionTest, EmitFunction_ReturnSampleMask) {
    Func("test", tint::Empty, ty.u32(), Vector{Return(1_u)},
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Builtin(core::BuiltinValue::kSampleMask)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @fragment func():u32 [@sample_mask] -> %b1 {
  %b1 = block {
    ret 1u
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
