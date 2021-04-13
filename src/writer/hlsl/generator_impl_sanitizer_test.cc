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

#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/access_control_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslSanitizerTest = TestHelper;

TEST_F(HlslSanitizerTest, ArrayLength) {
  auto* sb_ty = Structure("SB",
                          {
                              Member("x", ty.f32()),
                              Member("arr", ty.array(ty.vec4<f32>())),
                          },
                          {
                              create<ast::StructBlockDecoration>(),
                          });
  auto* ac_ty =
      create<type::AccessControl>(ast::AccessControl::kReadOnly, sb_ty);

  Global("sb", ac_ty, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(
               Var("len", ty.u32(), ast::StorageClass::kFunction,
                   Call("arrayLength", MemberAccessor("sb", "arr")))),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();

  auto got = result();
  auto* expect = R"(
ByteAddressBuffer sb : register(t0, space1);

void main() {
  uint tint_symbol_1 = 0u;
  sb.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = ((tint_symbol_1 - 16u) / 16u);
  uint len = tint_symbol_2;
  return;
}

)";
  EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, PromoteArrayInitializerToConstVar) {
  auto* array_init = array<i32, 4>(1, 2, 3, 4);
  auto* array_index = IndexAccessor(array_init, 3);
  auto* pos = Var("pos", ty.i32(), ast::StorageClass::kFunction, array_index);

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(pos),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();

  auto got = result();
  auto* expect = R"(void main() {
  const int tint_symbol[4] = {1, 2, 3, 4};
  int pos = tint_symbol[3];
  return;
}

)";
  EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, PromoteStructInitializerToConstVar) {
  auto* str = Structure("S", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.vec3<f32>()),
                                 Member("c", ty.i32()),
                             });
  auto* struct_init = Construct(str, 1, vec3<f32>(2.f, 3.f, 4.f), 4);
  auto* struct_access = MemberAccessor(struct_init, "b");
  auto* pos =
      Var("pos", ty.vec3<f32>(), ast::StorageClass::kFunction, struct_access);

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(pos),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();

  auto got = result();
  auto* expect = R"(struct S {
  int a;
  float3 b;
  int c;
};

void main() {
  const S tint_symbol = {1, float3(2.0f, 3.0f, 4.0f), 4};
  float3 pos = tint_symbol.b;
  return;
}

)";
  EXPECT_EQ(expect, got);
}
}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
