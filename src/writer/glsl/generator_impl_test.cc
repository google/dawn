// Copyright 2021 The Tint Authors.
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

#include "src/writer/glsl/test_helper.h"

namespace tint {
namespace writer {
namespace glsl {
namespace {

using GlslGeneratorImplTest = TestHelper;

TEST_F(GlslGeneratorImplTest, ErrorIfSanitizerNotRun) {
  auto program = std::make_unique<Program>(std::move(*this));
  GeneratorImpl gen(program.get());
  EXPECT_FALSE(gen.Generate());
  EXPECT_EQ(
      gen.error(),
      "error: GLSL writer requires the transform::Glsl sanitizer to have been "
      "applied to the input program");
}

TEST_F(GlslGeneratorImplTest, Generate) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

void my_func() {
}
)");
}

struct GlslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, GlslBuiltinData data) {
  out << data.builtin;
  return out;
}
using GlslBuiltinConversionTest = TestParamHelper<GlslBuiltinData>;
TEST_P(GlslBuiltinConversionTest, Emit) {
  auto params = GetParam();
  GeneratorImpl& gen = Build();

  EXPECT_EQ(gen.builtin_to_string(params.builtin, ast::PipelineStage::kVertex),
            std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest,
    GlslBuiltinConversionTest,
    testing::Values(
        GlslBuiltinData{ast::Builtin::kPosition, "gl_Position"},
        GlslBuiltinData{ast::Builtin::kVertexIndex, "gl_VertexID"},
        GlslBuiltinData{ast::Builtin::kInstanceIndex, "gl_InstanceID"},
        GlslBuiltinData{ast::Builtin::kFrontFacing, "gl_FrontFacing"},
        GlslBuiltinData{ast::Builtin::kFragDepth, "gl_FragDepth"},
        GlslBuiltinData{ast::Builtin::kLocalInvocationId,
                        "gl_LocalInvocationID"},
        GlslBuiltinData{ast::Builtin::kLocalInvocationIndex,
                        "gl_LocalInvocationIndex"},
        GlslBuiltinData{ast::Builtin::kGlobalInvocationId,
                        "gl_GlobalInvocationID"},
        GlslBuiltinData{ast::Builtin::kWorkgroupId, "gl_WorkGroupID"},
        GlslBuiltinData{ast::Builtin::kSampleIndex, "gl_SampleID"},
        GlslBuiltinData{ast::Builtin::kSampleMask, "gl_SampleMask"}));

}  // namespace
}  // namespace glsl
}  // namespace writer
}  // namespace tint
