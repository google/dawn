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

#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitAlias_F32) {
  auto* alias = ty.alias("a", ty.f32());
  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(type a = f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructedType_Struct) {
  auto* s = Structure("A", {
                               Member("a", ty.f32()),
                               Member("b", ty.i32()),
                           });

  auto* alias = ty.alias("B", s);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(s)) << gen.error();
  ASSERT_TRUE(gen.EmitConstructedType(alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct A {
  a : f32;
  b : i32;
};
type B = A;
)");
}

TEST_F(WgslGeneratorImplTest, EmitAlias_ToStruct) {
  auto* s = Structure("A", {
                               Member("a", ty.f32()),
                               Member("b", ty.i32()),
                           });

  auto* alias = ty.alias("B", s);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructedType(alias)) << gen.error();
  EXPECT_EQ(gen.result(), R"(type B = A;
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
