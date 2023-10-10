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

#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "gmock/gmock.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

using ::testing::ElementsAre;

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::ast {
namespace {

using TraverseExpressionsTest = TestHelper;

TEST_F(TraverseExpressionsTest, DescendTemplatedIdentifier) {
    tint::Vector e{Expr(1_i), Expr(2_i), Expr(1_i), Expr(1_i)};
    tint::Vector c{Expr(Ident("a", e[0], e[1])), Expr(Ident("b", e[2], e[3]))};
    auto* root = Expr(Ident("c", c[0], c[1]));
    {
        Vector<const Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, c[0], e[0], e[1], c[1], e[2], e[3]));
    }
    {
        Vector<const Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, c[1], e[3], e[2], c[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, DescendIndexAccessor) {
    Vector e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    Vector i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    {
        Vector<const ast::Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, i[0], e[0], e[1], i[1], e[2], e[3]));
    }
    {
        Vector<const ast::Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, i[1], e[3], e[2], i[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, DescendBinaryExpression) {
    Vector e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    Vector i = {Add(e[0], e[1]), Sub(e[2], e[3])};
    auto* root = Mul(i[0], i[1]);
    {
        Vector<const ast::Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, i[0], e[0], e[1], i[1], e[2], e[3]));
    }
    {
        Vector<const ast::Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, i[1], e[3], e[2], i[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, Depth) {
    Vector e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    Vector i = {Add(e[0], e[1]), Sub(e[2], e[3])};
    auto* root = Mul(i[0], i[1]);

    size_t j = 0;
    size_t depths[] = {0, 1, 2, 2, 1, 2, 2};
    {
        TraverseExpressions<TraverseOrder::LeftToRight>(  //
            root, [&](const Expression* expr, size_t depth) {
                (void)expr;
                EXPECT_THAT(depth, depths[j++]);
                return TraverseAction::Descend;
            });
    }
}

TEST_F(TraverseExpressionsTest, DescendBitcastExpression) {
    auto* e = Expr(1_i);
    auto* b0 = Bitcast<i32>(e);
    auto* b1 = Bitcast<i32>(b0);
    auto* b2 = Bitcast<i32>(b1);
    auto* root = Bitcast<i32>(b2);
    {
        Vector<const Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, b2, b1, b0, e));
    }
    {
        Vector<const Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, b2, b1, b0, e));
    }
}

TEST_F(TraverseExpressionsTest, DescendCallExpression) {
    tint::Vector i{Expr("a"), Expr("b"), Expr("c")};
    tint::Vector e{Expr(1_i), Expr(2_i), Expr(1_i), Expr(1_i)};
    tint::Vector c{Call(i[0], e[0], e[1]), Call(i[1], e[2], e[3])};
    auto* root = Call(i[2], c[0], c[1]);
    {
        Vector<const Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, i[2], c[0], i[0], e[0], e[1], c[1], i[1], e[2], e[3]));
    }
    {
        Vector<const Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, c[1], e[3], e[2], i[1], c[0], e[1], e[0], i[0], i[2]));
    }
}

TEST_F(TraverseExpressionsTest, DescendMemberAccessorExpression) {
    auto* e = Expr(1_i);
    auto* m = MemberAccessor(e, "a");
    auto* root = MemberAccessor(m, "b");
    {
        Vector<const ast::Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, m, e));
    }
    {
        Vector<const ast::Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, m, e));
    }
}

TEST_F(TraverseExpressionsTest, DescendMemberIndexExpression) {
    auto* a = Expr("a");
    auto* b = Expr("b");
    auto* c = IndexAccessor(a, b);
    auto* d = Expr("d");
    auto* e = Expr("e");
    auto* f = IndexAccessor(d, e);
    auto* root = IndexAccessor(c, f);
    {
        Vector<const ast::Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, c, a, b, f, d, e));
    }
    {
        Vector<const ast::Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, f, e, d, c, b, a));
    }
}

TEST_F(TraverseExpressionsTest, DescendUnaryExpression) {
    auto* e = Expr(1_i);
    auto* u0 = AddressOf(e);
    auto* u1 = Deref(u0);
    auto* u2 = AddressOf(u1);
    auto* root = Deref(u2);
    {
        Vector<const ast::Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
            l2r.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(l2r, ElementsAre(root, u2, u1, u0, e));
    }
    {
        Vector<const ast::Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, [&](const Expression* expr) {
            r2l.Push(expr);
            return TraverseAction::Descend;
        });
        EXPECT_THAT(r2l, ElementsAre(root, u2, u1, u0, e));
    }
}

TEST_F(TraverseExpressionsTest, Skip) {
    Vector e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    Vector i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    Vector<const ast::Expression*, 8> order;
    TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
        order.Push(expr);
        return expr == i[0] ? TraverseAction::Skip : TraverseAction::Descend;
    });
    EXPECT_THAT(order, ElementsAre(root, i[0], i[1], e[2], e[3]));
}

TEST_F(TraverseExpressionsTest, Stop) {
    Vector e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    Vector i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    Vector<const ast::Expression*, 8> order;
    TraverseExpressions<TraverseOrder::LeftToRight>(root, [&](const Expression* expr) {
        order.Push(expr);
        return expr == i[0] ? TraverseAction::Stop : TraverseAction::Descend;
    });
    EXPECT_THAT(order, ElementsAre(root, i[0]));
}

}  // namespace
}  // namespace tint::ast
