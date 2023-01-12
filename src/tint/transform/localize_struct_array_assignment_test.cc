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

#include "src/tint/transform/localize_struct_array_assignment.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/unshadow.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using LocalizeStructArrayAssignmentTest = TransformTest;

TEST_F(LocalizeStructArrayAssignmentTest, EmptyModule) {
    auto* src = R"()";
    EXPECT_FALSE(ShouldRun<LocalizeStructArrayAssignment>(src));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArray_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i] = v;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  a1 : array<InnerS, 8>,
};

struct InnerS {
  v : i32,
};

struct Uniforms {
  i : u32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  a1 : array<InnerS, 8>,
}

struct InnerS {
  v : i32,
}

struct Uniforms {
  i : u32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructStructArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  a : array<InnerS, 8>,
};

struct OuterS {
  s2 : S1,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.s2.a[uniforms.i] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  a : array<InnerS, 8>,
}

struct OuterS {
  s2 : S1,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.s2.a);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructStructArray_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.s2.a[uniforms.i] = v;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  s2 : S1,
};

struct S1 {
  a : array<InnerS, 8>,
};

struct InnerS {
  v : i32,
};

struct Uniforms {
  i : u32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.s2.a);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct OuterS {
  s2 : S1,
}

struct S1 {
  a : array<InnerS, 8>,
}

struct InnerS {
  v : i32,
}

struct Uniforms {
  i : u32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArrayArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
  j : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<array<InnerS, 8>, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i][uniforms.j] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
  j : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<array<InnerS, 8>, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i][uniforms.j] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArrayStruct) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  s2 : InnerS,
};

struct OuterS {
  a1 : array<S1, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  s1.a1[uniforms.i].s2 = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  s2 : InnerS,
}

struct OuterS {
  a1 : array<S1, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i].s2 = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, StructArrayStructArray) {
    auto* src = R"(
struct Uniforms {
  i : u32,
  j : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  a2 : array<InnerS, 8>,
};

struct OuterS {
  a1 : array<S1, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  s.a1[uniforms.i].a2[uniforms.j] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
  j : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  a2 : array<InnerS, 8>,
}

struct OuterS {
  a1 : array<S1, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  {
    let tint_symbol = &(s.a1);
    var tint_symbol_1 = *(tint_symbol);
    let tint_symbol_2 = &(tint_symbol_1[uniforms.i].a2);
    var tint_symbol_3 = *(tint_symbol_2);
    tint_symbol_3[uniforms.j] = v;
    *(tint_symbol_2) = tint_symbol_3;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, IndexingWithSideEffectFunc) {
    auto* src = R"(
struct Uniforms {
  i : u32,
  j : u32,
};

struct InnerS {
  v : i32,
};

struct S1 {
  a2 : array<InnerS, 8>,
};

struct OuterS {
  a1 : array<S1, 8>,
};

var<private> nextIndex : u32;
fn getNextIndex() -> u32 {
  nextIndex = nextIndex + 1u;
  return nextIndex;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  s.a1[getNextIndex()].a2[uniforms.j] = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
  j : u32,
}

struct InnerS {
  v : i32,
}

struct S1 {
  a2 : array<InnerS, 8>,
}

struct OuterS {
  a1 : array<S1, 8>,
}

var<private> nextIndex : u32;

fn getNextIndex() -> u32 {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  {
    let tint_symbol = &(s.a1);
    var tint_symbol_1 = *(tint_symbol);
    let tint_symbol_2 = &(tint_symbol_1[getNextIndex()].a2);
    var tint_symbol_3 = *(tint_symbol_2);
    tint_symbol_3[uniforms.j] = v;
    *(tint_symbol_2) = tint_symbol_3;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, IndexingWithSideEffectFunc_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  s.a1[getNextIndex()].a2[uniforms.j] = v;
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
  j : u32,
};

var<private> nextIndex : u32;
fn getNextIndex() -> u32 {
  nextIndex = nextIndex + 1u;
  return nextIndex;
}

struct OuterS {
  a1 : array<S1, 8>,
};

struct S1 {
  a2 : array<InnerS, 8>,
};

struct InnerS {
  v : i32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s : OuterS;
  {
    let tint_symbol = &(s.a1);
    var tint_symbol_1 = *(tint_symbol);
    let tint_symbol_2 = &(tint_symbol_1[getNextIndex()].a2);
    var tint_symbol_3 = *(tint_symbol_2);
    tint_symbol_3[uniforms.j] = v;
    *(tint_symbol_2) = tint_symbol_3;
    *(tint_symbol) = tint_symbol_1;
  }
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
  j : u32,
}

var<private> nextIndex : u32;

fn getNextIndex() -> u32 {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

struct OuterS {
  a1 : array<S1, 8>,
}

struct S1 {
  a2 : array<InnerS, 8>,
}

struct InnerS {
  v : i32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, ViaPointerArg) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};
struct InnerS {
  v : i32,
};
struct OuterS {
  a1 : array<InnerS, 8>,
};
@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  (*p).a1[uniforms.i] = v;
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&s1);
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  {
    let tint_symbol = &((*(p)).a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&(s1));
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, ViaPointerArg_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&s1);
}

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  (*p).a1[uniforms.i] = v;
}

struct InnerS {
  v : i32,
};
struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
};
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  f(&(s1));
}

fn f(p : ptr<function, OuterS>) {
  var v : InnerS;
  {
    let tint_symbol = &((*(p)).a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[uniforms.i] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

struct Uniforms {
  i : u32,
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, ViaPointerVar) {
    auto* src = R"(
struct Uniforms {
  i : u32,
};

struct InnerS {
  v : i32,
};

struct OuterS {
  a1 : array<InnerS, 8>,
};

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, InnerS>, v : InnerS) {
  *(p) = v;
}

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  let p = &(s1.a1[uniforms.i]);
  *(p) = v;
}
)";

    auto* expect = R"(
struct Uniforms {
  i : u32,
}

struct InnerS {
  v : i32,
}

struct OuterS {
  a1 : array<InnerS, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(p : ptr<function, InnerS>, v : InnerS) {
  *(p) = v;
}

@compute @workgroup_size(1)
fn main() {
  var v : InnerS;
  var s1 : OuterS;
  let p_save = uniforms.i;
  {
    let tint_symbol = &(s1.a1);
    var tint_symbol_1 = *(tint_symbol);
    tint_symbol_1[p_save] = v;
    *(tint_symbol) = tint_symbol_1;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, LocalizeStructArrayAssignment>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(LocalizeStructArrayAssignmentTest, VectorAssignment) {
    auto* src = R"(
struct Uniforms {
  i : u32,
}

struct OuterS {
  a1 : array<u32, 8>,
}

@group(1) @binding(4) var<uniform> uniforms : Uniforms;

fn f(i : u32) -> u32 {
  return (i + 1u);
}

@compute @workgroup_size(1)
fn main() {
  var s1 : OuterS;
  var v : vec3<f32>;
  v[s1.a1[uniforms.i]] = 1.0;
  v[f(s1.a1[uniforms.i])] = 1.0;
}
)";

    // Transform does nothing here as we're not actually assigning to the array in
    // the struct.
    EXPECT_FALSE(ShouldRun<LocalizeStructArrayAssignment>(src));
}

}  // namespace
}  // namespace tint::transform
