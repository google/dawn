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

        tint::ir::Disassembler d{ir_module.Get()};
        auto disassembly = d.Disassemble();

        auto output_program = ToProgram(ir_module.Get());
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

        auto expected = expected_wgsl.empty() ? input : utils::TrimSpace(expected_wgsl);
        auto got = utils::TrimSpace(output.wgsl);
        EXPECT_EQ(expected, got) << "IR:" << std::endl << disassembly;
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

TEST_F(IRToProgramRoundtripTest, SingleFunction_Parameters) {
    Test(R"(
fn f(i : i32, u : u32) -> i32 {
  return i;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Function Call
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, FnCall_NoArgs_NoRet) {
    Test(R"(
fn a() {
}

fn b() {
  a();
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_NoArgs_Ret_i32) {
    Test(R"(
fn a() -> i32 {
  return 1i;
}

fn b() {
  var i : i32 = a();
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_3Args_NoRet) {
    Test(R"(
fn a(x : i32, y : u32, z : f32) {
}

fn b() {
  a(1i, 2u, 3.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_3Args_Ret_f32) {
    Test(R"(
fn a(x : i32, y : u32, z : f32) -> f32 {
  return z;
}

fn b() {
  var v : f32 = a(1i, 2u, 3.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_PtrArgs) {
    Test(R"(
var<private> y : i32 = 2i;

fn a(px : ptr<function, i32>, py : ptr<private, i32>) -> i32 {
  return (*(px) + *(py));
}

fn b() -> i32 {
  var x : i32 = 1i;
  return a(&(x), &(y));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Builtin Call
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, BuiltinCall_Stmt) {
    Test(R"(
fn f() {
  workgroupBarrier();
}
)");
}

TEST_F(IRToProgramRoundtripTest, BuiltinCall_Expr) {
    Test(R"(
fn f(a : i32, b : i32) {
  var i : i32 = max(a, b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BuiltinCall_PtrArg) {
    Test(R"(
var<workgroup> v : bool;

fn foo() -> bool {
  return workgroupUniformLoad(&(v));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Type Construct
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, TypeConstruct_i32) {
    Test(R"(
fn f(i : i32) {
  var v : i32 = i32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_u32) {
    Test(R"(
fn f(i : u32) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_f32) {
    Test(R"(
fn f(i : f32) {
  var v : f32 = f32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_bool) {
    Test(R"(
fn f(i : bool) {
  var v : bool = bool(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_struct) {
    Test(R"(
struct S {
  a : i32,
  b : u32,
  c : f32,
}

fn f(a : i32, b : u32, c : f32) {
  var v : S = S(a, b, c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_array) {
    Test(R"(
fn f(i : i32) {
  var v : array<i32, 3u> = array<i32, 3u>(i, i, i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_vec3i_Splat) {
    Test(R"(
fn f(i : i32) {
  var v : vec3<i32> = vec3<i32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_vec3i_Scalars) {
    Test(R"(
fn f(i : i32) {
  var v : vec3<i32> = vec3<i32>(i, i, i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_mat2x3f_Scalars) {
    Test(R"(
fn f(i : f32) {
  var v : mat2x3<f32> = mat2x3<f32>(i, i, i, i, i, i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_mat2x3f_Columns) {
    Test(R"(
fn f(i : f32) {
  var v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(i, i, i), vec3<f32>(i, i, i));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Type Convert
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, TypeConvert_i32_to_u32) {
    Test(R"(
fn f(i : i32) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_u32_to_f32) {
    Test(R"(
fn f(i : u32) {
  var v : f32 = f32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_f32_to_i32) {
    Test(R"(
fn f(i : f32) {
  var v : i32 = i32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_bool_to_u32) {
    Test(R"(
fn f(i : bool) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_vec3i_to_vec3u) {
    Test(R"(
fn f(i : vec3<i32>) {
  var v : vec3<u32> = vec3<u32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_vec3u_to_vec3f) {
    Test(R"(
fn f(i : vec3<u32>) {
  var v : vec3<f32> = vec3<f32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_mat2x3f_to_mat2x3h) {
    Test(R"(
enable f16;

fn f(i : mat2x3<f32>) {
  var v : mat2x3<f16> = mat2x3<f16>(i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Access
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_1) {
    Test(R"(
fn f(v : vec3<f32>) -> f32 {
  return v[1];
}
)",
         R"(
fn f(v : vec3<f32>) -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_1) {
    Test(R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v[1];
}
)",
         R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_z) {
    Test(R"(
fn f(v : vec3<f32>) -> f32 {
  return v.z;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_z) {
    Test(R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.z;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_g) {
    Test(R"(
fn f(v : vec3<f32>) -> f32 {
  return v.g;
}
)",
         R"(
fn f(v : vec3<f32>) -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_g) {
    Test(R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.g;
}
)",
         R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_i) {
    Test(R"(
fn f(v : vec3<f32>, i : i32) -> f32 {
  return v[i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_i) {
    Test(R"(
var<private> v : vec3<f32>;

fn f(i : i32) -> f32 {
  return v[i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_mat3x2f_1_0) {
    Test(R"(
fn f(m : mat3x2<f32>) -> f32 {
  return m[1][0];
}
)",
         R"(
fn f(m : mat3x2<f32>) -> f32 {
  return m[1i].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_mat3x2f_1_0) {
    Test(R"(
var<private> m : mat3x2<f32>;

fn f() -> f32 {
  return m[1][0];
}
)",
         R"(
var<private> m : mat3x2<f32>;

fn f() -> f32 {
  return m[1i].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_mat3x2f_u_0) {
    Test(R"(
fn f(m : mat3x2<f32>, u : u32) -> f32 {
  return m[u][0];
}
)",
         R"(
fn f(m : mat3x2<f32>, u : u32) -> f32 {
  return m[u].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_mat3x2f_u_0) {
    Test(R"(
var<private> m : mat3x2<f32>;

fn f(u : u32) -> f32 {
  return m[u][0];
}
)",
         R"(
var<private> m : mat3x2<f32>;

fn f(u : u32) -> f32 {
  return m[u].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_mat3x2f_u_i) {
    Test(R"(
fn f(m : mat3x2<f32>, u : u32, i : i32) -> f32 {
  return m[u][i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_mat3x2f_u_i) {
    Test(R"(
var<private> m : mat3x2<f32>;

fn f(u : u32, i : i32) -> f32 {
  return m[u][i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_array_0u) {
    Test(R"(
fn f(a : array<i32, 4u>) -> i32 {
  return a[0u];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_array_0u) {
    Test(R"(
var<private> a : array<i32, 4u>;

fn f() -> i32 {
  return a[0u];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_array_i) {
    Test(R"(
fn f(a : array<i32, 4u>, i : i32) -> i32 {
  return a[i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_array_i) {
    Test(R"(
var<private> a : array<i32, 4u>;

fn f(i : i32) -> i32 {
  return a[i];
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Unary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, UnaryOp_Negate) {
    Test(R"(
fn f(i : i32) -> i32 {
  return -(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, UnaryOp_Complement) {
    Test(R"(
fn f(i : u32) -> u32 {
  return ~(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, UnaryOp_Not) {
    Test(R"(
fn f(b : bool) -> bool {
  return !(b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, BinaryOp_Add) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a + b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Subtract) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a - b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Multiply) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a * b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Divide) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a / b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Modulo) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a % b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_And) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a & b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Or) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a | b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Xor) {
    Test(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a ^ b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Equal) {
    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a == b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_NotEqual) {
    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a != b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_LessThan) {
    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a < b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_GreaterThan) {
    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a > b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_LessThanEqual) {
    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a <= b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_GreaterThanEqual) {
    Test(R"(
fn f(a : i32, b : i32) -> bool {
  return (a >= b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_ShiftLeft) {
    Test(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a << b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_ShiftRight) {
    Test(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a >> b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Short-circuiting binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Param_2) {
    Test(R"(
fn f(a : bool, b : bool) -> bool {
  return (a && b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Param_3_ab_c) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a && b) && c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Param_3_a_bc) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a && b) && c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Let_2) {
    Test(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a && b);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Let_3_ab_c) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a && b) && c);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Let_3_a_bc) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a && (b && c));
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Call_2) {
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

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Call_3_ab_c) {
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

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Call_3_a_bc) {
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

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Param_2) {
    Test(R"(
fn f(a : bool, b : bool) -> bool {
  return (a || b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Param_3_ab_c) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a || b) || c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Param_3_a_bc) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a || (b || c));
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Let_2) {
    Test(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a || b);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Let_3_ab_c) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a || b) || c);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Let_3_a_bc) {
    Test(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a || (b || c));
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Call_2) {
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

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Call_3_ab_c) {
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

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Call_3_a_bc) {
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

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Mixed) {
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
TEST_F(IRToProgramRoundtripTest, CompoundAssign_Increment) {
    Test(R"(
fn f() {
  var v : i32;
  v++;
}
)",
         R"(
fn f() {
  var v : i32;
  v = (v + 1i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Decrement) {
    Test(R"(
fn f() {
  var v : i32;
  v++;
}
)",
         R"(
fn f() {
  var v : i32;
  v = (v + 1i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Add) {
    Test(R"(
fn f() {
  var v : i32;
  v += 8i;
}
)",
         R"(
fn f() {
  var v : i32;
  v = (v + 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Subtract) {
    Test(R"(
fn f() {
  var v : i32;
  v -= 8i;
}
)",
         R"(
fn f() {
  var v : i32;
  v = (v - 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Multiply) {
    Test(R"(
fn f() {
  var v : i32;
  v *= 8i;
}
)",
         R"(
fn f() {
  var v : i32;
  v = (v * 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Divide) {
    Test(R"(
fn f() {
  var v : i32;
  v /= 8i;
}
)",
         R"(
fn f() {
  var v : i32;
  v = (v / 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Xor) {
    Test(R"(
fn f() {
  var v : i32;
  v ^= 8i;
}
)",
         R"(
fn f() {
  var v : i32;
  v = (v ^ 8i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// let
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, LetUsedOnce) {
    Test(R"(
fn f(i : u32) -> u32 {
  let v = ~(i);
  return v;
}
)");
}

TEST_F(IRToProgramRoundtripTest, LetUsedTwice) {
    Test(R"(
fn f(i : i32) -> i32 {
  let v = (i * 2i);
  return (v + v);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Module-scope var
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_i32) {
    Test("var<private> v : i32 = 1i;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_u32) {
    Test("var<private> v : u32 = 1u;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_f32) {
    Test("var<private> v : f32 = 1.0f;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_f16) {
    Test(R"(
enable f16;

var<private> v : f16 = 1.0h;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_bool) {
    Test("var<private> v : bool = true;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_NoArgs) {
    Test("var<private> v : array<i32, 4u> = array<i32, 4u>();");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_Zero) {
    Test("var<private> v : array<i32, 4u> = array<i32, 4u>(0i, 0i, 0i, 0i);",
         "var<private> v : array<i32, 4u> = array<i32, 4u>();");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_SameValue) {
    Test("var<private> v : array<i32, 4u> = array<i32, 4u>(4i, 4i, 4i, 4i);");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_DifferentValues) {
    Test("var<private> v : array<i32, 4u> = array<i32, 4u>(1i, 2i, 3i, 4i);");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_NoArgs) {
    Test(R"(
struct S {
  i : i32,
  u : u32,
  f : f32,
}

var<private> s : S = S();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_Zero) {
    Test(R"(
struct S {
  i : i32,
  u : u32,
  f : f32,
}

var<private> s : S = S(0i, 0u, 0f);
)",
         R"(
struct S {
  i : i32,
  u : u32,
  f : f32,
}

var<private> s : S = S();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_SameValue) {
    Test(R"(
struct S {
  a : i32,
  b : i32,
  c : i32,
}

var<private> s : S = S(4i, 4i, 4i);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_DifferentValues) {
    Test(R"(
struct S {
  a : i32,
  b : i32,
  c : i32,
}

var<private> s : S = S(1i, 2i, 3i);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_NoArgs) {
    Test("var<private> v : vec3<f32> = vec3<f32>();");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_Zero) {
    Test("var<private> v : vec3<f32> = vec3<f32>(0f);",
         "var<private> v : vec3<f32> = vec3<f32>();");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_Splat) {
    Test("var<private> v : vec3<f32> = vec3<f32>(1.0f);");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_Scalars) {
    Test("var<private> v : vec3<f32> = vec3<f32>(1.0f, 2.0f, 3.0f);");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_NoArgs) {
    Test("var<private> v : mat2x3<f32> = mat2x3<f32>();");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Scalars_SameValue) {
    Test("var<private> v : mat2x3<f32> = mat2x3<f32>(4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f);",
         "var<private> v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(4.0f), vec3<f32>(4.0f));");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Scalars) {
    Test("var<private> v : mat2x3<f32> = mat2x3<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);",
         "var<private> v : mat2x3<f32> = "
         "mat2x3<f32>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(4.0f, 5.0f, 6.0f));");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Columns) {
    Test(
        "var<private> v : mat2x3<f32> = "
        "mat2x3<f32>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(4.0f, 5.0f, 6.0f));");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Columns_SameValue) {
    Test(
        "var<private> v : mat2x3<f32> = "
        "mat2x3<f32>(vec3<f32>(4.0f, 4.0f, 4.0f), vec3<f32>(4.0f, 4.0f, 4.0f));",
        "var<private> v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(4.0f), vec3<f32>(4.0f));");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Uniform_vec4i) {
    Test("@group(10) @binding(20) var<uniform> v : vec4<i32>;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_StorageRead_u32) {
    Test("@group(10) @binding(20) var<storage, read> v : u32;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_StorageReadWrite_i32) {
    Test("@group(10) @binding(20) var<storage, read_write> v : i32;");
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
// Function-scope let
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, FunctionScopeLet_i32) {
    Test(R"(
fn f(i : i32) -> i32 {
  let a = (42i + i);
  let b = (24i + i);
  let c = (a + b);
  return c;
}
)");
}

TEST_F(IRToProgramRoundtripTest, FunctionScopeLet_ptr) {
    Test(R"(
fn f() -> i32 {
  var a : array<i32, 3u>;
  let b = &(a[1i]);
  let c = *(b);
  return c;
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

fn f(cond : bool) {
  if (cond) {
    a();
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return) {
    Test(R"(
fn f(cond : bool) {
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

fn f(cond : bool) {
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

TEST_F(IRToProgramRoundtripTest, If_Else_Chain) {
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

////////////////////////////////////////////////////////////////////////////////
// For
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, For_Empty) {
    Test(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_Empty_NoInit) {
    Test(R"(
fn f() {
  var i : i32 = 0i;
  for(; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_Empty_NoCond) {
    Test(R"(
fn f() {
  for(var i : i32 = 0i; ; i = (i + 1i)) {
    break;
  }
}
)",
         R"(
fn f() {
  {
    var i : i32 = 0i;
    loop {
      break;

      continuing {
        i = (i + 1i);
      }
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_Empty_NoCont) {
    Test(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); ) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_ComplexBody) {
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

TEST_F(IRToProgramRoundtripTest, For_ComplexBody_NoInit) {
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

TEST_F(IRToProgramRoundtripTest, For_ComplexBody_NoCond) {
    Test(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  for(var i : i32 = 0i; ; i = (i + 1i)) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
}
)",
         R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  {
    var i : i32 = 0i;
    loop {
      if (a(42i)) {
        return 1i;
      } else {
        return 2i;
      }

      continuing {
        i = (i + 1i);
      }
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_ComplexBody_NoCont) {
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

TEST_F(IRToProgramRoundtripTest, For_CallInInitCondCont) {
    Test(R"(
fn n(v : i32) -> i32 {
  return (v + 1i);
}

fn f() {
  for(var i : i32 = n(0i); (i < n(1i)); i = n(i)) {
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// While
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, While_Empty) {
    Test(R"(
fn f() {
  while(true) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, While_Cond) {
    Test(R"(
fn f(cond : bool) {
  while(cond) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, While_Break) {
    Test(R"(
fn f() {
  while(true) {
    break;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, While_IfBreak) {
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

TEST_F(IRToProgramRoundtripTest, While_IfReturn) {
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
TEST_F(IRToProgramRoundtripTest, Loop_Break) {
    Test(R"(
fn f() {
  loop {
    break;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_IfBreak) {
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

TEST_F(IRToProgramRoundtripTest, Loop_IfReturn) {
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

TEST_F(IRToProgramRoundtripTest, Loop_IfContinuing) {
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

TEST_F(IRToProgramRoundtripTest, Loop_VarsDeclaredOutsideAndInside) {
    Test(R"(
fn f() {
  var b : i32 = 1i;
  loop {
    var a : i32 = 2i;
    if ((a == b)) {
      return;
    }

    continuing {
      b = (a + b);
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
