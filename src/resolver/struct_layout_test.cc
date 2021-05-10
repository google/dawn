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
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/struct.h"

namespace tint {
namespace resolver {
namespace {

using ResolverStructLayoutTest = ResolverTest;

TEST_F(ResolverStructLayoutTest, Scalars) {
  auto* s = Structure("S", {
                               Member("a", ty.f32()),
                               Member("b", ty.u32()),
                               Member("c", ty.i32()),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 12u);
  EXPECT_EQ(sem->SizeNoPadding(), 12u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 3u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 4u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 4u);
  EXPECT_EQ(sem->Members()[1]->Align(), 4u);
  EXPECT_EQ(sem->Members()[1]->Size(), 4u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 8u);
  EXPECT_EQ(sem->Members()[2]->Align(), 4u);
  EXPECT_EQ(sem->Members()[2]->Size(), 4u);
}

TEST_F(ResolverStructLayoutTest, Alias) {
  auto* alias_a = ty.alias("a", ty.f32());
  AST().AddConstructedType(alias_a);
  auto* alias_b = ty.alias("b", ty.f32());
  AST().AddConstructedType(alias_b);

  auto* s = Structure("S", {
                               Member("a", alias_a),
                               Member("b", alias_b),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 8u);
  EXPECT_EQ(sem->SizeNoPadding(), 8u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 2u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 4u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 4u);
  EXPECT_EQ(sem->Members()[1]->Align(), 4u);
  EXPECT_EQ(sem->Members()[1]->Size(), 4u);
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayStaticSize) {
  auto* s = Structure("S", {
                               Member("a", ty.array<i32, 3>()),
                               Member("b", ty.array<f32, 5>()),
                               Member("c", ty.array<f32, 1>()),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 36u);
  EXPECT_EQ(sem->SizeNoPadding(), 36u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 3u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 12u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 12u);
  EXPECT_EQ(sem->Members()[1]->Align(), 4u);
  EXPECT_EQ(sem->Members()[1]->Size(), 20u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 32u);
  EXPECT_EQ(sem->Members()[2]->Align(), 4u);
  EXPECT_EQ(sem->Members()[2]->Size(), 4u);
}

TEST_F(ResolverStructLayoutTest, ExplicitStrideArrayStaticSize) {
  auto* s = Structure("S", {
                               Member("a", ty.array<i32, 3>(/*stride*/ 8)),
                               Member("b", ty.array<f32, 5>(/*stride*/ 16)),
                               Member("c", ty.array<f32, 1>(/*stride*/ 32)),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 136u);
  EXPECT_EQ(sem->SizeNoPadding(), 136u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 3u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 24u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 24u);
  EXPECT_EQ(sem->Members()[1]->Align(), 4u);
  EXPECT_EQ(sem->Members()[1]->Size(), 80u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 104u);
  EXPECT_EQ(sem->Members()[2]->Align(), 4u);
  EXPECT_EQ(sem->Members()[2]->Size(), 32u);
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayRuntimeSized) {
  auto* s =
      Structure("S",
                {
                    Member("c", ty.array<f32>()),
                },
                ast::DecorationList{create<ast::StructBlockDecoration>()});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 4u);
  EXPECT_EQ(sem->SizeNoPadding(), 4u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 1u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 4u);
}

TEST_F(ResolverStructLayoutTest, ExplicitStrideArrayRuntimeSized) {
  auto* s =
      Structure("S",
                {
                    Member("c", ty.array<f32>(/*stride*/ 32)),
                },
                ast::DecorationList{create<ast::StructBlockDecoration>()});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 32u);
  EXPECT_EQ(sem->SizeNoPadding(), 32u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 1u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 32u);
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayOfExplicitStrideArray) {
  auto* inner = ty.array<i32, 2>(/*stride*/ 16);  // size: 32
  auto* outer = ty.array(inner, 12);              // size: 12 * 32
  auto* s = Structure("S", {
                               Member("c", outer),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 384u);
  EXPECT_EQ(sem->SizeNoPadding(), 384u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 1u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 384u);
}

TEST_F(ResolverStructLayoutTest, ImplicitStrideArrayOfStructure) {
  auto* inner = Structure("Inner", {
                                       Member("a", ty.vec2<i32>()),
                                       Member("b", ty.vec3<i32>()),
                                       Member("c", ty.vec4<i32>()),
                                   });  // size: 48
  auto* outer = ty.array(inner, 12);    // size: 12 * 48
  auto* s = Structure("S", {
                               Member("c", outer),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 576u);
  EXPECT_EQ(sem->SizeNoPadding(), 576u);
  EXPECT_EQ(sem->Align(), 16u);
  ASSERT_EQ(sem->Members().size(), 1u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 16u);
  EXPECT_EQ(sem->Members()[0]->Size(), 576u);
}

TEST_F(ResolverStructLayoutTest, Vector) {
  auto* s = Structure("S", {
                               Member("a", ty.vec2<i32>()),
                               Member("b", ty.vec3<i32>()),
                               Member("c", ty.vec4<i32>()),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 48u);
  EXPECT_EQ(sem->SizeNoPadding(), 48u);
  EXPECT_EQ(sem->Align(), 16u);
  ASSERT_EQ(sem->Members().size(), 3u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);  // vec2
  EXPECT_EQ(sem->Members()[0]->Align(), 8u);
  EXPECT_EQ(sem->Members()[0]->Size(), 8u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 16u);  // vec3
  EXPECT_EQ(sem->Members()[1]->Align(), 16u);
  EXPECT_EQ(sem->Members()[1]->Size(), 12u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 32u);  // vec4
  EXPECT_EQ(sem->Members()[2]->Align(), 16u);
  EXPECT_EQ(sem->Members()[2]->Size(), 16u);
}

TEST_F(ResolverStructLayoutTest, Matrix) {
  auto* s = Structure("S", {
                               Member("a", ty.mat2x2<i32>()),
                               Member("b", ty.mat2x3<i32>()),
                               Member("c", ty.mat2x4<i32>()),
                               Member("d", ty.mat3x2<i32>()),
                               Member("e", ty.mat3x3<i32>()),
                               Member("f", ty.mat3x4<i32>()),
                               Member("g", ty.mat4x2<i32>()),
                               Member("h", ty.mat4x3<i32>()),
                               Member("i", ty.mat4x4<i32>()),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 368u);
  EXPECT_EQ(sem->SizeNoPadding(), 368u);
  EXPECT_EQ(sem->Align(), 16u);
  ASSERT_EQ(sem->Members().size(), 9u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);  // mat2x2
  EXPECT_EQ(sem->Members()[0]->Align(), 8u);
  EXPECT_EQ(sem->Members()[0]->Size(), 16u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 16u);  // mat2x3
  EXPECT_EQ(sem->Members()[1]->Align(), 16u);
  EXPECT_EQ(sem->Members()[1]->Size(), 32u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 48u);  // mat2x4
  EXPECT_EQ(sem->Members()[2]->Align(), 16u);
  EXPECT_EQ(sem->Members()[2]->Size(), 32u);
  EXPECT_EQ(sem->Members()[3]->Offset(), 80u);  // mat3x2
  EXPECT_EQ(sem->Members()[3]->Align(), 8u);
  EXPECT_EQ(sem->Members()[3]->Size(), 24u);
  EXPECT_EQ(sem->Members()[4]->Offset(), 112u);  // mat3x3
  EXPECT_EQ(sem->Members()[4]->Align(), 16u);
  EXPECT_EQ(sem->Members()[4]->Size(), 48u);
  EXPECT_EQ(sem->Members()[5]->Offset(), 160u);  // mat3x4
  EXPECT_EQ(sem->Members()[5]->Align(), 16u);
  EXPECT_EQ(sem->Members()[5]->Size(), 48u);
  EXPECT_EQ(sem->Members()[6]->Offset(), 208u);  // mat4x2
  EXPECT_EQ(sem->Members()[6]->Align(), 8u);
  EXPECT_EQ(sem->Members()[6]->Size(), 32u);
  EXPECT_EQ(sem->Members()[7]->Offset(), 240u);  // mat4x3
  EXPECT_EQ(sem->Members()[7]->Align(), 16u);
  EXPECT_EQ(sem->Members()[7]->Size(), 64u);
  EXPECT_EQ(sem->Members()[8]->Offset(), 304u);  // mat4x4
  EXPECT_EQ(sem->Members()[8]->Align(), 16u);
  EXPECT_EQ(sem->Members()[8]->Size(), 64u);
}

TEST_F(ResolverStructLayoutTest, NestedStruct) {
  auto* inner = Structure("Inner", {
                                       Member("a", ty.mat3x3<i32>()),
                                   });
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", inner),
                               Member("c", ty.i32()),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 80u);
  EXPECT_EQ(sem->SizeNoPadding(), 68u);
  EXPECT_EQ(sem->Align(), 16u);
  ASSERT_EQ(sem->Members().size(), 3u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 4u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 16u);
  EXPECT_EQ(sem->Members()[1]->Align(), 16u);
  EXPECT_EQ(sem->Members()[1]->Size(), 48u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 64u);
  EXPECT_EQ(sem->Members()[2]->Align(), 4u);
  EXPECT_EQ(sem->Members()[2]->Size(), 4u);
}

TEST_F(ResolverStructLayoutTest, SizeDecorations) {
  auto* inner = Structure("Inner", {
                                       Member("a", ty.f32(), {MemberSize(8)}),
                                       Member("b", ty.f32(), {MemberSize(16)}),
                                       Member("c", ty.f32(), {MemberSize(8)}),
                                   });
  auto* s = Structure("S", {
                               Member("a", ty.f32(), {MemberSize(4)}),
                               Member("b", ty.u32(), {MemberSize(8)}),
                               Member("c", inner),
                               Member("d", ty.i32(), {MemberSize(32)}),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 76u);
  EXPECT_EQ(sem->SizeNoPadding(), 76u);
  EXPECT_EQ(sem->Align(), 4u);
  ASSERT_EQ(sem->Members().size(), 4u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 4u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 4u);
  EXPECT_EQ(sem->Members()[1]->Align(), 4u);
  EXPECT_EQ(sem->Members()[1]->Size(), 8u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 12u);
  EXPECT_EQ(sem->Members()[2]->Align(), 4u);
  EXPECT_EQ(sem->Members()[2]->Size(), 32u);
  EXPECT_EQ(sem->Members()[3]->Offset(), 44u);
  EXPECT_EQ(sem->Members()[3]->Align(), 4u);
  EXPECT_EQ(sem->Members()[3]->Size(), 32u);
}

TEST_F(ResolverStructLayoutTest, AlignDecorations) {
  auto* inner = Structure("Inner", {
                                       Member("a", ty.f32(), {MemberAlign(8)}),
                                       Member("b", ty.f32(), {MemberAlign(16)}),
                                       Member("c", ty.f32(), {MemberAlign(4)}),
                                   });
  auto* s = Structure("S", {
                               Member("a", ty.f32(), {MemberAlign(4)}),
                               Member("b", ty.u32(), {MemberAlign(8)}),
                               Member("c", inner),
                               Member("d", ty.i32(), {MemberAlign(32)}),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 96u);
  EXPECT_EQ(sem->SizeNoPadding(), 68u);
  EXPECT_EQ(sem->Align(), 32u);
  ASSERT_EQ(sem->Members().size(), 4u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 4u);
  EXPECT_EQ(sem->Members()[0]->Size(), 4u);
  EXPECT_EQ(sem->Members()[1]->Offset(), 8u);
  EXPECT_EQ(sem->Members()[1]->Align(), 8u);
  EXPECT_EQ(sem->Members()[1]->Size(), 4u);
  EXPECT_EQ(sem->Members()[2]->Offset(), 16u);
  EXPECT_EQ(sem->Members()[2]->Align(), 16u);
  EXPECT_EQ(sem->Members()[2]->Size(), 32u);
  EXPECT_EQ(sem->Members()[3]->Offset(), 64u);
  EXPECT_EQ(sem->Members()[3]->Align(), 32u);
  EXPECT_EQ(sem->Members()[3]->Size(), 4u);
}

TEST_F(ResolverStructLayoutTest, StructWithLotsOfPadding) {
  auto* s = Structure("S", {
                               Member("a", ty.i32(), {MemberAlign(1024)}),
                           });

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = TypeOf(s)->As<sem::Struct>();
  ASSERT_NE(sem, nullptr);
  EXPECT_EQ(sem->Size(), 1024u);
  EXPECT_EQ(sem->SizeNoPadding(), 4u);
  EXPECT_EQ(sem->Align(), 1024u);
  ASSERT_EQ(sem->Members().size(), 1u);
  EXPECT_EQ(sem->Members()[0]->Offset(), 0u);
  EXPECT_EQ(sem->Members()[0]->Align(), 1024u);
  EXPECT_EQ(sem->Members()[0]->Size(), 4u);
}

}  // namespace
}  // namespace resolver
}  // namespace tint
