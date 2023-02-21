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

#include "src/tint/transform/add_block_attribute.h"

#include <memory>
#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using AddBlockAttributeTest = TransformTest;

TEST_F(AddBlockAttributeTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Noop_UsedForPrivateVar) {
    auto* src = R"(
struct S {
  f : f32,
}

var<private> p : S;

@fragment
fn main() {
  p.f = 1.0;
}
)";
    auto* expect = src;

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Noop_UsedForShaderIO) {
    auto* src = R"(
struct S {
  @location(0)
  f : f32,
}

@fragment
fn main() -> S {
  return S();
}
)";
    auto* expect = src;

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicScalar) {
    auto* src = R"(
@group(0) @binding(0)
var<uniform> u : f32;

@fragment
fn main() {
  let f = u;
}
)";
    auto* expect = R"(
@internal(block)
struct u_block {
  inner : f32,
}

@group(0) @binding(0) var<uniform> u : u_block;

@fragment
fn main() {
  let f = u.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicArray) {
    auto* src = R"(
@group(0) @binding(0)
var<uniform> u : array<vec4<f32>, 4u>;

@fragment
fn main() {
  let a = u;
}
)";
    auto* expect = R"(
@internal(block)
struct u_block {
  inner : array<vec4<f32>, 4u>,
}

@group(0) @binding(0) var<uniform> u : u_block;

@fragment
fn main() {
  let a = u.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicArray_Alias) {
    auto* src = R"(
alias Numbers = array<vec4<f32>, 4u>;

@group(0) @binding(0)
var<uniform> u : Numbers;

@fragment
fn main() {
  let a = u;
}
)";
    auto* expect = R"(
alias Numbers = array<vec4<f32>, 4u>;

@internal(block)
struct u_block {
  inner : array<vec4<f32>, 4u>,
}

@group(0) @binding(0) var<uniform> u : u_block;

@fragment
fn main() {
  let a = u.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicStruct_AccessRoot) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<uniform> u : S;

