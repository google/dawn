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
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
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

  GeneratorImpl g(&m);

  ASSERT_TRUE(g.Generate()) << g.error();
  EXPECT_EQ(g.result(), R"(#import <metal_lib>

compute void my_func() {
}
)");
}

TEST_F(MslGeneratorImplTest, InputStructName) {
  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_EQ(g.generate_name("func_main_in"), "func_main_in");
}

TEST_F(MslGeneratorImplTest, InputStructName_ConflictWithExisting) {
  ast::Module m;
  GeneratorImpl g(&m);

  // Register the struct name as existing.
  auto* namer = g.namer_for_testing();
  namer->NameFor("func_main_out");

  ASSERT_EQ(g.generate_name("func_main_out"), "func_main_out_0");
}

TEST_F(MslGeneratorImplTest, NameConflictWith_InputStructName) {
  ast::Module m;
  GeneratorImpl g(&m);
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

  ast::Module m;
  GeneratorImpl g(&m);
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
                    MslBuiltinData{ast::Builtin::kWorkgroupSize, ""},
                    MslBuiltinData{ast::Builtin::kLocalInvocationId,
                                   "thread_position_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationIdx,
                                   "thread_index_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kGlobalInvocationId,
                                   "thread_position_in_grid"}));

TEST_F(MslGeneratorImplTest, calculate_alignment_size_alias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("a", &f32);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(4u, g.calculate_alignment_size(&alias));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_array) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 4);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(4u * 4u, g.calculate_alignment_size(&ary));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_bool) {
  ast::type::BoolType bool_type;
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(1u, g.calculate_alignment_size(&bool_type));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_f32) {
  ast::type::F32Type f32;
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(4u, g.calculate_alignment_size(&f32));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_i32) {
  ast::type::I32Type i32;
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(4u, g.calculate_alignment_size(&i32));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 2);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(4u * 3u * 2u, g.calculate_alignment_size(&mat));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_pointer) {
  ast::type::BoolType bool_type;
  ast::type::PointerType ptr(&bool_type, ast::StorageClass::kPrivate);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(0u, g.calculate_alignment_size(&ptr));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberDecorationList decos;
  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));

  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(32));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(128));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &f32, std::move(decos)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));

  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(132u, g.calculate_alignment_size(&s));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_struct_of_struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType fvec(&f32, 3);

  ast::StructMemberDecorationList decos;
  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));

  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(16));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &fvec, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(32));
  members.push_back(
      std::make_unique<ast::StructMember>("c", &f32, std::move(decos)));

  auto inner_str = std::make_unique<ast::Struct>();
  inner_str->set_members(std::move(members));

  ast::type::StructType inner_s(std::move(inner_str));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("d", &f32, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(32));
  members.push_back(
      std::make_unique<ast::StructMember>("e", &inner_s, std::move(decos)));

  decos.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(64));
  members.push_back(
      std::make_unique<ast::StructMember>("f", &f32, std::move(decos)));

  auto outer_str = std::make_unique<ast::Struct>();
  outer_str->set_members(std::move(members));

  ast::type::StructType outer_s(std::move(outer_str));

  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(80u, g.calculate_alignment_size(&outer_s));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_u32) {
  ast::type::U32Type u32;
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(4u, g.calculate_alignment_size(&u32));
}

struct MslVectorSizeData {
  uint32_t elements;
  uint32_t byte_size;
};
inline std::ostream& operator<<(std::ostream& out, MslVectorSizeData data) {
  out << data.elements;
  return out;
}
using MslVectorSizeBoolTest = testing::TestWithParam<MslVectorSizeData>;
TEST_P(MslVectorSizeBoolTest, calculate) {
  auto param = GetParam();

  ast::type::BoolType bool_type;
  ast::type::VectorType vec(&bool_type, param.elements);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(param.byte_size, g.calculate_alignment_size(&vec));
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslVectorSizeBoolTest,
                         testing::Values(MslVectorSizeData{2u, 2u},
                                         MslVectorSizeData{3u, 4u},
                                         MslVectorSizeData{4u, 4u}));

using MslVectorSizeI32Test = testing::TestWithParam<MslVectorSizeData>;
TEST_P(MslVectorSizeI32Test, calculate) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, param.elements);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(param.byte_size, g.calculate_alignment_size(&vec));
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslVectorSizeI32Test,
                         testing::Values(MslVectorSizeData{2u, 8u},
                                         MslVectorSizeData{3u, 16u},
                                         MslVectorSizeData{4u, 16u}));

using MslVectorSizeU32Test = testing::TestWithParam<MslVectorSizeData>;
TEST_P(MslVectorSizeU32Test, calculate) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, param.elements);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(param.byte_size, g.calculate_alignment_size(&vec));
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslVectorSizeU32Test,
                         testing::Values(MslVectorSizeData{2u, 8u},
                                         MslVectorSizeData{3u, 16u},
                                         MslVectorSizeData{4u, 16u}));

using MslVectorSizeF32Test = testing::TestWithParam<MslVectorSizeData>;
TEST_P(MslVectorSizeF32Test, calculate) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, param.elements);
  ast::Module m;
  GeneratorImpl g(&m);
  EXPECT_EQ(param.byte_size, g.calculate_alignment_size(&vec));
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslVectorSizeF32Test,
                         testing::Values(MslVectorSizeData{2u, 8u},
                                         MslVectorSizeData{3u, 16u},
                                         MslVectorSizeData{4u, 16u}));

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
