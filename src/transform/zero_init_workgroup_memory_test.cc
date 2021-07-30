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

#include "src/transform/zero_init_workgroup_memory.h"

#include <utility>

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using ZeroInitWorkgroupMemoryTest = TransformTest;

TEST_F(ZeroInitWorkgroupMemoryTest, EmptyModule) {
  auto* src = "";
  auto* expect = src;

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, NoWorkgroupVars) {
  auto* src = R"(
var<private> v : i32;

fn f() {
  v = 1;
}
)";
  auto* expect = src;

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, UnreferencedWorkgroupVars) {
  auto* src = R"(
var<workgroup> a : i32;

var<workgroup> b : i32;

var<workgroup> c : i32;

fn unreferenced() {
  b = c;
}

[[stage(compute), workgroup_size(1)]]
fn f() {
}
)";
  auto* expect = src;

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndex) {
  auto* src = R"(
var<workgroup> v : i32;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  ignore(v); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
var<workgroup> v : i32;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  ignore(v);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest,
       SingleWorkgroupVar_ExistingLocalIndexInStruct) {
  auto* src = R"(
var<workgroup> v : i32;

struct Params {
  [[builtin(local_invocation_index)]] local_idx : u32;
};

[[stage(compute), workgroup_size(1)]]
fn f(params : Params) {
  ignore(v); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
var<workgroup> v : i32;

struct Params {
  [[builtin(local_invocation_index)]]
  local_idx : u32;
};

[[stage(compute), workgroup_size(1)]]
fn f(params : Params) {
  {
    v = i32();
  }
  workgroupBarrier();
  ignore(v);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_InjectedLocalIndex) {
  auto* src = R"(
var<workgroup> v : i32;

[[stage(compute), workgroup_size(1)]]
fn f() {
  ignore(v); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
var<workgroup> v : i32;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_invocation_index : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  ignore(v);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest,
       MultipleWorkgroupVar_ExistingLocalIndex_Size1) {
  auto* src = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  ignore(a); // Initialization should be inserted above this statement
  ignore(b);
  ignore(c);
}
)";
  auto* expect = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_idx; (idx < 8u); idx = (idx + 1u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 32u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_idx; (idx_2 < 256u); idx_2 = (idx_2 + 1u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
  ignore(a);
  ignore(b);
  ignore(c);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest,
       MultipleWorkgroupVar_ExistingLocalIndex_Size_2_3) {
  auto* src = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(2, 3)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  ignore(a); // Initialization should be inserted above this statement
  ignore(b);
  ignore(c);
}
)";
  auto* expect = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(2, 3)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  if ((local_idx < 1u)) {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_idx; (idx < 8u); idx = (idx + 6u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 32u); idx_1 = (idx_1 + 6u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_idx; (idx_2 < 256u); idx_2 = (idx_2 + 6u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
  ignore(a);
  ignore(b);
  ignore(c);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest,
       MultipleWorkgroupVar_ExistingLocalIndex_Size_2_3_X) {
  auto* src = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[override(1)]] let X : i32;

[[stage(compute), workgroup_size(2, 3, X)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  ignore(a); // Initialization should be inserted above this statement
  ignore(b);
  ignore(c);
}
)";
  auto* expect =
      R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[override(1)]] let X : i32;

[[stage(compute), workgroup_size(2, 3, X)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  for(var idx : u32 = local_idx; (idx < 1u); idx = (idx + (u32(X) * 6u))) {
    a = i32();
    b.x = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 8u); idx_1 = (idx_1 + (u32(X) * 6u))) {
    let i : u32 = idx_1;
    b.y[i] = i32();
  }
  for(var idx_2 : u32 = local_idx; (idx_2 < 32u); idx_2 = (idx_2 + (u32(X) * 6u))) {
    let i_1 : u32 = idx_2;
    c[i_1].x = i32();
  }
  for(var idx_3 : u32 = local_idx; (idx_3 < 256u); idx_3 = (idx_3 + (u32(X) * 6u))) {
    let i_2 : u32 = (idx_3 / 8u);
    let i : u32 = (idx_3 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
  ignore(a);
  ignore(b);
  ignore(c);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest,
       MultipleWorkgroupVar_ExistingLocalIndex_Size_5u_X_10u) {
  auto* src = R"(
struct S {
  x : array<array<i32, 8>, 10>;
  y : array<i32, 8>;
  z : array<array<array<i32, 8>, 10>, 20>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[override(1)]] let X : u32;

[[stage(compute), workgroup_size(5u, X, 10u)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  ignore(a); // Initialization should be inserted above this statement
  ignore(b);
  ignore(c);
}
)";
  auto* expect =
      R"(
struct S {
  x : array<array<i32, 8>, 10>;
  y : array<i32, 8>;
  z : array<array<array<i32, 8>, 10>, 20>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[override(1)]] let X : u32;

[[stage(compute), workgroup_size(5u, X, 10u)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  for(var idx : u32 = local_idx; (idx < 1u); idx = (idx + (X * 50u))) {
    a = i32();
  }
  for(var idx_1 : u32 = local_idx; (idx_1 < 8u); idx_1 = (idx_1 + (X * 50u))) {
    let i_1 : u32 = idx_1;
    b.y[i_1] = i32();
  }
  for(var idx_2 : u32 = local_idx; (idx_2 < 80u); idx_2 = (idx_2 + (X * 50u))) {
    let i : u32 = (idx_2 / 8u);
    let i_1 : u32 = (idx_2 % 8u);
    b.x[i][i_1] = i32();
  }
  for(var idx_3 : u32 = local_idx; (idx_3 < 256u); idx_3 = (idx_3 + (X * 50u))) {
    let i_4 : u32 = (idx_3 / 8u);
    let i_1 : u32 = (idx_3 % 8u);
    c[i_4].y[i_1] = i32();
  }
  for(var idx_4 : u32 = local_idx; (idx_4 < 1600u); idx_4 = (idx_4 + (X * 50u))) {
    let i_2 : u32 = (idx_4 / 80u);
    let i : u32 = ((idx_4 % 80u) / 8u);
    let i_1 : u32 = (idx_4 % 8u);
    b.z[i_2][i][i_1] = i32();
  }
  for(var idx_5 : u32 = local_idx; (idx_5 < 2560u); idx_5 = (idx_5 + (X * 50u))) {
    let i_3 : u32 = (idx_5 / 80u);
    let i : u32 = ((idx_5 % 80u) / 8u);
    let i_1 : u32 = (idx_5 % 8u);
    c[i_3].x[i][i_1] = i32();
  }
  for(var idx_6 : u32 = local_idx; (idx_6 < 51200u); idx_6 = (idx_6 + (X * 50u))) {
    let i_5 : u32 = (idx_6 / 1600u);
    let i_2 : u32 = ((idx_6 % 1600u) / 80u);
    let i : u32 = ((idx_6 % 80u) / 8u);
    let i_1 : u32 = (idx_6 % 8u);
    c[i_5].z[i_2][i][i_1] = i32();
  }
  workgroupBarrier();
  ignore(a);
  ignore(b);
  ignore(c);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_InjectedLocalIndex) {
  auto* src = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_id)]] local_invocation_id : vec3<u32>) {
  ignore(a); // Initialization should be inserted above this statement
  ignore(b);
  ignore(c);
}
)";
  auto* expect = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_id)]] local_invocation_id : vec3<u32>, [[builtin(local_invocation_index)]] local_invocation_index : u32) {
  {
    a = i32();
    b.x = i32();
  }
  for(var idx : u32 = local_invocation_index; (idx < 8u); idx = (idx + 1u)) {
    let i : u32 = idx;
    b.y[i] = i32();
  }
  for(var idx_1 : u32 = local_invocation_index; (idx_1 < 32u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = idx_1;
    c[i_1].x = i32();
  }
  for(var idx_2 : u32 = local_invocation_index; (idx_2 < 256u); idx_2 = (idx_2 + 1u)) {
    let i_2 : u32 = (idx_2 / 8u);
    let i : u32 = (idx_2 % 8u);
    c[i_2].y[i] = i32();
  }
  workgroupBarrier();
  ignore(a);
  ignore(b);
  ignore(c);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_MultipleEntryPoints) {
  auto* src = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(1)]]
