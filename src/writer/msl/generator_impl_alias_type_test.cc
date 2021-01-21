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

#include "gtest/gtest.h"
#include "src/ast/module.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/type/struct_type.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitConstructedType_F32) {
  auto* alias = ty.alias("a", ty.f32);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(typedef float a;
)");
}

TEST_F(MslGeneratorImplTest, EmitConstructedType_NameCollision) {
  auto* alias = ty.alias("float", ty.f32);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(typedef float float_tint_0;
)");
}

TEST_F(MslGeneratorImplTest, EmitConstructedType_Struct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.f32),
                            Member("b", ty.i32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("a", str);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct a {
  float a;
  int b;
};
)");
}

TEST_F(MslGeneratorImplTest, EmitConstructedType_AliasStructIdent) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.f32),
                            Member("b", ty.i32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("b", str);
  auto* alias = ty.alias("a", s);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(typedef b a;
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
