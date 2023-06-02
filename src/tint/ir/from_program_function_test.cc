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
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/program_test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_FromProgramFunctionTest = ProgramTestHelper;

TEST_F(IR_FromProgramFunctionTest, EmitFunction_Vertex) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(0_f, 0_f, 0_f, 0_f))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @vertex func():vec4<f32> [@position] -> %b1 {
  %b1 = block {
    ret vec4<f32>(0.0f)
  }
}
)");
}

TEST_F(IR_FromProgramFunctionTest, EmitFunction_Fragment) {
    Func("test", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @fragment func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramFunctionTest, EmitFunction_Compute) {
    Func("test", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(8_i, 4_i, 2_i)});

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

TEST_F(IR_FromProgramFunctionTest, EmitFunction_Return) {
    Func("test", utils::Empty, ty.vec3<f32>(), utils::Vector{Return(vec3<f32>(0_f, 0_f, 0_f))},
         utils::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = func():vec3<f32> -> %b1 {
  %b1 = block {
    ret vec3<f32>(0.0f)
  }
}
)");
}

TEST_F(IR_FromProgramFunctionTest, EmitFunction_ReturnPosition) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(1_f, 2_f, 3_f, 4_f))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @vertex func():vec4<f32> [@position] -> %b1 {
  %b1 = block {
    ret vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
  }
}
)");
}

TEST_F(IR_FromProgramFunctionTest, EmitFunction_ReturnPositionInvariant) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(1_f, 2_f, 3_f, 4_f))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition), Invariant()});

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

TEST_F(IR_FromProgramFunctionTest, EmitFunction_ReturnLocation) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(1_f, 2_f, 3_f, 4_f))},
         utils::Vector{Stage(ast::PipelineStage::kFragment)}, utils::Vector{Location(1_i)});

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

TEST_F(IR_FromProgramFunctionTest, EmitFunction_ReturnLocation_Interpolate) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(1_f, 2_f, 3_f, 4_f))},
         utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Location(1_i), Interpolate(builtin::InterpolationType::kLinear,
                                                  builtin::InterpolationSampling::kCentroid)});

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

TEST_F(IR_FromProgramFunctionTest, EmitFunction_ReturnFragDepth) {
    Func("test", utils::Empty, ty.f32(), utils::Vector{Return(1_f)},
         utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Builtin(builtin::BuiltinValue::kFragDepth)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test = @fragment func():f32 [@frag_depth] -> %b1 {
  %b1 = block {
    ret 1.0f
  }
}
)");
}

TEST_F(IR_FromProgramFunctionTest, EmitFunction_ReturnSampleMask) {
    Func("test", utils::Empty, ty.u32(), utils::Vector{Return(1_u)},
         utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Builtin(builtin::BuiltinValue::kSampleMask)});

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
}  // namespace tint::ir
