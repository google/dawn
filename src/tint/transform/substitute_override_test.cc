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

#include "src/tint/transform/substitute_override.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using SubstituteOverrideTest = TransformTest;

TEST_F(SubstituteOverrideTest, Error_NoData) {
    auto* src = R"(
override width: i32;
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = "error: Missing override substitution data";

    DataMap data;
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Error_NoOverrideValue) {
    auto* src = R"(
override width: i32;
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = "error: Initializer not provided for override, and override not overridden.";

    SubstituteOverride::Config cfg;
    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);

    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Module_NoOverrides) {
    auto* src = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Identifier) {
    auto* src = R"(
override i_width: i32;
override i_height = 1i;

override f_width: f32;
override f_height = 1.f;

// TODO(crbug.com/tint/1473)
// override h_width: f16;
// override h_height = 1.h;

override b_width: bool;
override b_height = true;

override o_width = 2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
const i_width : i32 = 42i;

const i_height = 11i;

const f_width : f32 = 22.299999237f;

const f_height = 12.399999619f;

const b_width : bool = true;

const b_height = false;

const o_width = 2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({"i_width", 42.0});
    cfg.map.insert({"i_height", 11.0});
    cfg.map.insert({"f_width", 22.3});
    cfg.map.insert({"f_height", 12.4});
    cfg.map.insert({"h_width", 9.4});
    cfg.map.insert({"h_height", 3.4});
    cfg.map.insert({"b_width", 1.0});
    cfg.map.insert({"b_height", 0.0});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Id) {
    auto* src = R"(
enable f16;

@id(0) override i_width: i32;
@id(10) override i_height = 1i;

@id(1) override f_width: f32;
@id(9) override f_height = 1.f;

// TODO(crbug.com/tint/1473)
// @id(2) override h_width: f16;
// @id(8) override h_height = 1.h;

@id(3) override b_width: bool;
@id(7) override b_height = true;

@id(5) override o_width = 2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
enable f16;

const i_width : i32 = 42i;

const i_height = 11i;

const f_width : f32 = 22.299999237f;

const f_height = 12.399999619f;

const b_width : bool = true;

const b_height = false;

const o_width = 2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({"0", 42.0});
    cfg.map.insert({"10", 11.0});
    cfg.map.insert({"1", 22.3});
    cfg.map.insert({"9", 12.4});
    cfg.map.insert({"2", 9.4});
    cfg.map.insert({"8", 3.4});
    cfg.map.insert({"3", 1.0});
    cfg.map.insert({"7", 0.0});
    // No effect because an @id is set for o_width
    cfg.map.insert({"o_width", 13});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(SubstituteOverrideTest, Identifier_Expression) {
    auto* src = R"(
override i_height = ~2i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    auto* expect = R"(
const i_height = 11i;

@vertex
fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)";

    SubstituteOverride::Config cfg;
    cfg.map.insert({"i_height", 11.0});

    DataMap data;
    data.Add<SubstituteOverride::Config>(cfg);
    auto got = Run<SubstituteOverride>(src, data);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
