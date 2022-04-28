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

#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/storage_texture.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;
template <typename T>
using mat2x2 = builder::mat2x2<T>;
template <typename T>
using mat3x3 = builder::mat3x3<T>;
template <typename T>
using mat4x4 = builder::mat4x4<T>;
template <int N, typename T>
using array = builder::array<N, T>;
template <typename T>
using alias = builder::alias<T>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;
using f32 = builder::f32;
using i32 = builder::i32;
using u32 = builder::u32;

class ResolverTypeValidationTest : public resolver::TestHelper,
                                   public testing::Test {};

TEST_F(ResolverTypeValidationTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, nullptr);
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body =
      Block(Decl(var), Assign(Source{Source::Location{12, 34}}, lhs, rhs));

  WrapInFunction(body);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
}

TEST_F(ResolverTypeValidationTest, GlobalConstantNoConstructor_Pass) {
  // @id(0) override a :i32;
  Override(Source{{12, 34}}, "a", ty.i32(), nullptr, ast::AttributeList{Id(0)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableWithStorageClass_Pass) {
  // var<private> global_var: f32;
  Global(Source{{12, 34}}, "global_var", ty.f32(), ast::StorageClass::kPrivate);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalConstantWithStorageClass_Fail) {
  // const<private> global_var: f32;
  AST().AddGlobalVariable(create<ast::Variable>(
      Source{{12, 34}}, Symbols().Register("global_var"),
      ast::StorageClass::kPrivate, ast::Access::kUndefined, ty.f32(), true,
      false, Expr(1.23f), ast::AttributeList{}));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: global constants shouldn't have a storage class");
}

TEST_F(ResolverTypeValidationTest, GlobalConstNoStorageClass_Pass) {
  // let global_var: f32;
  GlobalConst(Source{{12, 34}}, "global_var", ty.f32(), Construct(ty.f32()));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableUnique_Pass) {
  // var global_var0 : f32 = 0.1;
  // var global_var1 : i32 = 0;

  Global("global_var0", ty.f32(), ast::StorageClass::kPrivate, Expr(0.1f));

  Global(Source{{12, 34}}, "global_var1", ty.f32(), ast::StorageClass::kPrivate,
         Expr(1.0f));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest,
       GlobalVariableFunctionVariableNotUnique_Pass) {
  // fn my_func() {
  //   var a: f32 = 2.0;
  // }
  // var a: f32 = 2.1;

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  Func("my_func", ast::VariableList{}, ty.void_(), {Decl(var)});

  Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScope_Pass) {
  // {
  // if (true) { var a : f32 = 2.0; }
  // var a : f32 = 3.14;
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = Block(Decl(var));

  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

  auto* outer_body =
      Block(create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
            Decl(Source{{12, 34}}, var_a_float));

  WrapInFunction(outer_body);

  EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScopeBlock_Pass) {
  // {
  //  { var a : f32; }
  //  var a : f32;
  // }
  auto* var_inner = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* inner = Block(Decl(Source{{12, 34}}, var_inner));

  auto* var_outer = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* outer_body = Block(inner, Decl(var_outer));

  WrapInFunction(outer_body);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest,
       RedeclaredIdentifierDifferentFunctions_Pass) {
  // func0 { var a : f32 = 2.0; return; }
  // func1 { var a : f32 = 3.0; return; }
  auto* var0 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* var1 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(1.0f));

  Func("func0", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Source{{12, 34}}, var0),
           Return(),
       },
       ast::AttributeList{});

  Func("func1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Source{{13, 34}}, var1),
           Return(),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedLiteral_Pass) {
  // var<private> a : array<f32, 4u>;
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 4u)),
         ast::StorageClass::kPrivate);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedLiteral_Pass) {
  // var<private> a : array<f32, 4>;
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 4)),
         ast::StorageClass::kPrivate);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedConstant_Pass) {
  // let size = 4u;
  // var<private> a : array<f32, size>;
  GlobalConst("size", nullptr, Expr(4u));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedConstant_Pass) {
  // let size = 4;
  // var<private> a : array<f32, size>;
  GlobalConst("size", nullptr, Expr(4));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedLiteral_Zero) {
  // var<private> a : array<f32, 0u>;
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 0u)),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be at least 1");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedLiteral_Zero) {
  // var<private> a : array<f32, 0>;
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 0)),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be at least 1");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedLiteral_Negative) {
  // var<private> a : array<f32, -10>;
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, -10)),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be at least 1");
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedConstant_Zero) {
  // let size = 0u;
  // var<private> a : array<f32, size>;
  GlobalConst("size", nullptr, Expr(0u));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be at least 1");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedConstant_Zero) {
  // let size = 0;
  // var<private> a : array<f32, size>;
  GlobalConst("size", nullptr, Expr(0));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be at least 1");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedConstant_Negative) {
  // let size = -10;
  // var<private> a : array<f32, size>;
  GlobalConst("size", nullptr, Expr(-10));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be at least 1");
}

