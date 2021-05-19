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

#include "gtest/gtest-spi.h"
#include "src/program_builder.h"

namespace tint {
namespace {

struct Allocator {
  template <typename T, typename... ARGS>
  T* Create(ARGS&&... args) {
    return alloc.Create<T>(this, std::forward<ARGS>(args)...);
  }

 private:
  BlockAllocator<Cloneable> alloc;
};

struct Node : public Castable<Node, Cloneable> {
  Node(Allocator* alloc, Symbol n) : allocator(alloc), name(n) {}
  Allocator* const allocator;
  Symbol name;
  Node* a = nullptr;
  Node* b = nullptr;
  Node* c = nullptr;
  std::vector<Node*> vec;

  Node* Clone(CloneContext* ctx) const override {
    auto* out = allocator->Create<Node>(ctx->Clone(name));
    out->a = ctx->Clone(a);
    out->b = ctx->Clone(b);
    out->c = ctx->Clone(c);
    out->vec = ctx->Clone(vec);
    return out;
  }
};

struct Replaceable : public Castable<Replaceable, Node> {
  Replaceable(Allocator* alloc, Symbol n) : Base(alloc, n) {}
};

struct Replacement : public Castable<Replacement, Replaceable> {
  Replacement(Allocator* alloc, Symbol n) : Base(alloc, n) {}
};

struct NotANode : public Castable<NotANode, Cloneable> {
  explicit NotANode(Allocator* alloc) : allocator(alloc) {}

  Allocator* const allocator;
  NotANode* Clone(CloneContext*) const override {
    return allocator->Create<NotANode>();
  }
};

struct ProgramNode : public Castable<ProgramNode, Cloneable> {
  ProgramNode(Allocator* alloc, ProgramID id, ProgramID cloned_id)
      : allocator(alloc), program_id(id), cloned_program_id(cloned_id) {}

  Allocator* const allocator;
  ProgramID const program_id;
  ProgramID const cloned_program_id;

  ProgramNode* Clone(CloneContext*) const override {
    return allocator->Create<ProgramNode>(cloned_program_id, cloned_program_id);
  }
};

ProgramID ProgramIDOf(const ProgramNode* node) {
  return node->program_id;
}

using CloneContextNodeTest = ::testing::Test;

TEST_F(CloneContextNodeTest, Clone) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
  original_root->a = a.Create<Node>(builder.Symbols().New("a"));
  original_root->a->b = a.Create<Node>(builder.Symbols().New("a->b"));
  original_root->b = a.Create<Node>(builder.Symbols().New("b"));
  original_root->b->a = original_root->a;  // Aliased
  original_root->b->b = a.Create<Node>(builder.Symbols().New("b->b"));
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

  EXPECT_NE(cloned_root->b->a, cloned_root->a);  // De-aliased
  EXPECT_NE(cloned_root->c, cloned_root->b);     // De-aliased

  EXPECT_EQ(cloned_root->b->a->name, cloned_root->a->name);
  EXPECT_EQ(cloned_root->c->name, cloned_root->b->name);
}

TEST_F(CloneContextNodeTest, CloneWithReplaceAll_Cloneable) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
  original_root->a = a.Create<Node>(builder.Symbols().New("a"));
  original_root->a->b = a.Create<Replaceable>(builder.Symbols().New("a->b"));
  original_root->b = a.Create<Replaceable>(builder.Symbols().New("b"));
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
    auto* out = a.Create<Replacement>(out_name);
    out->b = a.Create<Node>(b_name);
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

  EXPECT_NE(cloned_root->b->c, cloned_root->a);  // De-aliased
  EXPECT_NE(cloned_root->c, cloned_root->b);     // De-aliased

  EXPECT_EQ(cloned_root->b->c->name, cloned_root->a->name);
  EXPECT_EQ(cloned_root->c->name, cloned_root->b->name);

  EXPECT_FALSE(Is<Replacement>(cloned_root->a));
  EXPECT_TRUE(Is<Replacement>(cloned_root->a->b));
  EXPECT_FALSE(Is<Replacement>(cloned_root->a->b->b));
  EXPECT_TRUE(Is<Replacement>(cloned_root->b));
  EXPECT_FALSE(Is<Replacement>(cloned_root->b->b));
}

