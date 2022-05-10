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

#include "src/tint/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/storage_texture.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverAssignmentValidationTest = ResolverTest;

TEST_F(ResolverAssignmentValidationTest, ReadOnlyBuffer) {
    // struct S { m : i32 };
    // @group(0) @binding(0)
    // var<storage,read> a : S;
    auto* s = Structure("S", {Member("m", ty.i32())});
    Global(Source{{12, 34}}, "a", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    WrapInFunction(Assign(Source{{56, 78}}, MemberAccessor("a", "m"), 1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: cannot store into a read-only type 'ref<storage, "
              "i32, read>'");
}

TEST_F(ResolverAssignmentValidationTest, AssignIncompatibleTypes) {
    // {
    //  var a : i32 = 2i;
    //  a = 2.3;
    // }

    auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2_i));

    auto* assign = Assign(Source{{12, 34}}, "a", 2.3_f);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignArraysWithDifferentSizeExpressions_Pass) {
    // let len = 4u;
    // {
    //   var a : array<f32, 4u>;
    //   var b : array<f32, len>;
    //   a = b;
    // }

    GlobalConst("len", nullptr, Expr(4_u));

    auto* a = Var("a", ty.array(ty.f32(), 4_u));
    auto* b = Var("b", ty.array(ty.f32(), "len"));

    auto* assign = Assign(Source{{12, 34}}, "a", "b");
    WrapInFunction(a, b, assign);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignArraysWithDifferentSizeExpressions_Fail) {
    // let len = 5u;
    // {
    //   var a : array<f32, 4u>;
    //   var b : array<f32, len>;
    //   a = b;
    // }

    GlobalConst("len", nullptr, Expr(5_u));

    auto* a = Var("a", ty.array(ty.f32(), 4_u));
    auto* b = Var("b", ty.array(ty.f32(), "len"));

    auto* assign = Assign(Source{{12, 34}}, "a", "b");
    WrapInFunction(a, b, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'array<f32, 5>' to 'array<f32, 4>'");
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypesInBlockStatement_Pass) {
    // {
    //  var a : i32 = 2i;
    //  a = 2i
    // }
    auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2_i));
    WrapInFunction(var, Assign("a", 2_i));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignIncompatibleTypesInBlockStatement_Fail) {
    // {
    //  var a : i32 = 2i;
    //  a = 2.3;
    // }

    auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2_i));
    WrapInFunction(var, Assign(Source{{12, 34}}, "a", 2.3_f));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignIncompatibleTypesInNestedBlockStatement_Fail) {
    // {
    //  {
    //   var a : i32 = 2i;
    //   a = 2.3;
    //  }
    // }

    auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2_i));
    auto* inner_block = Block(Decl(var), Assign(Source{{12, 34}}, "a", 2.3_f));
    auto* outer_block = Block(inner_block);
    WrapInFunction(outer_block);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignToScalar_Fail) {
    // var my_var : i32 = 2i;
    // 1 = my_var;

    auto* var = Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(2_i));
    WrapInFunction(var, Assign(Expr(Source{{12, 34}}, 1_i), "my_var"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot assign to value of type 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypes_Pass) {
    // var a : i32 = 2i;
    // a = 2i
    auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2_i));
    WrapInFunction(var, Assign(Source{{12, 34}}, "a", 2_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypesThroughAlias_Pass) {
    // alias myint = i32;
    // var a : myint = 2i;
    // a = 2
    auto* myint = Alias("myint", ty.i32());
    auto* var = Var("a", ty.Of(myint), ast::StorageClass::kNone, Expr(2_i));
    WrapInFunction(var, Assign(Source{{12, 34}}, "a", 2_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypesInferRHSLoad_Pass) {
    // var a : i32 = 2i;
    // var b : i32 = 3i;
    // a = b;
    auto* var_a = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2_i));
    auto* var_b = Var("b", ty.i32(), ast::StorageClass::kNone, Expr(3_i));
    WrapInFunction(var_a, var_b, Assign(Source{{12, 34}}, "a", "b"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignThroughPointer_Pass) {
    // var a : i32;
    // let b : ptr<function,i32> = &a;
    // *b = 2i;
    const auto func = ast::StorageClass::kFunction;
    auto* var_a = Var("a", ty.i32(), func, Expr(2_i));
    auto* var_b = Let("b", ty.pointer<i32>(func), AddressOf(Expr("a")));
    WrapInFunction(var_a, var_b, Assign(Source{{12, 34}}, Deref("b"), 2_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignToConstant_Fail) {
    // {
    //  let a : i32 = 2i;
    //  a = 2i
    // }
    auto* var = Let("a", ty.i32(), Expr(2_i));
    WrapInFunction(var, Assign(Expr(Source{{12, 34}}, "a"), 2_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot assign to const\nnote: 'a' is declared here:");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_Handle) {
    // var a : texture_storage_1d<rgba8unorm, write>;
    // var b : texture_storage_1d<rgba8unorm, write>;
    // a = b;

    auto make_type = [&] {
        return ty.storage_texture(ast::TextureDimension::k1d, ast::TexelFormat::kRgba8Unorm,
                                  ast::Access::kWrite);
    };

    Global("a", make_type(), ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });
    Global("b", make_type(), ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(0),
           });

    WrapInFunction(Assign(Source{{56, 78}}, "a", "b"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: storage type of assignment must be constructible");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_Atomic) {
    // struct S { a : atomic<i32>; };
    // @group(0) @binding(0) var<storage, read_write> v : S;
    // v.a = v.a;

    auto* s = Structure("S", {Member("a", ty.atomic(ty.i32()))});
    Global(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    WrapInFunction(Assign(Source{{56, 78}}, MemberAccessor("v", "a"), MemberAccessor("v", "a")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: storage type of assignment must be constructible");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_RuntimeArray) {
    // struct S { a : array<f32>; };
    // @group(0) @binding(0) var<storage, read_write> v : S;
    // v.a = v.a;

    auto* s = Structure("S", {Member("a", ty.array(ty.f32()))});
    Global(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    WrapInFunction(Assign(Source{{56, 78}}, MemberAccessor("v", "a"), MemberAccessor("v", "a")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: storage type of assignment must be constructible");
}

TEST_F(ResolverAssignmentValidationTest, AssignToPhony_NonConstructibleStruct_Fail) {
    // struct S {
    //   arr: array<i32>;
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // fn f() {
    //   _ = s;
    // }
    auto* s = Structure("S", {Member("arr", ty.array<i32>())});
    Global("s", ty.Of(s), ast::StorageClass::kStorage, GroupAndBinding(0, 0));

    WrapInFunction(Assign(Phony(), Expr(Source{{12, 34}}, "s")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot assign 'S' to '_'. "
              "'_' can only be assigned a constructible, pointer, texture or "
              "sampler type");
}

TEST_F(ResolverAssignmentValidationTest, AssignToPhony_DynamicArray_Fail) {
    // struct S {
    //   arr: array<i32>;
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // fn f() {
    //   _ = s.arr;
    // }
    auto* s = Structure("S", {Member("arr", ty.array<i32>())});
    Global("s", ty.Of(s), ast::StorageClass::kStorage, GroupAndBinding(0, 0));

    WrapInFunction(Assign(Phony(), MemberAccessor(Source{{12, 34}}, "s", "arr")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot assign 'array<i32>' to '_'. "
              "'_' can only be assigned a constructible, pointer, texture or sampler "
              "type");
}

TEST_F(ResolverAssignmentValidationTest, AssignToPhony_Pass) {
    // struct S {
    //   i:   i32;
    //   arr: array<i32>;
    // };
    // struct U {
    //   i:   i32;
    // };
    // @group(0) @binding(0) var tex texture_2d;
    // @group(0) @binding(1) var smp sampler;
    // @group(0) @binding(2) var<uniform> u : U;
    // @group(0) @binding(3) var<storage, read_write> s : S;
    // var<workgroup> wg : array<f32, 10>
    // fn f() {
    //   _ = 1i;
    //   _ = 2u;
    //   _ = 3.0;
    //   _ = vec2<bool>();
    //   _ = tex;
    //   _ = smp;
    //   _ = &s;
    //   _ = s.i;
    //   _ = &s.arr;
    //   _ = u;
    //   _ = u.i;
    //   _ = wg;
    //   _ = wg[3i];
    // }
    auto* S = Structure("S", {
                                 Member("i", ty.i32()),
                                 Member("arr", ty.array<i32>()),
                             });
    auto* U = Structure("U", {Member("i", ty.i32())});
    Global("tex", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()), GroupAndBinding(0, 0));
    Global("smp", ty.sampler(ast::SamplerKind::kSampler), GroupAndBinding(0, 1));
    Global("u", ty.Of(U), ast::StorageClass::kUniform, GroupAndBinding(0, 2));
    Global("s", ty.Of(S), ast::StorageClass::kStorage, GroupAndBinding(0, 3));
    Global("wg", ty.array<f32, 10>(), ast::StorageClass::kWorkgroup);

    WrapInFunction(Assign(Phony(), 1_i),                                    //
                   Assign(Phony(), 2_u),                                    //
                   Assign(Phony(), 3_f),                                    //
                   Assign(Phony(), vec2<bool>()),                           //
                   Assign(Phony(), "tex"),                                  //
                   Assign(Phony(), "smp"),                                  //
                   Assign(Phony(), AddressOf("s")),                         //
                   Assign(Phony(), MemberAccessor("s", "i")),               //
                   Assign(Phony(), AddressOf(MemberAccessor("s", "arr"))),  //
                   Assign(Phony(), "u"),                                    //
                   Assign(Phony(), MemberAccessor("u", "i")),               //
                   Assign(Phony(), "wg"),                                   //
                   Assign(Phony(), IndexAccessor("wg", 3_i)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
