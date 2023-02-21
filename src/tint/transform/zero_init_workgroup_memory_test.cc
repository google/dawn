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

#include "src/tint/transform/zero_init_workgroup_memory.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using ZeroInitWorkgroupMemoryTest = TransformTest;

TEST_F(ZeroInitWorkgroupMemoryTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<ZeroInitWorkgroupMemory>(src));
}

TEST_F(ZeroInitWorkgroupMemoryTest, ShouldRunHasNoWorkgroupVars) {
    auto* src = R"(
var<private> v : i32;
)";

    EXPECT_FALSE(ShouldRun<ZeroInitWorkgroupMemory>(src));
}

TEST_F(ZeroInitWorkgroupMemoryTest, ShouldRunHasWorkgroupVars) {
    auto* src = R"(
var<workgroup> a : i32;
)";

    EXPECT_TRUE(ShouldRun<ZeroInitWorkgroupMemory>(src));
}

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

@compute @workgroup_size(1)
fn f() {
}
)";
    auto* expect = src;

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, UnreferencedWorkgroupVars_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
}

fn unreferenced() {
  b = c;
}

var<workgroup> a : i32;

var<workgroup> b : i32;

var<workgroup> c : i32;
)";
    auto* expect = src;

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndex) {
    auto* src = R"(
var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = v; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  _ = v;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndex_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = v; // Initialization should be inserted above this statement
}

var<workgroup> v : i32;
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  _ = v;
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndexInStruct) {
    auto* src = R"(
var<workgroup> v : i32;

struct Params {
  @builtin(local_invocation_index) local_idx : u32,
};

@compute @workgroup_size(1)
fn f(params : Params) {
  _ = v; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
var<workgroup> v : i32;

struct Params {
  @builtin(local_invocation_index)
  local_idx : u32,
}

@compute @workgroup_size(1)
fn f(params : Params) {
  {
    v = i32();
  }
  workgroupBarrier();
  _ = v;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_ExistingLocalIndexInStruct_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(params : Params) {
  _ = v; // Initialization should be inserted above this statement
}

struct Params {
  @builtin(local_invocation_index) local_idx : u32,
};

var<workgroup> v : i32;
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(params : Params) {
  {
    v = i32();
  }
  workgroupBarrier();
  _ = v;
}

struct Params {
  @builtin(local_invocation_index)
  local_idx : u32,
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_InjectedLocalIndex) {
    auto* src = R"(
var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f() {
  _ = v; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
var<workgroup> v : i32;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  _ = v;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, SingleWorkgroupVar_InjectedLocalIndex_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  _ = v; // Initialization should be inserted above this statement
}

var<workgroup> v : i32;
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  _ = v;
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size1) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
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
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size1_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
};
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
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
  _ = a;
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size_2_3) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(2, 3)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(2, 3)
fn f(@builtin(local_invocation_index) local_idx : u32) {
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
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size_2_3_X) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : i32;

@compute @workgroup_size(2, 3, X)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect =
        R"(
struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : i32;

@compute @workgroup_size(2, 3, X)
fn f(@builtin(local_invocation_index) local_idx : u32) {
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
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_ExistingLocalIndex_Size_5u_X_10u) {
    auto* src = R"(
struct S {
  x : array<array<i32, 8>, 10>,
  y : array<i32, 8>,
  z : array<array<array<i32, 8>, 10>, 20>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : u32;

@compute @workgroup_size(5u, X, 10u)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect =
        R"(
struct S {
  x : array<array<i32, 8>, 10>,
  y : array<i32, 8>,
  z : array<array<array<i32, 8>, 10>, 20>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@id(1) override X : u32;

@compute @workgroup_size(5u, X, 10u)
fn f(@builtin(local_invocation_index) local_idx : u32) {
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
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_InjectedLocalIndex) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}
)";
    auto* expect = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index : u32) {
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
  _ = a;
  _ = b;
  _ = c;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_InjectedLocalIndex_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = a; // Initialization should be inserted above this statement
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
};
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index : u32) {
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
  _ = a;
  _ = b;
  _ = c;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_MultipleEntryPoints) {
    auto* src = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
};

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f1() {
  _ = a; // Initialization should be inserted above this statement
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = b; // Initialization should be inserted above this statement
}

