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

#include "src/tint/transform/merge_return.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using MergeReturnTest = TransformTest;

TEST_F(MergeReturnTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<MergeReturn>(src));
}

TEST_F(MergeReturnTest, ShouldRunOnlyEntryPoints) {
    auto* src = R"(
@fragment
fn foo() {}

@compute @workgroup_size(1)
fn bar() {}
)";

    EXPECT_FALSE(ShouldRun<MergeReturn>(src));
}

TEST_F(MergeReturnTest, ShouldRunUserFuncNoReturn) {
    auto* src = R"(
fn foo() {}

@compute @workgroup_size(1)
fn bar() {
  foo();
}
)";

    EXPECT_FALSE(ShouldRun<MergeReturn>(src));
}

TEST_F(MergeReturnTest, ShouldRunUserFuncSingleReturn) {
    auto* src = R"(
fn foo() -> i32 {
  return 42;
}

@compute @workgroup_size(1)
fn bar() {
  foo();
}
)";

    EXPECT_FALSE(ShouldRun<MergeReturn>(src));
}

TEST_F(MergeReturnTest, ShouldRunUserFuncMultipleReturn) {
    auto* src = R"(
fn foo(x : i32) -> i32 {
  if (x == 0) {
    return 42;
  }
  return 0;
}

@compute @workgroup_size(1)
fn bar() {
  foo(7);
}
)";

    EXPECT_TRUE(ShouldRun<MergeReturn>(src));
}

