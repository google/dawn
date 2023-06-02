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
#include "src/tint/ir/program_test_helper.h"
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

class IRToProgramRoundtripTest : public ProgramTestHelper {
  public:
    void Test(std::string_view input_wgsl, std::string_view expected_wgsl) {
        auto input = utils::TrimSpace(input_wgsl);
        Source::File file("test.wgsl", std::string(input));
        auto input_program = reader::wgsl::Parse(&file);
        ASSERT_TRUE(input_program.IsValid()) << input_program.Diagnostics().str();

        auto ir_module = FromProgram(&input_program);
        ASSERT_TRUE(ir_module);

        auto output_program = ToProgram(ir_module.Get());
        if (!output_program.IsValid()) {
            tint::ir::Disassembler d{ir_module.Get()};
            FAIL() << output_program.Diagnostics().str() << std::endl
                   << "IR:" << std::endl
                   << d.Disassemble();
        }

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

TEST_F(IRToProgramRoundtripTest, SingleFunction_Empty) {
    Test(R"(
fn f() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, SingleFunction_Return) {
    Test(R"(
fn f() {
  return;
}
)",
         R"(
fn f() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, SingleFunction_Return_i32) {
    Test(R"(
fn f() -> i32 {
  return 42i;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Function-scope var
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, FunctionScopeVar_i32) {
    Test(R"(
fn f() {
  var i : i32;
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

TEST_F(IRToProgramRoundtripTest, FunctionScopeVar_Chained) {
    Test(R"(
fn f() {
  var a : i32 = 42i;
  var b : i32 = a;
  var c : i32 = b;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// If
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, If_CallFn) {
    Test(R"(
fn a() {
}

fn f() {
  var cond : bool = true;
  if (cond) {
    a();
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return) {
    Test(R"(
fn f() {
  var cond : bool = true;
  if (cond) {
    return;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return_i32) {
    Test(R"(
fn f() -> i32 {
  var cond : bool = true;
  if (cond) {
    return 42i;
  }
  return 10i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_CallFn_Else_CallFn) {
    Test(R"(
fn a() {
}

fn b() {
}

fn f() {
  var cond : bool = true;
  if (cond) {
    a();
  } else {
    b();
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return_f32_Else_Return_f32) {
    Test(R"(
fn f() -> f32 {
  var cond : bool = true;
  if (cond) {
    return 1.0f;
  } else {
    return 2.0f;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return_u32_Else_CallFn) {
    Test(R"(
fn a() {
}

fn b() {
}

fn f() -> u32 {
  var cond : bool = true;
  if (cond) {
    return 1u;
  } else {
    a();
  }
  b();
  return 2u;
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_CallFn_ElseIf_CallFn) {
    Test(R"(
fn a() {
}

fn b() {
}

fn c() {
}

fn f() {
  var cond_a : bool = true;
  if (cond_a) {
    a();
  } else if (false) {
    b();
  }
  c();
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Switch
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Switch_Default) {
    Test(R"(
fn a() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    default: {
      a();
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Switch_3_Cases) {
    Test(R"(
fn a() {
}

fn b() {
}

fn c() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    case 0i: {
      a();
    }
    case 1i, default: {
      b();
    }
    case 2i: {
      c();
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Switch_3_Cases_AllReturn) {
    Test(R"(
fn a() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    case 0i: {
      return;
    }
    case 1i, default: {
      return;
    }
    case 2i: {
      return;
    }
  }
  a();
}
)",
         R"(
fn a() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    case 0i: {
      return;
    }
    case 1i, default: {
      return;
    }
    case 2i: {
      return;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Switch_Nested) {
    Test(R"(
fn a() {
}

fn b() {
}

fn c() {
}

fn f() {
  var v1 : i32 = 42i;
  var v2 : i32 = 24i;
  switch(v1) {
    case 0i: {
      a();
    }
    case 1i, default: {
      switch(v2) {
        case 0i: {
        }
        case 1i, default: {
          return;
        }
      }
    }
    case 2i: {
      c();
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