TEST_F(ResolverTypeValidationTest, ArraySize_FloatLiteral) {
  // var<private> a : array<f32, 10.0>;
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 10.f)),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be integer scalar");
}

TEST_F(ResolverTypeValidationTest, ArraySize_IVecLiteral) {
  // var<private> a : array<f32, vec2<i32>(10, 10)>;
  Global(
      "a",
      ty.array(ty.f32(), Construct(Source{{12, 34}}, ty.vec2<i32>(), 10, 10)),
      ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be integer scalar");
}

TEST_F(ResolverTypeValidationTest, ArraySize_FloatConstant) {
  // let size = 10.0;
  // var<private> a : array<f32, size>;
  GlobalConst("size", nullptr, Expr(10.f));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be integer scalar");
}

TEST_F(ResolverTypeValidationTest, ArraySize_IVecConstant) {
  // let size = vec2<i32>(100, 100);
  // var<private> a : array<f32, size>;
  GlobalConst("size", nullptr, Construct(ty.vec2<i32>(), 100, 100));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: array size must be integer scalar");
}

TEST_F(ResolverTypeValidationTest, ArraySize_TooBig_ImplicitStride) {
  // var<private> a : array<f32, 0x40000000>;
  Global("a", ty.array(Source{{12, 34}}, ty.f32(), 0x40000000),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: array size in bytes must not exceed 0xffffffff, but "
            "is 0x100000000");
}

TEST_F(ResolverTypeValidationTest, ArraySize_TooBig_ExplicitStride) {
  // var<private> a : @stride(8) array<f32, 0x20000000>;
  Global("a", ty.array(Source{{12, 34}}, ty.f32(), 0x20000000, 8),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: array size in bytes must not exceed 0xffffffff, but "
            "is 0x100000000");
}

TEST_F(ResolverTypeValidationTest, ArraySize_OverridableConstant) {
  // override size = 10;
  // var<private> a : array<f32, size>;
  Override("size", nullptr, Expr(10));
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: array size expression must not be pipeline-overridable");
}

TEST_F(ResolverTypeValidationTest, ArraySize_ModuleVar) {
  // var<private> size : i32 = 10;
  // var<private> a : array<f32, size>;
  Global("size", ty.i32(), Expr(10), ast::StorageClass::kPrivate);
  Global("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: array size identifier must be a module-scope constant");
}

TEST_F(ResolverTypeValidationTest, ArraySize_FunctionConstant) {
  // {
  //   let size = 10;
  //   var a : array<f32, size>;
  // }
  auto* size = Let("size", nullptr, Expr(10));
  auto* a = Var("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")));
  WrapInFunction(Block(Decl(size), Decl(a)));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: array size identifier must be a module-scope constant");
}

TEST_F(ResolverTypeValidationTest, ArraySize_InvalidExpr) {
  // var a : array<f32, i32(4)>;
  auto* size = Let("size", nullptr, Expr(10));
  auto* a =
      Var("a", ty.array(ty.f32(), Construct(Source{{12, 34}}, ty.i32(), 4)));
  WrapInFunction(Block(Decl(size), Decl(a)));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: array size expression must be either a literal or a "
            "module-scope constant");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInFunction_Fail) {
  /// @stage(vertex)
  // fn func() { var a : array<i32>; }

  auto* var =
      Var(Source{{12, 34}}, "a", ty.array<i32>(), ast::StorageClass::kNone);

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(var),
       },
       ast::AttributeList{
           Stage(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
12:34 note: while instantiating variable a)");
}

