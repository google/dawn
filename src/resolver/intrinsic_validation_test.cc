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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint {
namespace resolver {
namespace {

using IntrinsicType = sem::IntrinsicType;

using ResolverIntrinsicValidationTest = ResolverTest;

TEST_F(ResolverIntrinsicValidationTest, InvalidPipelineStageDirect) {
  // [[stage(compute), workgroup_size(1)]] fn func { return dpdx(1.0); }

  auto* dpdx = create<ast::CallExpression>(Source{{3, 4}}, Expr("dpdx"),
                                           ast::ExpressionList{Expr(1.0f)});
  Func(Source{{1, 2}}, "func", ast::VariableList{}, ty.void_(), {Ignore(dpdx)},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "3:4 error: built-in cannot be used by compute pipeline stage");
}

TEST_F(ResolverIntrinsicValidationTest, InvalidPipelineStageIndirect) {
  // fn f0 { return dpdx(1.0); }
  // fn f1 { f0(); }
  // fn f2 { f1(); }
  // [[stage(compute), workgroup_size(1)]] fn main { return f2(); }

  auto* dpdx = create<ast::CallExpression>(Source{{3, 4}}, Expr("dpdx"),
                                           ast::ExpressionList{Expr(1.0f)});
  Func(Source{{1, 2}}, "f0", ast::VariableList{}, ty.void_(), {Ignore(dpdx)});

  Func(Source{{3, 4}}, "f1", ast::VariableList{}, ty.void_(),
       {Ignore(Call("f0"))});

  Func(Source{{5, 6}}, "f2", ast::VariableList{}, ty.void_(),
       {Ignore(Call("f1"))});

  Func(Source{{7, 8}}, "main", ast::VariableList{}, ty.void_(),
       {Ignore(Call("f2"))},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(3:4 error: built-in cannot be used by compute pipeline stage
1:2 note: called by function 'f0'
3:4 note: called by function 'f1'
5:6 note: called by function 'f2'
7:8 note: called by entry point 'main')");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
