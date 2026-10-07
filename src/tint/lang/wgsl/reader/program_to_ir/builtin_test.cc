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

#include "src/tint/lang/wgsl/reader/program_to_ir/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRBuiltinTest = helpers::IRProgramTest;

TEST_F(ProgramToIRBuiltinTest, EmitExpression_Builtin) {
    auto i = GlobalVar("i", core::AddressSpace::kPrivate, Expr(1_f));
    auto* expr = Call("asin", i);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"($B1: {  # root
  %i:ptr<private, f32, read_write> = var 1.0f
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f32 = load %i
    %4:f32 = asin %3
    %tint_symbol:f32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_StringLiteral) {
    auto m = Build(R"(
fn foo() {
    print(`hello world`);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%foo = func():void {
  $B1: {
    %2:void = print "hello world"
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_StringLiteral_Empty) {
    auto m = Build(R"(
fn foo() {
    print(``);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%foo = func():void {
  $B1: {
    %2:void = print ""
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_InterpolatedString_SingleExpression) {
    auto m = Build(R"(
fn foo(x : i32) {
    print(`${x}`);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%foo = func(%x:i32):void {
  $B1: {
    %3:string = interpolate_string %x
    %4:void = print %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_InterpolatedString_Expression) {
    auto m = Build(R"(
fn foo(x : i32) {
    print(`x = ${x}`);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%foo = func(%x:i32):void {
  $B1: {
    %3:string = interpolate_string "x = ", %x
    %4:void = print %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_InterpolatedString_MultipleExpressions) {
    auto m = Build(R"(
fn foo(x : i32, y : f32) {
    print(`x = ${x}, y = ${y}!`);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%foo = func(%x:i32, %y:f32):void {
  $B1: {
    %4:string = interpolate_string "x = ", %x, ", y = ", %y, "!"
    %5:void = print %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_InterpolatedString_Vector) {
    auto m = Build(R"(
fn foo(v : vec2<f32>) {
    print(`vec: ${v}`);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%foo = func(%v:vec2<f32>):void {
  $B1: {
    %3:string = interpolate_string "vec: ", %v
    %4:void = print %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_InterpolatedString_Nested) {
    auto m = Build(R"(
fn foo(x : i32) {
    print(`outer ${`inner ${x}`} end`);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"(%foo = func(%x:i32):void {
  $B1: {
    %3:string = interpolate_string "inner ", %x
    %4:string = interpolate_string "outer ", %3, " end"
    %5:void = print %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBuiltinTest, Print_InterpolatedString_EvaluationOrder) {
    auto m = Build(R"(
var<private> a : i32 = 1i;

fn side_effect(x : i32) -> i32 {
    a = a + x;
    return a;
}

fn foo() {
    print(`${side_effect(1i)} ${side_effect(2i)}`);
}
)");
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()), R"($B1: {  # root
  %a:ptr<private, i32, read_write> = var 1i
}

%side_effect = func(%x:i32):i32 {
  $B2: {
    %4:i32 = load %a
    %5:i32 = add %4, %x
    store %a, %5
    %6:i32 = load %a
    ret %6
  }
}
%foo = func():void {
  $B3: {
    %8:i32 = call %side_effect, 1i
    %9:i32 = call %side_effect, 2i
    %10:string = interpolate_string %8, " ", %9
    %11:void = print %10
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
