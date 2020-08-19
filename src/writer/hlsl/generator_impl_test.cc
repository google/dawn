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

#include "src/writer/hlsl/generator_impl.h"

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/entry_point.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/void_type.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest = testing::Test;

TEST_F(HlslGeneratorImplTest, DISABLED_Generate) {
  ast::type::VoidType void_type;
  ast::Module m;
  m.AddFunction(std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                                &void_type));
  m.AddEntryPoint(std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kFragment, "my_func", ""));

  GeneratorImpl g(&m);
  ASSERT_TRUE(g.Generate()) << g.error();
  EXPECT_EQ(g.result(), R"(#import <metal_lib>

void my_func() {
}
)");
}

TEST_F(HlslGeneratorImplTest, InputStructName) {
  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_EQ(g.generate_name("func_main_in"), "func_main_in");
}

TEST_F(HlslGeneratorImplTest, InputStructName_ConflictWithExisting) {
  ast::Module m;
  GeneratorImpl g(&m);

  // Register the struct name as existing.
  auto* namer = g.namer_for_testing();
  namer->NameFor("func_main_out");

  ASSERT_EQ(g.generate_name("func_main_out"), "func_main_out_0");
}

TEST_F(HlslGeneratorImplTest, NameConflictWith_InputStructName) {
  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_EQ(g.generate_name("func_main_in"), "func_main_in");

  ast::IdentifierExpression ident("func_main_in");
  ASSERT_TRUE(g.EmitIdentifier(&ident));
  EXPECT_EQ(g.result(), "func_main_in_0");
}

struct HlslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, HlslBuiltinData data) {
  out << data.builtin;
  return out;
}
using HlslBuiltinConversionTest = testing::TestWithParam<HlslBuiltinData>;
TEST_P(HlslBuiltinConversionTest, Emit) {
  auto params = GetParam();

  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(g.builtin_to_attribute(params.builtin),
            std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest,
    HlslBuiltinConversionTest,
    testing::Values(
        HlslBuiltinData{ast::Builtin::kPosition, "SV_Position"},
        HlslBuiltinData{ast::Builtin::kVertexIdx, "SV_VertexID"},
        HlslBuiltinData{ast::Builtin::kInstanceIdx, "SV_InstanceID"},
        HlslBuiltinData{ast::Builtin::kFrontFacing, "SV_IsFrontFacing"},
        HlslBuiltinData{ast::Builtin::kFragCoord, "SV_Position"},
        HlslBuiltinData{ast::Builtin::kFragDepth, "SV_Depth"},
        HlslBuiltinData{ast::Builtin::kWorkgroupSize, ""},
        HlslBuiltinData{ast::Builtin::kLocalInvocationId, "SV_GroupThreadID"},
        HlslBuiltinData{ast::Builtin::kLocalInvocationIdx, "SV_GroupIndex"},
        HlslBuiltinData{ast::Builtin::kGlobalInvocationId,
                        "SV_DispatchThreadID"}));

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
