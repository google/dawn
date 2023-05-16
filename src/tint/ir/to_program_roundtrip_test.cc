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

#include "src/tint/ir/from_program.h"
#include "src/tint/ir/test_helper.h"
#include "src/tint/ir/to_program.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/utils/string.h"
#include "src/tint/writer/wgsl/generator.h"

#if !TINT_BUILD_WGSL_READER || !TINT_BUILD_WGSL_WRITER
#error "to_program_roundtrip_test.cc requires both the WGSL reader and writer to be enabled"
#endif

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

class IRToProgramRoundtripTest : public TestHelper {
  public:
    void Test(std::string_view input_wgsl, std::string_view expected_wgsl) {
        auto input = utils::TrimSpace(input_wgsl);
        Source::File file("test.wgsl", std::string(input));
        auto input_program = reader::wgsl::Parse(&file);
        ASSERT_TRUE(input_program.IsValid()) << input_program.Diagnostics().str();

        auto ir_module = FromProgram(&input_program);
        ASSERT_TRUE(ir_module);

        auto output_program = ToProgram(ir_module.Get());
        ASSERT_TRUE(output_program.IsValid()) << output_program.Diagnostics().str();

        auto output = writer::wgsl::Generate(&output_program, {});
        ASSERT_TRUE(output.success) << output.error;

        auto expected = expected_wgsl.empty() ? input : utils::TrimSpace(expected_wgsl);
        auto got = utils::TrimSpace(output.wgsl);
        if (expected != got) {
            tint::ir::Disassembler d{ir_module.Get()};
            EXPECT_EQ(expected, got) << "IR:" << std::endl << d.Disassemble();
        }
    }

    void Test(std::string_view wgsl) { Test(wgsl, wgsl); }
};

TEST_F(IRToProgramRoundtripTest, EmptyModule) {
    Test("");
}

TEST_F(IRToProgramRoundtripTest, EmptySingleFunction) {
    Test(R"(
fn f() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, FunctionScopeVar_i32_InitLiteral) {
    Test(R"(
fn f() {
  var i : i32 = 42i;
}
)");
}

}  // namespace
}  // namespace tint::ir
