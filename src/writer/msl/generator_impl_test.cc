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
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/program.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Generate) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
kernel void my_func() {
  return;
}

)");
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

  GeneratorImpl& gen = Build();

  EXPECT_EQ(gen.builtin_to_attribute(params.builtin),
            std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslBuiltinConversionTest,
    testing::Values(MslBuiltinData{ast::Builtin::kPosition, "position"},
                    MslBuiltinData{ast::Builtin::kVertexIndex, "vertex_id"},
                    MslBuiltinData{ast::Builtin::kInstanceIndex, "instance_id"},
                    MslBuiltinData{ast::Builtin::kFrontFacing, "front_facing"},
                    MslBuiltinData{ast::Builtin::kFragCoord, "position"},
                    MslBuiltinData{ast::Builtin::kFragDepth, "depth(any)"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationId,
                                   "thread_position_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationIndex,
                                   "thread_index_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kGlobalInvocationId,
                                   "thread_position_in_grid"},
                    MslBuiltinData{ast::Builtin::kSampleIndex, "sample_id"},
                    MslBuiltinData{ast::Builtin::kSampleMaskIn, "sample_mask"},
                    MslBuiltinData{ast::Builtin::kSampleMaskOut,
                                   "sample_mask"}));

TEST_F(MslGeneratorImplTest, calculate_alignment_size_alias) {
  auto* alias = ty.alias("a", ty.f32());

  GeneratorImpl& gen = Build();

  EXPECT_EQ(4u, gen.calculate_alignment_size(alias));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_array) {
  auto* array = ty.array<f32, 4>();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(4u * 4u, gen.calculate_alignment_size(array));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_bool) {
  auto* bool_ = ty.bool_();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(1u, gen.calculate_alignment_size(bool_));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_f32) {
  auto* f32 = ty.f32();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(4u, gen.calculate_alignment_size(f32));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_i32) {
  auto* i32 = ty.i32();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(4u, gen.calculate_alignment_size(i32));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_matrix) {
  auto* mat3x2 = ty.mat3x2<f32>();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(4u * 3u * 2u, gen.calculate_alignment_size(mat3x2));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_pointer) {
  type::Pointer ptr(ty.bool_(), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  EXPECT_EQ(0u, gen.calculate_alignment_size(&ptr));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_struct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32(), {MemberOffset(4)}),
                            Member("b", ty.f32(), {MemberOffset(32)}),
                            Member("c", ty.f32(), {MemberOffset(128)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);

  GeneratorImpl& gen = Build();

  EXPECT_EQ(132u, gen.calculate_alignment_size(s));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_struct_of_struct) {
  auto* inner_str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32(), {MemberOffset(0)}),
                            Member("b", ty.vec3<f32>(), {MemberOffset(16)}),
                            Member("c", ty.f32(), {MemberOffset(32)})},
      ast::StructDecorationList{});

  auto* inner_s = ty.struct_("Inner", inner_str);

  auto* outer_str = create<ast::Struct>(
      ast::StructMemberList{Member("d", ty.f32(), {MemberOffset(0)}),
                            Member("e", inner_s, {MemberOffset(32)}),
                            Member("f", ty.f32(), {MemberOffset(64)})},
      ast::StructDecorationList{});

  auto* outer_s = ty.struct_("Outer", outer_str);

  GeneratorImpl& gen = Build();

  EXPECT_EQ(80u, gen.calculate_alignment_size(outer_s));
}

TEST_F(MslGeneratorImplTest, calculate_alignment_size_u32) {
  auto* u32 = ty.u32();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(4u, gen.calculate_alignment_size(u32));
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

  type::Vector vec(ty.bool_(), param.elements);

  GeneratorImpl& gen = Build();

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

  type::Vector vec(ty.i32(), param.elements);

  GeneratorImpl& gen = Build();

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

  type::Vector vec(ty.u32(), param.elements);

  GeneratorImpl& gen = Build();

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

  type::Vector vec(ty.f32(), param.elements);

  GeneratorImpl& gen = Build();

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
