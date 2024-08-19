// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/ast_raise/pad_structs.h"

#include <memory>
#include <utility>

#include "src/tint/lang/wgsl/ast/transform/add_block_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::glsl::writer {
namespace {

using PadStructsTest = ast::transform::TransformTest;

TEST_F(PadStructsTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, Uniform) {
    auto* src = R"(
struct S {
  x : i32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    auto* expect = R"(
struct S {
  x : i32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, UniformStructSizeSmallerThan16) {
    auto* src = R"(
struct N {
  x : i32,
  y : i32,
  z : i32,
}
struct S {
  x : i32,
  @align(16)
  y : N,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    auto* expect = R"(
struct N {
  x : i32,
  y : i32,
  z : i32,
}

@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  @size(16)
  y : N,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, UniformStructSizeSmallerThan16NonLastMember) {
    auto* src = R"(
struct N {
  x : i32,
  y : i32,
  z : i32,
}
struct S {
  x : i32,
  @align(16)
  y : N,
  @align(16)
  z : i32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    auto* expect = R"(
struct N {
  x : i32,
  y : i32,
  z : i32,
}

@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  @size(16)
  y : N,
  z : i32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, UniformStructExplicitSizeAttrSmaller) {
    auto* src = R"(
struct N {
  x : i32,
  @align(4) @size(4) y : i32,
  z : i32,
}
struct S {
  x : i32,
  @align(16) n : N,
  @align(16) y : i32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    auto* expect = R"(
struct N {
  x : i32,
  y : i32,
  z : i32,
}

@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  @size(16)
  n : N,
  y : i32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, UniformStructExplicitSizeAttrLarger) {
    auto* src = R"(
struct N {
  x : i32,
  y : i32,
  z : i32,
}
struct S {
  x : i32,
  @align(32) @size(20)
  y : N,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    auto* expect = R"(
struct N {
  x : i32,
  y : i32,
  z : i32,
}

@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
  pad_6 : u32,
  @size(16)
  y : N,
  pad_7 : u32,
  pad_8 : u32,
  pad_9 : u32,
  pad_10 : u32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, UniformStructExplicitSizeAttrLarger2) {
    auto* src = R"(
struct I {
  @align(32) @size(33) x : u32,
}
struct S {
  x : I,
  y : I,
}

@group(0) @binding(0) var<uniform> in : S;

@compute @workgroup_size(1,1,1)
fn main() {
  let x = in.y.x;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct I {
  x : u32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
  pad_6 : u32,
  pad_7 : u32,
  pad_8 : u32,
  pad_9 : u32,
  pad_10 : u32,
  pad_11 : u32,
  pad_12 : u32,
  pad_13 : u32,
  pad_14 : u32,
}

@internal(disable_validation__ignore_struct_member)
struct S {
  @size(64)
  x : I,
  @size(64)
  y : I,
}

@group(0) @binding(0) var<uniform> in : S;

@compute @workgroup_size(1, 1, 1)
fn main() {
  let x = in.y.x;
}
)";
    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, UniformStructExplicitSizeAttrInParentStruct) {
    auto* src = R"(
struct A {
  @size(32)
  x : u32,
}

struct B {
  @size(64)
  x : A,
  y : u32,
}

struct U {
  b : B,
}

@group(0) @binding(1) var<uniform> u : U;

@compute @workgroup_size(1, 1, 1)
fn main() {
  let y = u.b.y;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct A {
  x : u32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
  pad_6 : u32,
}

@internal(disable_validation__ignore_struct_member)
struct B {
  @size(32)
  x : A,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
  pad_6 : u32,
  pad_7 : u32,
  y : u32,
}

@internal(disable_validation__ignore_struct_member)
struct U {
  @size(80)
  b : B,
}

@group(0) @binding(1) var<uniform> u : U;

@compute @workgroup_size(1, 1, 1)
fn main() {
  let y = u.b.y;
}
)";
    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, Size) {
    auto* src = R"(
struct S {
  @size(12)
  x : i32,
  y : i32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  y : i32,
}

@group(0) @binding(0) var<uniform> u : S;

fn main() {
  let x = u.x;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, SizeUniformAndPrivate) {
    auto* src = R"(
struct S {
  @size(12)
  x : i32,
  y : i32,
}

@group(0) @binding(0) var<uniform> u : S;

var<private> p : S;

fn main() {
  p.x = u.x;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  y : i32,
}

@group(0) @binding(0) var<uniform> u : S;

var<private> p : S;

fn main() {
  p.x = u.x;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, SizeStorageAndPrivate) {
    auto* src = R"(
struct S {
  @size(12)
  x : i32,
  y : i32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

var<private> p : S;

fn main() {
  p.x = 123;
  s.x = p.x;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  y : i32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

var<private> p : S;

fn main() {
  p.x = 123;
  s.x = p.x;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, SizeUniformAndStorage) {
    auto* src = R"(
struct S {
  @size(12)
  x : i32,
  y : i32,
}

@group(0) @binding(0) var<uniform> u : S;

@group(0) @binding(1) var<storage, read_write> s : S;

fn main() {
  s.x = u.x;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  y : i32,
}

@group(0) @binding(0) var<uniform> u : S;

@group(0) @binding(1) var<storage, read_write> s : S;

fn main() {
  s.x = u.x;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, SizePrivateOnly) {
    // Structs that are not host-visible should have no explicit padding.
    auto* src = R"(
struct S {
  @size(12)
  x : i32,
  y : i32,
}

var<private> p : S;

fn main() {
  p.x = 123;
}
)";
    auto* expect = R"(
struct S {
  @size(12)
  x : i32,
  y : i32,
}

var<private> p : S;

fn main() {
  p.x = 123;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, AlignUniformAndPrivate) {
    auto* src = R"(
struct S {
  a : i32,
  @align(16)
  b : i32,
}

@group(0) @binding(0) var<uniform> u : S;

var<private> p : S;

fn main() {
  p.a = u.b;
  p.b = u.a;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  a : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  b : i32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
}

@group(0) @binding(0) var<uniform> u : S;

var<private> p : S;

fn main() {
  p.a = u.b;
  p.b = u.a;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, AlignStorageAndPrivate) {
    auto* src = R"(
struct S {
  a : i32,
  @align(16)
  b : i32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

var<private> p : S;

fn main() {
  p.a = 123;
  p.b = 321;
  s.a = p.b;
  s.b = p.a;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  a : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  b : i32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

var<private> p : S;

fn main() {
  p.a = 123;
  p.b = 321;
  s.a = p.b;
  s.b = p.a;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, AlignUniformAndStorage) {
    auto* src = R"(
struct S {
  a : i32,
  @align(16)
  b : i32,
}

@group(0) @binding(0) var<uniform> u : S;

@group(0) @binding(1) var<storage, read_write> s : S;

fn main() {
  s.a = u.b;
  s.b = u.a;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  a : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
  b : i32,
  pad_3 : u32,
  pad_4 : u32,
  pad_5 : u32,
}

@group(0) @binding(0) var<uniform> u : S;

@group(0) @binding(1) var<storage, read_write> s : S;

fn main() {
  s.a = u.b;
  s.b = u.a;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, AlignPrivateOnly) {
    // Structs that are not host-visible should have no explicit padding.
    auto* src = R"(
struct S {
  a : i32,
  @align(16)
  b : i32,
}

var<private> p : S;

fn main() {
  p.a = 123;
  p.b = 321;
}
)";
    auto* expect = R"(
struct S {
  a : i32,
  @align(16)
  b : i32,
}

var<private> p : S;

fn main() {
  p.a = 123;
  p.b = 321;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, StructWithBlockAttribute) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> u : i32;

fn main() {
  let x = u;
}
)";
    auto* expect = R"(
@internal(block)
struct u_block {
  inner : i32,
}

@group(0) @binding(0) var<uniform> u : u_block;

fn main() {
  let x = u.inner;
}
)";
    ast::transform::DataMap data;
    auto blockAdded = Run<ast::transform::AddBlockAttribute>(src, data);
    auto got = Run<PadStructs>(std::move(blockAdded.program), data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, Initializer) {
    // Calls to a initializer of a padded struct must be modified to initialize the padding.
    auto* src = R"(
struct S {
  a : f32,
  @align(8)
  b : i32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s = S(1.0f, 2);
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  a : f32,
  pad : u32,
  b : i32,
  pad_1 : u32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s = S(1.0f, 0u, 2, 0u);
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, MemberNamedPad) {
    auto* src = R"(
struct S {
  @align(8) pad_5 : u32,
  @align(8) pad_3 : u32,
  @align(8) pad :   u32,
  @align(8) pad_1 : u32,
}

@group(0) @binding(0) var<uniform> s : S;

fn main() {
  _ = s;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  pad_5 : u32,
  pad_2 : u32,
  pad_3 : u32,
  pad_4 : u32,
  pad : u32,
  pad_6 : u32,
  pad_1 : u32,
  pad_7 : u32,
}

@group(0) @binding(0) var<uniform> s : S;

fn main() {
  _ = s;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, TwoStructs) {
    auto* src = R"(
struct S {
  @align(8) i : i32,
  @align(8) u : u32,
}

struct T {
  @align(8) i : i32,
  @align(8) u : u32,
}

@group(0) @binding(0) var<uniform> s : S;
@group(0) @binding(1) var<uniform> t : T;

fn main() {
  _ = s;
  _ = t;
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  i : i32,
  pad : u32,
  u : u32,
  pad_1 : u32,
}

@internal(disable_validation__ignore_struct_member)
struct T {
  i : i32,
  pad : u32,
  u : u32,
  pad_1 : u32,
}

@group(0) @binding(0) var<uniform> s : S;

@group(0) @binding(1) var<uniform> t : T;

fn main() {
  _ = s;
  _ = t;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, GlobalVariableNamedPad) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> pad : u32;
@group(0) @binding(1) var<uniform> pad_1 : u32;

struct S {
  @align(8) i : u32,
  @align(8) j : u32,
}

@group(0) @binding(1) var<uniform> s : S;

fn main() {
  _ = s;
}
)";
    auto* expect = R"(
@group(0) @binding(0) var<uniform> pad : u32;

@group(0) @binding(1) var<uniform> pad_1 : u32;

@internal(disable_validation__ignore_struct_member)
struct S {
  i : u32,
  pad : u32,
  j : u32,
  pad_1 : u32,
}

@group(0) @binding(1) var<uniform> s : S;

fn main() {
  _ = s;
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, InitializerZeroArgs) {
    // Calls to a zero-argument initializer of a padded struct should not be modified.
    auto* src = R"(
struct S {
  a : f32,
  @align(8)
  b : i32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s = S();
}
)";
    auto* expect = R"(
@internal(disable_validation__ignore_struct_member)
struct S {
  a : f32,
  pad : u32,
  b : i32,
  pad_1 : u32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s = S();
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::glsl::writer
