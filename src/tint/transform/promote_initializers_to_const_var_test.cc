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

#include "src/tint/transform/promote_initializers_to_const_var.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using PromoteInitializersToConstVarTest = TransformTest;

TEST_F(PromoteInitializersToConstVarTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, BasicArray) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, BasicStruct) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, BasicStruct_OutOfOrder) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInForLoopInit) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, StructInForLoopInit) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, StructInForLoopInit_OutOfOrder) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInForLoopCond) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInForLoopCont) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInForLoopInitCondCont) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInElseIf) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInElseIfChain) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInArrayArray) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, StructNested) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, Mixed) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, Mixed_OutOfOrder) {
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
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, NoChangeOnVarDecl) {
    auto* src = R"(
struct S {
  a : i32,
  b : f32,
  c : i32,
}

fn f() {
  var local_arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var local_str = S(1, 2.0, 3);
}

let module_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

let module_str : S = S(1, 2.0, 3);
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, NoChangeOnVarDecl_OutOfOrder) {
    auto* src = R"(
fn f() {
  var local_arr = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var local_str = S(1, 2.0, 3);
}

let module_str : S = S(1, 2.0, 3);

struct S {
  a : i32,
  b : f32,
  c : i32,
}

let module_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteInitializersToConstVar>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
