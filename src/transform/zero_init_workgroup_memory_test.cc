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
fn f([[builtin(local_invocation_index)]] idx : u32) {
  ignore(v); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
var<workgroup> v : i32;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] idx : u32) {
  if ((idx == 0u)) {
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
  [[builtin(local_invocation_index)]] idx : u32;
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
  idx : u32;
};

[[stage(compute), workgroup_size(1)]]
fn f(params : Params) {
  if ((params.idx == 0u)) {
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
  if ((local_invocation_index == 0u)) {
    v = i32();
  }
  workgroupBarrier();
  ignore(v);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex) {
  auto* src = R"(
struct S {
  x : i32;
  y : array<i32, 8>;
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] idx : u32) {
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
fn f([[builtin(local_invocation_index)]] idx : u32) {
  if ((idx == 0u)) {
    a = i32();
    b = S();
    c = array<S, 32>();
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
  if ((local_invocation_index == 0u)) {
    a = i32();
    b = S();
    c = array<S, 32>();
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

[[stage(compute), workgroup_size(1)]]
fn f2([[builtin(local_invocation_id)]] local_invocation_id : vec3<u32>) {
  ignore(b); // Initialization should be inserted above this statement
}

[[stage(compute), workgroup_size(1)]]
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
  if ((local_invocation_index == 0u)) {
    a = i32();
    c = array<S, 32>();
  }
  workgroupBarrier();
  ignore(a);
  ignore(c);
}

[[stage(compute), workgroup_size(1)]]
fn f2([[builtin(local_invocation_id)]] local_invocation_id : vec3<u32>, [[builtin(local_invocation_index)]] local_invocation_index_1 : u32) {
  if ((local_invocation_index_1 == 0u)) {
    b = S();
  }
  workgroupBarrier();
  ignore(b);
}

[[stage(compute), workgroup_size(1)]]
fn f3([[builtin(local_invocation_index)]] local_invocation_index_2 : u32) {
  if ((local_invocation_index_2 == 0u)) {
    c = array<S, 32>();
    a = i32();
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
fn f([[builtin(local_invocation_index)]] idx : u32) {
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
fn f([[builtin(local_invocation_index)]] idx : u32) {
  if ((idx == 0u)) {
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
  if ((local_invocation_index == 0u)) {
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
  if ((local_invocation_index == 0u)) {
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
  if ((local_invocation_index == 0u)) {
    atomicStore(&(w[0u]), u32());
    atomicStore(&(w[1u]), u32());
    atomicStore(&(w[2u]), u32());
    atomicStore(&(w[3u]), u32());
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
  if ((local_invocation_index == 0u)) {
    w[0u].a = i32();
    atomicStore(&(w[0u].i), i32());
    w[0u].b = f32();
    atomicStore(&(w[0u].u), u32());
    w[0u].c = u32();
    w[1u].a = i32();
    atomicStore(&(w[1u].i), i32());
    w[1u].b = f32();
    atomicStore(&(w[1u].u), u32());
    w[1u].c = u32();
    w[2u].a = i32();
    atomicStore(&(w[2u].i), i32());
    w[2u].b = f32();
    atomicStore(&(w[2u].u), u32());
    w[2u].c = u32();
    w[3u].a = i32();
    atomicStore(&(w[3u].i), i32());
    w[3u].b = f32();
    atomicStore(&(w[3u].u), u32());
    w[3u].c = u32();
  }
  workgroupBarrier();
  ignore(w);
}
)";

  auto got = Run<ZeroInitWorkgroupMemory>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArray_InitWithLoop) {
  auto* src = R"(
struct S {
  a : array<i32, 3>; // size: 12, less than the loop threshold
  b : array<i32, 4>; // size: 16, equal to the loop threshold
  c : array<i32, 5>; // size: 20, greater than the loop threshold
};

var<workgroup> w : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  ignore(w); // Initialization should be inserted above this statement
}
)";
  auto* expect = R"(
struct S {
  a : array<i32, 3>;
  b : array<i32, 4>;
  c : array<i32, 5>;
};

var<workgroup> w : S;

[[stage(compute), workgroup_size(1)]]
fn f([[builtin(local_invocation_index)]] local_invocation_index : u32) {
  if ((local_invocation_index == 0u)) {
    w.a = array<i32, 3>();
    for(var i : i32; (i < 4); i = (i + 1)) {
      w.b[i] = i32();
    }
    for(var i_1 : i32; (i_1 < 5); i_1 = (i_1 + 1)) {
      w.c[i_1] = i32();
    }
  }
  workgroupBarrier();
  ignore(w);
}
)";

  ZeroInitWorkgroupMemory::Config cfg;
  cfg.init_arrays_with_loop_size_threshold = 16;

  DataMap data;
  data.Add<ZeroInitWorkgroupMemory::Config>(cfg);
  auto got = Run<ZeroInitWorkgroupMemory>(src, data);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
