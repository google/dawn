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

#include "src/transform/simplify.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using SimplifyTest = TransformTest;

TEST_F(SimplifyTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<Simplify>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyTest, AddressOfDeref) {
  auto* src = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &(v);
  let x : ptr<function, i32> = &(*(p));
  let y : ptr<function, i32> = &(*(&(*(p))));
  let z : ptr<function, i32> = &(*(&(*(&(*(&(*(p))))))));
}
)";

  auto* expect = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &(v);
  let x : ptr<function, i32> = p;
  let y : ptr<function, i32> = p;
  let z : ptr<function, i32> = p;
}
)";

  auto got = Run<Simplify>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyTest, DerefAddressOf) {
  auto* src = R"(
fn f() {
  var v : i32;
  let x : i32 = *(&(v));
  let y : i32 = *(&(*(&(v))));
  let z : i32 = *(&(*(&(*(&(*(&(v))))))));
}
)";

  auto* expect = R"(
fn f() {
  var v : i32;
  let x : i32 = v;
  let y : i32 = v;
  let z : i32 = v;
}
)";

  auto got = Run<Simplify>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(SimplifyTest, NoChange) {
  auto* src = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &(v);
  let x : i32 = *(p);
}
)";

  auto* expect = R"(
fn f() {
  var v : i32;
  let p : ptr<function, i32> = &(v);
  let x : i32 = *(p);
}
)";

  auto got = Run<Simplify>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
