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

#include "src/tint/transform/promote_initializers_to_let.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using PromoteInitializersToLetTest = TransformTest;

TEST_F(PromoteInitializersToLetTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, BasicArray) {
    auto* src = R"(
fn f() {
  var f0 = 1.0;
  var f1 = 2.0;
  var f2 = 3.0;
  var f3 = 4.0;
  var i = array<f32, 4u>(f0, f1, f2, f3)[2];
}
)";

    auto* expect = R"(
fn f() {
  var f0 = 1.0;
  var f1 = 2.0;
  var f2 = 3.0;
  var f3 = 4.0;
  let tint_symbol = array<f32, 4u>(f0, f1, f2, f3);
  var i = tint_symbol[2];
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, BasicStruct) {
    auto* src = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};

fn f() {
  var x = S(1, 2.0, vec3<f32>()).b;
}
)";

    auto* expect = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}

fn f() {
  let tint_symbol = S(1, 2.0, vec3<f32>());
  var x = tint_symbol.b;
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, BasicStruct_OutOfOrder) {
    auto* src = R"(
fn f() {
  var x = S(1, 2.0, vec3<f32>()).b;
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};
)";

    auto* expect = R"(
fn f() {
  let tint_symbol = S(1, 2.0, vec3<f32>());
  var x = tint_symbol.b;
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstBasicArray) {
    auto* src = R"(
const f0 = 1.0;

const f1 = 2.0;

const C = array<f32, 2u>(f0, f1);

fn f() {
  var f0 = 100.0;
  var f1 = 100.0;
  var i = C[1];
}
)";

    auto* expect = R"(
const f0 = 1.0;

const f1 = 2.0;

const C = array<f32, 2u>(f0, f1);

fn f() {
  var f0 = 100.0;
  var f1 = 100.0;
  let tint_symbol = C;
  var i = tint_symbol[1];
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayDynamicIndex) {
    auto* src = R"(
const TRI_VERTICES = array(
  vec4(0., 0., 0., 1.),
  vec4(0., 1., 0., 1.),
  vec4(1., 1., 0., 1.),
);

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
  // note: TRI_VERTICES requires a materialize before the dynamic index.
  return TRI_VERTICES[in_vertex_index];
}
)";

    auto* expect = R"(
const TRI_VERTICES = array(vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), vec4(1.0, 1.0, 0.0, 1.0));

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index : u32) -> @builtin(position) vec4<f32> {
  let tint_symbol = TRI_VERTICES;
  return tint_symbol[in_vertex_index];
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstBasicArray_OutOfOrder) {
    auto* src = R"(
fn f() {
  var f0 = 100.0;
  var f1 = 100.0;
  var i = C[1];
}

const C = array<f32, 2u>(f0, f1);

const f0 = 1.0;

const f1 = 2.0;
)";

    auto* expect = R"(
fn f() {
  var f0 = 100.0;
  var f1 = 100.0;
  let tint_symbol = C;
  var i = tint_symbol[1];
}

const C = array<f32, 2u>(f0, f1);

const f0 = 1.0;

const f1 = 2.0;
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstBasicArray) {
    auto* src = R"(
fn f() {
  const f0 = 1.0;
  const f1 = 2.0;
  const C = array<f32, 2u>(f0, f1);
  var i = C[1];
}
)";

    auto* expect = R"(
