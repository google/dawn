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

#include "src/transform/promote_side_effects_to_decl.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using PromoteSideEffectsToDeclTest = TransformTest;

TEST_F(PromoteSideEffectsToDeclTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_BasicArray) {
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_BasicStruct) {
  auto* src = R"(
struct S {
  a : i32;
  b : f32;
  c : vec3<f32>;
};

fn f() {
  var x = S(1, 2.0, vec3<f32>()).b;
}
)";

  auto* expect = R"(
struct S {
  a : i32;
  b : f32;
  c : vec3<f32>;
}

fn f() {
  let tint_symbol = S(1, 2.0, vec3<f32>());
  var x = tint_symbol.b;
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_ArrayInForLoopInit) {
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_StructInForLoopInit) {
  auto* src = R"(
struct S {
  a : i32;
  b : f32;
  c : vec3<f32>;
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
  a : i32;
  b : f32;
  c : vec3<f32>;
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_ArrayInForLoopCond) {
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
    var marker = 1;

    continuing {
      f = (f + 1.0);
    }
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_ArrayInForLoopCont) {
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
    var marker = 1;

    continuing {
      let tint_symbol = array<f32, 1u>(1.0);
      f = (f + tint_symbol[0]);
    }
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_ArrayInForLoopInitCondCont) {
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
      var marker = 1;

      continuing {
        let tint_symbol_2 = array<f32, 1u>(2.0);
        f = (f + tint_symbol_2[0]);
      }
    }
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_ArrayInElseIf) {
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_ArrayInElseIfChain) {
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_ArrayInArrayArray) {
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_StructNested) {
  auto* src = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : i32;
  b : S1;
  c : i32;
};

struct S3 {
  a : S2;
};

fn f() {
  var x = S3(S2(1, S1(2), 3)).a.b.a;
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
}

struct S2 {
  a : i32;
  b : S1;
  c : i32;
}

struct S3 {
  a : S2;
}

fn f() {
  let tint_symbol = S1(2);
  let tint_symbol_1 = S2(1, tint_symbol, 3);
  let tint_symbol_2 = S3(tint_symbol_1);
  var x = tint_symbol_2.a.b.a;
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_Mixed) {
  auto* src = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : array<S1, 3u>;
};

fn f() {
  var x = S2(array<S1, 3u>(S1(1), S1(2), S1(3))).a[1].a;
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
}

struct S2 {
  a : array<S1, 3u>;
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtorToLet_NoChangeOnVarDecl) {
  auto* src = R"(
struct S {
  a : i32;
  b : f32;
  c : i32;
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ true,
                                             /* dynamic_index_to_var */ false);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_ArrayIndexDynamic) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<i32, 4>(1, 2, 3, 4);
  let x = p[i];
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = array<i32, 4>(1, 2, 3, 4);
  var var_for_index = p;
  let x = var_for_index[i];
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_MatrixIndexDynamic) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[i];
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  var var_for_index = p;
  let x = var_for_index[i];
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_ArrayIndexDynamicChain) {
  auto* src = R"(
fn f() {
  var i : i32;
  var j : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  let x = p[i][j];
}
)";

  // TODO(bclayton): Optimize this case:
  // This output is not as efficient as it could be.
  // We only actually need to hoist the inner-most array to a `var`
  // (`var_for_index`), as later indexing operations will be working with
  // references, not values.
  auto* expect = R"(
fn f() {
  var i : i32;
  var j : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  var var_for_index = p;
  var var_for_index_1 = var_for_index[i];
  let x = var_for_index_1[j];
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       DynamicIndexToVar_ArrayIndexInForLoopInit) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  for(let x = p[i]; ; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  var var_for_index = p;
  for(let x = var_for_index[i]; ; ) {
    break;
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       DynamicIndexToVar_MatrixIndexInForLoopInit) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  for(let x = p[i]; ; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  var var_for_index = p;
  for(let x = var_for_index[i]; ; ) {
    break;
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       DynamicIndexToVar_ArrayIndexInForLoopCond) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<i32, 2>(1, 2);
  for(; p[i] < 3; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = array<i32, 2>(1, 2);
  loop {
    var var_for_index = p;
    if (!((var_for_index[i] < 3))) {
      break;
    }
    break;
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       DynamicIndexToVar_MatrixIndexInForLoopCond) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  for(; p[i].x < 3.0; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  loop {
    var var_for_index = p;
    if (!((var_for_index[i].x < 3.0))) {
      break;
    }
    break;
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_ArrayIndexInElseIf) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<i32, 2>(1, 2);
  if (false) {
    var marker = 0;
  } else if (p[i] < 3) {
    var marker = 1;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = array<i32, 2>(1, 2);
  if (false) {
    var marker = 0;
  } else {
    var var_for_index = p;
    if ((var_for_index[i] < 3)) {
      var marker = 1;
    }
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       DynamicIndexToVar_ArrayIndexInElseIfChain) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<i32, 2>(1, 2);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (p[i] < 3) {
    var marker = 2;
  } else if (p[i] < 4) {
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
  var i : i32;
  let p = array<i32, 2>(1, 2);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    var var_for_index = p;
    if ((var_for_index[i] < 3)) {
      var marker = 2;
    } else {
      var var_for_index_1 = p;
      if ((var_for_index_1[i] < 4)) {
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_MatrixIndexInElseIf) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  if (false) {
    var marker_if = 1;
  } else if (p[i].x < 3.0) {
    var marker_else_if = 1;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  if (false) {
    var marker_if = 1;
  } else {
    var var_for_index = p;
    if ((var_for_index[i].x < 3.0)) {
      var marker_else_if = 1;
    }
  }
}
)";

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       DynamicIndexToVar_MatrixIndexInElseIfChain) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (p[i].x < 3.0) {
    var marker = 2;
  } else if (p[i].y < 3.0) {
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
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    var var_for_index = p;
    if ((var_for_index[i].x < 3.0)) {
      var marker = 2;
    } else {
      var var_for_index_1 = p;
      if ((var_for_index_1[i].y < 3.0)) {
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
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_ArrayIndexLiteral) {
  auto* src = R"(
fn f() {
  let p = array<i32, 4>(1, 2, 3, 4);
  let x = p[1];
}
)";

  auto* expect = src;

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_MatrixIndexLiteral) {
  auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[1];
}
)";

  auto* expect = src;

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_ArrayIndexConstantLet) {
  auto* src = R"(
fn f() {
  let p = array<i32, 4>(1, 2, 3, 4);
  let c = 1;
  let x = p[c];
}
)";

  auto* expect = src;

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_MatrixIndexConstantLet) {
  auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let c = 1;
  let x = p[c];
}
)";

  auto* expect = src;

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, DynamicIndexToVar_ArrayIndexLiteralChain) {
  auto* src = R"(
fn f() {
  let a = array<i32, 2>(1, 2);
  let b = array<i32, 2>(3, 4);
  let p = array<array<i32, 2>, 2>(a, b);
  let x = p[0][1];
}
)";

  auto* expect = src;

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       DynamicIndexToVar_MatrixIndexLiteralChain) {
  auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[0][1];
}
)";

  auto* expect = src;

  DataMap data;
  data.Add<PromoteSideEffectsToDecl::Config>(/* type_ctor_to_let */ false,
                                             /* dynamic_index_to_var */ true);
  auto got = Run<PromoteSideEffectsToDecl>(src, data);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
