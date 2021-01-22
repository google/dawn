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

#include "gtest/gtest.h"

#include "src/ast/module.h"

namespace tint {
namespace {

struct Cloneable : public Castable<Cloneable, ast::Node> {
  Cloneable() : Base(Source{}) {}

  Cloneable* a = nullptr;
  Cloneable* b = nullptr;
  Cloneable* c = nullptr;

  Cloneable* Clone(CloneContext* ctx) const override {
    auto* out = ctx->dst->create<Cloneable>();
    out->a = ctx->Clone(a);
    out->b = ctx->Clone(b);
    out->c = ctx->Clone(c);
    return out;
  }

  bool IsValid() const override { return true; }
  void to_str(std::ostream&, size_t) const override {}
};

struct Replaceable : public Castable<Replaceable, Cloneable> {};
struct Replacement : public Castable<Replacement, Replaceable> {};

TEST(CloneContext, Clone) {
  ast::Module original;
  auto* original_root = original.create<Cloneable>();
  original_root->a = original.create<Cloneable>();
  original_root->a->b = original.create<Cloneable>();
  original_root->b = original.create<Cloneable>();
  original_root->b->a = original_root->a;  // Aliased
  original_root->b->b = original.create<Cloneable>();
  original_root->c = original_root->b;  // Aliased

  //                          root
  //        ╭──────────────────┼──────────────────╮
  //       (a)                (b)                (c)
  //        C  <──────┐        C  <───────────────┘
  //   ╭────┼────╮    │   ╭────┼────╮
  //  (a)  (b)  (c)   │  (a)  (b)  (c)
  //        C         └───┘    C
  //
  // C: Clonable

  ast::Module cloned;
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

  EXPECT_EQ(cloned_root->b->a, cloned_root->a);  // Aliased
  EXPECT_EQ(cloned_root->c, cloned_root->b);     // Aliased
}

TEST(CloneContext, CloneWithReplacements) {
  ast::Module original;
  auto* original_root = original.create<Cloneable>();
  original_root->a = original.create<Cloneable>();
  original_root->a->b = original.create<Replaceable>();
  original_root->b = original.create<Replaceable>();
  original_root->b->a = original_root->a;  // Aliased
  original_root->c = original_root->b;     // Aliased

  //                          root
  //        ╭──────────────────┼──────────────────╮
  //       (a)                (b)                (c)
  //        C  <──────┐        R  <───────────────┘
  //   ╭────┼────╮    │   ╭────┼────╮
  //  (a)  (b)  (c)   │  (a)  (b)  (c)
  //        R         └───┘
  //
  // C: Clonable
  // R: Replaceable

  ast::Module cloned;
  auto* cloned_root = CloneContext(&cloned, &original)
                          .ReplaceAll([&](CloneContext* ctx, Replaceable* in) {
                            auto* out = cloned.create<Replacement>();
                            out->b = cloned.create<Cloneable>();
                            out->c = ctx->Clone(in->a);
                            return out;
                          })
                          .Clone(original_root);

  //                         root
  //        ╭─────────────────┼──────────────────╮
  //       (a)               (b)                (c)
  //        C  <──────┐       R  <───────────────┘
  //   ╭────┼────╮    │  ╭────┼────╮
  //  (a)  (b)  (c)   │ (a)  (b)  (c)
  //        R         │       C    |
  //   ╭────┼────╮    └────────────┘
  //  (a)  (b)  (c)
  //        C
  //
  // C: Clonable
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

  EXPECT_EQ(cloned_root->b->c, cloned_root->a);  // Aliased
  EXPECT_EQ(cloned_root->c, cloned_root->b);     // Aliased

  EXPECT_FALSE(cloned_root->a->Is<Replacement>());
  EXPECT_TRUE(cloned_root->a->b->Is<Replacement>());
  EXPECT_FALSE(cloned_root->a->b->b->Is<Replacement>());
  EXPECT_TRUE(cloned_root->b->Is<Replacement>());
  EXPECT_FALSE(cloned_root->b->b->Is<Replacement>());
}

}  // namespace

TINT_INSTANTIATE_CLASS_ID(Cloneable);
TINT_INSTANTIATE_CLASS_ID(Replaceable);
TINT_INSTANTIATE_CLASS_ID(Replacement);

}  // namespace tint
