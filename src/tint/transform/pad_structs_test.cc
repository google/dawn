// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/pad_structs.h"

#include <memory>
#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using PadStructsTest = TransformTest;

TEST_F(PadStructsTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    DataMap data;
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
    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
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

    DataMap data;
    auto got = Run<PadStructs>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
