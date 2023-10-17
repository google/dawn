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

// GEN_BUILD:CONDITION(tint_build_wgsl_writer)

#include <string>

#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program_test.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::writer {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

using IRToProgramInliningTest = IRToProgramTest;

////////////////////////////////////////////////////////////////////////////////
// Load / Store
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramInliningTest, LoadVar_ThenStoreVar_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        b.Store(var, 2_i);
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  v = 2i;
  return v_1;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Binary op
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramInliningTest, BinaryOpUnsequencedLHSThenUnsequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* rhs = b.Add(ty.i32(), 3_i, 4_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  return ((1i + 2i) + (3i + 4i));
}
)");
}

TEST_F(IRToProgramInliningTest, BinaryOpSequencedLHSThenUnsequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* rhs = b.Add(ty.i32(), 2_i, 3_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  return (a(1i) + (2i + 3i));
}
)");
}

TEST_F(IRToProgramInliningTest, BinaryOpUnsequencedLHSThenSequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* rhs = b.Call(ty.i32(), fn_a, 3_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  return ((1i + 2i) + a(3i));
}
)");
}

TEST_F(IRToProgramInliningTest, BinaryOpSequencedLHSThenSequencedRHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* rhs = b.Call(ty.i32(), fn_a, 2_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  return (a(1i) + a(2i));
}
)");
}

TEST_F(IRToProgramInliningTest, BinaryOpUnsequencedRHSThenUnsequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Add(ty.i32(), 3_i, 4_i);
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  return ((1i + 2i) + (3i + 4i));
}
)");
}

TEST_F(IRToProgramInliningTest, BinaryOpUnsequencedRHSThenSequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Add(ty.i32(), 2_i, 3_i);
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  return (a(1i) + (2i + 3i));
}
)");
}

TEST_F(IRToProgramInliningTest, BinaryOpSequencedRHSThenUnsequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Call(ty.i32(), fn_a, 3_i);
        auto* lhs = b.Add(ty.i32(), 1_i, 2_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  return ((1i + 2i) + a(3i));
}
)");
}

TEST_F(IRToProgramInliningTest, BinaryOpSequencedRHSThenSequencedLHS) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] {
        auto* rhs = b.Call(ty.i32(), fn_a, 2_i);
        auto* lhs = b.Call(ty.i32(), fn_a, 1_i);
        auto* bin = b.Add(ty.i32(), lhs, rhs);
        b.Return(fn_b, bin);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b() -> i32 {
  let v_1 = a(2i);
  return (a(1i) + v_1);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Call
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramInliningTest, CallSequencedXYZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b(v_1 : i32, v_2 : i32, v_3 : i32) -> i32 {
  return 0i;
}

fn c() -> i32 {
  return b(a(1i), a(2i), a(3i));
}
)");
}

TEST_F(IRToProgramInliningTest, CallSequencedYXZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b(v_1 : i32, v_2 : i32, v_3 : i32) -> i32 {
  return 0i;
}

fn c() -> i32 {
  let v_4 = a(2i);
  return b(a(1i), v_4, a(3i));
}
)");
}

TEST_F(IRToProgramInliningTest, CallSequencedXZY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b(v_1 : i32, v_2 : i32, v_3 : i32) -> i32 {
  return 0i;
}

fn c() -> i32 {
  let v_4 = a(1i);
  let v_5 = a(3i);
  return b(v_4, a(2i), v_5);
}
)");
}

TEST_F(IRToProgramInliningTest, CallSequencedZXY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b(v_1 : i32, v_2 : i32, v_3 : i32) -> i32 {
  return 0i;
}

fn c() -> i32 {
  let v_4 = a(3i);
  return b(a(1i), a(2i), v_4);
}
)");
}

TEST_F(IRToProgramInliningTest, CallSequencedYZX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b(v_1 : i32, v_2 : i32, v_3 : i32) -> i32 {
  return 0i;
}

fn c() -> i32 {
  let v_4 = a(2i);
  let v_5 = a(3i);
  return b(a(1i), v_4, v_5);
}
)");
}

