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
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverAssignmentValidationTest = ResolverTest;

TEST_F(ResolverAssignmentValidationTest, ReadOnlyBuffer) {
    // struct S { m : i32 };
    // @group(0) @binding(0)
    // var<storage,read> a : S;
    auto* s = Structure("S", utils::Vector{
                                 Member("m", ty.i32()),
                             });
    GlobalVar(Source{{12, 34}}, "a", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Binding(0_a), Group(0_a));

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

    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* assign = Assign(Source{{12, 34}}, "a", 2.3_f);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignArraysWithDifferentSizeExpressions_Pass) {
    // const len = 4u;
    // {
    //   var a : array<f32, 4u>;
    //   var b : array<f32, len>;
    //   a = b;
    // }

    GlobalConst("len", Expr(4_u));

    auto* a = Var("a", ty.array<f32, 4>());
    auto* b = Var("b", ty.array(ty.f32(), "len"));

    auto* assign = Assign(Source{{12, 34}}, "a", "b");
    WrapInFunction(a, b, assign);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignArraysWithDifferentSizeExpressions_Fail) {
    // const len = 5u;
    // {
    //   var a : array<f32, 4u>;
    //   var b : array<f32, len>;
    //   a = b;
    // }

    GlobalConst("len", Expr(5_u));

    auto* a = Var("a", ty.array<f32, 4>());
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
    auto* var = Var("a", ty.i32(), Expr(2_i));
    WrapInFunction(var, Assign("a", 2_i));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignIncompatibleTypesInBlockStatement_Fail) {
    // {
    //  var a : i32 = 2i;
    //  a = 2.3;
    // }

    auto* var = Var("a", ty.i32(), Expr(2_i));
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

    auto* var = Var("a", ty.i32(), Expr(2_i));
    auto* inner_block = Block(Decl(var), Assign(Source{{12, 34}}, "a", 2.3_f));
    auto* outer_block = Block(inner_block);
    WrapInFunction(outer_block);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'f32' to 'i32'");
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypes_Pass) {
    // var a : i32 = 1i;
    // a = 2i;
    // a = 3;
    WrapInFunction(Var("a", ty.i32(), Expr(1_i)),  //
                   Assign("a", 2_i),               //
                   Assign("a", 3_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypesThroughAlias_Pass) {
    // alias myint = u32;
    // var a : myint = 1u;
    // a = 2u;
    // a = 3;
    auto* myint = Alias("myint", ty.u32());
    WrapInFunction(Var("a", ty.Of(myint), Expr(1_u)),  //
                   Assign("a", 2_u),                   //
                   Assign("a", 3_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignCompatibleTypesInferRHSLoad_Pass) {
    // var a : i32 = 2i;
    // var b : i32 = 3i;
    // a = b;
    WrapInFunction(Var("a", ty.i32(), Expr(2_i)),  //
                   Var("b", ty.i32(), Expr(3_i)),  //
                   Assign("a", "b"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignThroughPointer_Pass) {
    // var a : i32;
    // let b : ptr<function,i32> = &a;
    // *b = 2i;
    const auto func = builtin::AddressSpace::kFunction;
    WrapInFunction(Var("a", ty.i32(), func, Expr(2_i)),                    //
                   Let("b", ty.pointer<i32>(func), AddressOf(Expr("a"))),  //
                   Assign(Deref("b"), 2_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignMaterializedThroughPointer_Pass) {
    // var a : i32;
    // let b : ptr<function,i32> = &a;
    // *b = 2;
    const auto func = builtin::AddressSpace::kFunction;
    auto* var_a = Var("a", ty.i32(), func, Expr(2_i));
    auto* var_b = Let("b", ty.pointer<i32>(func), AddressOf(Expr("a")));
    WrapInFunction(var_a, var_b, Assign(Deref("b"), 2_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAssignmentValidationTest, AssignToScalar_Fail) {
    // var my_var : i32 = 2i;
    // 1 = my_var;

    WrapInFunction(Var("my_var", ty.i32(), Expr(2_i)),  //
                   Assign(Expr(Source{{12, 34}}, 1_i), "my_var"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot assign to value expression of type 'i32')");
}

TEST_F(ResolverAssignmentValidationTest, AssignToOverride_Fail) {
    // override a : i32 = 2i;
    // {
    //  a = 2i
    // }
    Override(Source{{56, 78}}, "a", ty.i32(), Expr(2_i));
    WrapInFunction(Assign(Expr(Source{{12, 34}}, "a"), 2_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot assign to override 'a'
12:34 note: 'override' variables are immutable
56:78 note: override 'a' declared here)");
}

TEST_F(ResolverAssignmentValidationTest, AssignToLet_Fail) {
    // {
    //  let a : i32 = 2i;
    //  a = 2i
    // }
    WrapInFunction(Let(Source{{56, 78}}, "a", ty.i32(), Expr(2_i)),  //
                   Assign(Expr(Source{{12, 34}}, "a"), 2_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot assign to let 'a'
12:34 note: 'let' variables are immutable
56:78 note: let 'a' declared here)");
}

TEST_F(ResolverAssignmentValidationTest, AssignToConst_Fail) {
    // {
    //  const a : i32 = 2i;
    //  a = 2i
    // }
    WrapInFunction(Const(Source{{56, 78}}, "a", ty.i32(), Expr(2_i)),  //
                   Assign(Expr(Source{{12, 34}}, "a"), 2_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot assign to const 'a'
12:34 note: 'const' variables are immutable
56:78 note: const 'a' declared here)");
}

TEST_F(ResolverAssignmentValidationTest, AssignToParam_Fail) {
    Func("foo", utils::Vector{Param(Source{{56, 78}}, "arg", ty.i32())}, ty.void_(),
         utils::Vector{
             Assign(Expr(Source{{12, 34}}, "arg"), Expr(1_i)),
             Return(),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot assign to parameter 'arg'
12:34 note: parameters are immutable
56:78 note: parameter 'arg' declared here)");
}

TEST_F(ResolverAssignmentValidationTest, AssignToLetMember_Fail) {
    // struct S { i : i32 }
    // {
    //  let a : S;
    //  a.i = 2i
    // }
    Structure("S", utils::Vector{Member("i", ty.i32())});
    WrapInFunction(Let(Source{{98, 76}}, "a", ty("S"), Call("S")),  //
                   Assign(MemberAccessor(Source{{12, 34}}, Expr(Source{{56, 78}}, "a"), "i"), 2_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot assign to value expression of type 'i32'
56:78 note: 'let' variables are immutable
98:76 note: let 'a' declared here)");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_Handle) {
    // var a : texture_storage_1d<rgba8unorm, write>;
    // var b : texture_storage_1d<rgba8unorm, write>;
    // a = b;

    auto make_type = [&] {
        return ty.storage_texture(type::TextureDimension::k1d, builtin::TexelFormat::kRgba8Unorm,
                                  builtin::Access::kWrite);
    };

    GlobalVar("a", make_type(), Binding(0_a), Group(0_a));
    GlobalVar("b", make_type(), Binding(1_a), Group(0_a));

    WrapInFunction(Assign(Source{{56, 78}}, "a", "b"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: storage type of assignment must be constructible");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_Atomic) {
    // struct S { a : atomic<i32>; };
    // @group(0) @binding(0) var<storage, read_write> v : S;
    // v.a = v.a;

    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.atomic(ty.i32())),
                             });
    GlobalVar(Source{{12, 34}}, "v", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kReadWrite, Binding(0_a), Group(0_a));

    WrapInFunction(Assign(Source{{56, 78}}, MemberAccessor("v", "a"), MemberAccessor("v", "a")));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: storage type of assignment must be constructible");
}

TEST_F(ResolverAssignmentValidationTest, AssignNonConstructible_RuntimeArray) {
    // struct S { a : array<f32>; };
    // @group(0) @binding(0) var<storage, read_write> v : S;
    // v.a = v.a;

    auto* s = Structure("S", utils::Vector{
                                 Member("a", ty.array(ty.f32())),
                             });
    GlobalVar(Source{{12, 34}}, "v", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kReadWrite, Binding(0_a), Group(0_a));

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
    auto* s = Structure("S", utils::Vector{
                                 Member("arr", ty.array<i32>()),
                             });
    GlobalVar("s", ty.Of(s), builtin::AddressSpace::kStorage, Group(0_a), Binding(0_a));

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
    auto* s = Structure("S", utils::Vector{
                                 Member("arr", ty.array<i32>()),
                             });
    GlobalVar("s", ty.Of(s), builtin::AddressSpace::kStorage, Group(0_a), Binding(0_a));

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
    //   _ = 3.0f;
    //   _ = 4;
    //   _ = 5.0;
    //   _ = vec2(6);
    //   _ = vec3(7.0);
    //   _ = vec4<bool>();
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
    auto* S = Structure("S", utils::Vector{
                                 Member("i", ty.i32()),
                                 Member("arr", ty.array<i32>()),
                             });
    auto* U = Structure("U", utils::Vector{
                                 Member("i", ty.i32()),
                             });
    GlobalVar("tex", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Group(0_a),
              Binding(0_a));
    GlobalVar("smp", ty.sampler(type::SamplerKind::kSampler), Group(0_a), Binding(1_a));
    GlobalVar("u", ty.Of(U), builtin::AddressSpace::kUniform, Group(0_a), Binding(2_a));
    GlobalVar("s", ty.Of(S), builtin::AddressSpace::kStorage, Group(0_a), Binding(3_a));
    GlobalVar("wg", ty.array<f32, 10>(), builtin::AddressSpace::kWorkgroup);

    WrapInFunction(Assign(Phony(), 1_i),                                    //
                   Assign(Phony(), 2_u),                                    //
                   Assign(Phony(), 3_f),                                    //
                   Assign(Phony(), 4_a),                                    //
                   Assign(Phony(), 5.0_a),                                  //
                   Assign(Phony(), vec2<Infer>(6_a)),                       //
                   Assign(Phony(), vec3<Infer>(7.0_a)),                     //
                   Assign(Phony(), vec4<bool>()),                           //
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
