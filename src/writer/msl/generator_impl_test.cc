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

#include "src/writer/msl/generator_impl.h"

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/entry_point.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/type/void_type.h"
#include "src/writer/msl/namer.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, DISABLED_Generate) {
  ast::type::VoidType void_type;
  ast::Module m;
  m.AddFunction(std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                                &void_type));
  m.AddEntryPoint(std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kCompute, "my_func", ""));

  GeneratorImpl g;

  ASSERT_TRUE(g.Generate(m)) << g.error();
  EXPECT_EQ(g.result(), R"(#import <metal_lib>

compute void my_func() {
}
)");
}

TEST_F(MslGeneratorImplTest, InputStructName) {
  GeneratorImpl g;
  ASSERT_EQ(g.generate_name("func_main_in"), "func_main_in");
}

TEST_F(MslGeneratorImplTest, InputStructName_ConflictWithExisting) {
  GeneratorImpl g;

  // Register the struct name as existing.
  auto* namer = g.namer_for_testing();
  namer->NameFor("func_main_out");

  ASSERT_EQ(g.generate_name("func_main_out"), "func_main_out_0");
}

TEST_F(MslGeneratorImplTest, NameConflictWith_InputStructName) {
  GeneratorImpl g;
  ASSERT_EQ(g.generate_name("func_main_in"), "func_main_in");

  ast::IdentifierExpression ident("func_main_in");
  ASSERT_TRUE(g.EmitIdentifier(&ident));
  EXPECT_EQ(g.result(), "func_main_in_0");
}

struct MslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
  out << data.builtin;
  return out;
}
using MslBuiltinConversionTest = testing::TestWithParam<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
  auto params = GetParam();

  GeneratorImpl g;
  EXPECT_EQ(g.builtin_to_attribute(params.builtin),
            std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslBuiltinConversionTest,
    testing::Values(MslBuiltinData{ast::Builtin::kPosition, "position"},
                    MslBuiltinData{ast::Builtin::kVertexIdx, "vertex_id"},
                    MslBuiltinData{ast::Builtin::kInstanceIdx, "instance_id"},
                    MslBuiltinData{ast::Builtin::kFrontFacing, "front_facing"},
                    MslBuiltinData{ast::Builtin::kFragCoord, "position"},
                    MslBuiltinData{ast::Builtin::kFragDepth, "depth(any)"},
                    MslBuiltinData{ast::Builtin::kNumWorkgroups, ""},
                    MslBuiltinData{ast::Builtin::kWorkgroupSize, ""},
                    MslBuiltinData{ast::Builtin::kLocalInvocationId,
                                   "thread_position_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationIdx,
                                   "thread_index_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kGlobalInvocationId,
                                   "thread_position_in_grid"}));

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
