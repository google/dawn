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

#include <string>

#include "src/tint/ir/disassembler.h"
#include "src/tint/ir/ir_test_helper.h"
#include "src/tint/ir/to_program.h"
#include "src/tint/utils/string.h"
#include "src/tint/writer/wgsl/generator.h"

#if !TINT_BUILD_WGSL_WRITER
#error "to_program_test.cc requires both the WGSL writer to be enabled"
#endif

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;        // NOLINT
using namespace tint::builtin::fluent_types;  // NOLINT

class IRToProgramTest : public IRTestHelper {
  public:
    void Test(std::string_view expected_wgsl) {
        tint::ir::Disassembler d{mod};
        auto disassembly = d.Disassemble();

        auto output_program = ToProgram(mod);
        if (!output_program.IsValid()) {
            FAIL() << output_program.Diagnostics().str() << std::endl  //
                   << "IR:" << std::endl                               //
                   << disassembly << std::endl                         //
                   << "AST:" << std::endl                              //
                   << Program::printer(&output_program) << std::endl;
        }

        ASSERT_TRUE(output_program.IsValid()) << output_program.Diagnostics().str();

        auto output = writer::wgsl::Generate(&output_program, {});
        ASSERT_TRUE(output.success) << output.error;

        auto expected = std::string(utils::TrimSpace(expected_wgsl));
        if (!expected.empty()) {
            expected = "\n" + expected + "\n";
        }
        auto got = std::string(utils::TrimSpace(output.wgsl));
        if (!got.empty()) {
            got = "\n" + got + "\n";
        }
        EXPECT_EQ(expected, got) << "IR:" << std::endl << disassembly;
    }
};

TEST_F(IRToProgramTest, EmptyModule) {
    Test("");
}

TEST_F(IRToProgramTest, SingleFunction_Empty) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    Test(R"(
fn f() {
}
)");
}

TEST_F(IRToProgramTest, SingleFunction_Return) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    fn->Block()->Append(b.Return(fn));

    Test(R"(
fn f() {
}
)");
}

TEST_F(IRToProgramTest, SingleFunction_Return_i32) {
    auto* fn = b.Function("f", ty.i32());
    mod.functions.Push(fn);

    fn->Block()->Append(b.Return(fn, 42_i));

    Test(R"(
fn f() -> i32 {
  return 42i;
}
)");
}

TEST_F(IRToProgramTest, SingleFunction_Parameters) {
    auto* fn = b.Function("f", ty.i32());
    auto* i = b.FunctionParam(ty.i32());
    auto* u = b.FunctionParam(ty.u32());
    mod.SetName(i, "i");
    mod.SetName(u, "u");
    fn->SetParams({i, u});
    mod.functions.Push(fn);

    fn->Block()->Append(b.Return(fn, i));

    Test(R"(
fn f(i : i32, u : u32) -> i32 {
  return i;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Unary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, UnaryOp_Negate) {
    auto* fn = b.Function("f", ty.i32());
    auto* i = b.FunctionParam(ty.i32());
    mod.SetName(i, "i");
    fn->SetParams({i});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Negation(ty.i32(), i));

    Test(R"(
fn f(i : i32) -> i32 {
  return -(i);
}
)");
}

TEST_F(IRToProgramTest, UnaryOp_Complement) {
    auto* fn = b.Function("f", ty.u32());
    auto* i = b.FunctionParam(ty.u32());
    mod.SetName(i, "i");
    fn->SetParams({i});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Complement(ty.u32(), i));

    Test(R"(
fn f(i : u32) -> u32 {
  return ~(i);
}
)");
}