TEST_F(CloneContextNodeTest, CloneWithReplaceAll_Symbols) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
  original_root->a = a.Create<Node>(builder.Symbols().New("a"));
  original_root->a->b = a.Create<Node>(builder.Symbols().New("a->b"));
  original_root->b = a.Create<Node>(builder.Symbols().New("b"));
  original_root->b->a = original_root->a;  // Aliased
  original_root->b->b = a.Create<Node>(builder.Symbols().New("b->b"));
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
  auto* cloned_root = CloneContext(&cloned, &original, false)
                          .ReplaceAll([&](Symbol sym) {
                            auto in = original.Symbols().NameFor(sym);
                            auto out = "transformed<" + in + ">";
                            return cloned.Symbols().New(out);
                          })
                          .Clone(original_root);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("transformed<root>"));
  EXPECT_EQ(cloned_root->a->name, cloned.Symbols().Get("transformed<a>"));
  EXPECT_EQ(cloned_root->a->b->name, cloned.Symbols().Get("transformed<a->b>"));
  EXPECT_EQ(cloned_root->b->name, cloned.Symbols().Get("transformed<b>"));
  EXPECT_EQ(cloned_root->b->b->name, cloned.Symbols().Get("transformed<b->b>"));
}

TEST_F(CloneContextNodeTest, CloneWithoutTransform) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_node = a.Create<Node>(builder.Symbols().New("root"));
  Program original(std::move(builder));

  ProgramBuilder cloned;
  CloneContext ctx(&cloned, &original);
  ctx.ReplaceAll([&](Node*) {
    return a.Create<Replacement>(builder.Symbols().New("<unexpected-node>"));
  });

  auto* cloned_node = ctx.CloneWithoutTransform(original_node);
  EXPECT_NE(cloned_node, original_node);
  EXPECT_EQ(cloned_node->name, cloned.Symbols().Get("root"));
}

TEST_F(CloneContextNodeTest, CloneWithReplace) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().New("root"));
  original_root->a = a.Create<Node>(builder.Symbols().New("a"));
  original_root->b = a.Create<Node>(builder.Symbols().New("b"));
  original_root->c = a.Create<Node>(builder.Symbols().New("c"));
  Program original(std::move(builder));

  //                          root
  //        ╭──────────────────┼──────────────────╮
  //       (a)                (b)                (c)
  //                        Replaced

  ProgramBuilder cloned;
  auto* replacement = a.Create<Node>(cloned.Symbols().New("replacement"));

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

TEST_F(CloneContextNodeTest, CloneWithRemove) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
  original_root->a = a.Create<Node>(builder.Symbols().Register("a"));
  original_root->b = a.Create<Node>(builder.Symbols().Register("b"));
  original_root->c = a.Create<Node>(builder.Symbols().Register("c"));
  original_root->vec = {original_root->a, original_root->b, original_root->c};
  Program original(std::move(builder));

  ProgramBuilder cloned;
  auto* cloned_root = CloneContext(&cloned, &original)
                          .Remove(original_root->vec, original_root->b)
                          .Clone(original_root);

  EXPECT_EQ(cloned_root->vec.size(), 2u);

  EXPECT_NE(cloned_root->vec[0], cloned_root->a);
  EXPECT_NE(cloned_root->vec[1], cloned_root->c);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("c"));
}

