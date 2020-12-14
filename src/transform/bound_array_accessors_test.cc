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

#include "src/transform/bound_array_accessors.h"

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/binary_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/builder.h"
#include "src/ast/call_expression.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/storage_class.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/diagnostic/formatter.h"
#include "src/transform/manager.h"
#include "src/type_determiner.h"

namespace tint {
namespace transform {
namespace {

template <typename T = ast::Expression>
T* FindVariable(ast::Module* mod, std::string name) {
  if (auto* func = mod->FindFunctionBySymbol(mod->RegisterSymbol("func"))) {
    for (auto* stmt : *func->body()) {
      if (auto* decl = stmt->As<ast::VariableDeclStatement>()) {
        if (auto* var = decl->variable()) {
          if (var->name() == name) {
            return As<T>(var->constructor());
          }
        }
      }
    }
  }
  return nullptr;
}

class BoundArrayAccessorsTest : public testing::Test {
 public:
  ast::Module Transform(ast::Module in) {
    TypeDeterminer td(&in);
    if (!td.Determine()) {
      error = "Type determination failed: " + td.error();
      return {};
    }

    Manager manager;
    manager.append(std::make_unique<BoundArrayAccessors>());
    auto result = manager.Run(&in);

    if (result.diagnostics.contains_errors()) {
      error = "manager().Run() errored:\n" +
              diag::Formatter().format(result.diagnostics);
      return {};
    }

    return std::move(result.module);
  }

  std::string error;
};

struct ModuleBuilder : public ast::BuilderWithModule {
  ast::Module Module() {
    Build();
    auto* body = create<ast::BlockStatement>(Source{}, statements);
    mod->AddFunction(create<ast::Function>(
        Source{}, mod->RegisterSymbol("func"), "func", ast::VariableList{},
        ty.void_, body, ast::FunctionDecorationList{}));
    return std::move(*mod);
  }

