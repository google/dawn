// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// GEN_BUILD:CONDITION(tint_build_wgsl_reader && tint_build_wgsl_writer)

#include "src/tint/lang/wgsl/helpers/ir_program_test.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

class IRToProgramRoundtripTest : public helpers::IRProgramTest {
  public:
    void Test(std::string_view input_wgsl, std::string_view expected_wgsl) {
        auto input = tint::TrimSpace(input_wgsl);
        Source::File file("test.wgsl", std::string(input));
        auto ir_module = wgsl::reader::WgslToIR(&file);
        ASSERT_TRUE(ir_module) << ir_module;

        auto disassembly = tint::core::ir::Disassemble(ir_module.Get());

        auto output = wgsl::writer::WgslFromIR(ir_module.Get());
        if (!output) {
            FAIL() << output.Failure() << std::endl  //
                   << "IR:" << std::endl             //
                   << disassembly << std::endl;
        }

        auto expected = expected_wgsl.empty() ? input : tint::TrimSpace(expected_wgsl);
        auto got = tint::TrimSpace(output->wgsl);
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
// Struct declaration
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, StructDecl_Scalars) {
    Test(R"(
struct S {
  a : i32,
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberAlign) {
    Test(R"(
struct S {
  a : i32,
  @align(32u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberSize) {
    Test(R"(
struct S {
  a : i32,
  @size(32u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberLocation) {
    Test(R"(
struct S {
  a : i32,
  @location(1u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberIndex) {
    Test(R"(
enable chromium_internal_dual_source_blending;

struct S {
  a : i32,
  @location(0u) @index(0u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberBuiltin) {
    Test(R"(
struct S {
  a : i32,
  @builtin(position)
  b : vec4<f32>,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberInterpolateType) {
    Test(R"(
struct S {
  a : i32,
  @location(1u) @interpolate(flat)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberInterpolateTypeSampling) {
    Test(R"(
struct S {
  a : i32,
  @location(1u) @interpolate(perspective, centroid)
  b : f32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberInvariant) {
    Test(R"(
struct S {
  a : i32,
  @builtin(position) @invariant
  b : vec4<f32>,
  c : f32,
}

var<private> v : S;
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
// Core Builtin Call
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_Stmt) {
    Test(R"(
fn f() {
  workgroupBarrier();
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_Expr) {
    Test(R"(
fn f(a : i32, b : i32) {
  var i : i32 = max(a, b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_PhonyAssignment) {
    Test(R"(
fn f(a : i32, b : i32) {
  _ = max(a, b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_UnusedLet) {
    Test(R"(
fn f(a : i32, b : i32) {
  let unused = max(a, b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_PtrArg) {
    Test(R"(
@group(0) @binding(0) var<storage, read> v : array<u32>;

fn foo() -> u32 {
  return arrayLength(&(v));
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_DisableDerivativeUniformity) {
    Test(R"(
fn f(in : f32) {
  let x = dpdx(in);
  let y = dpdy(in);
}
)",
         R"(
diagnostic(off, derivative_uniformity);

fn f(in : f32) {
  let x = dpdx(in);
  let y = dpdy(in);
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
// Bitcast
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Bitcast_i32_to_u32) {
    Test(R"(
fn f(i : i32) {
  var v : u32 = bitcast<u32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Bitcast_u32_to_f32) {
    Test(R"(
fn f(i : u32) {
  var v : f32 = bitcast<f32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Bitcast_f32_to_i32) {
    Test(R"(
fn f(i : f32) {
  var v : i32 = bitcast<i32>(i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Discard
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Discard) {
    Test(R"(
fn f() {
  discard;
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

TEST_F(IRToProgramRoundtripTest, Access_ValueStruct) {
    Test(R"(
struct Y {
  a : i32,
  b : i32,
  c : i32,
}

struct X {
  a : i32,
  b : Y,
  c : i32,
}

fn f(x : X) -> i32 {
  return x.b.c;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ReferenceStruct) {
    Test(R"(
struct Y {
  a : i32,
  b : i32,
  c : i32,
}

struct X {
  a : i32,
  b : Y,
  c : i32,
}

fn f() -> i32 {
  var x : X;
  return x.b.c;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_123) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  return v_1[a(1i)][a(2i)][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_213) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  let v_2 = a(2i);
  return v_1[a(1i)][v_2][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_312) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  let v_2 = a(3i);
  return v_1[a(1i)][a(2i)][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_321) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  let v_2 = a(3i);
  let v_3 = a(2i);
  return v_1[a(1i)][v_3][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_123) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  return array<mat3x4<f32>, 5u>()[a(1i)][a(2i)][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_213) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = array<mat3x4<f32>, 5u>();
  let v_2 = a(2i);
  return v_1[a(1i)][v_2][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_312) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = array<mat3x4<f32>, 5u>();
  let v_2 = a(3i);
  return v_1[a(1i)][a(2i)][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_321) {
    Test(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = array<mat3x4<f32>, 5u>();
  let v_2 = a(3i);
  let v_3 = a(2i);
  return v_1[a(1i)][v_3][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_UsePartialChains) {
    Test(R"(
var<private> a : array<array<array<i32, 4u>, 5u>, 6u>;

fn f(i : i32) -> i32 {
  let p1 = &(a[i]);
  let p2 = &((*(p1))[i]);
  let p3 = &((*(p2))[i]);
  let v1 = *(p1);
  let v2 = *(p2);
  let v3 = *(p3);
  return v3;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Swizzle
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_xy) {
    Test(R"(
fn f(v : vec3<f32>) -> vec2<f32> {
  return v.xy;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_yz) {
    Test(R"(
fn f(v : vec3<f32>) -> vec2<f32> {
  return v.yz;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_yzx) {
    Test(R"(
fn f(v : vec3<f32>) -> vec3<f32> {
  return v.yzx;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_yzxy) {
    Test(R"(
fn f(v : vec3<f32>) -> vec4<f32> {
  return v.yzxy;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_xy) {
    Test(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec2<f32> {
  return (*(v)).xy;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_yz) {
    Test(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec2<f32> {
  return (*(v)).yz;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_yzx) {
    Test(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec3<f32> {
  return (*(v)).yzx;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_yzxy) {
    Test(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec4<f32> {
  return (*(v)).yzxy;
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
// Assignment
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfArrayOfArrayAccess_123456) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  v[e(1i)][e(2i)][e(3i)] = v[e(4i)][e(5i)][e(6i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfArrayOfArrayAccess_261345) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] = v[e(4i)][e(5i)][v_3];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfArrayOfArrayAccess_532614) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] = v[e(4i)][v_2][v_5];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfMatrixAccess_123456) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  v[e(1i)][e(2i)][e(3i)] = v[e(4i)][e(5i)][e(6i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfMatrixAccess_261345) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] = v[e(4i)][e(5i)][v_3];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfMatrixAccess_532614) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] = v[e(4i)][v_2][v_5];
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

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfArrayOfArrayAccess_123456) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  v[e(1i)][e(2i)][e(3i)] += v[e(4i)][e(5i)][e(6i)];
}
)",
         R"(fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_1 = &(v[e(1i)][e(2i)][e(3i)]);
  let v_2 = v[e(4i)][e(5i)][e(6i)];
  *(v_1) = (*(v_1) + v_2);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfArrayOfArrayAccess_261345) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] += v[e(4i)][e(5i)][v_3];
}
)",
         R"(fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  let v_1 = &(v[e(1i)][v_2][e(3i)]);
  let v_4 = v[e(4i)][e(5i)][v_3];
  *(v_1) = (*(v_1) + v_4);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfArrayOfArrayAccess_532614) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] += v[e(4i)][v_2][v_5];
}
)",
         R"(fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  let v_1 = &(v[e(1i)][v_4][v_3]);
  let v_6 = v[e(4i)][v_2][v_5];
  *(v_1) = (*(v_1) + v_6);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfMatrixAccess_123456) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  v[e(1i)][e(2i)][e(3i)] += v[e(4i)][e(5i)][e(6i)];
}
)",
         R"(fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_1 = &(v[e(1i)][e(2i)]);
  let v_2 = e(3i);
  let v_3 = v[e(4i)][e(5i)][e(6i)];
  (*(v_1))[v_2] = ((*(v_1))[v_2] + v_3);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfMatrixAccess_261345) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] += v[e(4i)][e(5i)][v_3];
}
)",
         R"(fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  let v_1 = &(v[e(1i)][v_2]);
  let v_4 = e(3i);
  let v_5 = v[e(4i)][e(5i)][v_3];
  (*(v_1))[v_4] = ((*(v_1))[v_4] + v_5);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfMatrixAccess_532614) {
    Test(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] += v[e(4i)][v_2][v_5];
}
)",
         R"(fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  let v_1 = &(v[e(1i)][v_4]);
  let v_6 = v[e(4i)][v_2][v_5];
  (*(v_1))[v_3] = ((*(v_1))[v_3] + v_6);
})");
}

////////////////////////////////////////////////////////////////////////////////
// Phony Assignment
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, PhonyAssign_PrivateVar) {
    Test(R"(
var<private> p : i32;

fn f() {
  _ = p;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_FunctionVar) {
    Test(R"(
fn f() {
  var i : i32;
  _ = i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_FunctionLet) {
    Test(R"(
fn f() {
  let i : i32 = 42i;
  _ = i;
}
)",
         R"(
fn f() {
  let i = 42i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_HandleVar) {
    Test(R"(
@group(0) @binding(0) var t : texture_2d<f32>;

fn f() {
  _ = t;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_Constant) {
    Test(R"(
fn f() {
  _ = 42i;
}
)",
         R"(
fn f() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_Call) {
    Test(R"(
fn v() -> i32 {
  return 42;
}

fn f() {
  _ = v();
}
)",
         R"(
fn v() -> i32 {
  return 42i;
}

fn f() {
  v();
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
TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Handle_Texture2D) {
    Test("@group(0) @binding(0) var t : texture_2d<f32>;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Handle_Sampler) {
    Test("@group(0) @binding(0) var s : sampler;");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Handle_SamplerCmp) {
    Test("@group(0) @binding(0) var s : sampler_comparison;");
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

TEST_F(IRToProgramRoundtripTest, FunctionScopeLet_NoConstEvalError) {
    // Tests that 'a' and 'b' are preserved as a let.
    // If their constant values were inlined, then the initializer for 'c' would be treated as a
    // constant expression instead of the authored runtime expression. Evaluating '1 / 0' as a
    // constant expression is a WGSL validation error.
    Test(R"(
fn f() {
  let a = 1i;
  let b = 0i;
  let c = (a / b);
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

TEST_F(IRToProgramRoundtripTest, For_AssignAsInit) {
    Test(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = 0i; (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_CompoundAssignAsInit) {
    Test(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i += 0i; (i < 10i); i = (i + 1i)) {
  }
}
)",
         R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = (i + 0i); (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_IncrementAsInit) {
    Test(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i++; (i < 10i); i = (i + 1i)) {
  }
}
)",
         R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = (i + 1i); (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_DecrementAsInit) {
    Test(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i--; (i < 10i); i = (i + 1i)) {
  }
}
)",
         R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = (i - 1i); (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_CallAsInit) {
    Test(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(n(); (i < 10i); i = (i + 1i)) {
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

TEST_F(IRToProgramRoundtripTest, Loop_BreakIf_EmptyBody) {
    Test(R"(
fn f() {
  loop {

    continuing {
      break if false;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_BreakIf_NotFalse) {
    Test(R"(
fn f() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
       break if !false;
    }
  }
}
)",
         R"(
fn f() {
  loop {
    if (!(false)) {
      break;
    }

    continuing {
      break if true;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_BreakIf_NotTrue) {
    Test(R"(
fn f() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
       break if !true;
    }
  }
}
)",
         R"(
fn f() {
  loop {
    if (!(false)) {
      break;
    }

    continuing {
      break if false;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_WithReturn) {
    Test(R"(
fn f() {
  loop {
    let i = 42i;
    return;
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Shadowing tests
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_Fn) {
    Test(R"(
fn f32() {
  var v = mat4x4f();
}
)",
         R"(
fn f32_1() {
  var v : mat4x4<f32> = mat4x4<f32>();
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_Struct) {
    Test(R"(
struct f32 {
  v : i32,
}

fn f(s : f32) {
  let f = vec2f(1.0f);
}
)",
         R"(
struct f32_1 {
  v : i32,
}

fn f(s : f32_1) {
  let f_1 = vec2<f32>(1.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_ModVar) {
    Test(R"(
var<private> f32 : vec2f = vec2f(0.0f, 1.0f);
)",
         R"(
var<private> f32_1 : vec2<f32> = vec2<f32>(0.0f, 1.0f);
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_ModVar2) {
    Test(R"(
var<private> f32 : i32 = 1i;

var<private> v = vec2(1.0).x;
)",
         R"(
var<private> f32_1 : i32 = 1i;

var<private> v : f32 = 1.0f;
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_Alias) {
    Test(R"(
alias f32 = i32;

fn f() {
  var v = vec3(1.0f, 2.0f, 3.0f);
}
)",
         R"(
fn f() {
  var v : vec3<f32> = vec3<f32>(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_Struct_With_FnVar) {
    Test(R"(
struct S {
  i : i32,
}

fn f() -> i32 {
  var S : S = S();
  return S.i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_Struct_With_Param) {
    Test(R"(
struct S {
  i : i32,
}

fn f(S : S) -> i32 {
  return S.i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_ModVar_With_FnVar) {
    Test(R"(
var<private> i : i32 = 1i;

fn f() -> i32 {
  i = (i + 1i);
  var i : i32 = (i + 1i);
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_ModVar_With_FnLet) {
    Test(R"(
var<private> i : i32 = 1i;

fn f() -> i32 {
  i = (i + 1i);
  let i = (i + 1i);
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_IfVar) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  if (true) {
    i = (i + 1i);
    var i : i32 = (i + 1i);
    i = (i + 1i);
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_IfLet) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  if (true) {
    i = (i + 1i);
    let i = (i + 1i);
    return i;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_WhileVar) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  while((i < 4i)) {
    var i : i32 = (i + 1i);
    return i;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_WhileLet) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  while((i < 4i)) {
    let i = (i + 1i);
    return i;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForInitVar) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  for(var i : f32 = 0.0f; (i < 4.0f); ) {
    let j = i;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForInitLet) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  for(let i = 0.0f; (i < 4.0f); ) {
    let j = i;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForBodyVar) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  for(var x : i32 = 0i; (i < 4i); ) {
    var i : i32 = (i + 1i);
    return i;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForBodyLet) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  for(var x : i32 = 0i; (i < 4i); ) {
    let i = (i + 1i);
    return i;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopBodyVar) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }
    var i : i32 = (i + 1i);
    if ((i == 3i)) {
      break;
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopBodyLet) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }
    let i = (i + 1i);
    if ((i == 3i)) {
      break;
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopContinuingVar) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }

    continuing {
      var i : i32 = (i + 1i);
      break if (i > 2i);
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopContinuingLet) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }

    continuing {
      let i = (i + 1i);
      break if (i > 2i);
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_SwitchCaseVar) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  switch(i) {
    case 0i: {
      return i;
    }
    case 1i: {
      var i : i32 = (i + 1i);
      return i;
    }
    default: {
      return i;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_SwitchCaseLet) {
    Test(R"(
fn f() -> i32 {
  var i : i32;
  switch(i) {
    case 0i: {
      return i;
    }
    case 1i: {
      let i = (i + 1i);
      return i;
    }
    default: {
      return i;
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl
