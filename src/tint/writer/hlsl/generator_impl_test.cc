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

#include "src/tint/writer/hlsl/test_helper.h"

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest = TestHelper;

TEST_F(HlslGeneratorImplTest, Generate) {
    Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{}, ast::AttributeList{});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(void my_func() {
}
)");
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

    EXPECT_EQ(gen.builtin_to_attribute(params.builtin), std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest,
    HlslBuiltinConversionTest,
    testing::Values(HlslBuiltinData{ast::Builtin::kPosition, "SV_Position"},
                    HlslBuiltinData{ast::Builtin::kVertexIndex, "SV_VertexID"},
                    HlslBuiltinData{ast::Builtin::kInstanceIndex, "SV_InstanceID"},
                    HlslBuiltinData{ast::Builtin::kFrontFacing, "SV_IsFrontFace"},
                    HlslBuiltinData{ast::Builtin::kFragDepth, "SV_Depth"},
                    HlslBuiltinData{ast::Builtin::kLocalInvocationId, "SV_GroupThreadID"},
                    HlslBuiltinData{ast::Builtin::kLocalInvocationIndex, "SV_GroupIndex"},
                    HlslBuiltinData{ast::Builtin::kGlobalInvocationId, "SV_DispatchThreadID"},
                    HlslBuiltinData{ast::Builtin::kWorkgroupId, "SV_GroupID"},
                    HlslBuiltinData{ast::Builtin::kSampleIndex, "SV_SampleIndex"},
                    HlslBuiltinData{ast::Builtin::kSampleMask, "SV_Coverage"}));

}  // namespace
}  // namespace tint::writer::hlsl
