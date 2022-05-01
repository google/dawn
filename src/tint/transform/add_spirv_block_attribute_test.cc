// Copyright 2021 The Tint Authors.
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

#include "src/tint/transform/add_spirv_block_attribute.h"

#include <memory>
#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using AddSpirvBlockAttributeTest = TransformTest;

TEST_F(AddSpirvBlockAttributeTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Noop_UsedForPrivateVar) {
    auto* src = R"(
struct S {
  f : f32,
}

var<private> p : S;

@stage(fragment)
fn main() {
  p.f = 1.0;
}
)";
    auto* expect = src;

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Noop_UsedForShaderIO) {
    auto* src = R"(
struct S {
  @location(0)
  f : f32,
}

@stage(fragment)
fn main() -> S {
  return S();
}
)";
    auto* expect = src;

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, BasicScalar) {
    auto* src = R"(
@group(0) @binding(0)
var<uniform> u : f32;

@stage(fragment)
fn main() {
  let f = u;
}
)";
    auto* expect = R"(
@internal(spirv_block)
struct u_block {
  inner : f32,
}

@group(0) @binding(0) var<uniform> u : u_block;

@stage(fragment)
fn main() {
  let f = u.inner;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, BasicArray) {
    auto* src = R"(
@group(0) @binding(0)
var<uniform> u : array<vec4<f32>, 4u>;

@stage(fragment)
fn main() {
  let a = u;
}
)";
    auto* expect = R"(
@internal(spirv_block)
struct u_block {
  inner : array<vec4<f32>, 4u>,
}

@group(0) @binding(0) var<uniform> u : u_block;

@stage(fragment)
fn main() {
  let a = u.inner;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, BasicArray_Alias) {
    auto* src = R"(
type Numbers = array<vec4<f32>, 4u>;

@group(0) @binding(0)
var<uniform> u : Numbers;

@stage(fragment)
fn main() {
  let a = u;
}
)";
    auto* expect = R"(
type Numbers = array<vec4<f32>, 4u>;

@internal(spirv_block)
struct u_block {
  inner : array<vec4<f32>, 4u>,
}

@group(0) @binding(0) var<uniform> u : u_block;

@stage(fragment)
fn main() {
  let a = u.inner;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, BasicStruct) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<uniform> u : S;

@stage(fragment)
fn main() {
  let f = u.f;
}
)";
    auto* expect = R"(
@internal(spirv_block)
struct S {
  f : f32,
}

@group(0) @binding(0) var<uniform> u : S;

@stage(fragment)
fn main() {
  let f = u.f;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Nested_OuterBuffer_InnerNotBuffer) {
    auto* src = R"(
struct Inner {
  f : f32,
};

struct Outer {
  i : Inner,
};

@group(0) @binding(0)
var<uniform> u : Outer;

@stage(fragment)
fn main() {
  let f = u.i.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

@internal(spirv_block)
struct Outer {
  i : Inner,
}

@group(0) @binding(0) var<uniform> u : Outer;

@stage(fragment)
fn main() {
  let f = u.i.f;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Nested_OuterBuffer_InnerBuffer) {
    auto* src = R"(
struct Inner {
  f : f32,
};

struct Outer {
  i : Inner,
};

@group(0) @binding(0)
var<uniform> u0 : Outer;

@group(0) @binding(1)
var<uniform> u1 : Inner;

@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

@internal(spirv_block)
struct Outer {
  i : Inner,
}

@group(0) @binding(0) var<uniform> u0 : Outer;

@internal(spirv_block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.inner.f;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Nested_OuterNotBuffer_InnerBuffer) {
    auto* src = R"(
struct Inner {
  f : f32,
};

struct Outer {
  i : Inner,
};

var<private> p : Outer;

@group(0) @binding(1)
var<uniform> u : Inner;

@stage(fragment)
fn main() {
  let f0 = p.i.f;
  let f1 = u.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

struct Outer {
  i : Inner,
}

var<private> p : Outer;

@internal(spirv_block)
struct u_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u : u_block;

@stage(fragment)
fn main() {
  let f0 = p.i.f;
  let f1 = u.inner.f;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Nested_InnerUsedForMultipleBuffers) {
    auto* src = R"(
struct Inner {
  f : f32,
};

struct S {
  i : Inner,
};

@group(0) @binding(0)
var<uniform> u0 : S;

@group(0) @binding(1)
var<uniform> u1 : Inner;

@group(0) @binding(2)
var<uniform> u2 : Inner;

@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
  let f2 = u2.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

@internal(spirv_block)
struct S {
  i : Inner,
}

@group(0) @binding(0) var<uniform> u0 : S;

@internal(spirv_block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

@group(0) @binding(2) var<uniform> u2 : u1_block;

@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.inner.f;
  let f2 = u2.inner.f;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, StructInArray) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<uniform> u : S;

@stage(fragment)
fn main() {
  let f = u.f;
  let a = array<S, 4>();
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(spirv_block)
struct u_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u : u_block;

@stage(fragment)
fn main() {
  let f = u.inner.f;
  let a = array<S, 4>();
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, StructInArray_MultipleBuffers) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<uniform> u0 : S;

@group(0) @binding(1)
var<uniform> u1 : S;

@stage(fragment)
fn main() {
  let f0 = u0.f;
  let f1 = u1.f;
  let a = array<S, 4>();
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(spirv_block)
struct u0_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u0 : u0_block;

@group(0) @binding(1) var<uniform> u1 : u0_block;

@stage(fragment)
fn main() {
  let f0 = u0.inner.f;
  let f1 = u1.inner.f;
  let a = array<S, 4>();
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Aliases_Nested_OuterBuffer_InnerBuffer) {
    auto* src = R"(
struct Inner {
  f : f32,
};

type MyInner = Inner;

struct Outer {
  i : MyInner,
};

type MyOuter = Outer;

@group(0) @binding(0)
var<uniform> u0 : MyOuter;

@group(0) @binding(1)
var<uniform> u1 : MyInner;

@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

type MyInner = Inner;

@internal(spirv_block)
struct Outer {
  i : MyInner,
}

type MyOuter = Outer;

@group(0) @binding(0) var<uniform> u0 : MyOuter;

@internal(spirv_block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.inner.f;
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockAttributeTest, Aliases_Nested_OuterBuffer_InnerBuffer_OutOfOrder) {
    auto* src = R"(
@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}

@group(0) @binding(1)
var<uniform> u1 : MyInner;

type MyInner = Inner;

@group(0) @binding(0)
var<uniform> u0 : MyOuter;

type MyOuter = Outer;

struct Outer {
  i : MyInner,
};

struct Inner {
  f : f32,
};
)";
    auto* expect = R"(
@stage(fragment)
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.inner.f;
}

@internal(spirv_block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

type MyInner = Inner;

@group(0) @binding(0) var<uniform> u0 : MyOuter;

type MyOuter = Outer;

@internal(spirv_block)
struct Outer {
  i : MyInner,
}

struct Inner {
  f : f32,
}
)";

    auto got = Run<AddSpirvBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