TEST_F(CloneContextNodeTest, CloneWithInsertBefore) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
  original_root->a = a.Create<Node>(builder.Symbols().Register("a"));
  original_root->b = a.Create<Node>(builder.Symbols().Register("b"));
  original_root->c = a.Create<Node>(builder.Symbols().Register("c"));
  original_root->vec = {original_root->a, original_root->b, original_root->c};
  Program original(std::move(builder));

  ProgramBuilder cloned;
  auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

  auto* cloned_root =
      CloneContext(&cloned, &original)
          .InsertBefore(original_root->vec, original_root->b, insertion)
          .Clone(original_root);

  EXPECT_EQ(cloned_root->vec.size(), 4u);

  EXPECT_NE(cloned_root->vec[0], cloned_root->a);
  EXPECT_NE(cloned_root->vec[2], cloned_root->b);
  EXPECT_NE(cloned_root->vec[3], cloned_root->c);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("insertion"));
  EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("b"));
  EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(CloneContextNodeTest, CloneWithInsertAfter) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
  original_root->a = a.Create<Node>(builder.Symbols().Register("a"));
  original_root->b = a.Create<Node>(builder.Symbols().Register("b"));
  original_root->c = a.Create<Node>(builder.Symbols().Register("c"));
  original_root->vec = {original_root->a, original_root->b, original_root->c};
  Program original(std::move(builder));

  ProgramBuilder cloned;
  auto* insertion = a.Create<Node>(cloned.Symbols().New("insertion"));

  auto* cloned_root =
      CloneContext(&cloned, &original)
          .InsertAfter(original_root->vec, original_root->b, insertion)
          .Clone(original_root);

  EXPECT_EQ(cloned_root->vec.size(), 4u);

  EXPECT_NE(cloned_root->vec[0], cloned_root->a);
  EXPECT_NE(cloned_root->vec[1], cloned_root->b);
  EXPECT_NE(cloned_root->vec[3], cloned_root->c);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->vec[1]->name, cloned.Symbols().Get("b"));
  EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion"));
  EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(CloneContextNodeTest, CloneWithInsertBeforeAndAfterRemoved) {
  Allocator a;

  ProgramBuilder builder;
  auto* original_root = a.Create<Node>(builder.Symbols().Register("root"));
  original_root->a = a.Create<Node>(builder.Symbols().Register("a"));
  original_root->b = a.Create<Node>(builder.Symbols().Register("b"));
  original_root->c = a.Create<Node>(builder.Symbols().Register("c"));
  original_root->vec = {original_root->a, original_root->b, original_root->c};
  Program original(std::move(builder));

  ProgramBuilder cloned;
  auto* insertion_before =
      a.Create<Node>(cloned.Symbols().New("insertion_before"));
  auto* insertion_after =
      a.Create<Node>(cloned.Symbols().New("insertion_after"));

  auto* cloned_root =
      CloneContext(&cloned, &original)
          .InsertBefore(original_root->vec, original_root->b, insertion_before)
          .InsertAfter(original_root->vec, original_root->b, insertion_after)
          .Remove(original_root->vec, original_root->b)
          .Clone(original_root);

  EXPECT_EQ(cloned_root->vec.size(), 4u);

  EXPECT_NE(cloned_root->vec[0], cloned_root->a);
  EXPECT_NE(cloned_root->vec[3], cloned_root->c);

  EXPECT_EQ(cloned_root->name, cloned.Symbols().Get("root"));
  EXPECT_EQ(cloned_root->vec[0]->name, cloned.Symbols().Get("a"));
  EXPECT_EQ(cloned_root->vec[1]->name,
            cloned.Symbols().Get("insertion_before"));
  EXPECT_EQ(cloned_root->vec[2]->name, cloned.Symbols().Get("insertion_after"));
  EXPECT_EQ(cloned_root->vec[3]->name, cloned.Symbols().Get("c"));
}

TEST_F(CloneContextNodeTest, CloneIntoSameBuilder) {
  ProgramBuilder builder;
  CloneContext ctx(&builder);
  Allocator allocator;
  auto* original = allocator.Create<Node>(builder.Symbols().New());
  auto* cloned_a = ctx.Clone(original);
  auto* cloned_b = ctx.Clone(original);
  EXPECT_NE(original, cloned_a);
  EXPECT_NE(original, cloned_b);

  EXPECT_NE(cloned_a, cloned_b);
}

TEST_F(CloneContextNodeTest, CloneWithReplaceAll_SameTypeTwice) {
  std::string node_name = TypeInfo::Of<Node>().name;

  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder cloned;
        Program original;
        CloneContext ctx(&cloned, &original);
        ctx.ReplaceAll([](Node*) { return nullptr; });
        ctx.ReplaceAll([](Node*) { return nullptr; });
      },
      "internal compiler error: ReplaceAll() called with a handler for type " +
          node_name + " that is already handled by a handler for type " +
          node_name);
}