TEST_F(ResolverTypeValidationTest, Struct_Member_VectorNoType) {
  // struct S {
  //   a: vec3;
  // };

  Structure("S",
            {Member("a", create<ast::Vector>(Source{{12, 34}}, nullptr, 3))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: missing vector element type");
}

TEST_F(ResolverTypeValidationTest, Struct_Member_MatrixNoType) {
  // struct S {
  //   a: mat3x3;
  // };
  Structure(
      "S", {Member("a", create<ast::Matrix>(Source{{12, 34}}, nullptr, 3, 3))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: missing matrix element type");
}

TEST_F(ResolverTypeValidationTest, Struct_TooBig) {
  // struct Foo {
  //   a: array<f32, 0x20000000>;
  //   b: array<f32, 0x20000000>;
  // };

  Structure(Source{{12, 34}}, "Foo",
            {
                Member("a", ty.array<f32, 0x20000000>()),
                Member("b", ty.array<f32, 0x20000000>()),
            });

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: struct size in bytes must not exceed 0xffffffff, but "
            "is 0x100000000");
}

TEST_F(ResolverTypeValidationTest, Struct_MemberOffset_TooBig) {
  // struct Foo {
  //   a: array<f32, 0x3fffffff>;
  //   b: f32;
  //   c: f32;
  // };

  Structure("Foo", {
                       Member("a", ty.array<f32, 0x3fffffff>()),
                       Member("b", ty.f32()),
                       Member(Source{{12, 34}}, "c", ty.f32()),
                   });

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: struct member has byte offset 0x100000000, but must "
            "not exceed 0xffffffff");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLast_Pass) {
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  Structure("Foo", {
                       Member("vf", ty.f32()),
                       Member("rt", ty.array<f32>()),
                   });

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInArray) {
  // struct Foo {
  //   rt : array<array<f32>, 4>;
  // };

  Structure("Foo",
            {Member("rt", ty.array(Source{{12, 34}}, ty.array<f32>(), 4))});

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: an array element type cannot contain a runtime-sized "
            "array");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInStructInArray) {
  // struct Foo {
  //   rt : array<f32>;
  // };
  // var<private> a : array<Foo, 4>;

  auto* foo = Structure("Foo", {Member("rt", ty.array<f32>())});
  Global("v", ty.array(Source{{12, 34}}, ty.Of(foo), 4),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: an array element type cannot contain a runtime-sized "
            "array");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInStructInStruct) {
  // struct Foo {
  //   rt : array<f32>;
  // };
  // struct Outer {
  //   inner : Foo;
  // };

  auto* foo = Structure("Foo", {Member("rt", ty.array<f32>())});
  Structure("Outer", {Member(Source{{12, 34}}, "inner", ty.Of(foo))});

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: a struct that contains a runtime array cannot be "
            "nested inside another struct");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsNotLast_Fail) {
  // struct Foo {
  //   rt: array<f32>;
  //   vf: f32;
  // };

  Structure("Foo", {
                       Member(Source{{12, 34}}, "rt", ty.array<f32>()),
                       Member("vf", ty.f32()),
                   });

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: runtime arrays may only appear as the last member of a struct)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsGlobalVariable) {
  Global(Source{{56, 78}}, "g", ty.array<i32>(), ast::StorageClass::kPrivate);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: runtime-sized arrays can only be used in the <storage> storage class
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsLocalVariable) {
  auto* v = Var(Source{{56, 78}}, "g", ty.array<i32>());
  WrapInFunction(v);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: runtime-sized arrays can only be used in the <storage> storage class
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsParameter_Fail) {
  // fn func(a : array<u32>) {}
  // @stage(vertex) fn main() {}

  auto* param = Param(Source{{12, 34}}, "a", ty.array<i32>());

  Func("func", ast::VariableList{param}, ty.void_(),
       ast::StatementList{
           Return(),
       },
       ast::AttributeList{});

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Return(),
       },
       ast::AttributeList{
           Stage(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
12:34 note: while instantiating parameter a)");
}