@compute @workgroup_size(4, 5, 6)
fn f3() {
  _ = c; // Initialization should be inserted above this statement
  _ = a;
}
)";
    auto* expect = R"(
struct S {
  x : i32,
  y : array<i32, 8>,
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

@compute @workgroup_size(1)
fn f1(@builtin(local_invocation_index) local_invocation_index : u32) {
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
  _ = a;
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index_1 : u32) {
  if ((local_invocation_index_1 < 1u)) {
    b.x = i32();
  }
  for(var idx_2 : u32 = local_invocation_index_1; (idx_2 < 8u); idx_2 = (idx_2 + 6u)) {
    let i_3 : u32 = idx_2;
    b.y[i_3] = i32();
  }
  workgroupBarrier();
  _ = b;
}

@compute @workgroup_size(4, 5, 6)
fn f3(@builtin(local_invocation_index) local_invocation_index_2 : u32) {
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
  _ = c;
  _ = a;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, MultipleWorkgroupVar_MultipleEntryPoints_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f1() {
  _ = a; // Initialization should be inserted above this statement
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>) {
  _ = b; // Initialization should be inserted above this statement
}

@compute @workgroup_size(4, 5, 6)
fn f3() {
  _ = c; // Initialization should be inserted above this statement
  _ = a;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
};
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f1(@builtin(local_invocation_index) local_invocation_index : u32) {
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
  _ = a;
  _ = c;
}

@compute @workgroup_size(1, 2, 3)
fn f2(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index_1 : u32) {
  if ((local_invocation_index_1 < 1u)) {
    b.x = i32();
  }
  for(var idx_2 : u32 = local_invocation_index_1; (idx_2 < 8u); idx_2 = (idx_2 + 6u)) {
    let i_3 : u32 = idx_2;
    b.y[i_3] = i32();
  }
  workgroupBarrier();
  _ = b;
}

@compute @workgroup_size(4, 5, 6)
fn f3(@builtin(local_invocation_index) local_invocation_index_2 : u32) {
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
  _ = c;
  _ = a;
}

var<workgroup> a : i32;

var<workgroup> b : S;

var<workgroup> c : array<S, 32>;

struct S {
  x : i32,
  y : array<i32, 8>,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, TransitiveUsage) {
    auto* src = R"(
var<workgroup> v : i32;

fn use_v() {
  _ = v;
}

fn call_use_v() {
  use_v();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  call_use_v(); // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
var<workgroup> v : i32;

fn use_v() {
  _ = v;
}

fn call_use_v() {
  use_v();
}

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
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

TEST_F(ZeroInitWorkgroupMemoryTest, TransitiveUsage_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  call_use_v(); // Initialization should be inserted above this statement
}

fn call_use_v() {
  use_v();
}

fn use_v() {
  _ = v;
}

var<workgroup> v : i32;
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_idx : u32) {
  {
    v = i32();
  }
  workgroupBarrier();
  call_use_v();
}

fn call_use_v() {
  use_v();
}

fn use_v() {
  _ = v;
}

var<workgroup> v : i32;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupAtomics) {
    auto* src = R"(
var<workgroup> i : atomic<i32>;
var<workgroup> u : atomic<u32>;

@compute @workgroup_size(1)
fn f() {
  atomicLoad(&(i)); // Initialization should be inserted above this statement
  atomicLoad(&(u));
}
)";
    auto* expect = R"(
var<workgroup> i : atomic<i32>;

var<workgroup> u : atomic<u32>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  {
    atomicStore(&(i), i32());
    atomicStore(&(u), u32());
  }
  workgroupBarrier();
  atomicLoad(&(i));
  atomicLoad(&(u));
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  atomicLoad(&(i)); // Initialization should be inserted above this statement
  atomicLoad(&(u));
}

var<workgroup> i : atomic<i32>;
var<workgroup> u : atomic<u32>;
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  {
    atomicStore(&(i), i32());
    atomicStore(&(u), u32());
  }
  workgroupBarrier();
  atomicLoad(&(i));
  atomicLoad(&(u));
}

