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

#include "src/transform/duplicate_storage_structs.h"

#include "src/transform/test_helper.h"
namespace tint {
namespace transform {
namespace {

using DuplicateStorageStructsTransformTest = TransformTest;

TEST_F(DuplicateStorageStructsTransformTest, Simple) {
  auto* src = R"(
[[block]]
  struct S {
  a : f32;
};

[[group(0), binding(0)]] var<storage> r : [[access(read)]] S;
[[group(0), binding(1)]] var<storage> w : [[access(write)]] S;

[[stage(compute)]]
  fn main() {
}
)";

  auto* expect = R"(
[[block]]
struct S_1 {
  a : f32;
};

[[block]]
struct S_2 {
  a : f32;
};

[[group(0), binding(0)]] var<storage> r : [[access(read)]] S_1;

[[group(0), binding(1)]] var<storage> w : [[access(write)]] S_2;

[[stage(compute)]]
fn main() {
}
)";

  auto got = Run<DuplicateStorageStructs>(src);
  EXPECT_EQ(expect, str(got));
}

TEST_F(DuplicateStorageStructsTransformTest, MultipleStorageStructs) {
  auto* src = R"(
[[block]]
  struct S1 {
  a : f32;
};

[[block]]
  struct S2 {
  a : i32;
};

[[group(0), binding(0)]] var<storage> r1 : [[access(read)]] S1;
[[group(0), binding(1)]] var<storage> w1 : [[access(write)]] S1;

[[group(0), binding(2)]] var<storage> r2 : [[access(read)]] S2;
[[group(0), binding(3)]] var<storage> w2 : [[access(write)]] S2;

[[stage(compute)]]
  fn main() {
}
)";

  auto* expect = R"(
[[block]]
struct S1_1 {
  a : f32;
};

[[block]]
struct S1_2 {
  a : f32;
};

[[block]]
struct S2_1 {
  a : i32;
};

[[block]]
struct S2_2 {
  a : i32;
};

[[group(0), binding(0)]] var<storage> r1 : [[access(read)]] S1_1;

[[group(0), binding(1)]] var<storage> w1 : [[access(write)]] S1_2;

[[group(0), binding(2)]] var<storage> r2 : [[access(read)]] S2_1;

[[group(0), binding(3)]] var<storage> w2 : [[access(write)]] S2_2;

[[stage(compute)]]
fn main() {
}
)";

  auto got = Run<DuplicateStorageStructs>(src);
  EXPECT_EQ(expect, str(got));
}

TEST_F(DuplicateStorageStructsTransformTest, MultipleStorageVariables) {
  auto* src = R"(
[[block]]
  struct S {
  a : f32;
};

[[group(0), binding(0)]] var<storage> r1 : [[access(read)]] S;
[[group(0), binding(1)]] var<storage> r2 : [[access(read)]] S;
[[group(0), binding(2)]] var<storage> r3 : [[access(read)]] S;
[[group(0), binding(3)]] var<storage> w1 : [[access(write)]] S;
[[group(0), binding(4)]] var<storage> w2 : [[access(write)]] S;
[[group(0), binding(5)]] var<storage> w3 : [[access(write)]] S;
[[group(0), binding(3)]] var<storage> rw1 : [[access(read_write)]] S;
[[group(0), binding(4)]] var<storage> rw2 : [[access(read_write)]] S;
[[group(0), binding(5)]] var<storage> rw3 : [[access(read_write)]] S;

[[stage(compute)]]
  fn main() {
}
)";

  auto* expect = R"(
[[block]]
struct S_1 {
  a : f32;
};

[[block]]
struct S_2 {
  a : f32;
};

[[block]]
struct S_3 {
  a : f32;
};

[[group(0), binding(0)]] var<storage> r1 : [[access(read)]] S_1;

[[group(0), binding(1)]] var<storage> r2 : [[access(read)]] S_1;

[[group(0), binding(2)]] var<storage> r3 : [[access(read)]] S_1;

[[group(0), binding(3)]] var<storage> w1 : [[access(write)]] S_2;

[[group(0), binding(4)]] var<storage> w2 : [[access(write)]] S_2;

[[group(0), binding(5)]] var<storage> w3 : [[access(write)]] S_2;

[[group(0), binding(3)]] var<storage> rw1 : [[access(read_write)]] S_3;

[[group(0), binding(4)]] var<storage> rw2 : [[access(read_write)]] S_3;

[[group(0), binding(5)]] var<storage> rw3 : [[access(read_write)]] S_3;

[[stage(compute)]]
fn main() {
}
)";

  auto got = Run<DuplicateStorageStructs>(src);
  EXPECT_EQ(expect, str(got));
}

TEST_F(DuplicateStorageStructsTransformTest, MixedStructTypes) {
  auto* src = R"(

[[block]]
struct S_1 {
  a : f32;
};

[[block]]
struct S_2 {
  a : f32;
};

[[block]]
struct S_3 {
  a : f32;
};

[[group(0), binding(0)]] var<storage> r1 : [[access(read)]] S_1;

[[group(0), binding(1)]] var<storage> r2 : [[access(read)]] S_1;

[[group(0), binding(2)]] var<storage> r3 : [[access(read)]] S_1;

[[group(0), binding(3)]] var<storage> w1 : [[access(write)]] S_2;

[[group(0), binding(4)]] var<storage> w2 : [[access(write)]] S_2;

[[group(0), binding(5)]] var<storage> w3 : [[access(write)]] S_2;

[[group(0), binding(3)]] var<storage> rw1 : [[access(read_write)]] S_3;

[[group(0), binding(4)]] var<storage> rw2 : [[access(read_write)]] S_3;

[[group(0), binding(5)]] var<storage> rw3 : [[access(read_write)]] S_3;

[[stage(compute)]]
fn main() {
}
)";

  auto* expect = R"(
[[block]]
struct S_1_1 {
  a : f32;
};

[[block]]
struct S_2_1 {
  a : f32;
};

[[block]]
struct S_3_1 {
  a : f32;
};

[[group(0), binding(0)]] var<storage> r1 : [[access(read)]] S_1_1;

[[group(0), binding(1)]] var<storage> r2 : [[access(read)]] S_1_1;

[[group(0), binding(2)]] var<storage> r3 : [[access(read)]] S_1_1;

[[group(0), binding(3)]] var<storage> w1 : [[access(write)]] S_2_1;

[[group(0), binding(4)]] var<storage> w2 : [[access(write)]] S_2_1;

[[group(0), binding(5)]] var<storage> w3 : [[access(write)]] S_2_1;

[[group(0), binding(3)]] var<storage> rw1 : [[access(read_write)]] S_3_1;

[[group(0), binding(4)]] var<storage> rw2 : [[access(read_write)]] S_3_1;

[[group(0), binding(5)]] var<storage> rw3 : [[access(read_write)]] S_3_1;

[[stage(compute)]]
fn main() {
}
)";

  auto got = Run<DuplicateStorageStructs>(src);
  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