@fragment
fn main() {
  let f = u;
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct u_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u : u_block;

@fragment
fn main() {
  let f = u.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicStruct_Storage_AccessRoot) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<storage, read_write> s : S;

@fragment
fn main() {
  let f = s;
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct s_block {
  inner : S,
}

@group(0) @binding(0) var<storage, read_write> s : s_block;

@fragment
fn main() {
  let f = s.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicStruct_Storage_TwoUsage_AccessRoot) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<storage, read_write> in : S;

@group(0) @binding(1)
var<storage, read_write> out : S;

@compute @workgroup_size(1)
fn main() {
  out = in;
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct in_block {
  inner : S,
}

@group(0) @binding(0) var<storage, read_write> in : in_block;

@group(0) @binding(1) var<storage, read_write> out : in_block;

@compute @workgroup_size(1)
fn main() {
  out.inner = in.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicStruct_AccessField) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<uniform> u : S;

@fragment
fn main() {
  let f = u.f;
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct u_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u : u_block;

@fragment
fn main() {
  let f = u.inner.f;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicScalar_PushConstant) {
    auto* src = R"(
enable chromium_experimental_push_constant;
var<push_constant> u : f32;

@fragment
fn main() {
  let f = u;
}
)";
    auto* expect = R"(
enable chromium_experimental_push_constant;

@internal(block)
struct u_block {
  inner : f32,
}

var<push_constant> u : u_block;

@fragment
fn main() {
  let f = u.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, BasicStruct_PushConstant) {
    auto* src = R"(
enable chromium_experimental_push_constant;
struct S {
  f : f32,
};
var<push_constant> u : S;

@fragment
fn main() {
  let f = u.f;
}
)";
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct S {
  f : f32,
}

@internal(block)
struct u_block {
  inner : S,
}

var<push_constant> u : u_block;

@fragment
fn main() {
  let f = u.inner.f;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Nested_OuterBuffer_InnerNotBuffer) {
    auto* src = R"(
struct Inner {
  f : f32,
};

struct Outer {
  i : Inner,
};

@group(0) @binding(0)
var<uniform> u : Outer;

@fragment
fn main() {
  let f = u.i.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

struct Outer {
  i : Inner,
}

@internal(block)
struct u_block {
  inner : Outer,
}

@group(0) @binding(0) var<uniform> u : u_block;

@fragment
fn main() {
  let f = u.inner.i.f;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Nested_OuterBuffer_InnerBuffer) {
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

@fragment
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

struct Outer {
  i : Inner,
}

@internal(block)
struct u0_block {
  inner : Outer,
}

@group(0) @binding(0) var<uniform> u0 : u0_block;

@internal(block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

@fragment
fn main() {
  let f0 = u0.inner.i.f;
  let f1 = u1.inner.f;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Nested_OuterNotBuffer_InnerBuffer) {
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

@fragment
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

@internal(block)
struct u_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u : u_block;

@fragment
fn main() {
  let f0 = p.i.f;
  let f1 = u.inner.f;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Nested_InnerUsedForMultipleBuffers) {
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

@fragment
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

struct S {
  i : Inner,
}

@internal(block)
struct u0_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u0 : u0_block;

@internal(block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

@group(0) @binding(2) var<uniform> u2 : u1_block;

@fragment
fn main() {
  let f0 = u0.inner.i.f;
  let f1 = u1.inner.f;
  let f2 = u2.inner.f;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, StructInArray) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<uniform> u : S;

@fragment
fn main() {
  let f = u.f;
  let a = array<S, 4>();
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct u_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u : u_block;

@fragment
fn main() {
  let f = u.inner.f;
  let a = array<S, 4>();
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, StructInArray_MultipleBuffers) {
    auto* src = R"(
struct S {
  f : f32,
};

@group(0) @binding(0)
var<uniform> u0 : S;

@group(0) @binding(1)
var<uniform> u1 : S;

@fragment
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

@internal(block)
struct u0_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u0 : u0_block;

@group(0) @binding(1) var<uniform> u1 : u0_block;

@fragment
fn main() {
  let f0 = u0.inner.f;
  let f1 = u1.inner.f;
  let a = array<S, 4>();
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Aliases_Nested_OuterBuffer_InnerBuffer) {
    auto* src = R"(
struct Inner {
  f : f32,
};

alias MyInner = Inner;

struct Outer {
  i : MyInner,
};

alias MyOuter = Outer;

@group(0) @binding(0)
var<uniform> u0 : MyOuter;

@group(0) @binding(1)
var<uniform> u1 : MyInner;

@fragment
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}
)";
    auto* expect = R"(
struct Inner {
  f : f32,
}

alias MyInner = Inner;

struct Outer {
  i : MyInner,
}

alias MyOuter = Outer;

@internal(block)
struct u0_block {
  inner : Outer,
}

@group(0) @binding(0) var<uniform> u0 : u0_block;

@internal(block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

@fragment
fn main() {
  let f0 = u0.inner.i.f;
  let f1 = u1.inner.f;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, Aliases_Nested_OuterBuffer_InnerBuffer_OutOfOrder) {
    auto* src = R"(
@fragment
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}

@group(0) @binding(1)
var<uniform> u1 : MyInner;

alias MyInner = Inner;

@group(0) @binding(0)
var<uniform> u0 : MyOuter;

alias MyOuter = Outer;

struct Outer {
  i : MyInner,
};

struct Inner {
  f : f32,
};
)";
    auto* expect = R"(
@fragment
fn main() {
  let f0 = u0.inner.i.f;
  let f1 = u1.inner.f;
}

@internal(block)
struct u1_block {
  inner : Inner,
}

@group(0) @binding(1) var<uniform> u1 : u1_block;

alias MyInner = Inner;

@internal(block)
struct u0_block {
  inner : Outer,
}

@group(0) @binding(0) var<uniform> u0 : u0_block;

alias MyOuter = Outer;

struct Outer {
  i : MyInner,
}

struct Inner {
  f : f32,
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, UniformAndPrivateUsages) {
    auto* src = R"(
struct S {
  f : f32,
}

@group(0) @binding(0) var<uniform> u : S;

var<private> p : S;

@fragment
fn main() {
  p = u;
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct u_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u : u_block;

var<private> p : S;

@fragment
fn main() {
  p = u.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, StorageAndPrivateUsages) {
    auto* src = R"(
struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read_write> s : S;

var<private> p : S;

@fragment
fn main() {
  p = s;
  p.f = 1234.0;
  s = p;
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct s_block {
  inner : S,
}

@group(0) @binding(0) var<storage, read_write> s : s_block;

var<private> p : S;

@fragment
fn main() {
  p = s.inner;
  p.f = 1234.0;
  s.inner = p;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, StorageAndUniformUsages) {
    auto* src = R"(
struct S {
  f : f32,
}

@group(0) @binding(0) var<uniform> u : S;

@group(0) @binding(1) var<storage, read_write> s : S;

@fragment
fn main() {
  s = u;
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block)
struct u_block {
  inner : S,
}

@group(0) @binding(0) var<uniform> u : u_block;

@group(0) @binding(1) var<storage, read_write> s : u_block;

@fragment
fn main() {
  s.inner = u.inner;
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, PrivateUsageOnly) {
    auto* src = R"(
struct S {
  f : f32,
}

var<private> p : S;

@fragment
fn main() {
  p.f = 4321.0f;
}
)";
    auto* expect = src;

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(AddBlockAttributeTest, StorageBufferWithRuntimeArray) {
    auto* src = R"(
struct S {
  f : f32,
}

struct SWithArr {
  f : f32,
  arr : array<f32>,
}

@group(0) @binding(0)
var<storage, read> in_1 : S;

@group(0) @binding(1)
var<storage, read> in_2 : SWithArr;

@group(1) @binding(0)
var<storage, read_write> out : SWithArr;

@fragment
fn main() {
  out.f = in_1.f;
  out.arr[0] = in_2.arr[1];
}
)";
    auto* expect = R"(
struct S {
  f : f32,
}

@internal(block) @internal(block)
struct SWithArr {
  f : f32,
  arr : array<f32>,
}

@internal(block)
struct in_1_block {
  inner : S,
}

@group(0) @binding(0) var<storage, read> in_1 : in_1_block;

@group(0) @binding(1) var<storage, read> in_2 : SWithArr;

@group(1) @binding(0) var<storage, read_write> out : SWithArr;

@fragment
fn main() {
  out.f = in_1.inner.f;
  out.arr[0] = in_2.arr[1];
}
)";

    auto got = Run<AddBlockAttribute>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
