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
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/reference_type.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

struct ResolverVarLetTest : public resolver::TestHelper,
                            public testing::Test {};

TEST_F(ResolverVarLetTest, TypeOfVar) {
  // struct S { i : i32; }
  // alias A = S;
  // fn F(){
  //   var i : i32;
  //   var u : u32;
  //   var f : f32;
  //   var b : bool;
  //   var s : S;
  //   var a : A;
  // }

  auto* S = Structure("S", {Member("i", ty.i32())});
  auto* A = ty.alias("A", S);
  AST().AddConstructedType(A);

  auto* i = Var("i", ty.i32(), ast::StorageClass::kNone);
  auto* u = Var("u", ty.u32(), ast::StorageClass::kNone);
  auto* f = Var("f", ty.f32(), ast::StorageClass::kNone);
  auto* b = Var("b", ty.bool_(), ast::StorageClass::kNone);
  auto* s = Var("s", S, ast::StorageClass::kNone);
  auto* a = Var("a", A, ast::StorageClass::kNone);

  Func("F", {}, ty.void_(),
       {
           Decl(i),
           Decl(u),
           Decl(f),
           Decl(b),
           Decl(s),
           Decl(a),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  // `var` declarations are always of reference type
  ASSERT_TRUE(TypeOf(i)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(u)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(f)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(b)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(s)->Is<sem::Reference>());
  ASSERT_TRUE(TypeOf(a)->Is<sem::Reference>());

  EXPECT_TRUE(TypeOf(i)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
  EXPECT_TRUE(TypeOf(u)->As<sem::Reference>()->StoreType()->Is<sem::U32>());
  EXPECT_TRUE(TypeOf(f)->As<sem::Reference>()->StoreType()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(b)->As<sem::Reference>()->StoreType()->Is<sem::Bool>());
  EXPECT_TRUE(TypeOf(s)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
  EXPECT_TRUE(TypeOf(a)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
}

TEST_F(ResolverVarLetTest, TypeOfLet) {
  // struct S { i : i32; }
  // fn F(){
  //   var v : i32;
  //   let i : i32 = 1;
  //   let u : u32 = 1u;
  //   let f : f32 = 1.;
  //   let b : bool = true;
  //   let s : S = S(1);
  //   let a : A = A(1);
  //   let p : pointer<function, i32> = &V;
  // }

  auto* S = Structure("S", {Member("i", ty.i32())});
  auto* A = ty.alias("A", S);
  AST().AddConstructedType(A);

  auto* v = Var("v", ty.i32(), ast::StorageClass::kNone);
  auto* i = Const("i", ty.i32(), Expr(1));
  auto* u = Const("u", ty.u32(), Expr(1u));
  auto* f = Const("f", ty.f32(), Expr(1.f));
  auto* b = Const("b", ty.bool_(), Expr(true));
  auto* s = Const("s", S, Construct(S, Expr(1)));
  auto* a = Const("a", A, Construct(A, Expr(1)));
  auto* p =
      Const("p", ty.pointer<i32>(ast::StorageClass::kFunction), AddressOf(v));

  Func("F", {}, ty.void_(),
       {
           Decl(v),
           Decl(i),
           Decl(u),
           Decl(f),
           Decl(b),
           Decl(s),
           Decl(a),
           Decl(p),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  // `let` declarations are always of the storage type
  EXPECT_TRUE(TypeOf(i)->Is<sem::I32>());
  EXPECT_TRUE(TypeOf(u)->Is<sem::U32>());
  EXPECT_TRUE(TypeOf(f)->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(b)->Is<sem::Bool>());
  EXPECT_TRUE(TypeOf(s)->Is<sem::Struct>());
  EXPECT_TRUE(TypeOf(a)->Is<sem::Struct>());
  ASSERT_TRUE(TypeOf(p)->Is<sem::Pointer>());
  EXPECT_TRUE(TypeOf(p)->As<sem::Pointer>()->StoreType()->Is<sem::I32>());
}

}  // namespace
}  // namespace resolver
}  // namespace tint
