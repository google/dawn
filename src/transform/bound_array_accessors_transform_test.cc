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

#include "src/transform/bound_array_accessors_transform.h"

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/binary_expression.h"
#include "src/ast/block_statement.h"
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
#include "src/context.h"
#include "src/type_determiner.h"

namespace tint {
namespace transform {
namespace {

class BoundArrayAccessorsTest : public testing::Test {
 public:
  BoundArrayAccessorsTest() : td_(&ctx_, &mod_), transform_(&ctx_, &mod_) {}

  ast::BlockStatement* SetupFunctionAndBody() {
    auto* block = create<ast::BlockStatement>();
    body_ = block;
    auto* func =
        create<ast::Function>("func", ast::VariableList{}, &void_type_, block);
    mod_.AddFunction(func);
    return body_;
  }

  void DeclareVariable(ast::Variable* var) {
    ASSERT_NE(body_, nullptr);
    body_->append(create<ast::VariableDeclStatement>(var));
  }

  TypeDeterminer* td() { return &td_; }

  BoundArrayAccessorsTransform* transform() { return &transform_; }

  /// Creates a new `ast::Node` owned by the Context. When the Context is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return ctx_.create<T>(std::forward<ARGS>(args)...);
  }

 private:
  Context ctx_;
  ast::Module mod_;
  TypeDeterminer td_;
  ast::type::VoidType void_type_;
  BoundArrayAccessorsTransform transform_;
  ast::BlockStatement* body_ = nullptr;
};

TEST_F(BoundArrayAccessorsTest, Ptrs_Clamp) {
  // var a : array<f32, 3>;
  // const c : u32 =  1;
  // const b : ptr<function, f32> = a[c]
  //
  //   -> const b : ptr<function, i32> = a[min(u32(c), 2)]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::ArrayType ary(&f32, 3);
  ast::type::PointerType ptr_type(&f32, ast::StorageClass::kFunction);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &ary));

  auto* c_var = create<ast::Variable>("c", ast::StorageClass::kFunction, &u32);
  c_var->set_is_const(true);
  DeclareVariable(c_var);

