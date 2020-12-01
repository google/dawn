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

#include "src/ast/case_statement.h"

#include "gtest/gtest.h"
#include "src/reader/wgsl/parser.h"
#include "src/writer/wgsl/generator.h"

namespace tint {
namespace ast {
namespace {

TEST(ModuleCloneTest, Clone) {
#if TINT_BUILD_WGSL_READER && TINT_BUILD_WGSL_WRITER
  // Shader that exercises the bulk of the AST nodes and types.
  // See also fuzzers/tint_ast_clone_fuzzer.cc for further coverage of cloning.
  Source::File file("test.wgsl", R"([[block]]
struct S {
  [[offset(0)]]
  m0 : u32;
  [[offset(4)]]
  m1 : array<u32>;
};

type t0 = [[stride(16)]] array<vec4<f32>>;
type t1 = [[stride(32)]] array<vec4<f32>>;

const c0 : i32 = 10;
const c1 : bool = true;

var<uniform> g0 : u32 = 20u;
var<out> g1 : f32 = 123.0;
var<uniform> g2 : texture_2d<f32>;
var<uniform> g3 : texture_storage_ro_2d<r32uint>;
var<uniform> g4 : texture_storage_wo_2d<rg32float>;
var<uniform> g5 : texture_storage_ro_2d<r32uint>;
var<uniform> g6 : texture_storage_wo_2d<rg32float>;

[[builtin(position)]] var<uniform> g7 : vec3<f32>;
[[set(10), binding(20)]] var<storage_buffer> g7 : S;
[[set(10), binding(20)]] var<storage_buffer> g8 : [[access(read)]]
S;
[[set(10), binding(20)]] var<storage_buffer> g9 : [[access(read_write)]]
S;

fn f0(p0 : bool) -> f32 {
  if (p0) {
    return 1.0;
  }
  return 0.0;
}

fn f1(p0 : f32, p1 : i32) -> f32 {
  var l0 : i32 = 3;
  var l1 : f32 = 8;
  var l2 : u32 = bitcast<u32>(4);
  var l3 : vec2<u32> = vec2<u32>(l0, l1);
  var l4 : S;
  var l5 : u32 = l4.m1[5];
  var l6 : ptr<private, u32>;
  l6 = null;
  loop {
    l0 = (p1 + 2);
    if (((l0 % 4) == 0)) {
      continue;
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
    case 0: {
      break;
    }
    case 1: {
      return f0(true);
    }
    default: {
      discard;
    }
  }
  return 1.0;
}

[[stage(fragment)]]
fn main() -> void {
  f1(1.0, 2);
}

)");

  // Parse the wgsl, create the src module
  Context ctx;
  reader::wgsl::Parser parser(&ctx, &file);
  ASSERT_TRUE(parser.Parse()) << parser.error();
  auto src = parser.module();

  // Clone the src module to dst
  auto dst = src.Clone();

  // Expect the AST printed with to_str() to match
  EXPECT_EQ(src.to_str(), dst.to_str());

  // Check that none of the AST nodes or type pointers in dst are found in src
  std::unordered_set<ast::Node*> src_nodes;
  for (auto& src_node : src.nodes()) {
    src_nodes.emplace(src_node.get());
  }
  std::unordered_set<ast::type::Type*> src_types;
  for (auto& src_type : src.types()) {
    src_types.emplace(src_type.second.get());
  }
  for (auto& dst_node : dst.nodes()) {
    ASSERT_EQ(src_nodes.count(dst_node.get()), 0u) << dst_node->str();
  }
  for (auto& dst_type : dst.types()) {
    ASSERT_EQ(src_types.count(dst_type.second.get()), 0u)
        << dst_type.second->type_name();
  }

  // Regenerate the wgsl for the src module. We use this instead of the original
  // source so that reformatting doesn't impact the final wgsl comparision.
  // Note that the src module is moved into the generator and this generator has
  // a limited scope, so that the src module is released before we attempt to
  // print the dst module.
  // This guarantee that all the source module nodes and types are destructed
  // and freed.
  // ASAN should error if there's any remaining references in dst when we try to
  // reconstruct the WGSL.
  std::string src_wgsl;
  {
    writer::wgsl::Generator src_gen(std::move(src));
    ASSERT_TRUE(src_gen.Generate());
    src_wgsl = src_gen.result();
  }

  // Print the dst module, check it matches the original source
  writer::wgsl::Generator dst_gen(std::move(dst));
  ASSERT_TRUE(dst_gen.Generate());
  auto dst_wgsl = dst_gen.result();
  ASSERT_EQ(src_wgsl, dst_wgsl);

#else  // #if TINT_BUILD_WGSL_READER && TINT_BUILD_WGSL_WRITER
  GTEST_SKIP() << "ModuleCloneTest requires TINT_BUILD_WGSL_READER and "
                  "TINT_BUILD_WGSL_WRITER to be enabled";
#endif
}

}  // namespace
}  // namespace ast
}  // namespace tint
