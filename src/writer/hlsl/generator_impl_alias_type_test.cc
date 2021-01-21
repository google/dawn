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

#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/type/struct_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Alias = TestHelper;

TEST_F(HlslGeneratorImplTest_Alias, EmitAlias_F32) {
  auto* alias = ty.alias("a", ty.f32);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(out, alias)) << gen.error();
  EXPECT_EQ(result(), R"(typedef float a;
)");
}

TEST_F(HlslGeneratorImplTest_Alias, EmitAlias_NameCollision) {
  auto* alias = ty.alias("float", ty.f32);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(out, alias)) << gen.error();
  EXPECT_EQ(result(), R"(typedef float float_tint_0;
)");
}

TEST_F(HlslGeneratorImplTest_Alias, EmitAlias_Struct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.f32),
                            Member("b", ty.i32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("A", str);
  auto* alias = ty.alias("B", s);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(out, alias)) << gen.error();
  EXPECT_EQ(result(), R"(struct B {
  float a;
  int b;
};
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