TEST_F(MergeReturnTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = src;

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, SingleExitPoint) {
    auto* src = R"(
fn foo(x : i32) -> i32 {
  var retval = 0;
  if ((x == 0)) {
    retval = 42;
  } else {
    retval = 7;
  }
  return retval;
}
)";

    auto* expect = src;

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, Basic) {
    auto* src = R"(
var<private> non_uniform_global : i32;

fn foo() {
  if (non_uniform_global == 0) {
    return;
  } else {
    return;
  }
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

fn foo() {
  var tint_return_flag : bool;
  if ((non_uniform_global == 0)) {
    {
      tint_return_flag = true;
    }
  } else {
    {
      tint_return_flag = true;
    }
  }
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, BasicRetval) {
    auto* src = R"(
var<private> non_uniform_global : i32;

fn foo() -> i32 {
  if (non_uniform_global == 0) {
    return 1;
  } else {
    return 2;
  }
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  if ((non_uniform_global == 0)) {
    {
      tint_return_flag = true;
      tint_return_value = 1;
    }
  } else {
    {
      tint_return_flag = true;
      tint_return_value = 2;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, CodeAfterIf) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  if (non_uniform_global == 0) {
    return 1;
  }
  bar = 42;
  return 2;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  if ((non_uniform_global == 0)) {
    {
      tint_return_flag = true;
      tint_return_value = 1;
    }
  }
  if (!(tint_return_flag)) {
    bar = 42;
    {
      tint_return_flag = true;
      tint_return_value = 2;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, SequenceOfConditionalReturns) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  if (non_uniform_global == 0) {
    return 1;
  }
  bar = 42;
  if (non_uniform_global == 1) {
    return 2;
  }
  bar = 43;
  if (non_uniform_global == 2) {
    return 3;
  }
  bar = 44;
  if (non_uniform_global == 3) {
    return 4;
  }
  bar = 45;
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  if ((non_uniform_global == 0)) {
    {
      tint_return_flag = true;
      tint_return_value = 1;
    }
  }
  if (!(tint_return_flag)) {
    bar = 42;
    if ((non_uniform_global == 1)) {
      {
        tint_return_flag = true;
        tint_return_value = 2;
      }
    }
    if (!(tint_return_flag)) {
      bar = 43;
      if ((non_uniform_global == 2)) {
        {
          tint_return_flag = true;
          tint_return_value = 3;
        }
      }
      if (!(tint_return_flag)) {
        bar = 44;
        if ((non_uniform_global == 3)) {
          {
            tint_return_flag = true;
            tint_return_value = 4;
          }
        }
        if (!(tint_return_flag)) {
          bar = 45;
          {
            tint_return_flag = true;
            tint_return_value = 0;
          }
        }
      }
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, NestedConditionals) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  if (non_uniform_global == 0) {
    if (true) {
      return 1;
    } else {
      return 2;
    }
  } else {
    if (true) {
      bar = 42;
    } else if (false) {
      return 3;
    } else if (true) {
      return 4;
    }
    return 5;
  }
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  if ((non_uniform_global == 0)) {
    if (true) {
      {
        tint_return_flag = true;
        tint_return_value = 1;
      }
    } else {
      {
        tint_return_flag = true;
        tint_return_value = 2;
      }
    }
  } else {
    if (true) {
      bar = 42;
    } else if (false) {
      {
        tint_return_flag = true;
        tint_return_value = 3;
      }
    } else if (true) {
      {
        tint_return_flag = true;
        tint_return_value = 4;
      }
    }
    if (!(tint_return_flag)) {
      {
        tint_return_flag = true;
        tint_return_value = 5;
      }
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, Loop) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var i = 0;
  loop {
    bar = i;
    if (non_uniform_global == 0) {
      return non_uniform_global;
    }
    if (i >= 10) {
      break;
    }
  }
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  var i = 0;
  loop {
    bar = i;
    if ((non_uniform_global == 0)) {
      {
        tint_return_flag = true;
        tint_return_value = non_uniform_global;
        break;
      }
    }
    if ((i >= 10)) {
      break;
    }
  }
  if (!(tint_return_flag)) {
    {
      tint_return_flag = true;
      tint_return_value = 0;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, NestedLoop) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var i = 0;
  loop {
    loop {
        bar = i;
        if (non_uniform_global == 0) {
          return non_uniform_global;
        }
        if (i >= 10) {
          break;
        }
    }
    bar = 10;
  }
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  var i = 0;
  loop {
    loop {
      bar = i;
      if ((non_uniform_global == 0)) {
        {
          tint_return_flag = true;
          tint_return_value = non_uniform_global;
          break;
        }
      }
      if ((i >= 10)) {
        break;
      }
    }
    if (tint_return_flag) {
      break;
    }
    bar = 10;
  }
  if (!(tint_return_flag)) {
    {
      tint_return_flag = true;
      tint_return_value = 0;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, NestedLoopAlwaysReturns) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var i = 0;
  loop {
    bar = i;
    loop {
      return non_uniform_global;
    }
    if (i == 10) {
      break;
    }
  }
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  var i = 0;
  loop {
    bar = i;
    loop {
      {
        tint_return_flag = true;
        tint_return_value = non_uniform_global;
        break;
      }
    }
    break;
    if ((i == 10)) {
      break;
    }
  }
  if (!(tint_return_flag)) {
    {
      tint_return_flag = true;
      tint_return_value = 0;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, ForLoop) {
    auto* src = R"(
var<private> non_uniform_global : array<i32, 10>;

fn foo() -> i32 {
  for (var i = 0; i < 10; i++) {
    if (non_uniform_global[i] == 0) {
      return i;
    }
  }
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : array<i32, 10>;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  for(var i = 0; (i < 10); i++) {
    if ((non_uniform_global[i] == 0)) {
      {
        tint_return_flag = true;
        tint_return_value = i;
        break;
      }
    }
  }
  if (!(tint_return_flag)) {
    {
      tint_return_flag = true;
      tint_return_value = 0;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, WhileLoop) {
    auto* src = R"(
var<private> non_uniform_global : array<i32, 10>;

fn foo() -> i32 {
  var i = 0;
  while (i < 10) {
    if (non_uniform_global[i] == 0) {
      return i;
    }
    i++;
  }
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : array<i32, 10>;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  var i = 0;
  while((i < 10)) {
    if ((non_uniform_global[i] == 0)) {
      {
        tint_return_flag = true;
        tint_return_value = i;
        break;
      }
    }
    i++;
  }
  if (!(tint_return_flag)) {
    {
      tint_return_flag = true;
      tint_return_value = 0;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, Switch) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  switch (non_uniform_global) {
    case 0 {
      return 0;
    }
    case 1 {
      if (false) {
        return 1;
      }
      bar = 6;
    }
    case 2 {
      bar = 5;
    }
    default {
      return 42;
    }
  }
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  switch(non_uniform_global) {
    case 0: {
      {
        tint_return_flag = true;
        tint_return_value = 0;
        break;
      }
    }
    case 1: {
      if (false) {
        {
          tint_return_flag = true;
          tint_return_value = 1;
          break;
        }
      }
      bar = 6;
    }
    case 2: {
      bar = 5;
    }
    default: {
      {
        tint_return_flag = true;
        tint_return_value = 42;
        break;
      }
    }
  }
  if (!(tint_return_flag)) {
    {
      tint_return_flag = true;
      tint_return_value = 0;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(MergeReturnTest, ComplexNesting) {
    auto* src = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  for (var i = 0; i < 10; i++) {
    bar = i;

    switch (non_uniform_global) {
      case 0 {
        return 0;
      }
      case 1 {
        if (false) {
          loop {
            if (i == non_uniform_global) {
              return 1;
            }
            continuing {
              bar++;
              break if (i == 7);
            }
          }
        }
        bar = 0;
      }
      default {
        return 42;
      }
    }
  }
  return 0;
}
)";

    auto* expect = R"(
var<private> non_uniform_global : i32;

var<private> bar : i32;

fn foo() -> i32 {
  var tint_return_flag : bool;
  var tint_return_value : i32;
  for(var i = 0; (i < 10); i++) {
    bar = i;
    switch(non_uniform_global) {
      case 0: {
        {
          tint_return_flag = true;
          tint_return_value = 0;
          break;
        }
      }
      case 1: {
        if (false) {
          loop {
            if ((i == non_uniform_global)) {
              {
                tint_return_flag = true;
                tint_return_value = 1;
                break;
              }
            }

            continuing {
              bar++;
              break if (i == 7);
            }
          }
          if (tint_return_flag) {
            break;
          }
        }
        bar = 0;
      }
      default: {
        {
          tint_return_flag = true;
          tint_return_value = 42;
          break;
        }
      }
    }
    if (tint_return_flag) {
      break;
    }
  }
  if (!(tint_return_flag)) {
    {
      tint_return_flag = true;
      tint_return_value = 0;
    }
  }
  return tint_return_value;
}
)";

    auto got = Run<MergeReturn>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
