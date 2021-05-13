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

#include "src/sem/variable.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Generate) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(fn my_func() {
}
)");
}

struct WgslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, WgslBuiltinData data) {
  out << data.builtin;
  return out;
}
using WgslBuiltinConversionTest = TestParamHelper<WgslBuiltinData>;
TEST_P(WgslBuiltinConversionTest, Emit) {
  auto params = GetParam();

  auto* var = Global("a", ty.f32(), ast::StorageClass::kPrivate, nullptr,
                     ast::DecorationList{
                         Builtin(params.builtin),
                     });

  GeneratorImpl& gen = Build();

  gen.EmitDecorations(var->decorations());

  EXPECT_EQ(gen.result(),
            "[[builtin(" + std::string(params.attribute_name) + ")]]");
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslBuiltinConversionTest,
    testing::Values(
        WgslBuiltinData{ast::Builtin::kPosition, "position"},
        WgslBuiltinData{ast::Builtin::kVertexIndex, "vertex_index"},
        WgslBuiltinData{ast::Builtin::kInstanceIndex, "instance_index"},
        WgslBuiltinData{ast::Builtin::kFrontFacing, "front_facing"},
        WgslBuiltinData{ast::Builtin::kFragCoord, "frag_coord"},
        WgslBuiltinData{ast::Builtin::kFragDepth, "frag_depth"},
        WgslBuiltinData{ast::Builtin::kLocalInvocationId,
                        "local_invocation_id"},
        WgslBuiltinData{ast::Builtin::kLocalInvocationIndex,
                        "local_invocation_index"},
        WgslBuiltinData{ast::Builtin::kGlobalInvocationId,
                        "global_invocation_id"},
        WgslBuiltinData{ast::Builtin::kWorkgroupId, "workgroup_id"},
        WgslBuiltinData{ast::Builtin::kSampleIndex, "sample_index"},
        WgslBuiltinData{ast::Builtin::kSampleMaskIn, "sample_mask_in"},
        WgslBuiltinData{ast::Builtin::kSampleMaskOut, "sample_mask_out"}));

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
