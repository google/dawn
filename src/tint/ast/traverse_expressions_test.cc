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

#include "src/tint/ast/traverse_expressions.h"
#include "gmock/gmock.h"
#include "src/tint/ast/test_helper.h"

using ::testing::ElementsAre;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using TraverseExpressionsTest = TestHelper;

TEST_F(TraverseExpressionsTest, DescendIndexAccessor) {
    std::vector<const Expression*> e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    std::vector<const Expression*> i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    {
        std::vector<const Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, i[0], e[0], e[1], i[1], e[2], e[3]));
    }
    {
        std::vector<const Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, i[1], e[3], e[2], i[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, DescendBinaryExpression) {
    std::vector<const Expression*> e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    std::vector<const Expression*> i = {Add(e[0], e[1]), Sub(e[2], e[3])};
    auto* root = Mul(i[0], i[1]);
    {
        std::vector<const Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, i[0], e[0], e[1], i[1], e[2], e[3]));
    }
    {
        std::vector<const Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, i[1], e[3], e[2], i[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, Depth) {
    std::vector<const Expression*> e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    std::vector<const Expression*> i = {Add(e[0], e[1]), Sub(e[2], e[3])};
    auto* root = Mul(i[0], i[1]);

    size_t j = 0;
    size_t depths[] = {0, 1, 2, 2, 1, 2, 2};
    {
        TraverseExpressions<TraverseOrder::LeftToRight>(  //
            root, Diagnostics(), [&](const Expression* expr, size_t depth) {
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
        utils::Vector<const Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            l2r.Push(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, b2, b1, b0, e));
    }
    {
        utils::Vector<const Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            r2l.Push(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, b2, b1, b0, e));
    }
}

TEST_F(TraverseExpressionsTest, DescendCallExpression) {
    utils::Vector e{Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    utils::Vector c{Call("a", e[0], e[1]), Call("b", e[2], e[3])};
    auto* root = Call("c", c[0], c[1]);
    {
        utils::Vector<const Expression*, 8> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            l2r.Push(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, c[0], e[0], e[1], c[1], e[2], e[3]));
    }
    {
        utils::Vector<const Expression*, 8> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            r2l.Push(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, c[1], e[3], e[2], c[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, DescendMemberAccessorExpression) {
    auto* e = Expr(1_i);
    auto* m = MemberAccessor(e, "a");
    auto* root = MemberAccessor(m, "b");
    {
        std::vector<const Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, m, e));
    }
    {
        std::vector<const Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            r2l.push_back(expr);
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
        std::vector<const Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, c, a, b, f, d, e));
    }
    {
        std::vector<const Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            r2l.push_back(expr);
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
        std::vector<const Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, u2, u1, u0, e));
    }
    {
        std::vector<const Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, u2, u1, u0, e));
    }
}

TEST_F(TraverseExpressionsTest, Skip) {
    std::vector<const Expression*> e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    std::vector<const Expression*> i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    std::vector<const Expression*> order;
    TraverseExpressions<TraverseOrder::LeftToRight>(
        root, Diagnostics(), [&](const Expression* expr) {
            order.push_back(expr);
            return expr == i[0] ? TraverseAction::Skip : TraverseAction::Descend;
        });
    EXPECT_THAT(order, ElementsAre(root, i[0], i[1], e[2], e[3]));
}

TEST_F(TraverseExpressionsTest, Stop) {
    std::vector<const Expression*> e = {Expr(1_i), Expr(1_i), Expr(1_i), Expr(1_i)};
    std::vector<const Expression*> i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    std::vector<const Expression*> order;
    TraverseExpressions<TraverseOrder::LeftToRight>(
        root, Diagnostics(), [&](const Expression* expr) {
            order.push_back(expr);
            return expr == i[0] ? TraverseAction::Stop : TraverseAction::Descend;
        });
    EXPECT_THAT(order, ElementsAre(root, i[0]));
}

}  // namespace
}  // namespace tint::ast
