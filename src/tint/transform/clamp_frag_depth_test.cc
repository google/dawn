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

#include "src/tint/transform/clamp_frag_depth.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using ClampFragDepthTest = TransformTest;

TEST_F(ClampFragDepthTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<ClampFragDepth>(src));
}

TEST_F(ClampFragDepthTest, ShouldRunNoFragmentShader) {
    auto* src = R"(
        fn f() -> f32 {
            return 0.0;
        }

        @compute @workgroup_size(1) fn cs() {
        }

        @vertex fn vs() -> @builtin(position) vec4<f32> {
            return vec4<f32>();
        }
    )";

    EXPECT_FALSE(ShouldRun<ClampFragDepth>(src));
}

TEST_F(ClampFragDepthTest, ShouldRunFragmentShaderNoReturnType) {
    auto* src = R"(
        @fragment fn main() {
        }
    )";

    EXPECT_FALSE(ShouldRun<ClampFragDepth>(src));
}

TEST_F(ClampFragDepthTest, ShouldRunFragmentShaderNoFragDepth) {
    auto* src = R"(
        @fragment fn main() -> @location(0) f32 {
            return 0.0;
        }

        struct S {
            @location(0) a : f32,
            @builtin(sample_mask) b : u32,
        }
        @fragment fn main2() -> S {
            return S();
        }
    )";

    EXPECT_FALSE(ShouldRun<ClampFragDepth>(src));
}

TEST_F(ClampFragDepthTest, ShouldRunFragDepthAsDirectReturn) {
    auto* src = R"(
        @fragment fn main() -> @builtin(frag_depth) f32 {
            return 0.0;
        }
    )";

    EXPECT_TRUE(ShouldRun<ClampFragDepth>(src));
}

TEST_F(ClampFragDepthTest, ShouldRunFragDepthInStruct) {
    auto* src = R"(
        struct S {
            @location(0) a : f32,
            @builtin(frag_depth) b : f32,
            @location(1) c : f32,
        }
        @fragment fn main() -> S {
            return S();
        }
    )";

    EXPECT_TRUE(ShouldRun<ClampFragDepth>(src));
}

TEST_F(ClampFragDepthTest, SingleReturnOfFragDepth) {
    auto* src = R"(
        @fragment fn main() -> @builtin(frag_depth) f32 {
            return 0.0;
        }
    )";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct FragDepthClampArgs {
  min : f32,
  max : f32,
}

var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;

fn clamp_frag_depth(v : f32) -> f32 {
  return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
}

@fragment
fn main() -> @builtin(frag_depth) f32 {
  return clamp_frag_depth(0.0);
}
)";

    auto got = Run<ClampFragDepth>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(ClampFragDepthTest, MultipleReturnOfFragDepth) {
    auto* src = R"(
        @fragment fn main() -> @builtin(frag_depth) f32 {
            if (false) {
                return 1.0;
            }
            return 0.0;
        }
    )";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct FragDepthClampArgs {
  min : f32,
  max : f32,
}

var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;

fn clamp_frag_depth(v : f32) -> f32 {
  return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
}

@fragment
fn main() -> @builtin(frag_depth) f32 {
  if (false) {
    return clamp_frag_depth(1.0);
  }
  return clamp_frag_depth(0.0);
}
)";

    auto got = Run<ClampFragDepth>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(ClampFragDepthTest, OtherFunctionWithoutFragDepth) {
    auto* src = R"(
        @fragment fn main() -> @builtin(frag_depth) f32 {
            return 0.0;
        }
        @fragment fn other() -> @location(0) f32 {
            return 0.0;
        }
    )";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct FragDepthClampArgs {
  min : f32,
  max : f32,
}

var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;

fn clamp_frag_depth(v : f32) -> f32 {
  return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
}

@fragment
fn main() -> @builtin(frag_depth) f32 {
  return clamp_frag_depth(0.0);
}