TEST_F(IRToProgramInliningTest, CallSequencedZYX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 0_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn_b = b.Function("b", ty.i32());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, 0_i); });
    fn_b->SetParams(
        {b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32()), b.FunctionParam(ty.i32())});

    auto* fn_c = b.Function("c", ty.i32());
    b.Append(fn_c->Block(), [&] {
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* call = b.Call(ty.i32(), fn_b, x, y, z);
        b.Return(fn_c, call);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 0i;
}

fn b(v_1 : i32, v_2 : i32, v_3 : i32) -> i32 {
  return 0i;
}

fn c() -> i32 {
  let v_4 = a(3i);
  let v_5 = a(2i);
  return b(a(1i), v_5, v_4);
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenCallVoidFn_ThenUseLoad) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        b.Call(ty.void_(), fn_a);
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn a() {
}

fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  a();
  return v_1;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenCallUnusedi32Fn_ThenUseLoad) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        b.Call(ty.i32(), fn_a);
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn a() -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  a();
  return v_1;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenCalli32Fn_ThenUseLoadBeforeCall) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* call = b.Call(ty.i32(), fn_a);
        b.Return(fn, b.Add(ty.i32(), load, call));
    });

    EXPECT_WGSL(R"(
fn a() -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v : i32;
  v = 1i;
  return (v + a());
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenCalli32Fn_ThenUseCallBeforeLoad) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* call = b.Call(ty.i32(), fn_a);
        b.Return(fn, b.Add(ty.i32(), call, load));
    });

    EXPECT_WGSL(R"(
fn a() -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  return (a() + v_1);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Access
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramInliningTest, Access_ArrayOfArrayOfArray_XYZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var(ty.ptr<function, array<array<array<i32, 3>, 4>, 5>>());
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  return v_1[a(1i)][a(2i)][a(3i)];
}
)");
}

TEST_F(IRToProgramInliningTest, Access_ArrayOfArrayOfArray_YXZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var(ty.ptr<function, array<array<array<i32, 3>, 4>, 5>>());
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramInliningTest, Access_ArrayOfArrayOfArray_ZXY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var(ty.ptr<function, array<array<array<i32, 3>, 4>, 5>>());
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramInliningTest, Access_ArrayOfArrayOfArray_ZYX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Var(ty.ptr<function, array<array<array<i32, 3>, 4>, 5>>());
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* access = b.Access(ty.ptr<function, i32>(), arr, x, y, z);
        b.Return(fn, b.Load(access));
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramInliningTest, Access_ArrayOfMat3x4f_XYZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  return array<mat3x4<f32>, 5u>()[a(1i)][a(2i)][a(3i)];
}
)");
}

TEST_F(IRToProgramInliningTest, Access_ArrayOfMat3x4f_YXZ) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = a(2i);
  return array<mat3x4<f32>, 5u>()[a(1i)][v_1][a(3i)];
}
)");
}

TEST_F(IRToProgramInliningTest, Access_ArrayOfMat3x4f_ZXY) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = a(3i);
  return array<mat3x4<f32>, 5u>()[a(1i)][a(2i)][v_1];
}
)");
}

