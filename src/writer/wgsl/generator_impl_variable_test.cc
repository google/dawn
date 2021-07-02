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

#include "src/ast/override_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitVariable) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_StorageClass) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Access_Read) {
  auto* s = Structure("S", {Member("a", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* v =
      Global("a", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
             ast::DecorationList{
                 create<ast::BindingDecoration>(0),
                 create<ast::GroupDecoration>(0),
             });

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(), R"([[binding(0), group(0)]] var<storage, read> a : S;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Access_Write) {
  auto* s = Structure("S", {Member("a", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* v =
      Global("a", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kWrite,
             ast::DecorationList{
                 create<ast::BindingDecoration>(0),
                 create<ast::GroupDecoration>(0),
             });

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(),
            R"([[binding(0), group(0)]] var<storage, write> a : S;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Access_ReadWrite) {
  auto* s = Structure("S", {Member("a", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* v = Global("a", ty.Of(s), ast::StorageClass::kStorage,
                   ast::Access::kReadWrite,
                   ast::DecorationList{
                       create<ast::BindingDecoration>(0),
                       create<ast::GroupDecoration>(0),
                   });

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(),
            R"([[binding(0), group(0)]] var<storage, read_write> a : S;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Decorated) {
  auto* v = Global("a", ty.sampler(ast::SamplerKind::kSampler),
                   ast::StorageClass::kNone, nullptr,
                   ast::DecorationList{
                       create<ast::GroupDecoration>(1),
                       create<ast::BindingDecoration>(2),
                   });

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(), R"([[group(1), binding(2)]] var a : sampler;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Constructor) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(1.0f));

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(), R"(var<private> a : f32 = 1.0;)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Const) {
  auto* v = Const("a", ty.f32(), Expr(1.0f));
  WrapInFunction(Decl(v));

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitVariable(out, v)) << gen.error();
  EXPECT_EQ(out.str(), R"(let a : f32 = 1.0;)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