TEST_F(CloneContextNodeTest, CloneWithReplaceAll_BaseThenDerived) {
  std::string node_name = TypeInfo::Of<Node>().name;
  std::string replaceable_name = TypeInfo::Of<Replaceable>().name;

  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder cloned;
        Program original;
        CloneContext ctx(&cloned, &original);
        ctx.ReplaceAll([](Node*) { return nullptr; });
        ctx.ReplaceAll([](Replaceable*) { return nullptr; });
      },
      "internal compiler error: ReplaceAll() called with a handler for type " +
          replaceable_name + " that is already handled by a handler for type " +
          node_name);
}

TEST_F(CloneContextNodeTest, CloneWithReplaceAll_DerivedThenBase) {
  std::string node_name = TypeInfo::Of<Node>().name;
  std::string replaceable_name = TypeInfo::Of<Replaceable>().name;

  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder cloned;
        Program original;
        CloneContext ctx(&cloned, &original);
        ctx.ReplaceAll([](Replaceable*) { return nullptr; });
        ctx.ReplaceAll([](Node*) { return nullptr; });
      },
      "internal compiler error: ReplaceAll() called with a handler for type " +
          node_name + " that is already handled by a handler for type " +
          replaceable_name);
}

TEST_F(CloneContextNodeTest, CloneWithReplace_WithNotANode) {
  EXPECT_FATAL_FAILURE(
      {
        Allocator allocator;
        ProgramBuilder builder;
        auto* original_root =
            allocator.Create<Node>(builder.Symbols().New("root"));
        original_root->a = allocator.Create<Node>(builder.Symbols().New("a"));
        original_root->b = allocator.Create<Node>(builder.Symbols().New("b"));
        original_root->c = allocator.Create<Node>(builder.Symbols().New("c"));
        Program original(std::move(builder));

        //                          root
        //        ╭──────────────────┼──────────────────╮
        //       (a)                (b)                (c)
        //                        Replaced

        ProgramBuilder cloned;
        auto* replacement = allocator.Create<NotANode>();

        CloneContext ctx(&cloned, &original);
        ctx.Replace(original_root->b, replacement);

        ctx.Clone(original_root);
      },
      "internal compiler error");
}

using CloneContextTest = ::testing::Test;