 protected:
  virtual void Build() = 0;
  void OnVariableBuilt(ast::Variable* var) override {
    statements.emplace_back(create<ast::VariableDeclStatement>(Source{}, var));
  }
  ast::StatementList statements;
};

TEST_F(BoundArrayAccessorsTest, Ptrs_Clamp) {
  // var a : array<f32, 3>;
  // const c : u32 =  1;
  // const b : ptr<function, f32> = a[c]
  //
  //   -> const b : ptr<function, i32> = a[min(u32(c), 2)]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.array<f32, 3>());
      Const("c", ast::StorageClass::kFunction, ty.u32);
      Const("b", ast::StorageClass::kFunction,
            ty.pointer<f32>(ast::StorageClass::kFunction), Index("a", "c"), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->idx_expr()->Is<ast::CallExpression>());

  auto* idx = b->idx_expr()->As<ast::CallExpression>();
  ASSERT_TRUE(idx->func()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(idx->func()->As<ast::IdentifierExpression>()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[0]->Is<ast::TypeConstructorExpression>());
  auto* tc = idx->params()[0]->As<ast::TypeConstructorExpression>();
  EXPECT_TRUE(tc->type()->Is<ast::type::U32>());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_TRUE(tc->values()[0]->Is<ast::IdentifierExpression>());
  ASSERT_EQ(tc->values()[0]->As<ast::IdentifierExpression>()->name(), "c");

  ASSERT_TRUE(idx->params()[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[1]->Is<ast::ScalarConstructorExpression>());
  auto* scalar = idx->params()[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Nested_Scalar) {
  // var a : array<f32, 3>;
  // var b : array<f32, 5>;
  // var i : u32;
  // var c : f32 = a[b[i]];
  //
  // -> var c : f32 = a[min(u32(b[min(u32(i), 4)]), 2)];
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.array<f32, 3>());
      Var("b", ast::StorageClass::kFunction, ty.array<f32, 5>());
      Var("i", ast::StorageClass::kFunction, ty.u32);
      Const("c", ast::StorageClass::kFunction, ty.f32,
            Index("a", Index("b", "i")), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* c = FindVariable<ast::ArrayAccessorExpression>(&module, "c");
  ASSERT_NE(c, nullptr);

  ASSERT_TRUE(c->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(c->idx_expr()->Is<ast::CallExpression>());

  auto* idx = c->idx_expr()->As<ast::CallExpression>();
  ASSERT_TRUE(idx->func()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(idx->func()->As<ast::IdentifierExpression>()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[0]->Is<ast::TypeConstructorExpression>());
  auto* tc = idx->params()[0]->As<ast::TypeConstructorExpression>();
  EXPECT_TRUE(tc->type()->Is<ast::type::U32>());
  ASSERT_EQ(tc->values().size(), 1u);

  auto* sub = tc->values()[0];
  ASSERT_TRUE(sub->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(sub->As<ast::ArrayAccessorExpression>()
                  ->idx_expr()
                  ->Is<ast::CallExpression>());

  auto* sub_idx = sub->As<ast::ArrayAccessorExpression>()
                      ->idx_expr()
                      ->As<ast::CallExpression>();
  ASSERT_TRUE(sub_idx->func()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(sub_idx->func()->As<ast::IdentifierExpression>()->name(), "min");

  ASSERT_TRUE(sub_idx->params()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(sub_idx->params()[0]->Is<ast::TypeConstructorExpression>());
  tc = sub_idx->params()[0]->As<ast::TypeConstructorExpression>();
  EXPECT_TRUE(tc->type()->Is<ast::type::U32>());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_TRUE(tc->values()[0]->Is<ast::IdentifierExpression>());
  ASSERT_EQ(tc->values()[0]->As<ast::IdentifierExpression>()->name(), "i");

  ASSERT_TRUE(sub_idx->params()[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(sub_idx->params()[1]->Is<ast::ScalarConstructorExpression>());
  auto* scalar = sub_idx->params()[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 4u);

  ASSERT_TRUE(idx->params()[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[1]->Is<ast::ScalarConstructorExpression>());
  scalar = idx->params()[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(c->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(c->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Scalar) {
  // var a : array<f32, 3>
  // var b : f32 = a[1];
  //
  // -> var b : f32 = a[1];
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.array(ty.f32, 3));
      Var("b", ast::StorageClass::kFunction, ty.f32, Index("a", 1u), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Expr) {
  // var a : array<f32, 3>
  // var c : u32;
  // var b : f32 = a[c + 2 - 3]
  //
  // -> var b : f32 = a[min(u32(c + 2 - 3), 2)]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.array<f32, 3>());
      Var("c", ast::StorageClass::kFunction, ty.u32);
      Var("b", ast::StorageClass::kFunction, ty.f32,
          Index("a", Add("c", Sub(2u, 3u))), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::CallExpression>());

  auto* idx = b->idx_expr()->As<ast::CallExpression>();
  ASSERT_TRUE(idx->func()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(idx->func()->As<ast::IdentifierExpression>()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[0]->Is<ast::TypeConstructorExpression>());
  auto* tc = idx->params()[0]->As<ast::TypeConstructorExpression>();
  EXPECT_TRUE(tc->type()->Is<ast::type::U32>());
  ASSERT_EQ(tc->values().size(), 1u);
  auto* add = tc->values()[0]->As<ast::BinaryExpression>();
  ASSERT_NE(add, nullptr);
  ASSERT_EQ(add->op(), ast::BinaryOp::kAdd);
  auto* add_lhs = add->lhs()->As<ast::IdentifierExpression>();
  ASSERT_NE(add_lhs, nullptr);
  ASSERT_EQ(add_lhs->name(), "c");
  auto* add_rhs = add->rhs()->As<ast::BinaryExpression>();
  ASSERT_NE(add_rhs, nullptr);
  ASSERT_TRUE(add_rhs->lhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->lhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            2u);
  ASSERT_TRUE(add_rhs->rhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->rhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            3u);

  ASSERT_TRUE(idx->params()[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[1]->Is<ast::ScalarConstructorExpression>());
  auto* scalar = idx->params()[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Negative) {
  // var a : array<f32, 3>
  // var b : f32 = a[-1]
  //
  // -> var b : f32 = a[0]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.array<f32, 3>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index("a", -1), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::SintLiteral>()->value(), 0);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::I32>());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_OutOfBounds) {
  // var a : array<f32, 3>
  // var b : f32 = a[3]
  //
  // -> var b : f32 = a[2]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.array<f32, 3>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index("a", 3u), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Scalar) {
  // var a : vec3<f32>
  // var b : f32 = a[1];
  //
  // -> var b : f32 = a[1]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.vec3<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index("a", 1u), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Expr) {
  // var a : vec3<f32>
  // var c : u32;
  // var b : f32 = a[c + 2 - 3]
  //
  // -> var b : f32 = a[min(u32(c + 2 - 3), 2)]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.vec3<f32>());
      Var("c", ast::StorageClass::kFunction, ty.u32);
      Var("b", ast::StorageClass::kFunction, ty.f32,
          Index("a", Add("c", Sub(2u, 3u))), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::CallExpression>());

  auto* idx = b->idx_expr()->As<ast::CallExpression>();
  ASSERT_TRUE(idx->func()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(idx->func()->As<ast::IdentifierExpression>()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);
  ASSERT_TRUE(idx->params()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[0]->Is<ast::TypeConstructorExpression>());
  auto* tc = idx->params()[0]->As<ast::TypeConstructorExpression>();
  EXPECT_TRUE(tc->type()->Is<ast::type::U32>());
  ASSERT_EQ(tc->values().size(), 1u);
  auto* add = tc->values()[0]->As<ast::BinaryExpression>();
  ASSERT_NE(add, nullptr);
  auto* add_lhs = add->lhs()->As<ast::IdentifierExpression>();
  ASSERT_NE(add_lhs, nullptr);
  ASSERT_EQ(add_lhs->name(), "c");
  auto* add_rhs = add->rhs()->As<ast::BinaryExpression>();
  ASSERT_NE(add_rhs, nullptr);
  ASSERT_TRUE(add_rhs->lhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->lhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            2u);
  ASSERT_TRUE(add_rhs->rhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->rhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            3u);

  ASSERT_TRUE(idx->params()[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[1]->Is<ast::ScalarConstructorExpression>());
  auto* scalar = idx->params()[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Negative) {
  // var a : vec3<f32>
  // var b : f32 = a[-1]
  //
  // -> var b : f32 = a[0]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.vec3<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index("a", -1), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::SintLiteral>()->value(), 0);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::I32>());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_OutOfBounds) {
  // var a : vec3<f32>
  // var b : f32 = a[3]
  //
  // -> var b : f32 = a[2]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.vec3<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index("a", 3u), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Scalar) {
  // var a : mat3x2<f32>
  // var b : f32 = a[2][1];
  //
  // -> var b : f32 = a[2][1]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index(Index("a", 2u), 1u),
          {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());

  ASSERT_TRUE(b->array()->Is<ast::ArrayAccessorExpression>());
  auto* ary = b->array()->As<ast::ArrayAccessorExpression>();
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = ary->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->Is<ast::type::U32>());

  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Expr_Column) {
  // var a : mat3x2<f32>
  // var c : u32;
  // var b : f32 = a[c + 2 - 3][1]
  //
  // -> var b : f32 = a[min(u32(c + 2 - 3), 2)][1]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());
      Var("c", ast::StorageClass::kFunction, ty.u32);
      Var("b", ast::StorageClass::kFunction, ty.f32,
          Index(Index("a", Add("c", Sub(2u, 3u))), 1u), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());

  ASSERT_TRUE(b->array()->Is<ast::ArrayAccessorExpression>());
  auto* ary = b->array()->As<ast::ArrayAccessorExpression>();

  ASSERT_TRUE(ary->idx_expr()->Is<ast::CallExpression>());
  auto* idx = ary->idx_expr()->As<ast::CallExpression>();
  ASSERT_TRUE(idx->func()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(idx->func()->As<ast::IdentifierExpression>()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[0]->Is<ast::TypeConstructorExpression>());
  auto* tc = idx->params()[0]->As<ast::TypeConstructorExpression>();
  EXPECT_TRUE(tc->type()->Is<ast::type::U32>());
  ASSERT_EQ(tc->values().size(), 1u);
  auto* add = tc->values()[0]->As<ast::BinaryExpression>();
  ASSERT_NE(add, nullptr);
  auto* add_lhs = add->lhs()->As<ast::IdentifierExpression>();
  ASSERT_NE(add_lhs, nullptr);
  ASSERT_EQ(add_lhs->name(), "c");
  auto* add_rhs = add->rhs()->As<ast::BinaryExpression>();
  ASSERT_NE(add_rhs, nullptr);
  ASSERT_TRUE(add_rhs->lhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->lhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            2u);
  ASSERT_TRUE(add_rhs->rhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->rhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            3u);

  ASSERT_TRUE(idx->params()[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[1]->Is<ast::ScalarConstructorExpression>());
  auto* scalar = idx->params()[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->Is<ast::type::U32>());

  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Expr_Row) {
  // var a : mat3x2<f32>
  // var c : u32;
  // var b : f32 = a[1][c + 2 - 3]
  //
  // -> var b : f32 = a[1][min(u32(c + 2 - 3), 1)]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());
      Var("c", ast::StorageClass::kFunction, ty.u32);
      Var("b", ast::StorageClass::kFunction, ty.f32,
          Index(Index("a", 1u), Add("c", Sub(2u, 3u))), {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());

  ASSERT_TRUE(b->array()->Is<ast::ArrayAccessorExpression>());
  auto* ary = b->array()->As<ast::ArrayAccessorExpression>();

  ASSERT_TRUE(ary->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = ary->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_TRUE(b->idx_expr()->Is<ast::CallExpression>());
  auto* idx = b->idx_expr()->As<ast::CallExpression>();
  ASSERT_TRUE(idx->func()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(idx->func()->As<ast::IdentifierExpression>()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[0]->Is<ast::TypeConstructorExpression>());
  auto* tc = idx->params()[0]->As<ast::TypeConstructorExpression>();
  EXPECT_TRUE(tc->type()->Is<ast::type::U32>());
  ASSERT_EQ(tc->values().size(), 1u);
  auto* add = tc->values()[0]->As<ast::BinaryExpression>();
  ASSERT_NE(add, nullptr);
  auto* add_lhs = add->lhs()->As<ast::IdentifierExpression>();
  ASSERT_NE(add_lhs, nullptr);
  ASSERT_EQ(add_lhs->name(), "c");
  auto* add_rhs = add->rhs()->As<ast::BinaryExpression>();
  ASSERT_NE(add_rhs, nullptr);
  ASSERT_TRUE(add_rhs->lhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->lhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            2u);
  ASSERT_TRUE(add_rhs->rhs()->Is<ast::ScalarConstructorExpression>());
  ASSERT_EQ(add_rhs->rhs()
                ->As<ast::ScalarConstructorExpression>()
                ->literal()
                ->As<ast::UintLiteral>()
                ->value(),
            3u);

  ASSERT_TRUE(idx->params()[1]->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(idx->params()[1]->Is<ast::ScalarConstructorExpression>());
  scalar = idx->params()[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->Is<ast::type::U32>());

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Negative_Column) {
  // var a : mat3x2<f32>
  // var b : f32 = a[-1][1]
  //
  // -> var b : f32 = a[0][1]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index(Index("a", -1), 1),
          {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());

  ASSERT_TRUE(b->array()->Is<ast::ArrayAccessorExpression>());
  auto* ary = b->array()->As<ast::ArrayAccessorExpression>();
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = ary->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::SintLiteral>()->value(), 0);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->Is<ast::type::I32>());

  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::SintLiteral>()->value(), 1);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::I32>());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Negative_Row) {
  // var a : mat3x2<f32>
  // var b : f32 = a[2][-1]
  //
  // -> var b : f32 = a[2][0]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index(Index("a", 2), -1),
          {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());

  ASSERT_TRUE(b->array()->Is<ast::ArrayAccessorExpression>());
  auto* ary = b->array()->As<ast::ArrayAccessorExpression>();
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = ary->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::SintLiteral>()->value(), 2);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->Is<ast::type::I32>());

  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::SintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::SintLiteral>()->value(), 0);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::I32>());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_OutOfBounds_Column) {
  // var a : mat3x2<f32>
  // var b : f32 = a[5][1]
  //
  // -> var b : f32 = a[2][1]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index(Index("a", 5u), 1u),
          {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());

  ASSERT_TRUE(b->array()->Is<ast::ArrayAccessorExpression>());
  auto* ary = b->array()->As<ast::ArrayAccessorExpression>();
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = ary->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->Is<ast::type::U32>());

  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_OutOfBounds_Row) {
  // var a : mat3x2<f32>
  // var b : f32 = a[2][5]
  //
  // -> var b : f32 = a[2][1]
  struct Builder : ModuleBuilder {
    void Build() override {
      Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());
      Var("b", ast::StorageClass::kFunction, ty.f32, Index(Index("a", 2u), 5u),
          {});
    }
  };

  ast::Module module = Transform(Builder{}.Module());
  ASSERT_EQ(error, "");

  auto* b = FindVariable<ast::ArrayAccessorExpression>(&module, "b");
  ASSERT_NE(b, nullptr);

  ASSERT_TRUE(b->Is<ast::ArrayAccessorExpression>());

  ASSERT_TRUE(b->array()->Is<ast::ArrayAccessorExpression>());
  auto* ary = b->array()->As<ast::ArrayAccessorExpression>();
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(ary->idx_expr()->Is<ast::ScalarConstructorExpression>());

  auto* scalar = ary->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->Is<ast::type::U32>());

  ASSERT_TRUE(b->idx_expr()->Is<ast::ConstructorExpression>());
  ASSERT_TRUE(b->idx_expr()->Is<ast::ScalarConstructorExpression>());

  scalar = b->idx_expr()->As<ast::ScalarConstructorExpression>();
  ASSERT_TRUE(scalar->literal()->Is<ast::UintLiteral>());
  EXPECT_EQ(scalar->literal()->As<ast::UintLiteral>()->value(), 1u);

  ASSERT_NE(b->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(b->idx_expr()->result_type()->Is<ast::type::U32>());
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Vector_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : vec3<f32>
  // var b : f32 = a[idx]
  //
  // ->var b : f32 =  a[min(u32(idx), 2)]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Array_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : array<f32, 4>
  // var b : f32 = a[idx]
  //
  // -> var b : f32 = a[min(u32(idx), 3)]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Matrix_Column_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : mat3x2<f32>
  // var b : f32 = a[idx][1]
  //
  // -> var b : f32 = a[min(u32(idx), 2)][1]
}

// TODO(dsinclair): Implement when constant_id exists
TEST_F(BoundArrayAccessorsTest, DISABLED_Matrix_Row_Constant_Id_Clamps) {
  // [[constant_id(1300)]] const idx : i32;
  // var a : mat3x2<f32>
  // var b : f32 = a[1][idx]
  //
  // -> var b : f32 = a[1][min(u32(idx), 0, 1)]
}

// TODO(dsinclair): Implement when we have arrayLength for Runtime Arrays
TEST_F(BoundArrayAccessorsTest, DISABLED_RuntimeArray_Clamps) {
  // struct S {
  //   a : f32;
  //   b : array<f32>;
  // }
  // S s;
  // var b : f32 = s.b[25]
  //
  // -> var b : f32 = s.b[min(u32(25), arrayLength(s.b))]
}

// TODO(dsinclair): Clamp atomics when available.
TEST_F(BoundArrayAccessorsTest, DISABLED_Atomics_Clamp) {
  FAIL();
}

// TODO(dsinclair): Clamp texture coord values. Depends on:
// https://github.com/gpuweb/gpuweb/issues/1107
TEST_F(BoundArrayAccessorsTest, DISABLED_TextureCoord_Clamp) {
  FAIL();
}

// TODO(dsinclair): Test for scoped variables when Lexical Scopes implemented
TEST_F(BoundArrayAccessorsTest, DISABLED_Scoped_Variable) {
  // var a : array<f32, 3>;
  // var i : u32;
  // {
  //    var a : array<f32, 5>;
  //    var b : f32 = a[i];
  // }
  // var c : f32 = a[i];
  //
  // -> var b : f32 = a[min(u32(i), 4)];
  //    var c : f32 = a[min(u32(i), 2)];
  FAIL();
}

}  // namespace
}  // namespace transform
}  // namespace tint
