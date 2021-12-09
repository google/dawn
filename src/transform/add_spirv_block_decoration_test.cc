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

#include "src/transform/add_spirv_block_decoration.h"

#include <memory>
#include <utility>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using AddSpirvBlockDecorationTest = TransformTest;

TEST_F(AddSpirvBlockDecorationTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Noop_UsedForPrivateVar) {
  auto* src = R"(
struct S {
  f : f32;
};

var<private> p : S;

[[stage(fragment)]]
fn main() {
  p.f = 1.0;
}
)";
  auto* expect = src;

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Noop_UsedForShaderIO) {
  auto* src = R"(
struct S {
  [[location(0)]]
  f : f32;
};

[[stage(fragment)]]
fn main() -> S {
  return S();
}
)";
  auto* expect = src;

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Basic) {
  auto* src = R"(
struct S {
  f : f32;
};

[[group(0), binding(0)]]
var<uniform> u : S;

[[stage(fragment)]]
fn main() {
  let f = u.f;
}
)";
  auto* expect = R"(
[[internal(spirv_block)]]
struct S {
  f : f32;
};

[[group(0), binding(0)]] var<uniform> u : S;

[[stage(fragment)]]
fn main() {
  let f = u.f;
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Nested_OuterBuffer_InnerNotBuffer) {
  auto* src = R"(
struct Inner {
  f : f32;
};

struct Outer {
  i : Inner;
};

[[group(0), binding(0)]]
var<uniform> u : Outer;

[[stage(fragment)]]
fn main() {
  let f = u.i.f;
}
)";
  auto* expect = R"(
struct Inner {
  f : f32;
};

[[internal(spirv_block)]]
struct Outer {
  i : Inner;
};

[[group(0), binding(0)]] var<uniform> u : Outer;

[[stage(fragment)]]
fn main() {
  let f = u.i.f;
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Nested_OuterBuffer_InnerBuffer) {
  auto* src = R"(
struct Inner {
  f : f32;
};

struct Outer {
  i : Inner;
};

[[group(0), binding(0)]]
var<uniform> u0 : Outer;

[[group(0), binding(1)]]
var<uniform> u1 : Inner;

[[stage(fragment)]]
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}
)";
  auto* expect = R"(
struct Inner {
  f : f32;
};

[[internal(spirv_block)]]
struct Inner_block {
  inner : Inner;
};

[[internal(spirv_block)]]
struct Outer {
  i : Inner;
};

[[group(0), binding(0)]] var<uniform> u0 : Outer;

[[group(0), binding(1)]] var<uniform> u1 : Inner_block;

[[stage(fragment)]]
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.inner.f;
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Nested_OuterNotBuffer_InnerBuffer) {
  auto* src = R"(
struct Inner {
  f : f32;
};

struct Outer {
  i : Inner;
};

var<private> p : Outer;

[[group(0), binding(1)]]
var<uniform> u : Inner;

[[stage(fragment)]]
fn main() {
  let f0 = p.i.f;
  let f1 = u.f;
}
)";
  auto* expect = R"(
struct Inner {
  f : f32;
};

[[internal(spirv_block)]]
struct Inner_block {
  inner : Inner;
};

struct Outer {
  i : Inner;
};

var<private> p : Outer;

[[group(0), binding(1)]] var<uniform> u : Inner_block;

[[stage(fragment)]]
fn main() {
  let f0 = p.i.f;
  let f1 = u.inner.f;
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Nested_InnerUsedForMultipleBuffers) {
  auto* src = R"(
struct Inner {
  f : f32;
};

struct S {
  i : Inner;
};

[[group(0), binding(0)]]
var<uniform> u0 : S;

[[group(0), binding(1)]]
var<uniform> u1 : Inner;

[[group(0), binding(2)]]
var<uniform> u2 : Inner;

[[stage(fragment)]]
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
  let f2 = u2.f;
}
)";
  auto* expect = R"(
struct Inner {
  f : f32;
};

[[internal(spirv_block)]]
struct Inner_block {
  inner : Inner;
};

[[internal(spirv_block)]]
struct S {
  i : Inner;
};

[[group(0), binding(0)]] var<uniform> u0 : S;

[[group(0), binding(1)]] var<uniform> u1 : Inner_block;

[[group(0), binding(2)]] var<uniform> u2 : Inner_block;

[[stage(fragment)]]
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.inner.f;
  let f2 = u2.inner.f;
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, StructInArray) {
  auto* src = R"(
struct S {
  f : f32;
};

[[group(0), binding(0)]]
var<uniform> u : S;

[[stage(fragment)]]
fn main() {
  let f = u.f;
  let a = array<S, 4>();
}
)";
  auto* expect = R"(
struct S {
  f : f32;
};

[[internal(spirv_block)]]
struct S_block {
  inner : S;
};

[[group(0), binding(0)]] var<uniform> u : S_block;

[[stage(fragment)]]
fn main() {
  let f = u.inner.f;
  let a = array<S, 4>();
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, StructInArray_MultipleBuffers) {
  auto* src = R"(
struct S {
  f : f32;
};

[[group(0), binding(0)]]
var<uniform> u0 : S;

[[group(0), binding(1)]]
var<uniform> u1 : S;

[[stage(fragment)]]
fn main() {
  let f0 = u0.f;
  let f1 = u1.f;
  let a = array<S, 4>();
}
)";
  auto* expect = R"(
struct S {
  f : f32;
};

[[internal(spirv_block)]]
struct S_block {
  inner : S;
};

[[group(0), binding(0)]] var<uniform> u0 : S_block;

[[group(0), binding(1)]] var<uniform> u1 : S_block;

[[stage(fragment)]]
fn main() {
  let f0 = u0.inner.f;
  let f1 = u1.inner.f;
  let a = array<S, 4>();
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(AddSpirvBlockDecorationTest, Aliases_Nested_OuterBuffer_InnerBuffer) {
  auto* src = R"(
struct Inner {
  f : f32;
};

type MyInner = Inner;

struct Outer {
  i : MyInner;
};

type MyOuter = Outer;

[[group(0), binding(0)]]
var<uniform> u0 : MyOuter;

[[group(0), binding(1)]]
var<uniform> u1 : MyInner;

[[stage(fragment)]]
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.f;
}
)";
  auto* expect = R"(
struct Inner {
  f : f32;
};

[[internal(spirv_block)]]
struct Inner_block {
  inner : Inner;
};

type MyInner = Inner;

[[internal(spirv_block)]]
struct Outer {
  i : MyInner;
};

type MyOuter = Outer;

[[group(0), binding(0)]] var<uniform> u0 : MyOuter;

[[group(0), binding(1)]] var<uniform> u1 : Inner_block;

[[stage(fragment)]]
fn main() {
  let f0 = u0.i.f;
  let f1 = u1.inner.f;
}
)";

  auto got = Run<AddSpirvBlockDecoration>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
