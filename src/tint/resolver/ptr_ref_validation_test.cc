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

#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/reference_type.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

struct ResolverPtrRefValidationTest : public resolver::TestHelper,
                                      public testing::Test {};

TEST_F(ResolverPtrRefValidationTest, AddressOfLiteral) {
  // &1

  auto* expr = AddressOf(Expr(Source{{12, 34}}, 1));

  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfLet) {
  // let l : i32 = 1;
  // &l
  auto* l = Let("l", ty.i32(), Expr(1));
  auto* expr = AddressOf(Expr(Source{{12, 34}}, "l"));

  WrapInFunction(l, expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfHandle) {
  // @group(0) @binding(0) var t: texture_3d<f32>;
  // &t
  Global("t", ty.sampled_texture(ast::TextureDimension::k3d, ty.f32()),
         GroupAndBinding(0u, 0u));
  auto* expr = AddressOf(Expr(Source{{12, 34}}, "t"));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: cannot take the address of expression in handle "
            "storage class");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfVectorComponent_MemberAccessor) {
  // var v : vec4<i32>;
  // &v.y
  auto* v = Var("v", ty.vec4<i32>());
  auto* expr = AddressOf(MemberAccessor(Source{{12, 34}}, "v", "y"));

  WrapInFunction(v, expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: cannot take the address of a vector component");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfVectorComponent_IndexAccessor) {
  // var v : vec4<i32>;
  // &v[2]
  auto* v = Var("v", ty.vec4<i32>());
  auto* expr = AddressOf(IndexAccessor(Source{{12, 34}}, "v", 2));

  WrapInFunction(v, expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: cannot take the address of a vector component");
}

TEST_F(ResolverPtrRefValidationTest, IndirectOfAddressOfHandle) {
  // @group(0) @binding(0) var t: texture_3d<f32>;
  // *&t
  Global("t", ty.sampled_texture(ast::TextureDimension::k3d, ty.f32()),
         GroupAndBinding(0u, 0u));
  auto* expr = Deref(AddressOf(Expr(Source{{12, 34}}, "t")));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: cannot take the address of expression in handle "
            "storage class");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfLiteral) {
  // *1

  auto* expr = Deref(Expr(Source{{12, 34}}, 1));

  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: cannot dereference expression of type 'i32'");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfVar) {
  // var v : i32 = 1;
  // *1
  auto* v = Var("v", ty.i32());
  auto* expr = Deref(Expr(Source{{12, 34}}, "v"));

  WrapInFunction(v, expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: cannot dereference expression of type 'i32'");
}

TEST_F(ResolverPtrRefValidationTest, InferredPtrAccessMismatch) {
  // struct Inner {
  //    arr: array<i32, 4>;
  // }
  // struct S {
  //    inner: Inner;
  // }
  // @group(0) @binding(0) var<storage, read_write> s : S;
  // fn f() {
  //   let p : pointer<storage, i32> = &s.inner.arr[2];
  // }
  auto* inner = Structure("Inner", {Member("arr", ty.array<i32, 4>())});
  auto* buf = Structure("S", {Member("inner", ty.Of(inner))});
  auto* storage = Global("s", ty.Of(buf), ast::StorageClass::kStorage,
                         ast::Access::kReadWrite,
                         ast::AttributeList{
                             create<ast::BindingAttribute>(0),
                             create<ast::GroupAttribute>(0),
                         });

  auto* expr =
      IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 4);
  auto* ptr =
      Let(Source{{12, 34}}, "p", ty.pointer<i32>(ast::StorageClass::kStorage),
          AddressOf(expr));

  WrapInFunction(ptr);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: cannot initialize let of type "
            "'ptr<storage, i32, read>' with value of type "
            "'ptr<storage, i32, read_write>'");
}

}  // namespace
}  // namespace tint::resolver