TEST_F(IRToProgramTest, UnaryOp_Not) {
    auto* fn = b.Function("f", ty.bool_());
    auto* i = b.FunctionParam(ty.bool_());
    mod.SetName(i, "b");
    fn->SetParams({i});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Not(ty.bool_(), i));

    Test(R"(
fn f(b : bool) -> bool {
  return !(b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, BinaryOp_Add) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Add(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a + b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Subtract) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Subtract(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a - b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Multiply) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Multiply(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a * b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Divide) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Divide(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a / b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Modulo) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Modulo(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a % b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_And) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.And(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a & b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Or) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Or(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a | b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Xor) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Xor(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a ^ b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Equal) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.Equal(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a == b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_NotEqual) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.NotEqual(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a != b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LessThan) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.LessThan(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a < b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_GreaterThan) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.GreaterThan(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a > b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LessThanEqual) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.LessThanEqual(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a <= b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_GreaterThanEqual) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.i32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.GreaterThanEqual(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a >= b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_ShiftLeft) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.u32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.ShiftLeft(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a << b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_ShiftRight) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam(ty.i32());
    auto* pb = b.FunctionParam(ty.u32());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    f.Return(fn, f.ShiftRight(ty.i32(), pa, pb));

    Test(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a >> b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Short-circuiting binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Param_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* if_ = f.If(pa);
    if_->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if_->True()).ExitIf(if_, pb);
    b.With(if_->False()).ExitIf(if_, false);

    f.Return(fn, if_->Result(0));

    Test(R"(
fn f(a : bool, b : bool) -> bool {
  return (a && b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Param_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pa);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, pb);
    b.With(if1->False()).ExitIf(if1, false);

    auto* if2 = f.If(if1->Result(0));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, pc);
    b.With(if2->False()).ExitIf(if2, false);

    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a && b) && c);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Param_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pb);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, pc);
    b.With(if1->False()).ExitIf(if1, false);

    auto* if2 = f.If(pa);
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, if1->Result(0));
    b.With(if2->False()).ExitIf(if2, false);

    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a && (b && c));
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Let_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* if_ = f.If(pa);
    if_->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if_->True()).ExitIf(if_, pb);
    b.With(if_->False()).ExitIf(if_, false);

    mod.SetName(if_->Result(0), "l");
    f.Return(fn, if_->Result(0));

    Test(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a && b);
  return l;
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Let_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pa);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, pb);
    b.With(if1->False()).ExitIf(if1, false);

    auto* if2 = f.If(if1->Result(0));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, pc);
    b.With(if2->False()).ExitIf(if2, false);

    mod.SetName(if2->Result(0), "l");
    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a && b) && c);
  return l;
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Let_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pb);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, pc);
    b.With(if1->False()).ExitIf(if1, false);

    auto* if2 = f.If(pa);
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, if1->Result(0));
    b.With(if2->False()).ExitIf(if2, false);

    mod.SetName(if2->Result(0), "l");
    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a && (b && c));
  return l;
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Call_2) {
    auto* fa = b.Function("a", ty.bool_());
    mod.functions.Push(fa);
    b.With(fa->Block()).Return(fa, true);

    auto* fb = b.Function("b", ty.bool_());
    mod.functions.Push(fb);
    b.With(fb->Block()).Return(fb, true);

    auto* fn = b.Function("f", ty.bool_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* if_ = f.If(f.Call(ty.bool_(), fa));
    if_->SetResults(b.InstructionResult(ty.bool_()));
    {
        auto true_ = b.With(if_->True());
        true_.ExitIf(if_, true_.Call(ty.bool_(), fb));
    }
    b.With(if_->False()).ExitIf(if_, false);

    f.Return(fn, if_->Result(0));

    Test(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn f() -> bool {
  return (a() && b());
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Call_3_ab_c) {
    auto* fa = b.Function("a", ty.bool_());
    mod.functions.Push(fa);
    b.With(fa->Block()).Return(fa, true);

    auto* fb = b.Function("b", ty.bool_());
    mod.functions.Push(fb);
    b.With(fb->Block()).Return(fb, true);

    auto* fc = b.Function("c", ty.bool_());
    mod.functions.Push(fc);
    b.With(fc->Block()).Return(fc, true);

    auto* fn = b.Function("f", ty.bool_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(f.Call(ty.bool_(), fa));
    if1->SetResults(b.InstructionResult(ty.bool_()));
    {
        auto true_ = b.With(if1->True());
        true_.ExitIf(if1, true_.Call(ty.bool_(), fb));
    }
    b.With(if1->False()).ExitIf(if1, false);

    auto* if2 = f.If(if1->Result(0));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    {
        auto true_ = b.With(if2->True());
        true_.ExitIf(if2, true_.Call(ty.bool_(), fc));
    }
    b.With(if2->False()).ExitIf(if2, false);

    f.Return(fn, if2->Result(0));

    Test(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return ((a() && b()) && c());
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalAnd_Call_3_a_bc) {
    auto* fa = b.Function("a", ty.bool_());
    mod.functions.Push(fa);
    b.With(fa->Block()).Return(fa, true);

    auto* fb = b.Function("b", ty.bool_());
    mod.functions.Push(fb);
    b.With(fb->Block()).Return(fb, true);

    auto* fc = b.Function("c", ty.bool_());
    mod.functions.Push(fc);
    b.With(fc->Block()).Return(fc, true);

    auto* fn = b.Function("f", ty.bool_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(f.Call(ty.bool_(), fb));
    if1->SetResults(b.InstructionResult(ty.bool_()));
    {
        auto true_ = b.With(if1->True());
        true_.ExitIf(if1, true_.Call(ty.bool_(), fc));
    }
    b.With(if1->False()).ExitIf(if1, false);

    auto* if2 = f.If(f.Call(ty.bool_(), fa));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, if1->Result(0));
    b.With(if2->False()).ExitIf(if2, false);

    f.Return(fn, if2->Result(0));

    Test(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return (a() && (b() && c()));
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Param_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* if_ = f.If(pa);
    if_->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if_->True()).ExitIf(if_, true);
    b.With(if_->False()).ExitIf(if_, pb);

    f.Return(fn, if_->Result(0));

    Test(R"(
fn f(a : bool, b : bool) -> bool {
  return (a || b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Param_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pa);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, true);
    b.With(if1->False()).ExitIf(if1, pb);

    auto* if2 = f.If(if1->Result(0));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, true);
    b.With(if2->False()).ExitIf(if2, pc);

    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a || b) || c);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Param_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pb);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, true);
    b.With(if1->False()).ExitIf(if1, pc);

    auto* if2 = f.If(pa);
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, true);
    b.With(if2->False()).ExitIf(if2, if1->Result(0));

    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a || (b || c));
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Let_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    fn->SetParams({pa, pb});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* if_ = f.If(pa);
    if_->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if_->True()).ExitIf(if_, true);
    b.With(if_->False()).ExitIf(if_, pb);

    mod.SetName(if_->Result(0), "l");
    f.Return(fn, if_->Result(0));

    Test(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a || b);
  return l;
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Let_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pa);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, true);
    b.With(if1->False()).ExitIf(if1, pb);

    auto* if2 = f.If(if1->Result(0));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, true);
    b.With(if2->False()).ExitIf(if2, pc);

    mod.SetName(if2->Result(0), "l");
    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a || b) || c);
  return l;
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Let_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pb);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, true);
    b.With(if1->False()).ExitIf(if1, pc);

    auto* if2 = f.If(pa);
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, true);
    b.With(if2->False()).ExitIf(if2, if1->Result(0));

    mod.SetName(if2->Result(0), "l");
    f.Return(fn, if2->Result(0));

    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a || (b || c));
  return l;
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Call_2) {
    auto* fa = b.Function("a", ty.bool_());
    mod.functions.Push(fa);
    b.With(fa->Block()).Return(fa, true);

    auto* fb = b.Function("b", ty.bool_());
    mod.functions.Push(fb);
    b.With(fb->Block()).Return(fb, true);

    auto* fn = b.Function("f", ty.bool_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* if_ = f.If(f.Call(ty.bool_(), fa));
    if_->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if_->True()).ExitIf(if_, true);
    {
        auto false_ = b.With(if_->False());
        false_.ExitIf(if_, false_.Call(ty.bool_(), fb));
    }

    f.Return(fn, if_->Result(0));

    Test(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn f() -> bool {
  return (a() || b());
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Call_3_ab_c) {
    auto* fa = b.Function("a", ty.bool_());
    mod.functions.Push(fa);
    b.With(fa->Block()).Return(fa, true);

    auto* fb = b.Function("b", ty.bool_());
    mod.functions.Push(fb);
    b.With(fb->Block()).Return(fb, true);

    auto* fc = b.Function("c", ty.bool_());
    mod.functions.Push(fc);
    b.With(fc->Block()).Return(fc, true);

    auto* fn = b.Function("f", ty.bool_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(f.Call(ty.bool_(), fa));
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, true);
    {
        auto false_ = b.With(if1->False());
        false_.ExitIf(if1, false_.Call(ty.bool_(), fb));
    }

    auto* if2 = f.If(if1->Result(0));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, true);
    {
        auto false_ = b.With(if2->False());
        false_.ExitIf(if2, false_.Call(ty.bool_(), fc));
    }

    f.Return(fn, if2->Result(0));

    Test(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return ((a() || b()) || c());
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalOr_Call_3_a_bc) {
    auto* fa = b.Function("a", ty.bool_());
    mod.functions.Push(fa);
    b.With(fa->Block()).Return(fa, true);

    auto* fb = b.Function("b", ty.bool_());
    mod.functions.Push(fb);
    b.With(fb->Block()).Return(fb, true);

    auto* fc = b.Function("c", ty.bool_());
    mod.functions.Push(fc);
    b.With(fc->Block()).Return(fc, true);

    auto* fn = b.Function("f", ty.bool_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(f.Call(ty.bool_(), fb));
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, true);
    {
        auto false_ = b.With(if1->False());
        false_.ExitIf(if1, false_.Call(ty.bool_(), fc));
    }

    auto* if2 = f.If(f.Call(ty.bool_(), fa));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if2->True()).ExitIf(if2, true);
    b.With(if2->False()).ExitIf(if2, if1->Result(0));

    f.Return(fn, if2->Result(0));

    Test(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return (a() || (b() || c()));
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LogicalMixed) {
    auto* fb = b.Function("b", ty.bool_());
    mod.functions.Push(fb);
    b.With(fb->Block()).Return(fb, true);

    auto* fd = b.Function("d", ty.bool_());
    mod.functions.Push(fd);
    b.With(fd->Block()).Return(fd, true);

    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pc});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* if1 = f.If(pa);
    if1->SetResults(b.InstructionResult(ty.bool_()));
    b.With(if1->True()).ExitIf(if1, true);
    {
        auto false_ = b.With(if1->False());
        false_.ExitIf(if1, false_.Call(ty.bool_(), fb));
    }

    auto* if2 = f.If(if1->Result(0));
    if2->SetResults(b.InstructionResult(ty.bool_()));
    {
        auto true_ = b.With(if2->True());

        auto* if3 = true_.If(pc);
        if3->SetResults(b.InstructionResult(ty.bool_()));
        b.With(if3->True()).ExitIf(if3, true);
        {
            auto false_ = b.With(if3->False());
            false_.ExitIf(if3, false_.Call(ty.bool_(), fd));
        }

        true_.ExitIf(if2, if3->Result(0));
    }
    b.With(if2->False()).ExitIf(if2, false);

    mod.SetName(if2->Result(0), "l");
    f.Return(fn, if2->Result(0));

    Test(R"(
fn b() -> bool {
  return true;
}

fn d() -> bool {
  return true;
}

fn f(a : bool, c : bool) -> bool {
  let l = ((a || b()) && (c || d()));
  return l;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Compound assignment
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, CompoundAssign_Increment) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    f.Store(v, f.Add(ty.i32(), f.Load(v), 1_i));

    mod.SetName(v, "v");

    Test(R"(
fn f() {
  var v : i32;
  v = (v + 1i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Decrement) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    f.Store(v, f.Subtract(ty.i32(), f.Load(v), 1_i));

    mod.SetName(v, "v");

    Test(R"(
fn f() {
  var v : i32;
  v = (v - 1i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Add) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    f.Store(v, f.Add(ty.i32(), f.Load(v), 8_i));

    mod.SetName(v, "v");

    Test(R"(
fn f() {
  var v : i32;
  v = (v + 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Subtract) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    f.Store(v, f.Subtract(ty.i32(), f.Load(v), 8_i));

    mod.SetName(v, "v");

    Test(R"(
fn f() {
  var v : i32;
  v = (v - 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Multiply) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    f.Store(v, f.Multiply(ty.i32(), f.Load(v), 8_i));

    mod.SetName(v, "v");

    Test(R"(
fn f() {
  var v : i32;
  v = (v * 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Divide) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    f.Store(v, f.Divide(ty.i32(), f.Load(v), 8_i));

    mod.SetName(v, "v");

    Test(R"(
fn f() {
  var v : i32;
  v = (v / 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Xor) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    f.Store(v, f.Xor(ty.i32(), f.Load(v), 8_i));

    mod.SetName(v, "v");

    Test(R"(
fn f() {
  var v : i32;
  v = (v ^ 8i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// let
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, LetUsedOnce) {
    auto* fn = b.Function("f", ty.u32());
    auto* i = b.FunctionParam(ty.u32());
    mod.SetName(i, "i");
    fn->SetParams({i});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Complement(ty.u32(), i);
    f.Return(fn, v);

    mod.SetName(v, "v");

    Test(R"(
fn f(i : u32) -> u32 {
  let v = ~(i);
  return v;
}
)");
}

TEST_F(IRToProgramTest, LetUsedTwice) {
    auto* fn = b.Function("f", ty.i32());
    auto* i = b.FunctionParam(ty.i32());
    mod.SetName(i, "i");
    fn->SetParams({i});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Multiply(ty.i32(), i, 2_i);
    f.Return(fn, f.Add(ty.i32(), v, v));

    mod.SetName(v, "v");

    Test(R"(
fn f(i : i32) -> i32 {
  let v = (i * 2i);
  return (v + v);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Function-scope var
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, FunctionScopeVar_i32) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* i = f.Var(ty.ptr<function, i32>());
    mod.SetName(i, "i");

    Test(R"(
fn f() {
  var i : i32;
}
)");
}

TEST_F(IRToProgramTest, FunctionScopeVar_i32_InitLiteral) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* i = f.Var(ty.ptr<function, i32>());
    i->SetInitializer(b.Constant(42_i));
    mod.SetName(i, "i");

    Test(R"(
fn f() {
  var i : i32 = 42i;
}
)");
}

TEST_F(IRToProgramTest, FunctionScopeVar_Chained) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* va = f.Var(ty.ptr<function, i32>());
    va->SetInitializer(f.Constant(42_i));

    auto* la = f.Load(va)->Result();
    auto* vb = f.Var(ty.ptr<function, i32>());
    vb->SetInitializer(la);

    auto* lb = f.Load(vb)->Result();
    auto* vc = f.Var(ty.ptr<function, i32>());
    vc->SetInitializer(lb);

    mod.SetName(va, "a");
    mod.SetName(vb, "b");
    mod.SetName(vc, "c");

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
TEST_F(IRToProgramTest, If_CallFn) {
    auto* a = b.Function("a", ty.void_());
    mod.functions.Push(a);

    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto if_ = f.If(cond);
    auto true_ = b.With(if_->True());
    true_.Call(ty.void_(), a);
    true_.ExitIf(if_);

    Test(R"(
fn a() {
}

fn f(cond : bool) {
  if (cond) {
    a();
  }
}
)");
}

TEST_F(IRToProgramTest, If_Return) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto if_ = f.If(cond);
    auto true_ = b.With(if_->True());
    true_.Return(fn);

    Test(R"(
fn f(cond : bool) {
  if (cond) {
    return;
  }
}
)");
}

TEST_F(IRToProgramTest, If_Return_i32) {
    auto* fn = b.Function("f", ty.i32());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* cond = f.Var(ty.ptr<function, bool>());
    mod.SetName(cond, "cond");
    cond->SetInitializer(b.Constant(true));
    auto if_ = f.If(f.Load(cond));
    auto true_ = b.With(if_->True());
    true_.Return(fn, 42_i);
    f.Return(fn, 10_i);

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

TEST_F(IRToProgramTest, If_CallFn_Else_CallFn) {
    auto* fa = b.Function("a", ty.void_());
    mod.functions.Push(fa);

    auto* fb = b.Function("b", ty.void_());
    mod.functions.Push(fb);

    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto if_ = f.If(cond);
    {
        auto true_ = b.With(if_->True());
        true_.Call(ty.void_(), fa);
        true_.ExitIf(if_);
    }
    {
        auto false_ = b.With(if_->False());
        false_.Call(ty.void_(), fb);
        false_.ExitIf(if_);
    }

    Test(R"(
fn a() {
}

fn b() {
}

fn f(cond : bool) {
  if (cond) {
    a();
  } else {
    b();
  }
}
)");
}

TEST_F(IRToProgramTest, If_Return_f32_Else_Return_f32) {
    auto* fn = b.Function("f", ty.f32());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* cond = f.Var(ty.ptr<function, bool>());
    mod.SetName(cond, "cond");
    cond->SetInitializer(b.Constant(true));
    auto if_ = f.If(f.Load(cond));
    b.With(if_->True()).Return(fn, 1.0_f);
    b.With(if_->False()).Return(fn, 2.0_f);

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

TEST_F(IRToProgramTest, If_Return_u32_Else_CallFn) {
    auto* fa = b.Function("a", ty.void_());
    mod.functions.Push(fa);

    auto* fb = b.Function("b", ty.void_());
    mod.functions.Push(fb);

    auto* fn = b.Function("f", ty.u32());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* cond = f.Var(ty.ptr<function, bool>());
    mod.SetName(cond, "cond");
    cond->SetInitializer(b.Constant(true));
    auto if_ = f.If(f.Load(cond));
    b.With(if_->True()).Return(fn, 1_u);
    {
        auto false_ = b.With(if_->False());
        false_.Call(ty.void_(), fa);
        false_.ExitIf(if_);
    }
    f.Call(ty.void_(), fb);
    f.Return(fn, 2_u);

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

TEST_F(IRToProgramTest, If_CallFn_ElseIf_CallFn) {
    auto* fa = b.Function("a", ty.void_());
    mod.functions.Push(fa);

    auto* fb = b.Function("b", ty.void_());
    mod.functions.Push(fb);

    auto* fc = b.Function("c", ty.void_());
    mod.functions.Push(fc);

    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* cond = f.Var(ty.ptr<function, bool>());
    mod.SetName(cond, "cond");
    cond->SetInitializer(b.Constant(true));
    auto if1 = f.If(f.Load(cond));
    {
        auto true_ = b.With(if1->True());
        true_.Call(ty.void_(), fa);
        true_.ExitIf(if1);
    }
    {
        auto false_ = b.With(if1->False());
        auto* if2 = false_.If(b.Constant(false));
        {
            auto true_ = b.With(if2->True());
            true_.Call(ty.void_(), fb);
            true_.ExitIf(if2);
        }
        false_.ExitIf(if1);
    }
    f.Call(ty.void_(), fc);

    Test(R"(
fn a() {
}

fn b() {
}

fn c() {
}

fn f() {
  var cond : bool = true;
  if (cond) {
    a();
  } else if (false) {
    b();
  }
  c();
}
)");
}

TEST_F(IRToProgramTest, If_Else_Chain) {
    auto* x = b.Function("x", ty.bool_());
    auto* i = b.FunctionParam(ty.i32());
    mod.SetName(i, "i");
    x->SetParams({i});
    mod.functions.Push(x);
    b.With(x->Block()).Return(x, true);

    auto* fn = b.Function("f", ty.void_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    auto* pd = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    mod.SetName(pd, "d");
    fn->SetParams({pa, pb, pc, pd});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto if1 = f.If(pa);
    {
        auto true1 = b.With(if1->True());
        true1.Call(ty.void_(), x, 0_i);
        true1.ExitIf(if1);
    }
    {
        auto false1 = b.With(if1->False());
        auto* if2 = false1.If(pb);
        {
            auto true2 = b.With(if2->True());
            true2.Call(ty.void_(), x, 1_i);
            true2.ExitIf(if2);
        }
        {
            auto false2 = b.With(if2->False());
            auto* if3 = false2.If(pc);
            {
                auto true3 = b.With(if3->True());
                true3.Call(ty.void_(), x, 2_i);
                true3.ExitIf(if3);
            }
            {
                auto false3 = b.With(if3->False());
                false3.Call(ty.void_(), x, 3_i);
                false3.ExitIf(if3);
            }
        }
    }

    Test(R"(
fn x(i : i32) -> bool {
  return true;
}

fn f(a : bool, b : bool, c : bool, d : bool) {
  if (a) {
    x(0i);
  } else if (b) {
    x(1i);
  } else if (c) {
    x(2i);
  } else {
    x(3i);
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Switch
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, Switch_Default) {
    auto* fa = b.Function("a", ty.void_());
    mod.functions.Push(fa);

    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    mod.SetName(v, "v");
    v->SetInitializer(b.Constant(42_i));

    auto s = f.Switch(f.Load(v));
    {
        auto block = b.With(b.Case(s, {Switch::CaseSelector{}}));
        block.Call(ty.void_(), fa);
        block.ExitSwitch(s);
    }

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

TEST_F(IRToProgramTest, Switch_3_Cases) {
    auto* fa = b.Function("a", ty.void_());
    mod.functions.Push(fa);

    auto* fb = b.Function("b", ty.void_());
    mod.functions.Push(fb);

    auto* fc = b.Function("c", ty.void_());
    mod.functions.Push(fc);

    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    mod.SetName(v, "v");
    v->SetInitializer(b.Constant(42_i));

    auto s = f.Switch(f.Load(v));
    {
        auto block = b.With(b.Case(s, {Switch::CaseSelector{b.Constant(0_i)}}));
        block.Call(ty.void_(), fa);
        block.ExitSwitch(s);
    }
    {
        auto block = b.With(b.Case(s, {
                                          Switch::CaseSelector{b.Constant(1_i)},
                                          Switch::CaseSelector{},
                                      }));
        block.Call(ty.void_(), fb);
        block.ExitSwitch(s);
    }
    {
        auto block = b.With(b.Case(s, {Switch::CaseSelector{b.Constant(2_i)}}));
        block.Call(ty.void_(), fc);
        block.ExitSwitch(s);
    }

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

TEST_F(IRToProgramTest, Switch_3_Cases_AllReturn) {
    auto* fa = b.Function("a", ty.void_());
    mod.functions.Push(fa);

    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());
    auto* v = f.Var(ty.ptr<function, i32>());
    mod.SetName(v, "v");
    v->SetInitializer(b.Constant(42_i));

    auto s = f.Switch(f.Load(v));
    {
        auto block = b.With(b.Case(s, {Switch::CaseSelector{b.Constant(0_i)}}));
        block.Return(fn);
    }
    {
        auto block = b.With(b.Case(s, {
                                          Switch::CaseSelector{b.Constant(1_i)},
                                          Switch::CaseSelector{},
                                      }));
        block.Return(fn);
    }
    {
        auto block = b.With(b.Case(s, {Switch::CaseSelector{b.Constant(2_i)}}));
        block.Return(fn);
    }

    f.Call(ty.void_(), fa);
    f.Return(fn);

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
)");
}

TEST_F(IRToProgramTest, Switch_Nested) {
    auto* fa = b.Function("a", ty.void_());
    mod.functions.Push(fa);

    auto* fb = b.Function("b", ty.void_());
    mod.functions.Push(fb);

    auto* fc = b.Function("c", ty.void_());
    mod.functions.Push(fc);

    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* v1 = f.Var(ty.ptr<function, i32>());
    mod.SetName(v1, "v1");
    v1->SetInitializer(b.Constant(42_i));

    auto* v2 = f.Var(ty.ptr<function, i32>());
    mod.SetName(v2, "v2");
    v2->SetInitializer(b.Constant(24_i));

    auto s1 = f.Switch(f.Load(v1));
    {
        auto block = b.With(b.Case(s1, {Switch::CaseSelector{b.Constant(0_i)}}));
        block.Call(ty.void_(), fa);
        block.ExitSwitch(s1);
    }
    {
        auto block1 = b.With(b.Case(s1, {
                                            Switch::CaseSelector{b.Constant(1_i)},
                                            Switch::CaseSelector{},
                                        }));

        auto s2 = block1.Switch(block1.Load(v2));
        {
            auto block2 = b.With(b.Case(s2, {Switch::CaseSelector{b.Constant(0_i)}}));
            block2.ExitSwitch(s2);
        }
        {
            auto block2 = b.With(b.Case(s2, {
                                                Switch::CaseSelector{b.Constant(1_i)},
                                                Switch::CaseSelector{},
                                            }));
            block2.Return(fn);
        }
    }
    {
        auto block = b.With(b.Case(s1, {Switch::CaseSelector{b.Constant(2_i)}}));
        block.Call(ty.void_(), fc);
        block.ExitSwitch(s1);
    }

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

////////////////////////////////////////////////////////////////////////////////
// For
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, For_Empty) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto init = b.With(loop->Initializer());
        auto* i = init.Var(ty.ptr<function, i32>());
        mod.SetName(i, "i");
        i->SetInitializer(b.Constant(0_i));

        {
            auto body = b.With(loop->Body());
            auto* if_ = body.If(body.LessThan(ty.bool_(), body.Load(i), 5_i));
            b.With(if_->True()).ExitIf(if_);
            b.With(if_->False()).ExitLoop(loop);
        }

        {
            auto cont = b.With(loop->Continuing());
            cont.Store(i, cont.Add(ty.i32(), cont.Load(i), 1_i));
        }
    }

    Test(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramTest, For_Empty_NoInit) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* i = f.Var(ty.ptr<function, i32>());
    mod.SetName(i, "i");
    i->SetInitializer(b.Constant(0_i));

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if_ = body.If(body.LessThan(ty.bool_(), body.Load(i), 5_i));
        b.With(if_->True()).ExitIf(if_);
        b.With(if_->False()).ExitLoop(loop);
    }

    {
        auto cont = b.With(loop->Continuing());
        cont.Store(i, cont.Add(ty.i32(), cont.Load(i), 1_i));
    }

    Test(R"(
fn f() {
  var i : i32 = 0i;
  for(; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramTest, For_Empty_NoCont) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto init = b.With(loop->Initializer());
        auto* i = init.Var(ty.ptr<function, i32>());
        mod.SetName(i, "i");
        i->SetInitializer(b.Constant(0_i));

        {
            auto body = b.With(loop->Body());
            auto* if_ = body.If(body.LessThan(ty.bool_(), body.Load(i), 5_i));
            b.With(if_->True()).ExitIf(if_);
            b.With(if_->False()).ExitLoop(loop);
        }
    }

    Test(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); ) {
  }
}
)");
}

TEST_F(IRToProgramTest, For_ComplexBody) {
    auto* a = b.Function("a", ty.bool_());
    auto* v = b.FunctionParam(ty.i32());
    mod.SetName(v, "v");
    a->SetParams({v});
    {
        auto f = b.With(a->Block());
        f.Return(a, f.Equal(ty.bool_(), v, 1_i));
    }
    mod.functions.Push(a);

    auto* fn = b.Function("f", ty.i32());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto init = b.With(loop->Initializer());
        auto* i = init.Var(ty.ptr<function, i32>());
        mod.SetName(i, "i");
        i->SetInitializer(b.Constant(0_i));

        {
            auto body = b.With(loop->Body());
            auto* if1 = body.If(body.LessThan(ty.bool_(), body.Load(i), 5_i));
            b.With(if1->True()).ExitIf(if1);
            b.With(if1->False()).ExitLoop(loop);

            auto* if2 = body.If(body.Call(ty.bool_(), a, 42_i));
            b.With(if2->True()).Return(fn, 1_i);
            b.With(if2->False()).Return(fn, 2_i);
        }

        {
            auto cont = b.With(loop->Continuing());
            cont.Store(i, cont.Add(ty.i32(), cont.Load(i), 1_i));
        }
    }
    f.Return(fn, 3_i);

    Test(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  for(var i : i32 = 0i; (i < 5i); i = (i + 1i)) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
  return 3i;
}
)");
}

TEST_F(IRToProgramTest, For_ComplexBody_NoInit) {
    auto* a = b.Function("a", ty.bool_());
    auto* v = b.FunctionParam(ty.i32());
    mod.SetName(v, "v");
    a->SetParams({v});
    {
        auto f = b.With(a->Block());
        f.Return(a, f.Equal(ty.bool_(), v, 1_i));
    }
    mod.functions.Push(a);

    auto* fn = b.Function("f", ty.i32());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* i = f.Var(ty.ptr<function, i32>());
    mod.SetName(i, "i");
    i->SetInitializer(b.Constant(0_i));

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if1 = body.If(body.LessThan(ty.bool_(), body.Load(i), 5_i));
        b.With(if1->True()).ExitIf(if1);
        b.With(if1->False()).ExitLoop(loop);

        auto* if2 = body.If(body.Call(ty.bool_(), a, 42_i));
        b.With(if2->True()).Return(fn, 1_i);
        b.With(if2->False()).Return(fn, 2_i);
    }

    {
        auto cont = b.With(loop->Continuing());
        cont.Store(i, cont.Add(ty.i32(), cont.Load(i), 1_i));
    }

    f.Return(fn, 3_i);

    Test(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  var i : i32 = 0i;
  for(; (i < 5i); i = (i + 1i)) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
  return 3i;
}
)");
}

TEST_F(IRToProgramTest, For_ComplexBody_NoCont) {
    auto* a = b.Function("a", ty.bool_());
    auto* v = b.FunctionParam(ty.i32());
    mod.SetName(v, "v");
    a->SetParams({v});
    {
        auto f = b.With(a->Block());
        f.Return(a, f.Equal(ty.bool_(), v, 1_i));
    }
    mod.functions.Push(a);

    auto* fn = b.Function("f", ty.i32());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto init = b.With(loop->Initializer());
        auto* i = init.Var(ty.ptr<function, i32>());
        mod.SetName(i, "i");
        i->SetInitializer(b.Constant(0_i));

        {
            auto body = b.With(loop->Body());
            auto* if1 = body.If(body.LessThan(ty.bool_(), body.Load(i), 5_i));
            b.With(if1->True()).ExitIf(if1);
            b.With(if1->False()).ExitLoop(loop);

            auto* if2 = body.If(body.Call(ty.bool_(), a, 42_i));
            b.With(if2->True()).Return(fn, 1_i);
            b.With(if2->False()).Return(fn, 2_i);
        }
    }
    f.Return(fn, 3_i);

    Test(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  for(var i : i32 = 0i; (i < 5i); ) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
  return 3i;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// While
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, While_Empty) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* cond = body.If(true);
        b.With(cond->True()).ExitIf(cond);
        b.With(cond->False()).ExitLoop(loop);
    }

    Test(R"(
fn f() {
  while(true) {
  }
}
)");
}

TEST_F(IRToProgramTest, While_Cond) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if_ = body.If(cond);
        b.With(if_->True()).ExitIf(if_);
        b.With(if_->False()).ExitLoop(loop);
    }

    Test(R"(
fn f(cond : bool) {
  while(cond) {
  }
}
)");
}

TEST_F(IRToProgramTest, While_Break) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* cond = body.If(true);
        b.With(cond->True()).ExitIf(cond);
        b.With(cond->False()).ExitLoop(loop);
        body.ExitLoop(loop);
    }

    Test(R"(
fn f() {
  while(true) {
    break;
  }
}
)");
}

TEST_F(IRToProgramTest, While_IfBreak) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if1 = body.If(true);
        b.With(if1->True()).ExitIf(if1);
        b.With(if1->False()).ExitLoop(loop);

        auto* if2 = body.If(cond);
        b.With(if2->True()).ExitLoop(loop);
    }

    Test(R"(
fn f(cond : bool) {
  while(true) {
    if (cond) {
      break;
    }
  }
}
)");
}

TEST_F(IRToProgramTest, While_IfReturn) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if1 = body.If(true);
        b.With(if1->True()).ExitIf(if1);
        b.With(if1->False()).ExitLoop(loop);

        auto* if2 = body.If(cond);
        b.With(if2->True()).Return(fn);
    }

    Test(R"(
fn f(cond : bool) {
  while(true) {
    if (cond) {
      return;
    }
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, Loop_Break) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    b.With(loop->Body()).ExitLoop(loop);

    Test(R"(
fn f() {
  loop {
    break;
  }
}
)");
}

TEST_F(IRToProgramTest, Loop_IfBreak) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if_ = body.If(cond);
        b.With(if_->True()).ExitLoop(loop);
    }

    Test(R"(
fn f(cond : bool) {
  loop {
    if (cond) {
      break;
    }
  }
}
)");
}

TEST_F(IRToProgramTest, Loop_IfReturn) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam(ty.bool_());
    mod.SetName(cond, "cond");
    fn->SetParams({cond});
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if_ = body.If(cond);
        b.With(if_->True()).Return(fn);
    }

    Test(R"(
fn f(cond : bool) {
  loop {
    if (cond) {
      return;
    }
  }
}
)");
}

TEST_F(IRToProgramTest, Loop_IfContinuing) {
    auto* fn = b.Function("f", ty.void_());
    mod.functions.Push(fn);

    auto f = b.With(fn->Block());

    auto* cond = f.Var(ty.ptr<function, bool>());
    cond->SetInitializer(b.Constant(false));
    mod.SetName(cond, "cond");

    auto* loop = f.Loop();

    {
        auto body = b.With(loop->Body());
        auto* if_ = body.If(cond);
        b.With(if_->True()).Return(fn);
    }

    {
        auto cont = b.With(loop->Continuing());
        cont.Store(cond, true);
    }

    Test(R"(
fn f() {
  var cond : bool = false;
  loop {
    if (cond) {
      return;
    }

    continuing {
      cond = true;
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