  auto* access_idx = create<ast::IdentifierExpression>("c");

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"), access_idx);
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &ptr_type);
  b->set_constructor(accessor);
  b->set_is_const(true);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsCall());

  auto* idx = ptr->idx_expr()->AsCall();
  ASSERT_TRUE(idx->func()->IsIdentifier());
  EXPECT_EQ(idx->func()->AsIdentifier()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->IsConstructor());
  ASSERT_TRUE(idx->params()[0]->AsConstructor()->IsTypeConstructor());
  auto* tc = idx->params()[0]->AsConstructor()->AsTypeConstructor();
  EXPECT_TRUE(tc->type()->IsU32());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_EQ(tc->values()[0], access_idx);

  ASSERT_TRUE(idx->params()[1]->IsConstructor());
  ASSERT_TRUE(idx->params()[1]->AsConstructor()->IsScalarConstructor());
  auto* scalar = idx->params()[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Nested_Scalar) {
  // var a : array<f32, 3>;
  // var b : array<f32, 5>;
  // var i : u32;
  // var c : f32 = a[b[i]];
  //
  // -> var c : f32 = a[min(u32(b[min(u32(i), 4)]), 2)];

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::ArrayType ary3(&f32, 3);
  ast::type::ArrayType ary5(&f32, 5);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &ary3));
  DeclareVariable(
      create<ast::Variable>("b", ast::StorageClass::kFunction, &ary5));
  DeclareVariable(
      create<ast::Variable>("i", ast::StorageClass::kFunction, &u32));

  auto* b_access_idx = create<ast::IdentifierExpression>("i");

  auto* a_access_idx = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("b"), b_access_idx);

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"), a_access_idx);
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("c", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsCall());

  auto* idx = ptr->idx_expr()->AsCall();
  ASSERT_TRUE(idx->func()->IsIdentifier());
  EXPECT_EQ(idx->func()->AsIdentifier()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->IsConstructor());
  ASSERT_TRUE(idx->params()[0]->AsConstructor()->IsTypeConstructor());
  auto* tc = idx->params()[0]->AsConstructor()->AsTypeConstructor();
  EXPECT_TRUE(tc->type()->IsU32());
  ASSERT_EQ(tc->values().size(), 1u);

  auto* sub = tc->values()[0];
  ASSERT_TRUE(sub->IsArrayAccessor());
  ASSERT_TRUE(sub->AsArrayAccessor()->idx_expr()->IsCall());

  auto* sub_idx = sub->AsArrayAccessor()->idx_expr()->AsCall();
  ASSERT_TRUE(sub_idx->func()->IsIdentifier());
  EXPECT_EQ(sub_idx->func()->AsIdentifier()->name(), "min");

  ASSERT_TRUE(sub_idx->params()[0]->IsConstructor());
  ASSERT_TRUE(sub_idx->params()[0]->AsConstructor()->IsTypeConstructor());
  tc = sub_idx->params()[0]->AsConstructor()->AsTypeConstructor();
  EXPECT_TRUE(tc->type()->IsU32());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_EQ(tc->values()[0], b_access_idx);

  ASSERT_TRUE(sub_idx->params()[1]->IsConstructor());
  ASSERT_TRUE(sub_idx->params()[1]->AsConstructor()->IsScalarConstructor());
  auto* scalar = sub_idx->params()[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 4u);

  ASSERT_TRUE(idx->params()[1]->IsConstructor());
  ASSERT_TRUE(idx->params()[1]->AsConstructor()->IsScalarConstructor());
  scalar = idx->params()[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Scalar) {
  // var a : array<f32, 3>
  // var b : f32 = a[1];
  //
  // -> var b : f32 = a[1];

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::ArrayType ary(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &ary));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 1u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Expr) {
  // var a : array<f32, 3>
  // var c : u32;
  // var b : f32 = a[c + 2 - 3]
  //
  // -> var b : f32 = a[min(u32(c + 2 - 3), 2)]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::ArrayType ary(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &ary));
  DeclareVariable(
      create<ast::Variable>("c", ast::StorageClass::kFunction, &u32));

  auto* access_idx = create<ast::BinaryExpression>(
      ast::BinaryOp::kAdd, create<ast::IdentifierExpression>("c"),
      create<ast::BinaryExpression>(ast::BinaryOp::kSubtract,
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 2)),
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 3))));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"), access_idx);
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsCall());

  auto* idx = ptr->idx_expr()->AsCall();
  ASSERT_TRUE(idx->func()->IsIdentifier());
  EXPECT_EQ(idx->func()->AsIdentifier()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->IsConstructor());
  ASSERT_TRUE(idx->params()[0]->AsConstructor()->IsTypeConstructor());
  auto* tc = idx->params()[0]->AsConstructor()->AsTypeConstructor();
  EXPECT_TRUE(tc->type()->IsU32());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_EQ(tc->values()[0], access_idx);

  ASSERT_TRUE(idx->params()[1]->IsConstructor());
  ASSERT_TRUE(idx->params()[1]->AsConstructor()->IsScalarConstructor());
  auto* scalar = idx->params()[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_Negative) {
  // var a : array<f32, 3>
  // var b : f32 = a[-1]
  //
  // -> var b : f32 = a[0]

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &ary));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, -1)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsSint());
  EXPECT_EQ(scalar->literal()->AsSint()->value(), 0);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsI32());
}

TEST_F(BoundArrayAccessorsTest, Array_Idx_OutOfBounds) {
  // var a : array<f32, 3>
  // var b : f32 = a[3]
  //
  // -> var b : f32 = a[2]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::ArrayType ary(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &ary));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 3u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Scalar) {
  // var a : vec3<f32>
  // var b : f32 = a[1];
  //
  // -> var b : f32 = a[1]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::VectorType vec(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &vec));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 1u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Expr) {
  // var a : vec3<f32>
  // var c : u32;
  // var b : f32 = a[c + 2 - 3]
  //
  // -> var b : f32 = a[min(u32(c + 2 - 3), 2)]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::VectorType vec(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &vec));
  DeclareVariable(
      create<ast::Variable>("c", ast::StorageClass::kFunction, &u32));

  auto* access_idx = create<ast::BinaryExpression>(
      ast::BinaryOp::kAdd, create<ast::IdentifierExpression>("c"),
      create<ast::BinaryExpression>(ast::BinaryOp::kSubtract,
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 2)),
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 3))));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"), access_idx);
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsCall());

  auto* idx = ptr->idx_expr()->AsCall();
  ASSERT_TRUE(idx->func()->IsIdentifier());
  EXPECT_EQ(idx->func()->AsIdentifier()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);
  ASSERT_TRUE(idx->params()[0]->IsConstructor());
  ASSERT_TRUE(idx->params()[0]->AsConstructor()->IsTypeConstructor());
  auto* tc = idx->params()[0]->AsConstructor()->AsTypeConstructor();
  EXPECT_TRUE(tc->type()->IsU32());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_EQ(tc->values()[0], access_idx);

  ASSERT_TRUE(idx->params()[1]->IsConstructor());
  ASSERT_TRUE(idx->params()[1]->AsConstructor()->IsScalarConstructor());
  auto* scalar = idx->params()[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_Negative) {
  // var a : vec3<f32>
  // var b : f32 = a[-1]
  //
  // -> var b : f32 = a[0]

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &vec));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, -1)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsSint());
  EXPECT_EQ(scalar->literal()->AsSint()->value(), 0);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsI32());
}