TEST_F(CloneContextTest, CloneWithReplaceAll_SymbolsTwice) {
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

TEST_F(CloneContextTest, CloneNewUnnamedSymbols) {
  ProgramBuilder builder;
  Symbol old_a = builder.Symbols().New();
  Symbol old_b = builder.Symbols().New();
  Symbol old_c = builder.Symbols().New();
  EXPECT_EQ(builder.Symbols().NameFor(old_a), "tint_symbol");
  EXPECT_EQ(builder.Symbols().NameFor(old_b), "tint_symbol_1");
  EXPECT_EQ(builder.Symbols().NameFor(old_c), "tint_symbol_2");

  Program original(std::move(builder));

  ProgramBuilder cloned;
  CloneContext ctx(&cloned, &original, false);
  Symbol new_x = cloned.Symbols().New();
  Symbol new_a = ctx.Clone(old_a);
  Symbol new_y = cloned.Symbols().New();
  Symbol new_b = ctx.Clone(old_b);
  Symbol new_z = cloned.Symbols().New();
  Symbol new_c = ctx.Clone(old_c);

  EXPECT_EQ(cloned.Symbols().NameFor(new_x), "tint_symbol");
  EXPECT_EQ(cloned.Symbols().NameFor(new_a), "tint_symbol_1");
  EXPECT_EQ(cloned.Symbols().NameFor(new_y), "tint_symbol_2");
  EXPECT_EQ(cloned.Symbols().NameFor(new_b), "tint_symbol_1_1");
  EXPECT_EQ(cloned.Symbols().NameFor(new_z), "tint_symbol_3");
  EXPECT_EQ(cloned.Symbols().NameFor(new_c), "tint_symbol_2_1");
}

TEST_F(CloneContextTest, CloneNewSymbols) {
  ProgramBuilder builder;
  Symbol old_a = builder.Symbols().New("a");
  Symbol old_b = builder.Symbols().New("b");
  Symbol old_c = builder.Symbols().New("c");
  EXPECT_EQ(builder.Symbols().NameFor(old_a), "a");
  EXPECT_EQ(builder.Symbols().NameFor(old_b), "b");
  EXPECT_EQ(builder.Symbols().NameFor(old_c), "c");

  Program original(std::move(builder));

  ProgramBuilder cloned;
  CloneContext ctx(&cloned, &original, false);
  Symbol new_x = cloned.Symbols().New("a");
  Symbol new_a = ctx.Clone(old_a);
  Symbol new_y = cloned.Symbols().New("b");
  Symbol new_b = ctx.Clone(old_b);
  Symbol new_z = cloned.Symbols().New("c");
  Symbol new_c = ctx.Clone(old_c);

  EXPECT_EQ(cloned.Symbols().NameFor(new_x), "a");
  EXPECT_EQ(cloned.Symbols().NameFor(new_a), "a_1");
  EXPECT_EQ(cloned.Symbols().NameFor(new_y), "b");
  EXPECT_EQ(cloned.Symbols().NameFor(new_b), "b_1");
  EXPECT_EQ(cloned.Symbols().NameFor(new_z), "c");
  EXPECT_EQ(cloned.Symbols().NameFor(new_c), "c_1");
}

TEST_F(CloneContextTest, CloneNewSymbols_AfterCloneSymbols) {
  ProgramBuilder builder;
  Symbol old_a = builder.Symbols().New("a");
  Symbol old_b = builder.Symbols().New("b");
  Symbol old_c = builder.Symbols().New("c");
  EXPECT_EQ(builder.Symbols().NameFor(old_a), "a");
  EXPECT_EQ(builder.Symbols().NameFor(old_b), "b");
  EXPECT_EQ(builder.Symbols().NameFor(old_c), "c");

  Program original(std::move(builder));

  ProgramBuilder cloned;
  CloneContext ctx(&cloned, &original);
  Symbol new_x = cloned.Symbols().New("a");
  Symbol new_a = ctx.Clone(old_a);
  Symbol new_y = cloned.Symbols().New("b");
  Symbol new_b = ctx.Clone(old_b);
  Symbol new_z = cloned.Symbols().New("c");
  Symbol new_c = ctx.Clone(old_c);

  EXPECT_EQ(cloned.Symbols().NameFor(new_x), "a_1");
  EXPECT_EQ(cloned.Symbols().NameFor(new_a), "a");
  EXPECT_EQ(cloned.Symbols().NameFor(new_y), "b_1");
  EXPECT_EQ(cloned.Symbols().NameFor(new_b), "b");
  EXPECT_EQ(cloned.Symbols().NameFor(new_z), "c_1");
  EXPECT_EQ(cloned.Symbols().NameFor(new_c), "c");
}

TEST_F(CloneContextTest, ProgramIDs) {
  ProgramBuilder dst;
  Program src(ProgramBuilder{});
  CloneContext ctx(&dst, &src);
  Allocator allocator;
  auto* cloned = ctx.Clone(allocator.Create<ProgramNode>(src.ID(), dst.ID()));
  EXPECT_EQ(cloned->program_id, dst.ID());
}

TEST_F(CloneContextTest, ProgramIDs_Clone_ObjectNotOwnedBySrc) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder dst;
        Program src(ProgramBuilder{});
        CloneContext ctx(&dst, &src);
        Allocator allocator;
        ctx.Clone(allocator.Create<ProgramNode>(ProgramID::New(), dst.ID()));
      },
      R"(internal compiler error: TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(src, a))");
}

TEST_F(CloneContextTest, ProgramIDs_Clone_ObjectNotOwnedByDst) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder dst;
        Program src(ProgramBuilder{});
        CloneContext ctx(&dst, &src);
        Allocator allocator;
        ctx.Clone(allocator.Create<ProgramNode>(src.ID(), ProgramID::New()));
      },
      R"(internal compiler error: TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(dst, out))");
}

}  // namespace

TINT_INSTANTIATE_TYPEINFO(Node);
TINT_INSTANTIATE_TYPEINFO(Replaceable);
TINT_INSTANTIATE_TYPEINFO(Replacement);
TINT_INSTANTIATE_TYPEINFO(NotANode);
TINT_INSTANTIATE_TYPEINFO(ProgramNode);

}  // namespace tint