TEST_F(IRToProgramInliningTest, Access_ArrayOfMat3x4f_ZYX) {
    auto* fn_a = b.Function("a", ty.i32());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, 1_i); });
    fn_a->SetParams({b.FunctionParam(ty.i32())});

    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* arr = b.Construct(ty.array<mat3x4<f32>, 5>());
        auto* z = b.Call(ty.i32(), fn_a, 3_i);
        auto* y = b.Call(ty.i32(), fn_a, 2_i);
        auto* x = b.Call(ty.i32(), fn_a, 1_i);
        auto* access = b.Access(ty.f32(), arr, x, y, z);
        b.Return(fn, access);
    });

    EXPECT_WGSL(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = a(3i);
  let v_2 = a(2i);
  return array<mat3x4<f32>, 5u>()[a(1i)][v_2][v_1];
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// If
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramInliningTest, UnsequencedOutsideIf) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(fn, v); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  if (true) {
    return (1i + 2i);
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, SequencedOutsideIf) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(fn, v_2); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32 = 1i;
  let v_1 = (v + 2i);
  if (true) {
    return v_1;
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, UnsequencedUsedByIfCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Equal(ty.bool_(), 1_i, 2_i);
        auto* if_ = b.If(v);
        b.Append(if_->True(), [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  if ((1i == 2i)) {
    return 3i;
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, SequencedUsedByIfCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Equal(ty.bool_(), v_1, 2_i);
        auto* if_ = b.If(v_2);
        b.Append(if_->True(), [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32 = 1i;
  if ((v == 2i)) {
    return 3i;
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenWriteToVarInIf_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Store(var, 2_i);
            b.ExitIf(if_);
        });
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  if (true) {
    v = 2i;
  }
  return v_1;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Switch
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramInliningTest, UnsequencedOutsideSwitch) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* switch_ = b.Switch(3_i);
        auto* case_ = b.Case(switch_, {core::ir::Switch::CaseSelector{}});
        b.Append(case_, [&] { b.Return(fn, v); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  switch(3i) {
    default: {
      return (1i + 2i);
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, SequencedOutsideSwitch) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* switch_ = b.Switch(3_i);
        auto* case_ = b.Case(switch_, {core::ir::Switch::CaseSelector{}});
        b.Append(case_, [&] { b.Return(fn, v_2); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32 = 1i;
  let v_1 = (v + 2i);
  switch(3i) {
    default: {
      return v_1;
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, UnsequencedUsedBySwitchCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* switch_ = b.Switch(v);
        auto* case_ = b.Case(switch_, {core::ir::Switch::CaseSelector{}});
        b.Append(case_, [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  switch((1i + 2i)) {
    default: {
      return 3i;
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, SequencedUsedBySwitchCondition) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        var->SetInitializer(b.Constant(1_i));
        auto* v_1 = b.Load(var);
        auto* switch_ = b.Switch(v_1);
        auto* case_ = b.Case(switch_, {core::ir::Switch::CaseSelector{}});
        b.Append(case_, [&] { b.Return(fn, 3_i); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32 = 1i;
  switch(v) {
    default: {
      return 3i;
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenWriteToVarInSwitch_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* switch_ = b.Switch(1_i);
        auto* case_ = b.Case(switch_, {core::ir::Switch::CaseSelector{}});
        b.Append(case_, [&] {
            b.Store(var, 2_i);
            b.ExitSwitch(switch_);
        });
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  switch(1i) {
    default: {
      v = 2i;
    }
  }
  return v_1;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramInliningTest, UnsequencedOutsideLoopInitializer) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Store(var, v);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  {
    v = (1i + 2i);
    loop {
      break;
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, SequencedOutsideLoopInitializer) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Store(var, v_2);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  let v_1 = (v + 2i);
  {
    v = v_1;
    loop {
      break;
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenWriteToVarInLoopInitializer_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            b.Store(var, 2_i);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  {
    v = 2i;
    loop {
      break;
    }
  }
  return v_1;
}
)");
}

TEST_F(IRToProgramInliningTest, UnsequencedOutsideLoopBody) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Return(fn, v); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  loop {
    return (1i + 2i);
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, SequencedOutsideLoopBody) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Return(fn, v_2); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  let v_1 = (v + 2i);
  loop {
    return v_1;
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenWriteToVarInLoopBody_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Store(var, 2_i);
            b.ExitLoop(loop);
        });
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  loop {
    v = 2i;
    break;
  }
  return v_1;
}
)");
}

TEST_F(IRToProgramInliningTest, UnsequencedOutsideLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Add(ty.i32(), 1_i, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, b.Equal(ty.bool_(), v, 3_i)); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  loop {

    continuing {
      break if ((1i + 2i) == 3i);
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, SequencedOutsideLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* v_1 = b.Load(var);
        auto* v_2 = b.Add(ty.i32(), v_1, 2_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, b.Equal(ty.bool_(), v_2, 3_i)); });
        b.Return(fn, 0_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  let v_1 = (v + 2i);
  loop {

    continuing {
      break if (v_1 == 3i);
    }
  }
  return 0i;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVar_ThenWriteToVarInLoopContinuing_ThenUseLoad) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* load = b.Load(var);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] {
            b.Store(var, 2_i);
            b.BreakIf(loop, true);
        });
        b.Return(fn, load);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  let v_1 = v;
  loop {

    continuing {
      v = 2i;
      break if true;
    }
  }
  return v_1;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVarInLoopInitializer_ThenReadAndWriteToVarInLoopBody) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* load = b.Load(var);
            b.NextIteration(loop);
            b.Append(loop->Body(), [&] {
                b.Store(var, b.Add(ty.i32(), load, 1_i));
                b.ExitLoop(loop);
            });
        });
        b.Return(fn, 3_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  {
    let v_1 = v;
    loop {
      v = (v_1 + 1i);
      break;
    }
  }
  return 3i;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVarInLoopInitializer_ThenReadAndWriteToVarInLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* load = b.Load(var);
            b.NextIteration(loop);
            b.Append(loop->Body(), [&] { b.Continue(loop); });
            b.Append(loop->Continuing(), [&] {
                b.Store(var, b.Add(ty.i32(), load, 1_i));
                b.BreakIf(loop, true);
            });
        });
        b.Return(fn, 3_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  {
    let v_1 = v;
    loop {

      continuing {
        v = (v_1 + 1i);
        break if true;
      }
    }
  }
  return 3i;
}
)");
}

TEST_F(IRToProgramInliningTest, LoadVarInLoopBody_ThenReadAndWriteToVarInLoopContinuing) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Store(var, 1_i);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* load = b.Load(var);
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {
                b.Store(var, b.Add(ty.i32(), load, 1_i));
                b.BreakIf(loop, true);
            });
        });
        b.Return(fn, 3_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var v : i32;
  v = 1i;
  loop {
    let v_1 = v;

    continuing {
      v = (v_1 + 1i);
      break if true;
    }
  }
  return 3i;
}
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