@fragment
fn other() -> @location(0) f32 {
  return 0.0;
}
)";

    auto got = Run<ClampFragDepth>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(ClampFragDepthTest, SimpleReturnOfStruct) {
    auto* src = R"(
        struct S {
            @builtin(frag_depth) frag_depth : f32,
        }

        @fragment fn main() -> S {
            return S(0.0);
        }
    )";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct FragDepthClampArgs {
  min : f32,
  max : f32,
}

var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;

fn clamp_frag_depth(v : f32) -> f32 {
  return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
}

struct S {
  @builtin(frag_depth)
  frag_depth : f32,
}

fn clamp_frag_depth_S(s : S) -> S {
  return S(clamp_frag_depth(s.frag_depth));
}

@fragment
fn main() -> S {
  return clamp_frag_depth_S(S(0.0));
}
)";

    auto got = Run<ClampFragDepth>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(ClampFragDepthTest, MixOfFunctionReturningStruct) {
    auto* src = R"(
        struct S {
            @builtin(frag_depth) frag_depth : f32,
        }
        struct S2 {
            @builtin(frag_depth) frag_depth : f32,
        }

        @fragment fn returnS() -> S {
            return S(0.0);
        }
        @fragment fn againReturnS() -> S {
            return S(0.0);
        }
        @fragment fn returnS2() -> S2 {
            return S2(0.0);
        }
    )";

    // clamp_frag_depth_S is emitted only once.
    // S2 gets its own clamping function.
    auto* expect = R"(
enable chromium_experimental_push_constant;

struct FragDepthClampArgs {
  min : f32,
  max : f32,
}

var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;

fn clamp_frag_depth(v : f32) -> f32 {
  return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
}

struct S {
  @builtin(frag_depth)
  frag_depth : f32,
}

struct S2 {
  @builtin(frag_depth)
  frag_depth : f32,
}

fn clamp_frag_depth_S(s : S) -> S {
  return S(clamp_frag_depth(s.frag_depth));
}

@fragment
fn returnS() -> S {
  return clamp_frag_depth_S(S(0.0));
}

@fragment
fn againReturnS() -> S {
  return clamp_frag_depth_S(S(0.0));
}

fn clamp_frag_depth_S2(s : S2) -> S2 {
  return S2(clamp_frag_depth(s.frag_depth));
}

@fragment
fn returnS2() -> S2 {
  return clamp_frag_depth_S2(S2(0.0));
}
)";

    auto got = Run<ClampFragDepth>(src);
    EXPECT_EQ(expect, str(got));
}

TEST_F(ClampFragDepthTest, ComplexIOStruct) {
    auto* src = R"(
        struct S {
            @location(0) blou : vec4<f32>,
            @location(1) bi : vec4<f32>,
            @builtin(frag_depth) frag_depth : f32,
            @location(2) boul : i32,
            @builtin(sample_mask) ga : u32,
        }

        @fragment fn main() -> S {
            return S(vec4<f32>(), vec4<f32>(), 0.0, 1, 0u);
        }
    )";

    auto* expect = R"(
enable chromium_experimental_push_constant;

struct FragDepthClampArgs {
  min : f32,
  max : f32,
}

var<push_constant> frag_depth_clamp_args : FragDepthClampArgs;

fn clamp_frag_depth(v : f32) -> f32 {
  return clamp(v, frag_depth_clamp_args.min, frag_depth_clamp_args.max);
}

struct S {
  @location(0)
  blou : vec4<f32>,
  @location(1)
  bi : vec4<f32>,
  @builtin(frag_depth)
  frag_depth : f32,
  @location(2)
  boul : i32,
  @builtin(sample_mask)
  ga : u32,
}

fn clamp_frag_depth_S(s : S) -> S {
  return S(s.blou, s.bi, clamp_frag_depth(s.frag_depth), s.boul, s.ga);
}

@fragment
fn main() -> S {
  return clamp_frag_depth_S(S(vec4<f32>(), vec4<f32>(), 0.0, 1, 0u));
}
)";

    auto got = Run<ClampFragDepth>(src);
    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
