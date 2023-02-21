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

#include "src/tint/transform/single_entry_point.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using SingleEntryPointTest = TransformTest;

TEST_F(SingleEntryPointTest, Error_MissingTransformData) {
    auto* src = "";

    auto* expect = "error: missing transform data for tint::transform::SingleEntryPoint";

    auto got = Run<SingleEntryPoint>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, Error_NoEntryPoints) {
    auto* src = "";

    auto* expect = "error: entry point 'main' not found";

    DataMap data;
    data.Add<SingleEntryPoint::Config>("main");
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, Error_InvalidEntryPoint) {
    auto* src = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = "error: entry point '_' not found";

    SingleEntryPoint::Config cfg("_");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, Error_NotAnEntryPoint) {
    auto* src = R"(
fn foo() {}

@fragment
fn main() {}
)";

    auto* expect = "error: entry point 'foo' not found";

    SingleEntryPoint::Config cfg("foo");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, SingleEntryPoint) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
}
)";

    SingleEntryPoint::Config cfg("main");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(src, str(got));
}

TEST_F(SingleEntryPointTest, MultipleEntryPoints) {
    auto* src = R"(
@vertex
fn vert_main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}

@fragment
fn frag_main() {
}

@compute @workgroup_size(1)
fn comp_main1() {
}

@compute @workgroup_size(1)
fn comp_main2() {
}
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn comp_main1() {
}
)";

    SingleEntryPoint::Config cfg("comp_main1");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, GlobalVariables) {
    auto* src = R"(
var<private> a : f32;

var<private> b : f32;

var<private> c : f32;

var<private> d : f32;

@vertex
fn vert_main() -> @builtin(position) vec4<f32> {
  a = 0.0;
  return vec4<f32>();
}

@fragment
fn frag_main() {
  b = 0.0;
}

@compute @workgroup_size(1)
fn comp_main1() {
  c = 0.0;
}

@compute @workgroup_size(1)
fn comp_main2() {
  d = 0.0;
}
)";

    auto* expect = R"(
var<private> c : f32;

@compute @workgroup_size(1)
fn comp_main1() {
  c = 0.0;
}
)";

    SingleEntryPoint::Config cfg("comp_main1");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, GlobalConstants) {
    auto* src = R"(
const a : f32 = 1.0;

const b : f32 = 1.0;

const c : f32 = 1.0;

const d : f32 = 1.0;

@vertex
fn vert_main() -> @builtin(position) vec4<f32> {
  let local_a : f32 = a;
  return vec4<f32>();
}

@fragment
fn frag_main() {
  let local_b : f32 = b;
}

@compute @workgroup_size(1)
fn comp_main1() {
  let local_c : f32 = c;
}

@compute @workgroup_size(1)
fn comp_main2() {
  let local_d : f32 = d;
}
)";

    auto* expect = R"(
const a : f32 = 1.0;

const b : f32 = 1.0;

const c : f32 = 1.0;

const d : f32 = 1.0;

@compute @workgroup_size(1)
fn comp_main1() {
  let local_c : f32 = c;
}
)";

    SingleEntryPoint::Config cfg("comp_main1");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, WorkgroupSizeConstPreserved) {
    auto* src = R"(
const size : i32 = 1;

@compute @workgroup_size(size)
fn main() {
}
)";

    auto* expect = src;

    SingleEntryPoint::Config cfg("main");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, OverridableConstants) {
    auto* src = R"(
@id(1001) override c1 : u32 = 1u;
          override c2 : u32 = 1u;
@id(0)    override c3 : u32 = 1u;
@id(9999) override c4 : u32 = 1u;

@compute @workgroup_size(1)
fn comp_main1() {
    let local_d = c1;
}

@compute @workgroup_size(1)
fn comp_main2() {
    let local_d = c2;
}

@compute @workgroup_size(1)
fn comp_main3() {
    let local_d = c3;
}

@compute @workgroup_size(1)
fn comp_main4() {
    let local_d = c4;
}

@compute @workgroup_size(1)
fn comp_main5() {
    let local_d = 1u;
}
)";

    {
        SingleEntryPoint::Config cfg("comp_main1");
        auto* expect = R"(
@id(1001) override c1 : u32 = 1u;

@compute @workgroup_size(1)
fn comp_main1() {
  let local_d = c1;
}
)";
        DataMap data;
        data.Add<SingleEntryPoint::Config>(cfg);
        auto got = Run<SingleEntryPoint>(src, data);
        EXPECT_EQ(expect, str(got));
    }

    {
        SingleEntryPoint::Config cfg("comp_main2");
        // The decorator is replaced with the one with explicit id
        // And should not be affected by other constants stripped away
        auto* expect = R"(
@id(1) override c2 : u32 = 1u;

@compute @workgroup_size(1)
fn comp_main2() {
  let local_d = c2;
}
)";
        DataMap data;
        data.Add<SingleEntryPoint::Config>(cfg);
        auto got = Run<SingleEntryPoint>(src, data);
        EXPECT_EQ(expect, str(got));
    }

    {
        SingleEntryPoint::Config cfg("comp_main3");
        auto* expect = R"(
@id(0) override c3 : u32 = 1u;

@compute @workgroup_size(1)
fn comp_main3() {
  let local_d = c3;
}
)";
        DataMap data;
        data.Add<SingleEntryPoint::Config>(cfg);
        auto got = Run<SingleEntryPoint>(src, data);
        EXPECT_EQ(expect, str(got));
    }

    {
        SingleEntryPoint::Config cfg("comp_main4");
        auto* expect = R"(
@id(9999) override c4 : u32 = 1u;

@compute @workgroup_size(1)
fn comp_main4() {
  let local_d = c4;
}
)";
        DataMap data;
        data.Add<SingleEntryPoint::Config>(cfg);
        auto got = Run<SingleEntryPoint>(src, data);
        EXPECT_EQ(expect, str(got));
    }

    {
        SingleEntryPoint::Config cfg("comp_main5");
        auto* expect = R"(
@compute @workgroup_size(1)
fn comp_main5() {
  let local_d = 1u;
}
)";
        DataMap data;
        data.Add<SingleEntryPoint::Config>(cfg);
        auto got = Run<SingleEntryPoint>(src, data);
        EXPECT_EQ(expect, str(got));
    }
}

