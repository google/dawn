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

#include "src/sem/call.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

struct MslImportData {
  const char* name;
  const char* msl_name;
};
inline std::ostream& operator<<(std::ostream& out, MslImportData data) {
  out << data.name;
  return out;
}
using MslImportData_SingleParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_SingleParamTest, FloatScalar) {
  auto param = GetParam();
  auto* call = Call(param.name, 1.f);

  // The resolver will set the intrinsic data for the ident
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  auto* sem = program->Sem().Get(call);
  ASSERT_NE(sem, nullptr);
  auto* target = sem->Target();
  ASSERT_NE(target, nullptr);
  auto* intrinsic = target->As<sem::Intrinsic>();
  ASSERT_NE(intrinsic, nullptr);

  ASSERT_EQ(gen.generate_builtin_name(intrinsic), param.msl_name);
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_SingleParamTest,
                         testing::Values(MslImportData{"abs", "fabs"},
                                         MslImportData{"acos", "acos"},
                                         MslImportData{"asin", "asin"},
                                         MslImportData{"atan", "atan"},
                                         MslImportData{"ceil", "ceil"},
                                         MslImportData{"cos", "cos"},
                                         MslImportData{"cosh", "cosh"},
                                         MslImportData{"exp", "exp"},
                                         MslImportData{"exp2", "exp2"},
                                         MslImportData{"floor", "floor"},
                                         MslImportData{"fract", "fract"},
                                         MslImportData{"inverseSqrt", "rsqrt"},
                                         MslImportData{"length", "length"},
                                         MslImportData{"log", "log"},
                                         MslImportData{"log2", "log2"},
                                         MslImportData{"round", "rint"},
                                         MslImportData{"sign", "sign"},
                                         MslImportData{"sin", "sin"},
                                         MslImportData{"sinh", "sinh"},
                                         MslImportData{"sqrt", "sqrt"},
                                         MslImportData{"tan", "tan"},
                                         MslImportData{"tanh", "tanh"},
                                         MslImportData{"trunc", "trunc"}));

TEST_F(MslGeneratorImplTest, MslImportData_SingleParamTest_IntScalar) {
  auto* expr = Call("abs", 1);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitCall(expr)) << gen.error();
  EXPECT_EQ(gen.result(), R"(abs(1))");
}

using MslImportData_DualParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParamTest, FloatScalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1.0f, 2.0f);

  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitCall(expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.msl_name) + "(1.0f, 2.0f)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParamTest,
                         testing::Values(MslImportData{"atan2", "atan2"},
                                         MslImportData{"distance", "distance"},
                                         MslImportData{"max", "fmax"},
                                         MslImportData{"min", "fmin"},
                                         MslImportData{"pow", "pow"},
                                         MslImportData{"reflect", "reflect"},
                                         MslImportData{"step", "step"}));

using MslImportData_DualParam_VectorTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_VectorTest, FloatVector) {
  auto param = GetParam();

  auto* expr =
      Call(param.name, vec3<f32>(1.f, 2.f, 3.f), vec3<f32>(4.f, 5.f, 6.f));
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitCall(expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.msl_name) +
                              "(float3(1.0f, 2.0f, 3.0f), "
                              "float3(4.0f, 5.0f, 6.0f))");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParam_VectorTest,
                         testing::Values(MslImportData{"cross", "cross"}));

using MslImportData_DualParam_Int_Test = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_Int_Test, IntScalar) {
  auto param = GetParam();

  auto* expr = Call(param.name, 1, 2);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitCall(expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.msl_name) + "(1, 2)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_DualParam_Int_Test,
                         testing::Values(MslImportData{"max", "max"},
                                         MslImportData{"min", "min"}));

using MslImportData_TripleParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_TripleParamTest, FloatScalar) {
  auto param = GetParam();

  auto* expr = Call(param.name, 1.f, 2.f, 3.f);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitCall(expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.msl_name) + "(1.0f, 2.0f, 3.0f)");
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslImportData_TripleParamTest,
    testing::Values(MslImportData{"faceForward", "faceforward"},
                    MslImportData{"fma", "fma"},
                    MslImportData{"mix", "mix"},
                    MslImportData{"clamp", "clamp"},
                    MslImportData{"smoothStep", "smoothstep"}));

using MslImportData_TripleParam_Int_Test = TestParamHelper<MslImportData>;
TEST_P(MslImportData_TripleParam_Int_Test, IntScalar) {
  auto param = GetParam();

  auto* expr = Call(param.name, 1, 2, 3);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitCall(expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.msl_name) + "(1, 2, 3)");
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslImportData_TripleParam_Int_Test,
                         testing::Values(MslImportData{"clamp", "clamp"},
                                         MslImportData{"clamp", "clamp"}));

TEST_F(MslGeneratorImplTest, MslImportData_Determinant) {
  Global("var", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = Call("determinant", "var");

  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitCall(expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("determinant(var)"));
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