fn f() {
  const f0 = 1.0;
  const f1 = 2.0;
  const C = array<f32, 2u>(f0, f1);
  let tint_symbol = C;
  var i = tint_symbol[1];
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopInit) {
    auto* src = R"(
fn f() {
  var insert_after = 1;
  for(var i = array<f32, 4u>(0.0, 1.0, 2.0, 3.0)[2]; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var insert_after = 1;
  let tint_symbol = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  for(var i = tint_symbol[2]; ; ) {
    break;
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopInit) {
    auto* src = R"(
fn f() {
  const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var insert_after = 1;
  for(var i = arr[2]; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var insert_after = 1;
  let tint_symbol = arr;
  for(var i = tint_symbol[2]; ; ) {
    break;
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInForLoopInit) {
    auto* src = R"(
const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

fn f() {
  var insert_after = 1;
  for(var i = arr[2]; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
const arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

fn f() {
  var insert_after = 1;
  let tint_symbol = arr;
  for(var i = tint_symbol[2]; ; ) {
    break;
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, StructInForLoopInit) {
    auto* src = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};

fn f() {
  var insert_after = 1;
  for(var x = S(1, 2.0, vec3<f32>()).b; ; ) {
    break;
  }
}
)";

    auto* expect = R"(
struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}

fn f() {
  var insert_after = 1;
  let tint_symbol = S(1, 2.0, vec3<f32>());
  for(var x = tint_symbol.b; ; ) {
    break;
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, StructInForLoopInit_OutOfOrder) {
    auto* src = R"(
fn f() {
  var insert_after = 1;
  for(var x = S(1, 2.0, vec3<f32>()).b; ; ) {
    break;
  }
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
};
)";

    auto* expect = R"(
fn f() {
  var insert_after = 1;
  let tint_symbol = S(1, 2.0, vec3<f32>());
  for(var x = tint_symbol.b; ; ) {
    break;
  }
}

struct S {
  a : i32,
  b : f32,
  c : vec3<f32>,
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopCond) {
    auto* src = R"(
fn f() {
  var f = 1.0;
  for(; f == array<f32, 1u>(f)[0]; f = f + 1.0) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  var f = 1.0;
  loop {
    let tint_symbol = array<f32, 1u>(f);
    if (!((f == tint_symbol[0]))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      f = (f + 1.0);
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopCond) {
    auto* src = R"(
fn f() {
  const f = 1.0;
  const arr = array<f32, 1u>(f);
  for(var i = f; i == arr[0]; i = i + 1.0) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  const f = 1.0;
  const arr = array<f32, 1u>(f);
  {
    var i = f;
    loop {
      let tint_symbol = arr;
      if (!((i == tint_symbol[0]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        i = (i + 1.0);
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInForLoopCond) {
    auto* src = R"(
const f = 1.0;

const arr = array<f32, 1u>(f);

fn F() {
  for(var i = f; i == arr[0]; i = i + 1.0) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
const f = 1.0;

const arr = array<f32, 1u>(f);

fn F() {
  {
    var i = f;
    loop {
      let tint_symbol = arr;
      if (!((i == tint_symbol[0]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        i = (i + 1.0);
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopCont) {
    auto* src = R"(
fn f() {
  var f = 0.0;
  for(; f < 10.0; f = f + array<f32, 1u>(1.0)[0]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  var f = 0.0;
  loop {
    if (!((f < 10.0))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      let tint_symbol = array<f32, 1u>(1.0);
      f = (f + tint_symbol[0]);
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopCont) {
    auto* src = R"(
fn f() {
  const arr = array<f32, 1u>(1.0);
  var f = 0.0;
  for(; f < 10.0; f = f + arr[0]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  const arr = array<f32, 1u>(1.0);
  var f = 0.0;
  loop {
    if (!((f < 10.0))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      let tint_symbol = arr;
      f = (f + tint_symbol[0]);
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInForLoopCont) {
    auto* src = R"(
const arr = array<f32, 1u>(1.0);

fn f() {
  var f = 0.0;
  for(; f < 10.0; f = f + arr[0]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
const arr = array<f32, 1u>(1.0);

fn f() {
  var f = 0.0;
  loop {
    if (!((f < 10.0))) {
      break;
    }
    {
      var marker = 1;
    }

    continuing {
      let tint_symbol = arr;
      f = (f + tint_symbol[0]);
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInForLoopInitCondCont) {
    auto* src = R"(
fn f() {
  for(var f = array<f32, 1u>(0.0)[0];
      f < array<f32, 1u>(1.0)[0];
      f = f + array<f32, 1u>(2.0)[0]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  let tint_symbol = array<f32, 1u>(0.0);
  {
    var f = tint_symbol[0];
    loop {
      let tint_symbol_1 = array<f32, 1u>(1.0);
      if (!((f < tint_symbol_1[0]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        let tint_symbol_2 = array<f32, 1u>(2.0);
        f = (f + tint_symbol_2[0]);
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInForLoopInitCondCont) {
    auto* src = R"(
fn f() {
  const arr_a = array<f32, 1u>(0.0);
  const arr_b = array<f32, 1u>(1.0);
  const arr_c = array<f32, 1u>(2.0);
  for(var f = arr_a[0]; f < arr_b[0]; f = f + arr_c[0]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  const arr_a = array<f32, 1u>(0.0);
  const arr_b = array<f32, 1u>(1.0);
  const arr_c = array<f32, 1u>(2.0);
  let tint_symbol = arr_a;
  {
    var f = tint_symbol[0];
    loop {
      let tint_symbol_1 = arr_b;
      if (!((f < tint_symbol_1[0]))) {
        break;
      }
      {
        var marker = 1;
      }

      continuing {
        let tint_symbol_2 = arr_c;
        f = (f + tint_symbol_2[0]);
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInElseIf) {
    auto* src = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else if (f == array<f32, 2u>(f, f)[0]) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else {
    let tint_symbol = array<f32, 2u>(f, f);
    if ((f == tint_symbol[0])) {
      var marker = 1;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInElseIfChain) {
    auto* src = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (f == array<f32, 2u>(f, f)[0]) {
    var marker = 2;
  } else if (f == array<f32, 2u>(f, f)[1]) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
fn f() {
  var f = 1.0;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    let tint_symbol = array<f32, 2u>(f, f);
    if ((f == tint_symbol[0])) {
      var marker = 2;
    } else {
      let tint_symbol_1 = array<f32, 2u>(f, f);
      if ((f == tint_symbol_1[1])) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInElseIfChain) {
    auto* src = R"(
fn f() {
  const f = 1.0;
  const arr = array<f32, 2u>(f, f);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (f == arr[0]) {
    var marker = 2;
  } else if (f == arr[1]) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
fn f() {
  const f = 1.0;
  const arr = array<f32, 2u>(f, f);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    let tint_symbol = arr;
    if ((f == tint_symbol[0])) {
      var marker = 2;
    } else {
      let tint_symbol_1 = arr;
      if ((f == tint_symbol_1[1])) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInElseIfChain) {
    auto* src = R"(
const f = 1.0;

const arr = array<f32, 2u>(f, f);

fn F() {
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (f == arr[0]) {
    var marker = 2;
  } else if (f == arr[1]) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
const f = 1.0;

const arr = array<f32, 2u>(f, f);

fn F() {
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    let tint_symbol = arr;
    if ((f == tint_symbol[0])) {
      var marker = 2;
    } else {
      let tint_symbol_1 = arr;
      if ((f == tint_symbol_1[1])) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ArrayInArrayArray) {
    auto* src = R"(
fn f() {
  var i = array<array<f32, 2u>, 2u>(array<f32, 2u>(1.0, 2.0), array<f32, 2u>(3.0, 4.0))[0][1];
}
)";

    auto* expect = R"(
fn f() {
  let tint_symbol = array<f32, 2u>(1.0, 2.0);
  let tint_symbol_1 = array<f32, 2u>(3.0, 4.0);
  let tint_symbol_2 = array<array<f32, 2u>, 2u>(tint_symbol, tint_symbol_1);
  var i = tint_symbol_2[0][1];
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, LocalConstArrayInArrayArray) {
    auto* src = R"(
fn f() {
  const arr_0 = array<f32, 2u>(1.0, 2.0);
  const arr_1 = array<f32, 2u>(3.0, 4.0);
  const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);
  var i = arr_2[0][1];
}
)";

    auto* expect = R"(
fn f() {
  const arr_0 = array<f32, 2u>(1.0, 2.0);
  const arr_1 = array<f32, 2u>(3.0, 4.0);
  const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);
  let tint_symbol = arr_2;
  var i = tint_symbol[0][1];
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, GlobalConstArrayInArrayArray) {
    auto* src = R"(
const arr_0 = array<f32, 2u>(1.0, 2.0);

const arr_1 = array<f32, 2u>(3.0, 4.0);

const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);

fn f() {
  var i = arr_2[0][1];
}
)";

    auto* expect = R"(
const arr_0 = array<f32, 2u>(1.0, 2.0);

const arr_1 = array<f32, 2u>(3.0, 4.0);

const arr_2 = array<array<f32, 2u>, 2u>(arr_0, arr_1);

fn f() {
  let tint_symbol = arr_2;
  var i = tint_symbol[0][1];
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, StructNested) {
    auto* src = R"(
struct S1 {
  a : i32,
};

struct S2 {
  a : i32,
  b : S1,
  c : i32,
};

struct S3 {
  a : S2,
};

fn f() {
  var x = S3(S2(1, S1(2), 3)).a.b.a;
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
}

struct S2 {
  a : i32,
  b : S1,
  c : i32,
}

struct S3 {
  a : S2,
}

fn f() {
  let tint_symbol = S1(2);
  let tint_symbol_1 = S2(1, tint_symbol, 3);
  let tint_symbol_2 = S3(tint_symbol_1);
  var x = tint_symbol_2.a.b.a;
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, Mixed) {
    auto* src = R"(
struct S1 {
  a : i32,
};

struct S2 {
  a : array<S1, 3u>,
};

fn f() {
  var x = S2(array<S1, 3u>(S1(1), S1(2), S1(3))).a[1].a;
}
)";

    auto* expect = R"(
struct S1 {
  a : i32,
}

struct S2 {
  a : array<S1, 3u>,
}

fn f() {
  let tint_symbol = S1(1);
  let tint_symbol_1 = S1(2);
  let tint_symbol_2 = S1(3);
  let tint_symbol_3 = array<S1, 3u>(tint_symbol, tint_symbol_1, tint_symbol_2);
  let tint_symbol_4 = S2(tint_symbol_3);
  var x = tint_symbol_4.a[1].a;
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, Mixed_OutOfOrder) {
    auto* src = R"(
fn f() {
  var x = S2(array<S1, 3u>(S1(1), S1(2), S1(3))).a[1].a;
}

struct S2 {
  a : array<S1, 3u>,
};

struct S1 {
  a : i32,
};
)";

    auto* expect = R"(
fn f() {
  let tint_symbol = S1(1);
  let tint_symbol_1 = S1(2);
  let tint_symbol_2 = S1(3);
  let tint_symbol_3 = array<S1, 3u>(tint_symbol, tint_symbol_1, tint_symbol_2);
  let tint_symbol_4 = S2(tint_symbol_3);
  var x = tint_symbol_4.a[1].a;
}

struct S2 {
  a : array<S1, 3u>,
}

struct S1 {
  a : i32,
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, NoChangeOnVarDecl) {
    auto* src = R"(
type F = f32;

fn f() {
  var local_arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var local_str = F(3.0);
}

const module_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

const module_str : F = F(2.0);
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, NoChangeOnVarDecl_OutOfOrder) {
    auto* src = R"(
fn f() {
  var local_arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var local_str = F(3.0);
}

const module_str : F = F(2.0);

type F = f32;

const module_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToLetTest, ForLoopShadowing) {
    auto* src = R"(
fn X() {
  var i = 10;
  for(var f = 0; f < 10; f = f + array<i32, 1u>(i)[0]) {
      var i = 20;
  }
}

fn Y() {
  var i = 10;
  for(var f = 0; f < array<i32, 1u>(i)[0]; f = f + 1) {
      var i = 20;
  }
}

fn Z() {
  var i = 10;
  for(var f = array<i32, 1u>(i)[0]; f < 10; f = f + 1) {
      var i = 20;
  }
}
)";

    auto* expect = R"(
fn X() {
  var i = 10;
  {
    var f = 0;
    loop {
      if (!((f < 10))) {
        break;
      }
      {
        var i = 20;
      }

      continuing {
        let tint_symbol = array<i32, 1u>(i);
        f = (f + tint_symbol[0]);
      }
    }
  }
}

fn Y() {
  var i = 10;
  {
    var f = 0;
    loop {
      let tint_symbol_1 = array<i32, 1u>(i);
      if (!((f < tint_symbol_1[0]))) {
        break;
      }
      {
        var i = 20;
      }

      continuing {
        f = (f + 1);
      }
    }
  }
}

fn Z() {
  var i = 10;
  let tint_symbol_2 = array<i32, 1u>(i);
  for(var f = tint_symbol_2[0]; (f < 10); f = (f + 1)) {
    var i = 20;
  }
}
)";

    DataMap data;
    auto got = Run<PromoteInitializersToLet>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