TEST_F(SingleEntryPointTest, OverridableConstants_TransitiveUses) {
    // Make sure we do not strip away transitive uses of overridable constants.
    auto* src = R"(
@id(0) override c0 : u32;

@id(1) override c1 : u32 = (2 * c0);

@id(2) override c2 : u32;

@id(3) override c3 : u32 = (2 * c2);

@id(4) override c4 : u32;

@id(5) override c5 : u32 = (2 * c4);

alias arr_ty = array<i32, (2 * c5)>;

var<workgroup> arr : arr_ty;

@compute @workgroup_size(1, 1, (2 * c3))
fn main() {
  let local_d = c1;
  arr[0] = 42;
}
)";

    auto* expect = src;

    SingleEntryPoint::Config cfg("main");
    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, OverridableConstants_UnusedAliasForOverrideSizedArray) {
    // Make sure we strip away aliases that reference unused overridable constants.
    auto* src = R"(
@id(0) override c0 : u32;

// This is all unused by the target entry point.
@id(1) override c1 : u32;
alias arr_ty = array<i32, c1>;
var<workgroup> arr : arr_ty;

@compute @workgroup_size(64)
fn unused() {
  arr[0] = 42;
}

@compute @workgroup_size(64)
fn main() {
  let local_d = c0;
}
)";

    auto* expect = R"(
@id(0) override c0 : u32;

@compute @workgroup_size(64)
fn main() {
  let local_d = c0;
}
)";

    SingleEntryPoint::Config cfg("main");
    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);
    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, CalledFunctions) {
    auto* src = R"(
fn inner1() {
}

fn inner2() {
}

fn inner_shared() {
}

fn outer1() {
  inner1();
  inner_shared();
}

fn outer2() {
  inner2();
  inner_shared();
}

@compute @workgroup_size(1)
fn comp_main1() {
  outer1();
}

@compute @workgroup_size(1)
fn comp_main2() {
  outer2();
}
)";

    auto* expect = R"(
fn inner1() {
}

fn inner_shared() {
}

fn outer1() {
  inner1();
  inner_shared();
}

@compute @workgroup_size(1)
fn comp_main1() {
  outer1();
}
)";

    SingleEntryPoint::Config cfg("comp_main1");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, GlobalsReferencedByCalledFunctions) {
    auto* src = R"(
var<private> inner1_var : f32;

var<private> inner2_var : f32;

var<private> inner_shared_var : f32;

var<private> outer1_var : f32;

var<private> outer2_var : f32;

fn inner1() {
  inner1_var = 0.0;
}

fn inner2() {
  inner2_var = 0.0;
}

fn inner_shared() {
  inner_shared_var = 0.0;
}

fn outer1() {
  inner1();
  inner_shared();
  outer1_var = 0.0;
}

fn outer2() {
  inner2();
  inner_shared();
  outer2_var = 0.0;
}

@compute @workgroup_size(1)
fn comp_main1() {
  outer1();
}

@compute @workgroup_size(1)
fn comp_main2() {
  outer2();
}
)";

    auto* expect = R"(
var<private> inner1_var : f32;

var<private> inner_shared_var : f32;

var<private> outer1_var : f32;

fn inner1() {
  inner1_var = 0.0;
}

fn inner_shared() {
  inner_shared_var = 0.0;
}

fn outer1() {
  inner1();
  inner_shared();
  outer1_var = 0.0;
}

@compute @workgroup_size(1)
fn comp_main1() {
  outer1();
}
)";

    SingleEntryPoint::Config cfg("comp_main1");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, GlobalConstUsedAsArraySize) {
    // See crbug.com/tint/1598
    auto* src = R"(
const MY_SIZE = 5u;

alias Arr = array<i32, MY_SIZE>;

@fragment
fn main() {
}
)";

    auto* expect = src;

    SingleEntryPoint::Config cfg("main");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SingleEntryPointTest, Directives) {
    // Make sure that directives are preserved.
    auto* src = R"(
enable f16;
diagnostic(off, derivative_uniformity);

@compute @workgroup_size(1)
fn main() {
}
)";

    SingleEntryPoint::Config cfg("main");

    DataMap data;
    data.Add<SingleEntryPoint::Config>(cfg);
    auto got = Run<SingleEntryPoint>(src, data);

    EXPECT_EQ(src, str(got));
}

}  // namespace
}  // namespace tint::transform
