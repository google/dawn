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

#include "src/clone_context.h"

#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest-spi.h"

#include "src/program_builder.h"

namespace tint {
namespace {

struct Node : public Castable<Node, ast::Node> {
  explicit Node(const Source& source, std::string n) : Base(source), name(n) {}

  std::string name;
  Node* a = nullptr;
  Node* b = nullptr;
  Node* c = nullptr;
  std::vector<Node*> vec;

  Node* Clone(CloneContext* ctx) const override {
    auto* out = ctx->dst->create<Node>(name);
    out->a = ctx->Clone(a);
    out->b = ctx->Clone(b);
    out->c = ctx->Clone(c);
    out->vec = ctx->Clone(vec);
    return out;
  }

  bool IsValid() const override { return true; }
  void to_str(const semantic::Info&, std::ostream&, size_t) const override {}
};

struct Replaceable : public Castable<Replaceable, Node> {
  explicit Replaceable(const Source& source, std::string n) : Base(source, n) {}
};
struct Replacement : public Castable<Replacement, Replaceable> {
  explicit Replacement(const Source& source, std::string n) : Base(source, n) {}
};

struct NotANode : public Castable<NotANode, ast::Node> {
  explicit NotANode(const Source& source) : Base(source) {}

  NotANode* Clone(CloneContext* ctx) const override {
    return ctx->dst->create<NotANode>();
  }