fn f1() {
  ignore(a); // Initialization should be inserted above this statement
  ignore(c);
}

[[stage(compute), workgroup_size(1, 2, 3)]]
fn f2([[builtin(local_invocation_id)]] local_invocation_id : vec3<u32>) {
  ignore(b); // Initialization should be inserted above this statement
}

[[stage(compute), workgroup_size(4, 5, 6)]]
fn f3() {
  ignore(c); // Initialization should be inserted above this statement
  ignore(a);
}
)";
  auto* expect = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(1)]]
fn f1([[builtin(local_invocation_index)]] local_invocation_index : u32) {
  {
    a = i32();
  }
  for(var idx : u32 = local_invocation_index; (idx < 32u); idx = (idx + 1u)) {
    let i : u32 = idx;
    c[i].x = i32();
  }
  for(var idx_1 : u32 = local_invocation_index; (idx_1 < 256u); idx_1 = (idx_1 + 1u)) {
    let i_1 : u32 = (idx_1 / 8u);
    let i_2 : u32 = (idx_1 % 8u);
    c[i_1].y[i_2] = i32();
  }
  workgroupBarrier();
  ignore(a);
  ignore(c);
}

[[stage(compute), workgroup_size(1, 2, 3)]]
fn f2([[builtin(local_invocation_id)]] local_invocation_id : vec3<u32>, [[builtin(local_invocation_index)]] local_invocation_index_1 : u32) {
  if ((local_invocation_index_1 < 1u)) {
    b.x = i32();
  }
  for(var idx_2 : u32 = local_invocation_index_1; (idx_2 < 8u); idx_2 = (idx_2 + 6u)) {
    let i_3 : u32 = idx_2;
    b.y[i_3] = i32();
  }
  workgroupBarrier();
  ignore(b);
}