TEST_F(BoundArrayAccessorsTest, Vector_Idx_OutOfBounds) {
  // var a : vec3<f32>
  // var b : f32 = a[3]
  //
  // -> var b : f32 = a[2]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::VectorType vec(&f32, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &vec));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::IdentifierExpression>("a"),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 3u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());
  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Scalar) {
  // var a : mat3x2<f32>
  // var b : f32 = a[2][1];
  //
  // -> var b : f32 = a[2][1]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::MatrixType mat(&f32, 2, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &mat));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>("a"),
          create<ast::ScalarConstructorExpression>(
              create<ast::UintLiteral>(&u32, 2u))),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 1u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());

  ASSERT_TRUE(ptr->array()->IsArrayAccessor());
  auto* ary = ptr->array()->AsArrayAccessor();
  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->IsU32());

  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Expr_Column) {
  // var a : mat3x2<f32>
  // var c : u32;
  // var b : f32 = a[c + 2 - 3][1]
  //
  // -> var b : f32 = a[min(u32(c + 2 - 3), 2)][1]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::MatrixType mat(&f32, 2, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &mat));
  DeclareVariable(
      create<ast::Variable>("c", ast::StorageClass::kFunction, &u32));

  auto* access_idx = create<ast::BinaryExpression>(
      ast::BinaryOp::kAdd, create<ast::IdentifierExpression>("c"),
      create<ast::BinaryExpression>(ast::BinaryOp::kSubtract,
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 2)),
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 3))));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>("a"), access_idx),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 1u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());

  ASSERT_TRUE(ptr->array()->IsArrayAccessor());
  auto* ary = ptr->array()->AsArrayAccessor();

  ASSERT_TRUE(ary->idx_expr()->IsCall());
  auto* idx = ary->idx_expr()->AsCall();
  ASSERT_TRUE(idx->func()->IsIdentifier());
  EXPECT_EQ(idx->func()->AsIdentifier()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->IsConstructor());
  ASSERT_TRUE(idx->params()[0]->AsConstructor()->IsTypeConstructor());
  auto* tc = idx->params()[0]->AsConstructor()->AsTypeConstructor();
  EXPECT_TRUE(tc->type()->IsU32());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_EQ(tc->values()[0], access_idx);

  ASSERT_TRUE(idx->params()[1]->IsConstructor());
  ASSERT_TRUE(idx->params()[1]->AsConstructor()->IsScalarConstructor());
  auto* scalar = idx->params()[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->IsU32());

  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Expr_Row) {
  // var a : mat3x2<f32>
  // var c : u32;
  // var b : f32 = a[1][c + 2 - 3]
  //
  // -> var b : f32 = a[1][min(u32(c + 2 - 3), 1)]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::MatrixType mat(&f32, 2, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &mat));
  DeclareVariable(
      create<ast::Variable>("c", ast::StorageClass::kFunction, &u32));

  auto* access_idx = create<ast::BinaryExpression>(
      ast::BinaryOp::kAdd, create<ast::IdentifierExpression>("c"),
      create<ast::BinaryExpression>(ast::BinaryOp::kSubtract,
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 2)),
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::UintLiteral>(&u32, 3))));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>("a"),
          create<ast::ScalarConstructorExpression>(
              create<ast::UintLiteral>(&u32, 1u))),
      access_idx);
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());

  ASSERT_TRUE(ptr->array()->IsArrayAccessor());
  auto* ary = ptr->array()->AsArrayAccessor();

  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_TRUE(ptr->idx_expr()->IsCall());
  auto* idx = ptr->idx_expr()->AsCall();
  ASSERT_TRUE(idx->func()->IsIdentifier());
  EXPECT_EQ(idx->func()->AsIdentifier()->name(), "min");

  ASSERT_EQ(idx->params().size(), 2u);

  ASSERT_TRUE(idx->params()[0]->IsConstructor());
  ASSERT_TRUE(idx->params()[0]->AsConstructor()->IsTypeConstructor());
  auto* tc = idx->params()[0]->AsConstructor()->AsTypeConstructor();
  EXPECT_TRUE(tc->type()->IsU32());
  ASSERT_EQ(tc->values().size(), 1u);
  ASSERT_EQ(tc->values()[0], access_idx);

  ASSERT_TRUE(idx->params()[1]->IsConstructor());
  ASSERT_TRUE(idx->params()[1]->AsConstructor()->IsScalarConstructor());
  scalar = idx->params()[1]->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->IsU32());

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Negative_Column) {
  // var a : mat3x2<f32>
  // var b : f32 = a[-1][1]
  //
  // -> var b : f32 = a[0][1]
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 2, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &mat));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>("a"),
          create<ast::ScalarConstructorExpression>(
              create<ast::SintLiteral>(&i32, -1))),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 1)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());

  ASSERT_TRUE(ptr->array()->IsArrayAccessor());
  auto* ary = ptr->array()->AsArrayAccessor();
  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsSint());
  EXPECT_EQ(scalar->literal()->AsSint()->value(), 0);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->IsI32());

  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsSint());
  EXPECT_EQ(scalar->literal()->AsSint()->value(), 1);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsI32());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_Negative_Row) {
  // var a : mat3x2<f32>
  // var b : f32 = a[2][-1]
  //
  // -> var b : f32 = a[2][0]
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::MatrixType mat(&f32, 2, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &mat));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>("a"),
          create<ast::ScalarConstructorExpression>(
              create<ast::SintLiteral>(&i32, 2))),
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, -1)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());

  ASSERT_TRUE(ptr->array()->IsArrayAccessor());
  auto* ary = ptr->array()->AsArrayAccessor();
  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsSint());
  EXPECT_EQ(scalar->literal()->AsSint()->value(), 2);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->IsI32());

  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsSint());
  EXPECT_EQ(scalar->literal()->AsSint()->value(), 0);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsI32());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_OutOfBounds_Column) {
  // var a : mat3x2<f32>
  // var b : f32 = a[5][1]
  //
  // -> var b : f32 = a[2][1]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::MatrixType mat(&f32, 2, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &mat));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>("a"),
          create<ast::ScalarConstructorExpression>(
              create<ast::UintLiteral>(&u32, 5u))),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 1u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());

  ASSERT_TRUE(ptr->array()->IsArrayAccessor());
  auto* ary = ptr->array()->AsArrayAccessor();
  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->IsU32());

  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
}

