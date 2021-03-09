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

#include "gtest/gtest-spi.h"

#include "src/program_builder.h"

namespace tint {
namespace {

struct Node : public Castable<Node, ast::Node> {
  explicit Node(const Source& source, Symbol n) : Base(source), name(n) {}

  Symbol name;
  Node* a = nullptr;
  Node* b = nullptr;
  Node* c = nullptr;
  std::vector<Node*> vec;

  Node* Clone(CloneContext* ctx) const override {
    auto* out = ctx->dst->create<Node>(ctx->Clone(name));
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
  explicit Replaceable(const Source& source, Symbol n) : Base(source, n) {}
};
struct Replacement : public Castable<Replacement, Replaceable> {
  explicit Replacement(const Source& source, Symbol n) : Base(source, n) {}
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
  auto* original_root =
      builder.create<Node>(builder.Symbols().Register("root"));
  original_root->a = builder.create<Node>(builder.Symbols().Register("a"));
  original_root->a->b =
      builder.create<Node>(builder.Symbols().Register("a->b"));
  original_root->b = builder.create<Node>(builder.Symbols().Register("b"));
  original_root->b->a = original_root->a;  // Aliased
  original_root->b->b =
      builder.create<Node>(builder.Symbols().Register("b->b"));
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

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->a->b->name, cloned.Symbols().Get("a->b"));
  EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("b"));
  EXPECT_EQ(cloned_root->b->b->name, cloned.Symbols().Get("b->b"));

  EXPECT_EQ(cloned_root->b->a, cloned_root->a);  // Aliased
  EXPECT_EQ(cloned_root->c, cloned_root->b);     // Aliased
}

TEST(CloneContext, CloneWithReplaceAll_Cloneable) {
  ProgramBuilder builder;
  auto* original_root =
      builder.create<Node>(builder.Symbols().Register("root"));
  original_root->a = builder.create<Node>(builder.Symbols().Register("a"));
  original_root->a->b =
      builder.create<Replaceable>(builder.Symbols().Register("a->b"));
  original_root->b =
      builder.create<Replaceable>(builder.Symbols().Register("b"));
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

  CloneContext ctx(&cloned, &original);
  ctx.ReplaceAll([&](Replaceable* in) {
    auto out_name = cloned.Symbols().Register(
        "replacement:" + original.Symbols().NameFor(in->name));
    auto b_name = cloned.Symbols().Register(
        "replacement-child:" + original.Symbols().NameFor(in->name));
    auto* out = cloned.create<Replacement>(out_name);
    out->b = cloned.create<Node>(b_name);
    out->c = ctx.Clone(in->a);
    return out;
  });
  auto* cloned_root = ctx.Clone(original_root);

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

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->a->b->name, cloned.Symbols().Get("replacement:a->b"));
  EXPECT_EQ(cloned_root->a->b->b->name,
            cloned.Symbols().Get("replacement-child:a->b"));
  EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("replacement:b"));
  EXPECT_EQ(cloned_root->b->b->name,
            cloned.Symbols().Get("replacement-child:b"));

  EXPECT_EQ(cloned_root->b->c, cloned_root->a);  // Aliased
  EXPECT_EQ(cloned_root->c, cloned_root->b);     // Aliased

  EXPECT_FALSE(cloned_root->a->Is<Replacement>());
  EXPECT_TRUE(cloned_root->a->b->Is<Replacement>());
  EXPECT_FALSE(cloned_root->a->b->b->Is<Replacement>());
  EXPECT_TRUE(cloned_root->b->Is<Replacement>());
  EXPECT_FALSE(cloned_root->b->b->Is<Replacement>());
}

TEST(CloneContext, CloneWithReplaceAll_Symbols) {
  ProgramBuilder builder;
  auto* original_root =
      builder.create<Node>(builder.Symbols().Register("root"));
  original_root->a = builder.create<Node>(builder.Symbols().Register("a"));
  original_root->a->b =
      builder.create<Node>(builder.Symbols().Register("a->b"));
  original_root->b = builder.create<Node>(builder.Symbols().Register("b"));
  original_root->b->a = original_root->a;  // Aliased
  original_root->b->b =
      builder.create<Node>(builder.Symbols().Register("b->b"));
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
  auto* cloned_root = CloneContext(&cloned, &original)
                          .ReplaceAll([&](Symbol sym) {
                            auto in = original.Symbols().NameFor(sym);
                            auto out = "transformed<" + in + ">";
                            return cloned.Symbols().Register(out);
                          })
                          .Clone(original_root);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("transformed<root>"));
  EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("transformed<a>"));
  EXPECT_EQ(cloned_root->a->b->name, cloned.Symbols().Get("transformed<a->b>"));
  EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("transformed<b>"));
  EXPECT_EQ(cloned_root->b->b->name, cloned.Symbols().Get("transformed<b->b>"));
}

