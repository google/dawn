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

#include "src/sem/struct.h"
#include "src/sem/test_helper.h"
#include "src/sem/texture_type.h"

namespace tint {
namespace sem {
namespace {

using StructTest = TestHelper;

TEST_F(StructTest, Creation) {
  auto name = Sym("S");
  auto* impl =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::AttributeList{});
  auto* ptr = impl;
  auto* s =
      create<sem::Struct>(impl, impl->name, StructMemberList{}, 4 /* align */,
                          8 /* size */, 16 /* size_no_padding */);
  EXPECT_EQ(s->Declaration(), ptr);
  EXPECT_EQ(s->Align(), 4u);
  EXPECT_EQ(s->Size(), 8u);
  EXPECT_EQ(s->SizeNoPadding(), 16u);
}

TEST_F(StructTest, TypeName) {
  auto name = Sym("my_struct");
  auto* impl =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::AttributeList{});
  auto* s =
      create<sem::Struct>(impl, impl->name, StructMemberList{}, 4 /* align */,
                          4 /* size */, 4 /* size_no_padding */);
  EXPECT_EQ(s->type_name(), "__struct_$1");
}

TEST_F(StructTest, FriendlyName) {
  auto name = Sym("my_struct");
  auto* impl =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::AttributeList{});
  auto* s =
      create<sem::Struct>(impl, impl->name, StructMemberList{}, 4 /* align */,
                          4 /* size */, 4 /* size_no_padding */);
  EXPECT_EQ(s->FriendlyName(Symbols()), "my_struct");
}

TEST_F(StructTest, Layout) {
  auto* inner_st =  //
      Structure("Inner", {
                             Member("a", ty.i32()),
                             Member("b", ty.u32()),
                             Member("c", ty.f32()),
                             Member("d", ty.vec3<f32>()),
                             Member("e", ty.mat4x2<f32>()),
                         });

  auto* outer_st =
      Structure("Outer", {
                             Member("inner", ty.type_name("Inner")),
                             Member("a", ty.i32()),
                         });

  auto p = Build();
  ASSERT_TRUE(p.IsValid()) << p.Diagnostics().str();

  auto* sem_inner_st = p.Sem().Get(inner_st);
  auto* sem_outer_st = p.Sem().Get(outer_st);

  EXPECT_EQ(sem_inner_st->Layout(p.Symbols()),
            R"(/*            align(16) size(64) */ struct Inner {
/* offset( 0) align( 4) size( 4) */   a : i32;
/* offset( 4) align( 4) size( 4) */   b : u32;
/* offset( 8) align( 4) size( 4) */   c : f32;
/* offset(12) align( 1) size( 4) */   // -- implicit field alignment padding --;
/* offset(16) align(16) size(12) */   d : vec3<f32>;
/* offset(28) align( 1) size( 4) */   // -- implicit field alignment padding --;
/* offset(32) align( 8) size(32) */   e : mat4x2<f32>;
/*                               */ };)");

  EXPECT_EQ(sem_outer_st->Layout(p.Symbols()),
            R"(/*            align(16) size(80) */ struct Outer {
/* offset( 0) align(16) size(64) */   inner : Inner;
/* offset(64) align( 4) size( 4) */   a : i32;
/* offset(68) align( 1) size(12) */   // -- implicit struct size padding --;
/*                               */ };)");
}

}  // namespace
}  // namespace sem
}  // namespace tint
