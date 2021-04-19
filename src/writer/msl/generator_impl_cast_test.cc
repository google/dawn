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

#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitExpression_Cast_Scalar) {
  auto* cast = Construct<f32>(1);
  WrapInFunction(cast);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(cast)) << gen.error();
  EXPECT_EQ(gen.result(), "float(1)");
}

TEST_F(MslGeneratorImplTest, EmitExpression_Cast_Vector) {
  auto* cast = vec3<f32>(vec3<i32>(1, 2, 3));
  WrapInFunction(cast);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(cast)) << gen.error();
  EXPECT_EQ(gen.result(), "float3(int3(1, 2, 3))");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
