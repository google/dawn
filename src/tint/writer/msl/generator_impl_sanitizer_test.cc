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

#include "gmock/gmock.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

using ::testing::HasSubstr;

using MslSanitizerTest = TestHelper;

TEST_F(MslSanitizerTest, Call_ArrayLength) {
  auto* s = Structure("my_struct", {Member(0, "a", ty.array<f32>(4))});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(1),
             create<ast::GroupAttribute>(2),
         });

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Var("len", ty.u32(), ast::StorageClass::kNone,
                    Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
       },
       ast::AttributeList{
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();

  auto got = gen.result();
  auto* expect = R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol {
  /* 0x0000 */ uint4 buffer_size[1];
};

struct my_struct {
  float a[1];
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(30)]]) {
  uint len = (((*(tint_symbol_2)).buffer_size[0u][1u] - 0u) / 4u);
  return;
}

)";
  EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_OtherMembersInStruct) {
  auto* s = Structure("my_struct", {
                                       Member(0, "z", ty.f32()),
                                       Member(4, "a", ty.array<f32>(4)),
                                   });
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(1),
             create<ast::GroupAttribute>(2),
         });

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Var("len", ty.u32(), ast::StorageClass::kNone,
                    Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
       },
       ast::AttributeList{
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();

  auto got = gen.result();
  auto* expect = R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol {
  /* 0x0000 */ uint4 buffer_size[1];
};

struct my_struct {
  float z;
  float a[1];
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(30)]]) {
  uint len = (((*(tint_symbol_2)).buffer_size[0u][1u] - 4u) / 4u);
  return;
}

)";

  EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ViaLets) {
  auto* s = Structure("my_struct", {Member(0, "a", ty.array<f32>(4))});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(1),
             create<ast::GroupAttribute>(2),
         });

  auto* p = Let("p", nullptr, AddressOf("b"));
  auto* p2 = Let("p2", nullptr, AddressOf(MemberAccessor(Deref(p), "a")));

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(p),
           Decl(p2),
           Decl(Var("len", ty.u32(), ast::StorageClass::kNone,
                    Call("arrayLength", p2))),
       },
       ast::AttributeList{
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();

  auto got = gen.result();
  auto* expect = R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol {
  /* 0x0000 */ uint4 buffer_size[1];
};

struct my_struct {
  float a[1];
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(30)]]) {
  uint len = (((*(tint_symbol_2)).buffer_size[0u][1u] - 0u) / 4u);
  return;
}

)";

  EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest, Call_ArrayLength_ArrayLengthFromUniform) {
  auto* s = Structure("my_struct", {Member(0, "a", ty.array<f32>(4))});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(1),
             create<ast::GroupAttribute>(0),
         });
  Global("c", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(2),
             create<ast::GroupAttribute>(0),
         });

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Var(
               "len", ty.u32(), ast::StorageClass::kNone,
               Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                   Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
       },
       ast::AttributeList{
           Stage(ast::PipelineStage::kFragment),
       });

  Options options;
  options.array_length_from_uniform.ubo_binding = {0, 29};
  options.array_length_from_uniform.bindpoint_to_size_index.emplace(
      sem::BindingPoint{0, 1}, 7u);
  options.array_length_from_uniform.bindpoint_to_size_index.emplace(
      sem::BindingPoint{0, 2}, 2u);
  GeneratorImpl& gen = SanitizeAndBuild(options);

  ASSERT_TRUE(gen.Generate()) << gen.error();

  auto got = gen.result();
  auto* expect = R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol {
  /* 0x0000 */ uint4 buffer_size[2];
};

struct my_struct {
  float a[1];
};

fragment void a_func(const constant tint_symbol* tint_symbol_2 [[buffer(29)]]) {
  uint len = ((((*(tint_symbol_2)).buffer_size[1u][3u] - 0u) / 4u) + (((*(tint_symbol_2)).buffer_size[0u][2u] - 0u) / 4u));
  return;
}

)";
  EXPECT_EQ(expect, got);
}

TEST_F(MslSanitizerTest,
       Call_ArrayLength_ArrayLengthFromUniformMissingBinding) {
  auto* s = Structure("my_struct", {Member(0, "a", ty.array<f32>(4))});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(1),
             create<ast::GroupAttribute>(0),
         });
  Global("c", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::AttributeList{
             create<ast::BindingAttribute>(2),
             create<ast::GroupAttribute>(0),
         });

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Var(
               "len", ty.u32(), ast::StorageClass::kNone,
               Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                   Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
       },
       ast::AttributeList{
           Stage(ast::PipelineStage::kFragment),
       });

  Options options;
  options.array_length_from_uniform.ubo_binding = {0, 29};
  options.array_length_from_uniform.bindpoint_to_size_index.emplace(
      sem::BindingPoint{0, 2}, 2u);
  GeneratorImpl& gen = SanitizeAndBuild(options);

  ASSERT_FALSE(gen.Generate());
  EXPECT_THAT(gen.error(),
              HasSubstr("Unable to translate builtin: arrayLength"));
}

}  // namespace
}  // namespace tint::writer::msl