  bool IsValid() const override { return true; }
  void to_str(const semantic::Info&, std::ostream&, size_t) const override {}
};

TEST(CloneContext, Clone) {
  ProgramBuilder builder;
  auto* original_root = builder.create<Node>("root");
  original_root->a = builder.create<Node>("a");
  original_root->a->b = builder.create<Node>("a->b");
  original_root->b = builder.create<Node>("b");
  original_root->b->a = original_root->a;  // Aliased
  original_root->b->b = builder.create<Node>("b->b");
  original_root->c = original_root->b;  // Aliased
  Program original(std::move(builder));

  //                          root
  //        ╭──────────────────┼──────────────────╮
  //       (a)                (b)                (c)
  //        N  <──────┐        N  <───────────────┘
  //   ╭────┼────╮    │   ╭────┼────╮
  //  (a)  (b)  (c)   │  (a)  (b)  (c)
  //        N         └───┘    N
  //
  // N: Node

  ProgramBuilder cloned;
  auto* cloned_root = CloneContext(&cloned, &original).Clone(original_root);

  EXPECT_NE(cloned_root->a, nullptr);
  EXPECT_EQ(cloned_root->a->a, nullptr);
  EXPECT_NE(cloned_root->a->b, nullptr);
  EXPECT_EQ(cloned_root->a->c, nullptr);
  EXPECT_NE(cloned_root->b, nullptr);
  EXPECT_NE(cloned_root->b->a, nullptr);
  EXPECT_NE(cloned_root->b->b, nullptr);
  EXPECT_EQ(cloned_root->b->c, nullptr);
  EXPECT_NE(cloned_root->c, nullptr);

  EXPECT_NE(cloned_root->a, original_root->a);
  EXPECT_NE(cloned_root->a->b, original_root->a->b);
  EXPECT_NE(cloned_root->b, original_root->b);
  EXPECT_NE(cloned_root->b->a, original_root->b->a);
  EXPECT_NE(cloned_root->b->b, original_root->b->b);
  EXPECT_NE(cloned_root->c, original_root->c);

  EXPECT_EQ(cloned_root->name, "root");
  EXPECT_EQ(cloned_root->a->name, "a");
  EXPECT_EQ(cloned_root->a->b->name, "a->b");
  EXPECT_EQ(cloned_root->b->name, "b");
  EXPECT_EQ(cloned_root->b->b->name, "b->b");

  EXPECT_EQ(cloned_root->b->a, cloned_root->a);  // Aliased
  EXPECT_EQ(cloned_root->c, cloned_root->b);     // Aliased
}

TEST(CloneContext, CloneWithReplacements) {
  ProgramBuilder builder;
  auto* original_root = builder.create<Node>("root");
  original_root->a = builder.create<Node>("a");
  original_root->a->b = builder.create<Replaceable>("a->b");
  original_root->b = builder.create<Replaceable>("b");
  original_root->b->a = original_root->a;  // Aliased
  original_root->c = original_root->b;     // Aliased
  Program original(std::move(builder));

  //                          root
  //        ╭──────────────────┼──────────────────╮
  //       (a)                (b)                (c)
  //        N  <──────┐        R  <───────────────┘
  //   ╭────┼────╮    │   ╭────┼────╮
  //  (a)  (b)  (c)   │  (a)  (b)  (c)
  //        R         └───┘
  //
  // N: Node
  // R: Replaceable

  ProgramBuilder cloned;
  auto* cloned_root =
      CloneContext(&cloned, &original)
          .ReplaceAll([&](CloneContext* ctx, Replaceable* in) {
            auto* out = cloned.create<Replacement>("replacement:" + in->name);
            out->b = cloned.create<Node>("replacement-child:" + in->name);
            out->c = ctx->Clone(in->a);
            return out;
          })
          .Clone(original_root);

  //                         root
  //        ╭─────────────────┼──────────────────╮
  //       (a)               (b)                (c)
  //        N  <──────┐       R  <───────────────┘
  //   ╭────┼────╮    │  ╭────┼────╮
  //  (a)  (b)  (c)   │ (a)  (b)  (c)
  //        R         │       N    |
  //   ╭────┼────╮    └────────────┘
  //  (a)  (b)  (c)
  //        N
  //
  // N: Node
  // R: Replacement

  EXPECT_NE(cloned_root->a, nullptr);
  EXPECT_EQ(cloned_root->a->a, nullptr);
  EXPECT_NE(cloned_root->a->b, nullptr);     // Replaced
  EXPECT_EQ(cloned_root->a->b->a, nullptr);  // From replacement
  EXPECT_NE(cloned_root->a->b->b, nullptr);  // From replacement
  EXPECT_EQ(cloned_root->a->b->c, nullptr);  // From replacement
  EXPECT_EQ(cloned_root->a->c, nullptr);
  EXPECT_NE(cloned_root->b, nullptr);
  EXPECT_EQ(cloned_root->b->a, nullptr);  // From replacement
  EXPECT_NE(cloned_root->b->b, nullptr);  // From replacement
  EXPECT_NE(cloned_root->b->c, nullptr);  // From replacement
  EXPECT_NE(cloned_root->c, nullptr);

  EXPECT_NE(cloned_root->a, original_root->a);
  EXPECT_NE(cloned_root->a->b, original_root->a->b);
  EXPECT_NE(cloned_root->b, original_root->b);
  EXPECT_NE(cloned_root->b->a, original_root->b->a);
  EXPECT_NE(cloned_root->c, original_root->c);

  EXPECT_EQ(cloned_root->name, "root");
  EXPECT_EQ(cloned_root->a->name, "a");
  EXPECT_EQ(cloned_root->a->b->name, "replacement:a->b");
  EXPECT_EQ(cloned_root->a->b->b->name, "replacement-child:a->b");
  EXPECT_EQ(cloned_root->b->name, "replacement:b");
  EXPECT_EQ(cloned_root->b->b->name, "replacement-child:b");

  EXPECT_EQ(cloned_root->b->c, cloned_root->a);  // Aliased
  EXPECT_EQ(cloned_root->c, cloned_root->b);     // Aliased

  EXPECT_FALSE(cloned_root->a->Is<Replacement>());
  EXPECT_TRUE(cloned_root->a->b->Is<Replacement>());
  EXPECT_FALSE(cloned_root->a->b->b->Is<Replacement>());
  EXPECT_TRUE(cloned_root->b->Is<Replacement>());
  EXPECT_FALSE(cloned_root->b->b->Is<Replacement>());
}

TEST(CloneContext, CloneWithReplace) {
  ProgramBuilder builder;
  auto* original_root = builder.create<Node>("root");
  original_root->a = builder.create<Node>("a");
  original_root->b = builder.create<Node>("b");
  original_root->c = builder.create<Node>("c");
  Program original(std::move(builder));

  //                          root
  //        ╭──────────────────┼──────────────────╮
  //       (a)                (b)                (c)
  //                        Replaced

  ProgramBuilder cloned;
  auto* replacement = cloned.create<Node>("replacement");

  auto* cloned_root = CloneContext(&cloned, &original)
                          .Replace(original_root->b, replacement)
                          .Clone(original_root);

  EXPECT_NE(cloned_root->a, replacement);
  EXPECT_EQ(cloned_root->b, replacement);
  EXPECT_NE(cloned_root->c, replacement);

  EXPECT_EQ(cloned_root->name, "root");
  EXPECT_EQ(cloned_root->a->name, "a");
  EXPECT_EQ(cloned_root->b->name, "replacement");
  EXPECT_EQ(cloned_root->c->name, "c");
}

TEST(CloneContext, CloneWithInsertBefore) {
  ProgramBuilder builder;
  auto* original_root = builder.create<Node>("root");
  original_root->a = builder.create<Node>("a");
  original_root->b = builder.create<Node>("b");
  original_root->c = builder.create<Node>("c");
  original_root->vec = {original_root->a, original_root->b, original_root->c};
  Program original(std::move(builder));

  ProgramBuilder cloned;
  auto* insertion = cloned.create<Node>("insertion");

  auto* cloned_root = CloneContext(&cloned, &original)
                          .InsertBefore(original_root->b, insertion)
                          .Clone(original_root);

  EXPECT_EQ(cloned_root->vec.size(), 4u);
  EXPECT_EQ(cloned_root->vec[0], cloned_root->a);
  EXPECT_EQ(cloned_root->vec[2], cloned_root->b);
  EXPECT_EQ(cloned_root->vec[3], cloned_root->c);

  EXPECT_EQ(cloned_root->name, "root");
  EXPECT_EQ(cloned_root->vec[0]->name, "a");
  EXPECT_EQ(cloned_root->vec[1]->name, "insertion");
  EXPECT_EQ(cloned_root->vec[2]->name, "b");
  EXPECT_EQ(cloned_root->vec[3]->name, "c");
}

TEST(CloneContext, CloneWithReplace_WithNotANode) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder builder;
        auto* original_root = builder.create<Node>("root");
        original_root->a = builder.create<Node>("a");
        original_root->b = builder.create<Node>("b");
        original_root->c = builder.create<Node>("c");
        Program original(std::move(builder));

        //                          root
        //        ╭──────────────────┼──────────────────╮
        //       (a)                (b)                (c)
        //                        Replaced

        ProgramBuilder cloned;
        auto* replacement = cloned.create<NotANode>();

        CloneContext ctx(&cloned, &original);
        ctx.Replace(original_root->b, replacement);

        ctx.Clone(original_root);
      },
      "internal compiler error");

  // Ensure that this test does not leak memory.
  // This will be automatically called by main() in src/test_main.cc, but
  // chromium uses it's own test entry point.
  // TODO(ben-clayton): Add this call to the end of Chromium's main(), and we
  // can remove this call.
  FreeInternalCompilerErrors();
}

}  // namespace

TINT_INSTANTIATE_CLASS_ID(Node);
TINT_INSTANTIATE_CLASS_ID(Replaceable);
TINT_INSTANTIATE_CLASS_ID(Replacement);
TINT_INSTANTIATE_CLASS_ID(NotANode);

}  // namespace tint