TEST_F(ResolverTypeValidationTest, PtrToRuntimeArrayAsParameter_Fail) {
  // fn func(a : ptr<workgroup, array<u32>>) {}

  auto* param =
      Param(Source{{12, 34}}, "a",
            ty.pointer(ty.array<i32>(), ast::StorageClass::kWorkgroup));

  Func("func", ast::VariableList{param}, ty.void_(),
       ast::StatementList{
           Return(),
       },
       ast::AttributeList{});

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
12:34 note: while instantiating parameter a)");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsNotLast_Fail) {
  // type RTArr = array<u32>;
  // struct s {
  //  b: RTArr;
  //  a: u32;
  //}

  auto* alias = Alias("RTArr", ty.array<u32>());
  Structure("s", {
                     Member(Source{{12, 34}}, "b", ty.Of(alias)),
                     Member("a", ty.u32()),
                 });

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: runtime arrays may only appear as the last member of "
            "a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsLast_Pass) {
  // type RTArr = array<u32>;
  // struct s {
  //  a: u32;
  //  b: RTArr;
  //}

  auto* alias = Alias("RTArr", ty.array<u32>());
  Structure("s", {
                     Member("a", ty.u32()),
                     Member("b", ty.Of(alias)),
                 });

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArrayOfNonStorableType) {
  auto* tex_ty = ty.sampled_texture(ast::TextureDimension::k2d, ty.f32());
  Global("arr", ty.array(Source{{12, 34}}, tex_ty, 4),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: texture_2d<f32> cannot be used as an element type of "
            "an array");
}

