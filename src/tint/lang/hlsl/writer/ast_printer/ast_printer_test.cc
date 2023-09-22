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

#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"

#include "src/tint/lang/hlsl/writer/writer.h"

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest = TestHelper;

TEST_F(HlslASTPrinterTest, InvalidProgram) {
    Diagnostics().add_error(diag::System::Writer, "make the program invalid");
    ASSERT_FALSE(IsValid());
    auto program = resolver::Resolve(*this);
    ASSERT_FALSE(program.IsValid());
    auto result = Generate(program, Options{});
    EXPECT_FALSE(result);
    EXPECT_EQ(result.Failure(), "input program is not valid");
}

TEST_F(HlslASTPrinterTest, UnsupportedExtension) {
    Enable(Source{{12, 34}}, wgsl::Extension::kUndefined);

    ASTPrinter& gen = Build();

    ASSERT_FALSE(gen.Generate());
    EXPECT_EQ(gen.Diagnostics().str(),
              R"(12:34 error: HLSL backend does not support extension 'undefined')");
}

TEST_F(HlslASTPrinterTest, Generate) {
    Func("my_func", {}, ty.void_(), {});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(void my_func() {
}
)");
}

struct HlslBuiltinData {
    core::BuiltinValue builtin;
    const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, HlslBuiltinData data) {
    StringStream str;
    str << data.builtin;
    out << str.str();
    return out;
}
using HlslBuiltinConversionTest = TestParamHelper<HlslBuiltinData>;
TEST_P(HlslBuiltinConversionTest, Emit) {
    auto params = GetParam();
    ASTPrinter& gen = Build();

    EXPECT_EQ(gen.builtin_to_attribute(params.builtin), std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    HlslASTPrinterTest,
    HlslBuiltinConversionTest,
    testing::Values(HlslBuiltinData{core::BuiltinValue::kPosition, "SV_Position"},
                    HlslBuiltinData{core::BuiltinValue::kVertexIndex, "SV_VertexID"},
                    HlslBuiltinData{core::BuiltinValue::kInstanceIndex, "SV_InstanceID"},
                    HlslBuiltinData{core::BuiltinValue::kFrontFacing, "SV_IsFrontFace"},
                    HlslBuiltinData{core::BuiltinValue::kFragDepth, "SV_Depth"},
                    HlslBuiltinData{core::BuiltinValue::kLocalInvocationId, "SV_GroupThreadID"},
                    HlslBuiltinData{core::BuiltinValue::kLocalInvocationIndex, "SV_GroupIndex"},
                    HlslBuiltinData{core::BuiltinValue::kGlobalInvocationId, "SV_DispatchThreadID"},
                    HlslBuiltinData{core::BuiltinValue::kWorkgroupId, "SV_GroupID"},
                    HlslBuiltinData{core::BuiltinValue::kSampleIndex, "SV_SampleIndex"},
                    HlslBuiltinData{core::BuiltinValue::kSampleMask, "SV_Coverage"}));

}  // namespace
}  // namespace tint::hlsl::writer
