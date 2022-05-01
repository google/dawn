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

namespace tint::ast {
namespace {

using ::testing::ElementsAre;

using TraverseExpressionsTest = TestHelper;

TEST_F(TraverseExpressionsTest, DescendIndexAccessor) {
    std::vector<const ast::Expression*> e = {Expr(1), Expr(1), Expr(1), Expr(1)};
    std::vector<const ast::Expression*> i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    {
        std::vector<const ast::Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, i[0], e[0], e[1], i[1], e[2], e[3]));
    }
    {
        std::vector<const ast::Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, i[1], e[3], e[2], i[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, DescendBinaryExpression) {
    std::vector<const ast::Expression*> e = {Expr(1), Expr(1), Expr(1), Expr(1)};
    std::vector<const ast::Expression*> i = {Add(e[0], e[1]), Sub(e[2], e[3])};
    auto* root = Mul(i[0], i[1]);
    {
        std::vector<const ast::Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, i[0], e[0], e[1], i[1], e[2], e[3]));
    }
    {
        std::vector<const ast::Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, i[1], e[3], e[2], i[0], e[1], e[0]));
    }
}

TEST_F(TraverseExpressionsTest, DescendBitcastExpression) {
    auto* e = Expr(1);
    auto* b0 = Bitcast<i32>(e);
    auto* b1 = Bitcast<i32>(b0);
    auto* b2 = Bitcast<i32>(b1);
    auto* root = Bitcast<i32>(b2);
    {
        std::vector<const ast::Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, b2, b1, b0, e));
    }
    {
        std::vector<const ast::Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, b2, b1, b0, e));
    }
}

TEST_F(TraverseExpressionsTest, DescendCallExpression) {
    std::vector<const ast::Expression*> e = {Expr(1), Expr(1), Expr(1), Expr(1)};
    std::vector<const ast::Expression*> c = {Call("a", e[0], e[1]), Call("b", e[2], e[3])};
    auto* root = Call("c", c[0], c[1]);
    {
        std::vector<const ast::Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, c[0], e[0], e[1], c[1], e[2], e[3]));
    }
    {
        std::vector<const ast::Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, c[1], e[3], e[2], c[0], e[1], e[0]));
    }
}

// TODO(crbug.com/tint/1257): Test ignores member accessor 'member' field.
// Replace with the test below when fixed.
TEST_F(TraverseExpressionsTest, DescendMemberIndexExpression) {
    auto* e = Expr(1);
    auto* m = MemberAccessor(e, Expr("a"));
    auto* root = MemberAccessor(m, Expr("b"));
    {
        std::vector<const ast::Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, m, e));
    }
    {
        std::vector<const ast::Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, m, e));
    }
}

// TODO(crbug.com/tint/1257): The correct test for DescendMemberIndexExpression.
TEST_F(TraverseExpressionsTest, DISABLED_DescendMemberIndexExpression) {
    auto* e = Expr(1);
    std::vector<const ast::IdentifierExpression*> i = {Expr("a"), Expr("b")};
    auto* m = MemberAccessor(e, i[0]);
    auto* root = MemberAccessor(m, i[1]);
    {
        std::vector<const ast::Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, m, e, i[0], i[1]));
    }
    {
        std::vector<const ast::Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, i[1], m, i[0], e));
    }
}

TEST_F(TraverseExpressionsTest, DescendUnaryExpression) {
    auto* e = Expr(1);
    auto* u0 = AddressOf(e);
    auto* u1 = Deref(u0);
    auto* u2 = AddressOf(u1);
    auto* root = Deref(u2);
    {
        std::vector<const ast::Expression*> l2r;
        TraverseExpressions<TraverseOrder::LeftToRight>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            l2r.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(l2r, ElementsAre(root, u2, u1, u0, e));
    }
    {
        std::vector<const ast::Expression*> r2l;
        TraverseExpressions<TraverseOrder::RightToLeft>(root, Diagnostics(),
                                                        [&](const ast::Expression* expr) {
                                                            r2l.push_back(expr);
                                                            return ast::TraverseAction::Descend;
                                                        });
        EXPECT_THAT(r2l, ElementsAre(root, u2, u1, u0, e));
    }
}

TEST_F(TraverseExpressionsTest, Skip) {
    std::vector<const ast::Expression*> e = {Expr(1), Expr(1), Expr(1), Expr(1)};
    std::vector<const ast::Expression*> i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    std::vector<const ast::Expression*> order;
    TraverseExpressions<TraverseOrder::LeftToRight>(
        root, Diagnostics(), [&](const ast::Expression* expr) {
            order.push_back(expr);
            return expr == i[0] ? ast::TraverseAction::Skip : ast::TraverseAction::Descend;
        });
    EXPECT_THAT(order, ElementsAre(root, i[0], i[1], e[2], e[3]));
}

TEST_F(TraverseExpressionsTest, Stop) {
    std::vector<const ast::Expression*> e = {Expr(1), Expr(1), Expr(1), Expr(1)};
    std::vector<const ast::Expression*> i = {IndexAccessor(e[0], e[1]), IndexAccessor(e[2], e[3])};
    auto* root = IndexAccessor(i[0], i[1]);
    std::vector<const ast::Expression*> order;
    TraverseExpressions<TraverseOrder::LeftToRight>(
        root, Diagnostics(), [&](const ast::Expression* expr) {
            order.push_back(expr);
            return expr == i[0] ? ast::TraverseAction::Stop : ast::TraverseAction::Descend;
        });
    EXPECT_THAT(order, ElementsAre(root, i[0]));
}

}  // namespace
}  // namespace tint::ast
