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
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
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
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Generate) {
  ast::type::Void void_type;

  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("my_func"), "my_func", ast::VariableList{},
      &void_type, create<ast::BlockStatement>(),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });
  mod.AddFunction(func);

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

kernel void my_func() {
}

)");
}

TEST_F(MslGeneratorImplTest, InputStructName) {
  ASSERT_EQ(gen.generate_name("func_main_in"), "func_main_in");
}

TEST_F(MslGeneratorImplTest, InputStructName_ConflictWithExisting) {
  gen.namer_for_testing()->NameFor("func_main_out");
  ASSERT_EQ(gen.generate_name("func_main_out"), "func_main_out_0");
}

TEST_F(MslGeneratorImplTest, NameConflictWith_InputStructName) {
  ASSERT_EQ(gen.generate_name("func_main_in"), "func_main_in");

  ast::IdentifierExpression ident("func_main_in");
  ASSERT_TRUE(gen.EmitIdentifier(&ident));
  EXPECT_EQ(gen.result(), "func_main_in_0");
}

struct MslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
  out << data.builtin;
  return out;
}
using MslBuiltinConversionTest = TestParamHelper<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
  auto params = GetParam();

  EXPECT_EQ(gen.builtin_to_attribute(params.builtin),
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
                    MslBuiltinData{ast::Builtin::kLocalInvocationId,
                                   "thread_position_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationIdx,
                                   "thread_index_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kGlobalInvocationId,
                                   "thread_position_in_grid"}));

TEST_F(MslGeneratorImplTest, calculate_alignment_size_alias) {
  ast::type::F32 f32;
  ast::type::Alias alias("a", &f32);
  EXPECT_EQ(4u, gen.calculate_alignment_size(&alias));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_array) {
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 4, ast::ArrayDecorationList{});
  EXPECT_EQ(4u * 4u, gen.calculate_alignment_size(&ary));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_bool) {
  ast::type::Bool bool_type;
  EXPECT_EQ(1u, gen.calculate_alignment_size(&bool_type));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_f32) {
  ast::type::F32 f32;
  EXPECT_EQ(4u, gen.calculate_alignment_size(&f32));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_i32) {
  ast::type::I32 i32;
  EXPECT_EQ(4u, gen.calculate_alignment_size(&i32));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_matrix) {
  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 3, 2);
  EXPECT_EQ(4u * 3u * 2u, gen.calculate_alignment_size(&mat));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_pointer) {
  ast::type::Bool bool_type;
  ast::type::Pointer ptr(&bool_type, ast::StorageClass::kPrivate);
  EXPECT_EQ(0u, gen.calculate_alignment_size(&ptr));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_struct) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  ast::StructMemberDecorationList decos;
  decos.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));

  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("a", &i32, decos));

  decos.push_back(create<ast::StructMemberOffsetDecoration>(32, Source{}));
  members.push_back(create<ast::StructMember>("b", &f32, decos));

  decos.push_back(create<ast::StructMemberOffsetDecoration>(128, Source{}));
  members.push_back(create<ast::StructMember>("c", &f32, decos));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("S", str);

  EXPECT_EQ(132u, gen.calculate_alignment_size(&s));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_struct_of_struct) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector fvec(&f32, 3);

  ast::StructMemberDecorationList decos;
  decos.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));

  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("a", &i32, decos));

  decos.push_back(create<ast::StructMemberOffsetDecoration>(16, Source{}));
  members.push_back(create<ast::StructMember>("b", &fvec, decos));

  decos.push_back(create<ast::StructMemberOffsetDecoration>(32, Source{}));
  members.push_back(create<ast::StructMember>("c", &f32, decos));

  auto* inner_str = create<ast::Struct>(members);

  ast::type::Struct inner_s("Inner", inner_str);

  decos.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("d", &f32, decos));

  decos.push_back(create<ast::StructMemberOffsetDecoration>(32, Source{}));
  members.push_back(create<ast::StructMember>("e", &inner_s, decos));

  decos.push_back(create<ast::StructMemberOffsetDecoration>(64, Source{}));
  members.push_back(create<ast::StructMember>("f", &f32, decos));

  auto* outer_str = create<ast::Struct>(members);

  ast::type::Struct outer_s("Outer", outer_str);

  EXPECT_EQ(80u, gen.calculate_alignment_size(&outer_s));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_u32) {
  ast::type::U32 u32;
  EXPECT_EQ(4u, gen.calculate_alignment_size(&u32));
}

struct MslVectorSizeData {
  uint32_t elements;
  uint32_t byte_size;
};
inline std::ostream& operator<<(std::ostream& out, MslVectorSizeData data) {
  out << data.elements;
  return out;
}
using MslVectorSizeBoolTest = TestParamHelper<MslVectorSizeData>;
TEST_P(MslVectorSizeBoolTest, calculate) {
  auto param = GetParam();

  ast::type::Bool bool_type;
  ast::type::Vector vec(&bool_type, param.elements);
  EXPECT_EQ(param.byte_size, gen.calculate_alignment_size(&vec));
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslVectorSizeBoolTest,
                         testing::Values(MslVectorSizeData{2u, 2u},
                                         MslVectorSizeData{3u, 4u},
                                         MslVectorSizeData{4u, 4u}));

using MslVectorSizeI32Test = TestParamHelper<MslVectorSizeData>;
TEST_P(MslVectorSizeI32Test, calculate) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec(&i32, param.elements);
  EXPECT_EQ(param.byte_size, gen.calculate_alignment_size(&vec));
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslVectorSizeI32Test,
                         testing::Values(MslVectorSizeData{2u, 8u},
                                         MslVectorSizeData{3u, 16u},
                                         MslVectorSizeData{4u, 16u}));

using MslVectorSizeU32Test = TestParamHelper<MslVectorSizeData>;
TEST_P(MslVectorSizeU32Test, calculate) {
  auto param = GetParam();

  ast::type::U32 u32;
  ast::type::Vector vec(&u32, param.elements);
  EXPECT_EQ(param.byte_size, gen.calculate_alignment_size(&vec));
}
INSTANTIATE_TEST_SUITE_P(MslGeneratorImplTest,
                         MslVectorSizeU32Test,
                         testing::Values(MslVectorSizeData{2u, 8u},
                                         MslVectorSizeData{3u, 16u},
                                         MslVectorSizeData{4u, 16u}));

using MslVectorSizeF32Test = TestParamHelper<MslVectorSizeData>;
TEST_P(MslVectorSizeF32Test, calculate) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, param.elements);
  EXPECT_EQ(param.byte_size, gen.calculate_alignment_size(&vec));
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
