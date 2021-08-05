// Copyright 2020 The Tint Authors.
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

#include "src/ast/stage_decoration.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, ErrorIfSanitizerNotRun) {
  auto program = std::make_unique<Program>(std::move(*this));
  GeneratorImpl gen(program.get());
  EXPECT_FALSE(gen.Generate());
  EXPECT_EQ(
      gen.error(),
      "error: MSL writer requires the transform::Msl sanitizer to have been "
      "applied to the input program");
}

TEST_F(MslGeneratorImplTest, Generate) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
kernel void my_func() {
  return;
}

)");
}

struct MslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
  out << data.builtin;
  return out;
}
using MslBuiltinConversionTest = TestParamHelper<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
  auto params = GetParam();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(gen.builtin_to_attribute(params.builtin),
            std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslBuiltinConversionTest,
    testing::Values(MslBuiltinData{ast::Builtin::kPosition, "position"},
                    MslBuiltinData{ast::Builtin::kVertexIndex, "vertex_id"},
                    MslBuiltinData{ast::Builtin::kInstanceIndex, "instance_id"},
                    MslBuiltinData{ast::Builtin::kFrontFacing, "front_facing"},
                    MslBuiltinData{ast::Builtin::kFragDepth, "depth(any)"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationId,
                                   "thread_position_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationIndex,
                                   "thread_index_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kGlobalInvocationId,
                                   "thread_position_in_grid"},
                    MslBuiltinData{ast::Builtin::kWorkgroupId,
                                   "threadgroup_position_in_grid"},
                    MslBuiltinData{ast::Builtin::kSampleIndex, "sample_id"},
                    MslBuiltinData{ast::Builtin::kSampleMask, "sample_mask"},
                    MslBuiltinData{ast::Builtin::kPointSize, "point_size"}));

TEST_F(MslGeneratorImplTest, HasInvariantAttribute_True) {
  auto* out = Structure(
      "Out", {Member("pos", ty.vec4<f32>(),
                     {Builtin(ast::Builtin::kPosition), Invariant()})});
  Func("vert_main", ast::VariableList{}, ty.Of(out),
       {Return(Construct(ty.Of(out)))}, {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_TRUE(gen.HasInvariant());
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Out {
  float4 pos [[position]] [[invariant]];
};

vertex Out vert_main() {
  return {};
}

)");
}

TEST_F(MslGeneratorImplTest, HasInvariantAttribute_False) {
  auto* out = Structure("Out", {Member("pos", ty.vec4<f32>(),
                                       {Builtin(ast::Builtin::kPosition)})});
  Func("vert_main", ast::VariableList{}, ty.Of(out),
       {Return(Construct(ty.Of(out)))}, {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_FALSE(gen.HasInvariant());
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Out {
  float4 pos [[position]];
};

vertex Out vert_main() {
  return {};
}

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