TEST_F(ResolverTypeValidationTest, VariableAsType) {
  // var<private> a : i32;
  // var<private> b : a;
  Global("a", ty.i32(), ast::StorageClass::kPrivate);
  Global("b", ty.type_name("a"), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(error: cannot use variable 'a' as type
note: 'a' declared here)");
}

TEST_F(ResolverTypeValidationTest, FunctionAsType) {
  // fn f() {}
  // var<private> v : f;
  Func("f", {}, ty.void_(), {});
  Global("v", ty.type_name("f"), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(error: cannot use function 'f' as type
note: 'f' declared here)");
}

TEST_F(ResolverTypeValidationTest, BuiltinAsType) {
  // var<private> v : max;
  Global("v", ty.type_name("max"), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "error: cannot use builtin 'max' as type");
}

namespace GetCanonicalTests {
struct Params {
  builder::ast_type_func_ptr create_ast_type;
  builder::sem_type_func_ptr create_sem_type;
};

template <typename T>
constexpr Params ParamsFor() {
  return Params{DataType<T>::AST, DataType<T>::Sem};
}

static constexpr Params cases[] = {
    ParamsFor<bool>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias1<alias<bool>>>(),

    ParamsFor<vec3<f32>>(),
    ParamsFor<alias<vec3<f32>>>(),
    ParamsFor<alias1<alias<vec3<f32>>>>(),

    ParamsFor<vec3<alias<f32>>>(),
    ParamsFor<alias1<vec3<alias<f32>>>>(),
    ParamsFor<alias2<alias1<vec3<alias<f32>>>>>(),
    ParamsFor<alias3<alias2<vec3<alias1<alias<f32>>>>>>(),

    ParamsFor<mat3x3<alias<f32>>>(),
    ParamsFor<alias1<mat3x3<alias<f32>>>>(),
    ParamsFor<alias2<alias1<mat3x3<alias<f32>>>>>(),
    ParamsFor<alias3<alias2<mat3x3<alias1<alias<f32>>>>>>(),

    ParamsFor<alias1<alias<bool>>>(),
    ParamsFor<alias1<alias<vec3<f32>>>>(),
    ParamsFor<alias1<alias<mat3x3<f32>>>>(),
};

using CanonicalTest = ResolverTestWithParam<Params>;
TEST_P(CanonicalTest, All) {
  auto& params = GetParam();

  auto* type = params.create_ast_type(*this);

  auto* var = Var("v", type);
  auto* expr = Expr("v");
  WrapInFunction(var, expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* got = TypeOf(expr)->UnwrapRef();
  auto* expected = params.create_sem_type(*this);

  EXPECT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         CanonicalTest,
                         testing::ValuesIn(cases));

}  // namespace GetCanonicalTests

namespace MultisampledTextureTests {
struct DimensionParams {
  ast::TextureDimension dim;
  bool is_valid;
};

static constexpr DimensionParams dimension_cases[] = {
    DimensionParams{ast::TextureDimension::k1d, false},
    DimensionParams{ast::TextureDimension::k2d, true},
    DimensionParams{ast::TextureDimension::k2dArray, false},
    DimensionParams{ast::TextureDimension::k3d, false},
    DimensionParams{ast::TextureDimension::kCube, false},
    DimensionParams{ast::TextureDimension::kCubeArray, false}};

using MultisampledTextureDimensionTest = ResolverTestWithParam<DimensionParams>;
TEST_P(MultisampledTextureDimensionTest, All) {
  auto& params = GetParam();
  Global(Source{{12, 34}}, "a", ty.multisampled_texture(params.dim, ty.i32()),
         ast::StorageClass::kNone, nullptr,
         ast::AttributeList{GroupAndBinding(0, 0)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: only 2d multisampled textures are supported");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         MultisampledTextureDimensionTest,
                         testing::ValuesIn(dimension_cases));

struct TypeParams {
  builder::ast_type_func_ptr type_func;
  bool is_valid;
};

template <typename T>
constexpr TypeParams TypeParamsFor(bool is_valid) {
  return TypeParams{DataType<T>::AST, is_valid};
}

static constexpr TypeParams type_cases[] = {
    TypeParamsFor<bool>(false),
    TypeParamsFor<i32>(true),
    TypeParamsFor<u32>(true),
    TypeParamsFor<f32>(true),

    TypeParamsFor<alias<bool>>(false),
    TypeParamsFor<alias<i32>>(true),
    TypeParamsFor<alias<u32>>(true),
    TypeParamsFor<alias<f32>>(true),

    TypeParamsFor<vec3<f32>>(false),
    TypeParamsFor<mat3x3<f32>>(false),

    TypeParamsFor<alias<vec3<f32>>>(false),
    TypeParamsFor<alias<mat3x3<f32>>>(false),
};

using MultisampledTextureTypeTest = ResolverTestWithParam<TypeParams>;
TEST_P(MultisampledTextureTypeTest, All) {
  auto& params = GetParam();
  Global(Source{{12, 34}}, "a",
         ty.multisampled_texture(ast::TextureDimension::k2d,
                                 params.type_func(*this)),
         ast::StorageClass::kNone, nullptr,
         ast::AttributeList{GroupAndBinding(0, 0)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: texture_multisampled_2d<type>: type must be f32, "
              "i32 or u32");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         MultisampledTextureTypeTest,
                         testing::ValuesIn(type_cases));

}  // namespace MultisampledTextureTests

namespace StorageTextureTests {
struct DimensionParams {
  ast::TextureDimension dim;
  bool is_valid;
};

static constexpr DimensionParams Dimension_cases[] = {
    DimensionParams{ast::TextureDimension::k1d, true},
    DimensionParams{ast::TextureDimension::k2d, true},
    DimensionParams{ast::TextureDimension::k2dArray, true},
    DimensionParams{ast::TextureDimension::k3d, true},
    DimensionParams{ast::TextureDimension::kCube, false},
    DimensionParams{ast::TextureDimension::kCubeArray, false}};

using StorageTextureDimensionTest = ResolverTestWithParam<DimensionParams>;
TEST_P(StorageTextureDimensionTest, All) {
  // @group(0) @binding(0)
  // var a : texture_storage_*<ru32int, write>;
  auto& params = GetParam();

  auto* st =
      ty.storage_texture(Source{{12, 34}}, params.dim,
                         ast::TexelFormat::kR32Uint, ast::Access::kWrite);

  Global("a", st, ast::StorageClass::kNone,
         ast::AttributeList{GroupAndBinding(0, 0)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cube dimensions for storage textures are not "
              "supported");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         StorageTextureDimensionTest,
                         testing::ValuesIn(Dimension_cases));

struct FormatParams {
  ast::TexelFormat format;
  bool is_valid;
};

static constexpr FormatParams format_cases[] = {
    FormatParams{ast::TexelFormat::kR32Float, true},
    FormatParams{ast::TexelFormat::kR32Sint, true},
    FormatParams{ast::TexelFormat::kR32Uint, true},
    FormatParams{ast::TexelFormat::kRg32Float, true},
    FormatParams{ast::TexelFormat::kRg32Sint, true},
    FormatParams{ast::TexelFormat::kRg32Uint, true},
    FormatParams{ast::TexelFormat::kRgba16Float, true},
    FormatParams{ast::TexelFormat::kRgba16Sint, true},
    FormatParams{ast::TexelFormat::kRgba16Uint, true},
    FormatParams{ast::TexelFormat::kRgba32Float, true},
    FormatParams{ast::TexelFormat::kRgba32Sint, true},
    FormatParams{ast::TexelFormat::kRgba32Uint, true},
    FormatParams{ast::TexelFormat::kRgba8Sint, true},
    FormatParams{ast::TexelFormat::kRgba8Snorm, true},
    FormatParams{ast::TexelFormat::kRgba8Uint, true},
    FormatParams{ast::TexelFormat::kRgba8Unorm, true}};

using StorageTextureFormatTest = ResolverTestWithParam<FormatParams>;
TEST_P(StorageTextureFormatTest, All) {
  auto& params = GetParam();
  // @group(0) @binding(0)
  // var a : texture_storage_1d<*, write>;
  // @group(0) @binding(1)
  // var b : texture_storage_2d<*, write>;
  // @group(0) @binding(2)
  // var c : texture_storage_2d_array<*, write>;
  // @group(0) @binding(3)
  // var d : texture_storage_3d<*, write>;

  auto* st_a = ty.storage_texture(Source{{12, 34}}, ast::TextureDimension::k1d,
                                  params.format, ast::Access::kWrite);
  Global("a", st_a, ast::StorageClass::kNone,
         ast::AttributeList{GroupAndBinding(0, 0)});

  auto* st_b = ty.storage_texture(ast::TextureDimension::k2d, params.format,
                                  ast::Access::kWrite);
  Global("b", st_b, ast::StorageClass::kNone,
         ast::AttributeList{GroupAndBinding(0, 1)});

  auto* st_c = ty.storage_texture(ast::TextureDimension::k2dArray,
                                  params.format, ast::Access::kWrite);
  Global("c", st_c, ast::StorageClass::kNone,
         ast::AttributeList{GroupAndBinding(0, 2)});

  auto* st_d = ty.storage_texture(ast::TextureDimension::k3d, params.format,
                                  ast::Access::kWrite);
  Global("d", st_d, ast::StorageClass::kNone,
         ast::AttributeList{GroupAndBinding(0, 3)});

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: image format must be one of the texel formats "
              "specified for storage textues in "
              "https://gpuweb.github.io/gpuweb/wgsl/#texel-formats");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         StorageTextureFormatTest,
                         testing::ValuesIn(format_cases));

using StorageTextureAccessTest = ResolverTest;

TEST_F(StorageTextureAccessTest, MissingAccess_Fail) {
  // @group(0) @binding(0)
  // var a : texture_storage_1d<ru32int>;

  auto* st =
      ty.storage_texture(Source{{12, 34}}, ast::TextureDimension::k1d,
                         ast::TexelFormat::kR32Uint, ast::Access::kUndefined);

  Global("a", st, ast::StorageClass::kNone,
         ast::AttributeList{GroupAndBinding(0, 0)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: storage texture missing access control");
}

TEST_F(StorageTextureAccessTest, RWAccess_Fail) {
  // @group(0) @binding(0)
  // var a : texture_storage_1d<ru32int, read_write>;

  auto* st =
      ty.storage_texture(Source{{12, 34}}, ast::TextureDimension::k1d,
                         ast::TexelFormat::kR32Uint, ast::Access::kReadWrite);

  Global("a", st, ast::StorageClass::kNone, nullptr,
         ast::AttributeList{GroupAndBinding(0, 0)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: storage textures currently only support 'write' "
            "access control");
}

TEST_F(StorageTextureAccessTest, ReadOnlyAccess_Fail) {
  // @group(0) @binding(0)
  // var a : texture_storage_1d<ru32int, read>;

  auto* st = ty.storage_texture(Source{{12, 34}}, ast::TextureDimension::k1d,
                                ast::TexelFormat::kR32Uint, ast::Access::kRead);

  Global("a", st, ast::StorageClass::kNone, nullptr,
         ast::AttributeList{GroupAndBinding(0, 0)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: storage textures currently only support 'write' "
            "access control");
}

TEST_F(StorageTextureAccessTest, WriteOnlyAccess_Pass) {
  // @group(0) @binding(0)
  // var a : texture_storage_1d<ru32int, write>;

  auto* st =
      ty.storage_texture(ast::TextureDimension::k1d, ast::TexelFormat::kR32Uint,
                         ast::Access::kWrite);

  Global("a", st, ast::StorageClass::kNone, nullptr,
         ast::AttributeList{GroupAndBinding(0, 0)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace StorageTextureTests

namespace MatrixTests {
struct Params {
  uint32_t columns;
  uint32_t rows;
  builder::ast_type_func_ptr elem_ty;
};

template <typename T>
constexpr Params ParamsFor(uint32_t columns, uint32_t rows) {
  return Params{columns, rows, DataType<T>::AST};
}

using ValidMatrixTypes = ResolverTestWithParam<Params>;
TEST_P(ValidMatrixTypes, Okay) {
  // var a : matNxM<EL_TY>;
  auto& params = GetParam();
  Global("a", ty.mat(params.elem_ty(*this), params.columns, params.rows),
         ast::StorageClass::kPrivate);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         ValidMatrixTypes,
                         testing::Values(ParamsFor<f32>(2, 2),
                                         ParamsFor<f32>(2, 3),
                                         ParamsFor<f32>(2, 4),
                                         ParamsFor<f32>(3, 2),
                                         ParamsFor<f32>(3, 3),
                                         ParamsFor<f32>(3, 4),
                                         ParamsFor<f32>(4, 2),
                                         ParamsFor<f32>(4, 3),
                                         ParamsFor<f32>(4, 4),
                                         ParamsFor<alias<f32>>(4, 2),
                                         ParamsFor<alias<f32>>(4, 3),
                                         ParamsFor<alias<f32>>(4, 4)));

using InvalidMatrixElementTypes = ResolverTestWithParam<Params>;
TEST_P(InvalidMatrixElementTypes, InvalidElementType) {
  // var a : matNxM<EL_TY>;
  auto& params = GetParam();
  Global("a",
         ty.mat(Source{{12, 34}}, params.elem_ty(*this), params.columns,
                params.rows),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: matrix element type must be 'f32'");
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         InvalidMatrixElementTypes,
                         testing::Values(ParamsFor<bool>(4, 2),
                                         ParamsFor<i32>(4, 3),
                                         ParamsFor<u32>(4, 4),
                                         ParamsFor<vec2<f32>>(2, 2),
                                         ParamsFor<vec3<i32>>(2, 3),
                                         ParamsFor<vec4<u32>>(2, 4),
                                         ParamsFor<mat2x2<f32>>(3, 2),
                                         ParamsFor<mat3x3<f32>>(3, 3),
                                         ParamsFor<mat4x4<f32>>(3, 4),
                                         ParamsFor<array<2, f32>>(4, 2)));
}  // namespace MatrixTests

namespace VectorTests {
struct Params {
  uint32_t width;
  builder::ast_type_func_ptr elem_ty;
};

template <typename T>
constexpr Params ParamsFor(uint32_t width) {
  return Params{width, DataType<T>::AST};
}

using ValidVectorTypes = ResolverTestWithParam<Params>;
TEST_P(ValidVectorTypes, Okay) {
  // var a : vecN<EL_TY>;
  auto& params = GetParam();
  Global("a", ty.vec(params.elem_ty(*this), params.width),
         ast::StorageClass::kPrivate);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         ValidVectorTypes,
                         testing::Values(ParamsFor<bool>(2),
                                         ParamsFor<f32>(2),
                                         ParamsFor<i32>(2),
                                         ParamsFor<u32>(2),
                                         ParamsFor<bool>(3),
                                         ParamsFor<f32>(3),
                                         ParamsFor<i32>(3),
                                         ParamsFor<u32>(3),
                                         ParamsFor<bool>(4),
                                         ParamsFor<f32>(4),
                                         ParamsFor<i32>(4),
                                         ParamsFor<u32>(4),
                                         ParamsFor<alias<bool>>(4),
                                         ParamsFor<alias<f32>>(4),
                                         ParamsFor<alias<i32>>(4),
                                         ParamsFor<alias<u32>>(4)));

using InvalidVectorElementTypes = ResolverTestWithParam<Params>;
TEST_P(InvalidVectorElementTypes, InvalidElementType) {
  // var a : vecN<EL_TY>;
  auto& params = GetParam();
  Global("a", ty.vec(Source{{12, 34}}, params.elem_ty(*this), params.width),
         ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: vector element type must be 'bool', 'f32', 'i32' "
            "or 'u32'");
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         InvalidVectorElementTypes,
                         testing::Values(ParamsFor<vec2<f32>>(2),
                                         ParamsFor<vec3<i32>>(2),
                                         ParamsFor<vec4<u32>>(2),
                                         ParamsFor<mat2x2<f32>>(2),
                                         ParamsFor<mat3x3<f32>>(2),
                                         ParamsFor<mat4x4<f32>>(2),
                                         ParamsFor<array<2, f32>>(2)));
}  // namespace VectorTests

}  // namespace
}  // namespace tint::resolver
