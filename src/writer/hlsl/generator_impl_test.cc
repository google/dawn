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

#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest = TestHelper;

TEST_F(HlslGeneratorImplTest, Generate) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void my_func() {
}

)");
}

TEST_F(HlslGeneratorImplTest, InputStructName) {
  GeneratorImpl& gen = Build();

  ASSERT_EQ(gen.generate_name("func_main_in"), "func_main_in");
}

TEST_F(HlslGeneratorImplTest, InputStructName_ConflictWithExisting) {
  Symbols().Register("func_main_out_1");
  Symbols().Register("func_main_out_2");

  GeneratorImpl& gen = Build();

  ASSERT_EQ(gen.generate_name("func_main_out"), "func_main_out");
  ASSERT_EQ(gen.generate_name("func_main_out"), "func_main_out_3");
  ASSERT_EQ(gen.generate_name("func_main_out"), "func_main_out_4");
  ASSERT_EQ(gen.generate_name("func_main_out"), "func_main_out_5");
}

struct HlslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, HlslBuiltinData data) {
  out << data.builtin;
  return out;
}
using HlslBuiltinConversionTest = TestParamHelper<HlslBuiltinData>;
TEST_P(HlslBuiltinConversionTest, Emit) {
  auto params = GetParam();
  GeneratorImpl& gen = Build();

  EXPECT_EQ(gen.builtin_to_attribute(params.builtin),
            std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest,
    HlslBuiltinConversionTest,
    testing::Values(
        HlslBuiltinData{ast::Builtin::kPosition, "SV_Position"},
        HlslBuiltinData{ast::Builtin::kVertexIndex, "SV_VertexID"},
        HlslBuiltinData{ast::Builtin::kInstanceIndex, "SV_InstanceID"},
        HlslBuiltinData{ast::Builtin::kFrontFacing, "SV_IsFrontFace"},
        HlslBuiltinData{ast::Builtin::kFragCoord, "SV_Position"},
        HlslBuiltinData{ast::Builtin::kFragDepth, "SV_Depth"},
        HlslBuiltinData{ast::Builtin::kLocalInvocationId, "SV_GroupThreadID"},
        HlslBuiltinData{ast::Builtin::kLocalInvocationIndex, "SV_GroupIndex"},
        HlslBuiltinData{ast::Builtin::kGlobalInvocationId,
                        "SV_DispatchThreadID"},
        HlslBuiltinData{ast::Builtin::kWorkgroupId, "SV_GroupID"},
        HlslBuiltinData{ast::Builtin::kSampleIndex, "SV_SampleIndex"},
        HlslBuiltinData{ast::Builtin::kSampleMask, "SV_Coverage"},
        HlslBuiltinData{ast::Builtin::kSampleMaskIn, "SV_Coverage"},
        HlslBuiltinData{ast::Builtin::kSampleMaskOut, "SV_Coverage"}));

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