var<workgroup> i : atomic<i32>;

var<workgroup> u : atomic<u32>;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupStructOfAtomics) {
    auto* src = R"(
struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};

var<workgroup> w : S;

@compute @workgroup_size(1)
fn f() {
  _ = w.a; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}

var<workgroup> w : S;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  {
    w.a = i32();
    atomicStore(&(w.i), i32());
    w.b = f32();
    atomicStore(&(w.u), u32());
    w.c = u32();
  }
  workgroupBarrier();
  _ = w.a;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupStructOfAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  _ = w.a; // Initialization should be inserted above this statement
}

var<workgroup> w : S;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  {
    w.a = i32();
    atomicStore(&(w.i), i32());
    w.b = f32();
    atomicStore(&(w.u), u32());
    w.c = u32();
  }
  workgroupBarrier();
  _ = w.a;
}

var<workgroup> w : S;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfAtomics) {
    auto* src = R"(
var<workgroup> w : array<atomic<u32>, 4>;

@compute @workgroup_size(1)
fn f() {
  atomicLoad(&w[0]); // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
var<workgroup> w : array<atomic<u32>, 4>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  for(var idx : u32 = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    let i : u32 = idx;
    atomicStore(&(w[i]), u32());
  }
  workgroupBarrier();
  atomicLoad(&(w[0]));
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  atomicLoad(&w[0]); // Initialization should be inserted above this statement
}

var<workgroup> w : array<atomic<u32>, 4>;
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  for(var idx : u32 = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    let i : u32 = idx;
    atomicStore(&(w[i]), u32());
  }
  workgroupBarrier();
  atomicLoad(&(w[0]));
}

var<workgroup> w : array<atomic<u32>, 4>;
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfStructOfAtomics) {
    auto* src = R"(
struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};

var<workgroup> w : array<S, 4>;

@compute @workgroup_size(1)
fn f() {
  _ = w[0].a; // Initialization should be inserted above this statement
}
)";
    auto* expect = R"(
struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}

var<workgroup> w : array<S, 4>;

@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  for(var idx : u32 = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    let i_1 : u32 = idx;
    w[i_1].a = i32();
    atomicStore(&(w[i_1].i), i32());
    w[i_1].b = f32();
    atomicStore(&(w[i_1].u), u32());
    w[i_1].c = u32();
  }
  workgroupBarrier();
  _ = w[0].a;
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, WorkgroupArrayOfStructOfAtomics_OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn f() {
  _ = w[0].a; // Initialization should be inserted above this statement
}

var<workgroup> w : array<S, 4>;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
};
)";
    auto* expect = R"(
@compute @workgroup_size(1)
fn f(@builtin(local_invocation_index) local_invocation_index : u32) {
  for(var idx : u32 = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    let i_1 : u32 = idx;
    w[i_1].a = i32();
    atomicStore(&(w[i_1].i), i32());
    w[i_1].b = f32();
    atomicStore(&(w[i_1].u), u32());
    w[i_1].c = u32();
  }
  workgroupBarrier();
  _ = w[0].a;
}

var<workgroup> w : array<S, 4>;

struct S {
  a : i32,
  i : atomic<i32>,
  b : f32,
  u : atomic<u32>,
  c : u32,
}
)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(ZeroInitWorkgroupMemoryTest, ArrayWithOverrideCount) {
    auto* src =
        R"(override O = 123;
alias A = array<i32, O*2>;

var<workgroup> W : A;

@compute @workgroup_size(1)
fn main() {
    let p : ptr<workgroup, A> = &W;
    (*p)[0] = 42;
}
)";

    auto* expect =
        R"(error: array size is an override-expression, when expected a constant-expression.
Was the SubstituteOverride transform run?)";

    auto got = Run<ZeroInitWorkgroupMemory>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
