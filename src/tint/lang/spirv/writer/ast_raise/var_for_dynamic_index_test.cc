// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/ast_raise/var_for_dynamic_index.h"
#include "src/tint/lang/spirv/writer/ast_raise/for_loop_to_loop.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::spirv::writer {
namespace {

using VarForDynamicIndexTest = ast::transform::TransformTest;

TEST_F(VarForDynamicIndexTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexDynamic) {
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
  var var_for_index : array<i32, 4u> = p;
  let x = var_for_index[i];
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexDynamic) {
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
  var var_for_index : mat2x2<f32> = p;
  let x = var_for_index[i];
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexDynamicChain) {
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
  var var_for_index : array<array<i32, 2u>, 2u> = p;
  var var_for_index_1 : array<i32, 2u> = var_for_index[i];
  let x = var_for_index_1[j];
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexInForLoopInit) {
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
  {
    var var_for_index : array<array<i32, 2u>, 2u> = p;
    let x = var_for_index[i];
    loop {
      {
        break;
      }
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexInForLoopInit) {
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
  {
    var var_for_index : mat2x2<f32> = p;
    let x = var_for_index[i];
    loop {
      {
        break;
      }
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexInForLoopCond) {
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
    var var_for_index : array<i32, 2u> = p;
    if (!((var_for_index[i] < 3))) {
      break;
    }
    {
      break;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexInForLoopCond) {
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
    var var_for_index : mat2x2<f32> = p;
    if (!((var_for_index[i].x < 3.0))) {
      break;
    }
    {
      break;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexInForLoopCondWithNestedIndex) {
    auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  for(; p[i].x < 3.0; ) {
    if (p[i].x < 1.0) {
        var marker = 1;
    }
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  loop {
    var var_for_index : mat2x2<f32> = p;
    if (!((var_for_index[i].x < 3.0))) {
      break;
    }
    {
      var var_for_index_1 : mat2x2<f32> = p;
      if ((var_for_index_1[i].x < 1.0)) {
        var marker = 1;
      }
      break;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexInElseIf) {
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
    var var_for_index : array<i32, 2u> = p;
    if ((var_for_index[i] < 3)) {
      var marker = 1;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexInElseIfChain) {
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
    var var_for_index : array<i32, 2u> = p;
    if ((var_for_index[i] < 3)) {
      var marker = 2;
    } else {
      var var_for_index_1 : array<i32, 2u> = p;
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

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexInElseIf) {
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
    var var_for_index : mat2x2<f32> = p;
    if ((var_for_index[i].x < 3.0)) {
      var marker_else_if = 1;
    }
  }
}
)";

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexInElseIfChain) {
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
    var var_for_index : mat2x2<f32> = p;
    if ((var_for_index[i].x < 3.0)) {
      var marker = 2;
    } else {
      var var_for_index_1 : mat2x2<f32> = p;
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

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexLiteral) {
    auto* src = R"(
fn f() {
  let p = array<i32, 4>(1, 2, 3, 4);
  let x = p[1];
}
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexLiteral) {
    auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[1];
}
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexConstantLet) {
    auto* src = R"(
fn f() {
  let p = array<i32, 4>(1, 2, 3, 4);
  let c = 1;
  var var_for_index = p;
  let x = var_for_index[c];
}
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexConstantLet) {
    auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let c = 1;
  var var_for_index = p;
  let x = var_for_index[c];
}
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexLiteralChain) {
    auto* src = R"(
fn f() {
  let a = array<i32, 2>(1, 2);
  let b = array<i32, 2>(3, 4);
  let p = array<array<i32, 2>, 2>(a, b);
  let x = p[0][1];
}
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexLiteralChain) {
    auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[0][1];
}
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ShortCircuitedArrayAccess) {
    auto* src = R"(
const foo = (false && (array<f32, 4>()[0] == 0));
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ShortCircuitedMatrixAccess) {
    auto* src = R"(
const foo = (false && (mat4x4<f32>()[0][0] == 0));
)";

    auto* expect = src;

    ast::transform::DataMap data;
    auto got = Run<VarForDynamicIndex>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::spirv::writer
