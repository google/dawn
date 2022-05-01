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
@stage(vertex)
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

@stage(fragment)
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
@stage(compute) @workgroup_size(1)
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
@stage(vertex)
fn vert_main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}

@stage(fragment)
fn frag_main() {
}

@stage(compute) @workgroup_size(1)
fn comp_main1() {
}

@stage(compute) @workgroup_size(1)
fn comp_main2() {
}
)";

    auto* expect = R"(
@stage(compute) @workgroup_size(1)
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

@stage(vertex)
fn vert_main() -> @builtin(position) vec4<f32> {
  a = 0.0;
  return vec4<f32>();
}

@stage(fragment)
fn frag_main() {
  b = 0.0;
}

@stage(compute) @workgroup_size(1)
fn comp_main1() {
  c = 0.0;
}

@stage(compute) @workgroup_size(1)
fn comp_main2() {
  d = 0.0;
}
)";

    auto* expect = R"(
var<private> c : f32;

@stage(compute) @workgroup_size(1)
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
let a : f32 = 1.0;

let b : f32 = 1.0;

let c : f32 = 1.0;

let d : f32 = 1.0;

@stage(vertex)
fn vert_main() -> @builtin(position) vec4<f32> {
  let local_a : f32 = a;
  return vec4<f32>();
}

@stage(fragment)
fn frag_main() {
  let local_b : f32 = b;
}

@stage(compute) @workgroup_size(1)
fn comp_main1() {
  let local_c : f32 = c;
}

@stage(compute) @workgroup_size(1)
fn comp_main2() {
  let local_d : f32 = d;
}
)";

    auto* expect = R"(
let c : f32 = 1.0;

@stage(compute) @workgroup_size(1)
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

TEST_F(SingleEntryPointTest, WorkgroupSizeLetPreserved) {
    auto* src = R"(
let size : i32 = 1;

@stage(compute) @workgroup_size(size)
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

@stage(compute) @workgroup_size(1)
fn comp_main1() {
    let local_d = c1;
}

@stage(compute) @workgroup_size(1)
fn comp_main2() {
    let local_d = c2;
}

@stage(compute) @workgroup_size(1)
fn comp_main3() {
    let local_d = c3;
}

@stage(compute) @workgroup_size(1)
fn comp_main4() {
    let local_d = c4;
}

@stage(compute) @workgroup_size(1)
fn comp_main5() {
    let local_d = 1u;
}
)";

    {
        SingleEntryPoint::Config cfg("comp_main1");
        auto* expect = R"(
@id(1001) override c1 : u32 = 1u;

@stage(compute) @workgroup_size(1)
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

@stage(compute) @workgroup_size(1)
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

@stage(compute) @workgroup_size(1)
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

@stage(compute) @workgroup_size(1)
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
@stage(compute) @workgroup_size(1)
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

@stage(compute) @workgroup_size(1)
fn comp_main1() {
  outer1();
}

@stage(compute) @workgroup_size(1)
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

@stage(compute) @workgroup_size(1)
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

@stage(compute) @workgroup_size(1)
fn comp_main1() {
  outer1();
}

@stage(compute) @workgroup_size(1)
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

@stage(compute) @workgroup_size(1)
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

}  // namespace
}  // namespace tint::transform
