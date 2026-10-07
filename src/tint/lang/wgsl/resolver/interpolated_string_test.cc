// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using ResolverInterpolatedStringTest = ResolverTest;

TEST_F(ResolverInterpolatedStringTest, Empty) {
    EXPECT_SUCCESS(R"(
fn f() {
  print(``);
}
)");
}

TEST_F(ResolverInterpolatedStringTest, StringLiteral) {
    EXPECT_SUCCESS(R"(
fn f() {
  print(`hello world`);
}
)");
}

TEST_F(ResolverInterpolatedStringTest, ConstantInterpolation) {
    EXPECT_SUCCESS(R"(
fn f() {
  print(`val: ${42i}`);
}
)");
}

TEST_F(ResolverInterpolatedStringTest, RuntimeInterpolation) {
    EXPECT_SUCCESS(R"(
fn f() {
  let x = 42i;
  print(`val: ${x}`);
}
)");
}

TEST_F(ResolverInterpolatedStringTest, OverrideInterpolation) {
    EXPECT_SUCCESS(R"(
override x: i32;
fn f() {
  print(`val: ${x}`);
}
)");
}

TEST_F(ResolverInterpolatedStringTest, ValidInterpolationTypes) {
    EXPECT_SUCCESS(R"(
enable f16;
fn f() {
  let b: bool = true;
  let i: i32 = 1i;
  let u: u32 = 2u;
  let f_val: f32 = 3.0f;
  let h: f16 = 4.0h;
  let v2i: vec2<i32> = vec2<i32>(1i, 2i);
  let v3f: vec3<f32> = vec3<f32>(1.0f, 2.0f, 3.0f);
  print(`bool: ${b} i32: ${i} u32: ${u} f32: ${f_val} f16: ${h} abstract_int: ${5} abstract_float: ${6.0} vec2i: ${v2i} vec3f: ${v3f} nested: ${`nested_str`}`);
}
)");
}

TEST_F(ResolverInterpolatedStringTest, InvalidInterpolationType_Struct) {
    EXPECT_ERROR(
        R"(
struct S {
  a: i32,
}
fn f() {
  var s: S;
  print(`s: ${s}`);
}
)",
        R"(
input.wgsl:7:15 error: S cannot be interpolated in a string
  print(`s: ${s}`);
              ^
)");
}

TEST_F(ResolverInterpolatedStringTest, InvalidInterpolationType_Array) {
    EXPECT_ERROR(
        R"(
fn f() {
  var a: array<i32, 4>;
  print(`a: ${a}`);
}
)",
        R"(
input.wgsl:4:15 error: array<i32, 4> cannot be interpolated in a string
  print(`a: ${a}`);
              ^
)");
}

TEST_F(ResolverInterpolatedStringTest, InvalidInterpolationType_Matrix) {
    EXPECT_ERROR(
        R"(
fn f() {
  var m: mat2x2<f32>;
  print(`m: ${m}`);
}
)",
        R"(
input.wgsl:4:15 error: mat2x2<f32> cannot be interpolated in a string
  print(`m: ${m}`);
              ^
)");
}

TEST_F(ResolverInterpolatedStringTest, InvalidInterpolationType_Atomic) {
    EXPECT_ERROR(
        R"(
var<workgroup> a: atomic<i32>;
fn f() {
  print(`a: ${a}`);
}
)",
        R"(
input.wgsl:4:15 error: atomic<i32> cannot be interpolated in a string
  print(`a: ${a}`);
              ^
)");
}

TEST_F(ResolverInterpolatedStringTest, ChromiumPrint_FeatureDisallowed) {
    ExpectError(
        R"(
fn f() {
  print(`hello`);
}
)",
        R"(
input.wgsl:3:9 error: the 'chromium_print' language feature is not allowed in the current environment
  print(`hello`);
        ^^^^^^^
)",
        wgsl::AllowedFeatures{});
}

TEST_F(ResolverInterpolatedStringTest, ChromiumPrint_FeatureDisallowed_InterpolatedString) {
    ExpectError(
        R"(
fn f() {
  print(`${42}`);
}
)",
        R"(
input.wgsl:3:9 error: the 'chromium_print' language feature is not allowed in the current environment
  print(`${42}`);
        ^^^^^^^
)",
        wgsl::AllowedFeatures{});
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInConst) {
    EXPECT_ERROR(
        R"(
fn f() {
  const s = `hello`;
}
)",
        R"(
input.wgsl:3:9 error: string cannot be used as the type of a 'const'
  const s = `hello`;
        ^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInGlobalConst) {
    EXPECT_ERROR(
        R"(
const s = `hello`;
)",
        R"(
input.wgsl:2:1 error: string cannot be used as the type of a 'const'
const s = `hello`;
^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInLet) {
    EXPECT_ERROR(
        R"(
fn f() {
  let s = `hello`;
}
)",
        R"(
input.wgsl:3:7 error: string cannot be used as the type of a 'let'
  let s = `hello`;
      ^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInLocalVar) {
    EXPECT_ERROR(
        R"(
fn f() {
  var s = `hello`;
}
)",
        R"(
input.wgsl:3:3 error: function-scope 'var' must have a constructible type
  var s = `hello`;
  ^^^^^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInGlobalVar) {
    EXPECT_ERROR(
        R"(
var<private> s = `hello`;
)",
        R"(
input.wgsl:2:1 error: string cannot be used as the type of a var
var<private> s = `hello`;
^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInOverride) {
    EXPECT_ERROR(
        R"(
override s = `hello`;
)",
        R"(
input.wgsl:2:1 error: string cannot be used as the type of a 'override'
override s = `hello`;
^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInParam) {
    EXPECT_ERROR(
        R"(
fn f(s: string) {}
)",
        R"(
input.wgsl:2:9 error: unresolved type 'string'
fn f(s: string) {}
        ^^^^^^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInReturnType) {
    EXPECT_ERROR(
        R"(
fn f() -> string {
  return `hello`;
}
)",
        R"(
input.wgsl:2:11 error: unresolved type 'string'
fn f() -> string {
          ^^^^^^
)");
}

TEST_F(ResolverInterpolatedStringTest, StringDisallowedInPhonyAssignment) {
    EXPECT_ERROR(
        R"(
fn f() {
  _ = `hello`;
}
)",
        R"(
input.wgsl:3:7 error: cannot assign 'string' to '_'. '_' can only be assigned a constructible, pointer, texture or sampler type
  _ = `hello`;
      ^^^^^^^
)");
}

}  // namespace
}  // namespace tint::resolver
