// Copyright 2020 The Tint Authors.
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

#include <unordered_set>

#include "gtest/gtest.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/writer/wgsl/generator.h"

namespace tint::ast {
namespace {

TEST(ModuleCloneTest, Clone) {
    // Shader that exercises the bulk of the AST nodes and types.
    // See also fuzzers/tint_ast_clone_fuzzer.cc for further coverage of cloning.
    Source::File file("test.wgsl", R"(enable f16;
diagnostic(off, chromium.unreachable_code);

struct S0 {
  @size(4)
  m0 : u32,
  m1 : array<u32>,
};

struct S1 {
  @size(4)
  m0 : u32,
  m1 : array<u32, 6>,
};

const c0 : i32 = 10;
const c1 : bool = true;

alias t0 = array<vec4<f32>>;
alias t1 = array<vec4<f32>>;

var<private> g0 : u32 = 20u;
var<private> g1 : f32 = 123.0;
@group(0) @binding(0) var g2 : texture_2d<f32>;
@group(1) @binding(0) var g3 : texture_depth_2d;
@group(2) @binding(0) var g4 : texture_storage_2d<rg32float, write>;
@group(3) @binding(0) var g5 : texture_depth_cube_array;
@group(4) @binding(0) var g6 : texture_external;

var<private> g7 : vec3<f32>;
@group(0) @binding(1) var<storage, read_write> g8 : S0;
@group(1) @binding(1) var<storage, read> g9 : S0;
@group(2) @binding(1) var<storage, read_write> g10 : S0;

fn f0(p0 : bool) -> f32 {
  if (p0) {
    return 1.0;
  }
  return 0.0;
}

@diagnostic(warning, chromium.unreachable_code)
fn f1(p0 : f32, p1 : i32) -> f32 {
  var l0 : i32 = 3;
  var l1 : f32 = 8.0;
  var l2 : u32 = bitcast<u32>(4);
  var l3 : vec2<u32> = vec2<u32>(u32(l0), u32(l1));
  var l4 : S1;
  var l5 : u32 = l4.m1[5];
  let l6 : ptr<private, u32> = &g0;
  const l7 = 123;
  const l8 : i32 = 123;
  loop {
    l0 = (p1 + 2);
    if (((l0 % 4) == 0)) {
      break;
    }

    continuing {
      if (1 == 2) {
        l0 = l0 - 1;
      } else {
        l0 = l0 - 2;
      }
    }
  }
  switch(l2) {
    case 0u: {
      break;
    }
    case 1u: {
      return f0(true);
    }
    default: {
      discard;
    }
  }
  return 1.0;
}

@fragment
fn main() {
  f1(1.0, 2);
}

const declaration_order_check_0 : i32 = 1;

alias declaration_order_check_1 = f32;

fn declaration_order_check_2() {}

alias declaration_order_check_3 = f32;

const declaration_order_check_4 : i32 = 1;

)");

    // Parse the wgsl, create the src program
    auto src = reader::wgsl::Parse(&file);

    ASSERT_TRUE(src.IsValid()) << diag::Formatter().format(src.Diagnostics());

    // Clone the src program to dst
    Program dst(src.Clone());

    ASSERT_TRUE(dst.IsValid()) << diag::Formatter().format(dst.Diagnostics());

    // Expect the printed strings to match
    EXPECT_EQ(Program::printer(&src), Program::printer(&dst));

    // Check that none of the AST nodes or type pointers in dst are found in src
    std::unordered_set<const Node*> src_nodes;
    for (auto* src_node : src.ASTNodes().Objects()) {
        src_nodes.emplace(src_node);
    }
    std::unordered_set<const type::Type*> src_types;
    for (auto* src_type : src.Types()) {
        src_types.emplace(src_type);
    }
    for (auto* dst_node : dst.ASTNodes().Objects()) {
        ASSERT_EQ(src_nodes.count(dst_node), 0u);
    }
    for (auto* dst_type : dst.Types()) {
        ASSERT_EQ(src_types.count(dst_type), 0u);
    }

    // Regenerate the wgsl for the src program. We use this instead of the
    // original source so that reformatting doesn't impact the final wgsl
    // comparison.
    writer::wgsl::Options options;
    std::string src_wgsl;
    {
        auto result = writer::wgsl::Generate(&src, options);
        ASSERT_TRUE(result.success) << result.error;
        src_wgsl = result.wgsl;

        // Move the src program to a temporary that'll be dropped, so that the src
        // program is released before we attempt to print the dst program. This
        // guarantee that all the source program nodes and types are destructed and
        // freed. ASAN should error if there's any remaining references in dst when
        // we try to reconstruct the WGSL.
        auto tmp = std::move(src);
    }

    // Print the dst module, check it matches the original source
    auto result = writer::wgsl::Generate(&dst, options);
    ASSERT_TRUE(result.success);
    auto dst_wgsl = result.wgsl;
    ASSERT_EQ(src_wgsl, dst_wgsl);
}

}  // namespace
}  // namespace tint::ast