TEST_F(BoundArrayAccessorsTest, Matrix_Idx_OutOfBounds_Row) {
  // var a : mat3x2<f32>
  // var b : f32 = a[2][5]
  //
  // -> var b : f32 = a[2][1]

  ast::type::F32Type f32;
  ast::type::U32Type u32;
  ast::type::MatrixType mat(&f32, 2, 3);

  SetupFunctionAndBody();
  DeclareVariable(
      create<ast::Variable>("a", ast::StorageClass::kFunction, &mat));

  auto* accessor = create<ast::ArrayAccessorExpression>(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>("a"),
          create<ast::ScalarConstructorExpression>(
              create<ast::UintLiteral>(&u32, 2u))),
      create<ast::ScalarConstructorExpression>(
          create<ast::UintLiteral>(&u32, 5u)));
  auto* ptr = accessor;

  auto* b = create<ast::Variable>("b", ast::StorageClass::kFunction, &f32);
  b->set_constructor(accessor);
  DeclareVariable(b);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  ASSERT_TRUE(transform()->Run());
  ASSERT_TRUE(ptr->IsArrayAccessor());

  ASSERT_TRUE(ptr->array()->IsArrayAccessor());
  auto* ary = ptr->array()->AsArrayAccessor();
  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());

  auto* scalar = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 2u);

  ASSERT_NE(ary->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ary->idx_expr()->result_type()->IsU32());

  ASSERT_TRUE(ptr->idx_expr()->IsConstructor());
  ASSERT_TRUE(ptr->idx_expr()->AsConstructor()->IsScalarConstructor());

  scalar = ptr->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_TRUE(scalar->literal()->IsUint());
  EXPECT_EQ(scalar->literal()->AsUint()->value(), 1u);

  ASSERT_NE(ptr->idx_expr()->result_type(), nullptr);
  ASSERT_TRUE(ptr->idx_expr()->result_type()->IsU32());
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
