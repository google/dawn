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

#include <iostream>
#include <string>
#include <unordered_set>

#include "src/demangler.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/writer/wgsl/generator.h"

#define ASSERT_EQ(A, B)                                  \
  do {                                                   \
    decltype(A) assert_a = (A);                          \
    decltype(B) assert_b = (B);                          \
    if (assert_a != assert_b) {                          \
      std::cerr << "ASSERT_EQ(" #A ", " #B ") failed:\n" \
                << #A << " was: " << assert_a << "\n"    \
                << #B << " was: " << assert_b << "\n";   \
      __builtin_trap();                                  \
    }                                                    \
  } while (false)

#define ASSERT_TRUE(A)                                 \
  do {                                                 \
    decltype(A) assert_a = (A);                        \
    if (!assert_a) {                                   \
      std::cerr << "ASSERT_TRUE(" #A ") failed:\n"     \
                << #A << " was: " << assert_a << "\n"; \
      __builtin_trap();                                \
    }                                                  \
  } while (false)

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string str(reinterpret_cast<const char*>(data), size);

  tint::Source::File file("test.wgsl", str);

  // Parse the wgsl, create the src module
  tint::reader::wgsl::ParserImpl parser(&file);
  parser.set_max_errors(1);
  if (!parser.Parse()) {
    return 0;
  }
  auto src = parser.module();

  // Clone the src module to dst
  auto dst = src.Clone();

  // Expect the demangled AST printed with to_str() to match
  tint::Demangler d;
  ASSERT_EQ(d.Demangle(src, src.to_str()), d.Demangle(dst, dst.to_str()));

  // Check that none of the AST nodes or type pointers in dst are found in src
  std::unordered_set<tint::ast::Node*> src_nodes;
  for (auto& src_node : src.nodes()) {
    src_nodes.emplace(src_node.get());
  }
  std::unordered_set<tint::ast::type::Type*> src_types;
  for (auto& src_type : src.types()) {
    src_types.emplace(src_type.second.get());
  }
  for (auto& dst_node : dst.nodes()) {
    ASSERT_EQ(src_nodes.count(dst_node.get()), 0u);
  }
  for (auto& dst_type : dst.types()) {
    ASSERT_EQ(src_types.count(dst_type.second.get()), 0u);
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
    tint::writer::wgsl::Generator src_gen(std::move(src));
    ASSERT_TRUE(src_gen.Generate());
    src_wgsl = src_gen.result();
  }

  // Print the dst module, check it matches the original source
  tint::writer::wgsl::Generator dst_gen(std::move(dst));
  ASSERT_TRUE(dst_gen.Generate());
  auto dst_wgsl = dst_gen.result();
  ASSERT_EQ(src_wgsl, dst_wgsl);

  return 0;
}
