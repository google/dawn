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
@internal(disable_validation__ignore_struct_member)
struct S {
  x : i32,
  pad : u32,
  pad_1 : u32,
  pad_2 : u32,
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

TEST_F(PadStructsTest, LastMemberRuntimeSizeArray) {
    // Structs with runtime-sized arrays should not be padded after the
    // last member.
    auto* src = R"(
struct T {
  a : f32,
  b : i32,
}

struct S {
  a : vec4<f32>,
  b : array<T>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s.b[0] = T(1.0f, 23);
}
)";
    auto* expect = R"(
struct T {
  a : f32,
  b : i32,
}

struct S {
  a : vec4<f32>,
  b : array<T>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s.b[0] = T(1.0f, 23);
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PadStructsTest, LastMemberFixedSizeArray) {
    // Structs without runtime-sized arrays should be padded after the last
    // member.
    auto* src = R"(
struct T {
  a : f32,
  b : i32,
}

struct S {
  a : vec4<f32>,
  b : array<T, 1u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s.b[0] = T(1.0f, 23);
}
)";
    auto* expect = R"(
struct T {
  a : f32,
  b : i32,
}

@internal(disable_validation__ignore_struct_member)
struct S {
  a : vec4<f32>,
  b : array<T, 1u>,
  pad : u32,
  pad_1 : u32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn main() {
  s.b[0] = T(1.0f, 23);
}
)";

    ast::transform::DataMap data;
    auto got = Run<PadStructs>(src, data);

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