TEST(CloneContext, CloneWithReplace) {
  ProgramBuilder builder;
  auto* original_root =
      builder.create<Node>(builder.Symbols().Register("root"));
  original_root->a = builder.create<Node>(builder.Symbols().Register("a"));
  original_root->b = builder.create<Node>(builder.Symbols().Register("b"));
  original_root->c = builder.create<Node>(builder.Symbols().Register("c"));
  Program original(std::move(builder));

  //                          root
  //        ╭──────────────────┼──────────────────╮
  //       (a)                (b)                (c)
  //                        Replaced

  ProgramBuilder cloned;
  auto* replacement =
      cloned.create<Node>(cloned.Symbols().Register("replacement"));

  auto* cloned_root = CloneContext(&cloned, &original)
                          .Replace(original_root->b, replacement)
                          .Clone(original_root);

  EXPECT_NE(cloned_root->a, replacement);
  EXPECT_EQ(cloned_root->b, replacement);
  EXPECT_NE(cloned_root->c, replacement);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("replacement"));
  EXPECT_EQ(cloned_root->c->name, cloned.Symbols().Get("c"));
}

TEST(CloneContext, CloneWithInsertBefore) {
  ProgramBuilder builder;
  auto* original_root =
      builder.create<Node>(builder.Symbols().Register("root"));
  original_root->a = builder.create<Node>(builder.Symbols().Register("a"));
  original_root->b = builder.create<Node>(builder.Symbols().Register("b"));
  original_root->c = builder.create<Node>(builder.Symbols().Register("c"));
  original_root->vec = {original_root->a, original_root->b, original_root->c};
  Program original(std::move(builder));

  ProgramBuilder cloned;
  auto* insertion = cloned.create<Node>(cloned.Symbols().Register("insertion"));

  auto* cloned_root = CloneContext(&cloned, &original)
                          .InsertBefore(original_root->b, insertion)
                          .Clone(original_root);

  EXPECT_EQ(cloned_root->vec.size(), 4u);
  EXPECT_EQ(cloned_root->vec[0], cloned_root->a);
  EXPECT_EQ(cloned_root->vec[2], cloned_root->b);
  EXPECT_EQ(cloned_root->vec[3], cloned_root->c);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion"));
  EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
  EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST(CloneContext, CloneWithReplaceAll_SameTypeTwice) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder cloned;
        Program original;
        CloneContext ctx(&cloned, &original);
        ctx.ReplaceAll([](Node*) { return nullptr; });
        ctx.ReplaceAll([](Node*) { return nullptr; });
      },
      "internal compiler error: ReplaceAll() called with a handler for type "
      "Node that is already handled by a handler for type Node");
}

TEST(CloneContext, CloneWithReplaceAll_BaseThenDerived) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder cloned;
        Program original;
        CloneContext ctx(&cloned, &original);
        ctx.ReplaceAll([](Node*) { return nullptr; });
        ctx.ReplaceAll([](Replaceable*) { return nullptr; });
      },
      "internal compiler error: ReplaceAll() called with a handler for type "
      "Replaceable that is already handled by a handler for type Node");
}

TEST(CloneContext, CloneWithReplaceAll_DerivedThenBase) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder cloned;
        Program original;
        CloneContext ctx(&cloned, &original);
        ctx.ReplaceAll([](Replaceable*) { return nullptr; });
        ctx.ReplaceAll([](Node*) { return nullptr; });
      },
      "internal compiler error: ReplaceAll() called with a handler for type "
      "Node that is already handled by a handler for type Replaceable");
}

TEST(CloneContext, CloneWithReplaceAll_SymbolsTwice) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder cloned;
        Program original;
        CloneContext ctx(&cloned, &original);
        ctx.ReplaceAll([](Symbol s) { return s; });
        ctx.ReplaceAll([](Symbol s) { return s; });
      },
      "internal compiler error: ReplaceAll(const SymbolTransform&) called "
      "multiple times on the same CloneContext");
}

TEST(CloneContext, CloneWithReplace_WithNotANode) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder builder;
        auto* original_root =
            builder.create<Node>(builder.Symbols().Register("root"));
        original_root->a =
            builder.create<Node>(builder.Symbols().Register("a"));
        original_root->b =
            builder.create<Node>(builder.Symbols().Register("b"));
        original_root->c =
            builder.create<Node>(builder.Symbols().Register("c"));
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
}

}  // namespace

TINT_INSTANTIATE_TYPEINFO(Node);
TINT_INSTANTIATE_TYPEINFO(Replaceable);
TINT_INSTANTIATE_TYPEINFO(Replacement);
TINT_INSTANTIATE_TYPEINFO(NotANode);

}  // namespace tint