[[stage(compute), workgroup_size(4, 5, 6)]]
fn f3([[builtin(local_invocation_index)]] local_invocation_index_2 : u32) {
  if ((local_invocation_index_2 < 1u)) {
    a = i32();
  }
  if ((local_invocation_index_2 < 32u)) {
    let i_4 : u32 = local_invocation_index_2;
    c[i_4].x = i32();
  }
  for(var idx_3 : u32 = local_invocation_index_2; (idx_3 < 256u); idx_3 = (idx_3 + 120u)) {
    let i_5 : u32 = (idx_3 / 8u);
    let i_6 : u32 = (idx_3 % 8u);
    c[i_5].y[i_6] = i32();
  }
  workgroupBarrier();
  ignore(c);
  ignore(a);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, TransitiveUsage) {
  auto* src = R"(
var<workgroup> v : i32;

fn use_v() {
  ignore(v);
}

fn call_use_v() {
  use_v();
}

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  call_use_v(); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
var<workgroup> v : i32;

fn use_v() {
  ignore(v);
}

fn call_use_v() {
  use_v();
}

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_idx : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  call_use_v();
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupAtomics) {
  auto* src = R"(
var<workgroup> i : atomic<i32>;
var<workgroup> u : atomic<u32>;

[[stage(compute), workgroup_size(1)]]
fn f() {
  ignore(i); // Initialization should be inserted above this statement
  ignore(u);
}
)";
  auto* expect = R"(
var<workgroup> i : atomic<i32>;

var<workgroup> u : atomic<u32>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_invocation_index : u32) {
  {
    atomicStore(&(i), i32());
    atomicStore(&(u), u32());
  }
  workgroupBarrier();
  ignore(i);
  ignore(u);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupStructOfAtomics) {
  auto* src = R"(
struct S {
  a : i32;
  i : atomic<i32>;
  b : f32;
  u : atomic<u32>;
  c : u32;
};

var<workgroup> w : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  ignore(w); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
struct S {
  a : i32;
  i : atomic<i32>;
  b : f32;
  u : atomic<u32>;
  c : u32;
};

var<workgroup> w : S;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_invocation_index : u32) {
  {
    w.a = i32();
    atomicStore(&(w.i), i32());
    w.b = f32();
    atomicStore(&(w.u), u32());
    w.c = u32();
  }
  workgroupBarrier();
  ignore(w);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfAtomics) {
  auto* src = R"(
var<workgroup> w : array<atomic<u32>, 4>;

[[stage(compute), workgroup_size(1)]]
fn f() {
  ignore(w); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
var<workgroup> w : array<atomic<u32>, 4>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_invocation_index : u32) {
  for(var idx : u32 = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    let i : u32 = idx;
    atomicStore(&(w[i]), u32());
  }
  workgroupBarrier();
  ignore(w);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfStructOfAtomics) {
  auto* src = R"(
struct S {
  a : i32;
  i : atomic<i32>;
  b : f32;
  u : atomic<u32>;
  c : u32;
};

var<workgroup> w : array<S, 4>;

[[stage(compute), workgroup_size(1)]]
fn f() {
  ignore(w); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
struct S {
  a : i32;
  i : atomic<i32>;
  b : f32;
  u : atomic<u32>;
  c : u32;
};

var<workgroup> w : array<S, 4>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_invocation_index : u32) {
  for(var idx : u32 = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    let i_1 : u32 = idx;
    w[i_1].a = i32();
    atomicStore(&(w[i_1].i), i32());
    w[i_1].b = f32();
    atomicStore(&(w[i_1].u), u32());
    w[i_1].c = u32();
  }
  workgroupBarrier();
  ignore(w);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
