// Copyright 2021 The Tint Authors.
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

#include "src/ast/access_decoration.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {

namespace DecorationTests {
namespace {

enum class DecorationKind {
  kAccess,
  kAlign,
  kBinding,
  kBuiltin,
  kConstantId,
  kGroup,
  kLocation,
  kOffset,
  kSize,
  kStage,
  kStride,
  kStructBlock,
  kWorkgroup,
};
struct TestParams {
  DecorationKind kind;
  bool should_pass;
};
struct TestWithParams : ResolverTestWithParam<TestParams> {};

static ast::Decoration* createDecoration(const Source& source,
                                         ProgramBuilder& builder,
                                         DecorationKind kind) {
  switch (kind) {
    case DecorationKind::kAccess:
      return builder.create<ast::AccessDecoration>(
          source, ast::AccessControl::kReadOnly);
    case DecorationKind::kAlign:
      return builder.create<ast::StructMemberAlignDecoration>(source, 4u);
    case DecorationKind::kBinding:
      return builder.create<ast::BindingDecoration>(source, 1);
    case DecorationKind::kBuiltin:
      return builder.create<ast::BuiltinDecoration>(source,
                                                    ast::Builtin::kPosition);
    case DecorationKind::kConstantId:
      return builder.create<ast::ConstantIdDecoration>(source, 0u);
    case DecorationKind::kGroup:
      return builder.create<ast::GroupDecoration>(source, 1u);
    case DecorationKind::kLocation:
      return builder.create<ast::LocationDecoration>(source, 1);
    case DecorationKind::kOffset:
      return builder.create<ast::StructMemberOffsetDecoration>(source, 4u);
    case DecorationKind::kSize:
      return builder.create<ast::StructMemberSizeDecoration>(source, 4u);
    case DecorationKind::kStage:
      return builder.create<ast::StageDecoration>(source,
                                                  ast::PipelineStage::kCompute);
    case DecorationKind::kStride:
      return builder.create<ast::StrideDecoration>(source, 4u);
    case DecorationKind::kStructBlock:
      return builder.create<ast::StructBlockDecoration>(source);
    case DecorationKind::kWorkgroup:
      return builder.create<ast::WorkgroupDecoration>(source, 1u, 1u, 1u);
  }
  return nullptr;
}

using FunctionReturnTypeDecorationTest = TestWithParams;
TEST_P(FunctionReturnTypeDecorationTest, IsValid) {
  auto& params = GetParam();

  Func("main", ast::VariableList{}, ty.f32(),
       ast::StatementList{create<ast::ReturnStatement>(Expr(1.f))},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex)},
       ast::DecorationList{createDecoration({}, *this, params.kind)});

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "error: decoration is not valid for function return types");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    FunctionReturnTypeDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, true},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using ArrayDecorationTest = TestWithParams;
TEST_P(ArrayDecorationTest, IsValid) {
  auto& params = GetParam();

  ast::StructMemberList members{Member(
      "a", create<type::Array>(ty.f32(), 0,
                               ast::DecorationList{createDecoration(
                                   Source{{12, 34}}, *this, params.kind)}))};
  auto* s = create<ast::Struct>(
      members, ast::DecorationList{create<ast::StructBlockDecoration>()});
  auto* s_ty = ty.struct_("mystruct", s);
  AST().AddConstructedType(s_ty);

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for array types");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    ArrayDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, false},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, true},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using StructDecorationTest = TestWithParams;
TEST_P(StructDecorationTest, IsValid) {
  auto& params = GetParam();

  auto* s = create<ast::Struct>(ast::StructMemberList{},
                                ast::DecorationList{createDecoration(
                                    Source{{12, 34}}, *this, params.kind)});
  auto* s_ty = ty.struct_("mystruct", s);
  AST().AddConstructedType(s_ty);

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for struct declarations");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    StructDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, false},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, true},
                    TestParams{DecorationKind::kWorkgroup, false}));

using StructMemberDecorationTest = TestWithParams;
TEST_P(StructMemberDecorationTest, IsValid) {
  auto& params = GetParam();

  ast::StructMemberList members{
      Member("a", ty.i32(),
             ast::DecorationList{
                 createDecoration(Source{{12, 34}}, *this, params.kind)})};
  auto* s = create<ast::Struct>(members, ast::DecorationList{});
  auto* s_ty = ty.struct_("mystruct", s);
  AST().AddConstructedType(s_ty);

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for structure members");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    StructMemberDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, true},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, true},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOffset, true},
                    TestParams{DecorationKind::kSize, true},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using VariableDecorationTest = TestWithParams;
TEST_P(VariableDecorationTest, IsValid) {
  auto& params = GetParam();

  Global("a", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             createDecoration(Source{{12, 34}}, *this, params.kind)});

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for variables");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    VariableDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, true},
                    TestParams{DecorationKind::kBuiltin, true},
                    TestParams{DecorationKind::kConstantId, true},
                    TestParams{DecorationKind::kGroup, true},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using FunctionDecorationTest = TestWithParams;
TEST_P(FunctionDecorationTest, IsValid) {
  auto& params = GetParam();

  Func("foo", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           createDecoration(Source{{12, 34}}, *this, params.kind)});

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for functions");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    FunctionDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, false},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    // Skip kStage as we always apply it in this test
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, true}));

}  // namespace
}  // namespace DecorationTests

namespace ArrayStrideTests {
namespace {

struct Params {
  create_type_func_ptr create_el_type;
  uint32_t stride;
  bool should_pass;
};

struct TestWithParams : ResolverTestWithParam<Params> {};

using ArrayStrideTest = TestWithParams;
TEST_P(ArrayStrideTest, All) {
  auto& params = GetParam();
  auto* el_ty = params.create_el_type(ty);

  std::stringstream ss;
  ss << "el_ty: " << el_ty->FriendlyName(Symbols())
     << ", stride: " << params.stride
     << ", should_pass: " << params.should_pass;
  SCOPED_TRACE(ss.str());

  auto* arr =
      create<type::Array>(el_ty, 4,
                          ast::DecorationList{
                              create<ast::StrideDecoration>(params.stride),
                          });

  Global(Source{{12, 34}}, "myarray", arr, ast::StorageClass::kInput);

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: arrays decorated with the stride attribute must "
              "have a stride that is at least the size of the element type, "
              "and be a multiple of the element type's alignment value.");
  }
}

// Helpers and typedefs
using i32 = ProgramBuilder::i32;
using u32 = ProgramBuilder::u32;
using f32 = ProgramBuilder::f32;

struct SizeAndAlignment {
  uint32_t size;
  uint32_t align;
};
constexpr SizeAndAlignment default_u32 = {4, 4};
constexpr SizeAndAlignment default_i32 = {4, 4};
constexpr SizeAndAlignment default_f32 = {4, 4};
constexpr SizeAndAlignment default_vec2 = {8, 8};
constexpr SizeAndAlignment default_vec3 = {12, 16};
constexpr SizeAndAlignment default_vec4 = {16, 16};
constexpr SizeAndAlignment default_mat2x2 = {16, 8};
constexpr SizeAndAlignment default_mat3x3 = {48, 16};
constexpr SizeAndAlignment default_mat4x4 = {64, 16};

INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    ArrayStrideTest,
    testing::Values(
        // Succeed because stride >= element size (while being multiple of
        // element alignment)
        Params{ty_u32, default_u32.size, true},
        Params{ty_i32, default_i32.size, true},
        Params{ty_f32, default_f32.size, true},
        Params{ty_vec2<f32>, default_vec2.size, true},
        // vec3's default size is not a multiple of its alignment
        // Params{ty_vec3<f32>, default_vec3.size, true},
        Params{ty_vec4<f32>, default_vec4.size, true},
        Params{ty_mat2x2<f32>, default_mat2x2.size, true},
        Params{ty_mat3x3<f32>, default_mat3x3.size, true},
        Params{ty_mat4x4<f32>, default_mat4x4.size, true},

        // Fail because stride is < element size
        Params{ty_u32, default_u32.size - 1, false},
        Params{ty_i32, default_i32.size - 1, false},
        Params{ty_f32, default_f32.size - 1, false},
        Params{ty_vec2<f32>, default_vec2.size - 1, false},
        Params{ty_vec3<f32>, default_vec3.size - 1, false},
        Params{ty_vec4<f32>, default_vec4.size - 1, false},
        Params{ty_mat2x2<f32>, default_mat2x2.size - 1, false},
        Params{ty_mat3x3<f32>, default_mat3x3.size - 1, false},
        Params{ty_mat4x4<f32>, default_mat4x4.size - 1, false},

        // Succeed because stride equals multiple of element alignment
        Params{ty_u32, default_u32.align * 7, true},
        Params{ty_i32, default_i32.align * 7, true},
        Params{ty_f32, default_f32.align * 7, true},
        Params{ty_vec2<f32>, default_vec2.align * 7, true},
        Params{ty_vec3<f32>, default_vec3.align * 7, true},
        Params{ty_vec4<f32>, default_vec4.align * 7, true},
        Params{ty_mat2x2<f32>, default_mat2x2.align * 7, true},
        Params{ty_mat3x3<f32>, default_mat3x3.align * 7, true},
        Params{ty_mat4x4<f32>, default_mat4x4.align * 7, true},

        // Fail because stride is not multiple of element alignment
        Params{ty_u32, (default_u32.align - 1) * 7, false},
        Params{ty_i32, (default_i32.align - 1) * 7, false},
        Params{ty_f32, (default_f32.align - 1) * 7, false},
        Params{ty_vec2<f32>, (default_vec2.align - 1) * 7, false},
        Params{ty_vec3<f32>, (default_vec3.align - 1) * 7, false},
        Params{ty_vec4<f32>, (default_vec4.align - 1) * 7, false},
        Params{ty_mat2x2<f32>, (default_mat2x2.align - 1) * 7, false},
        Params{ty_mat3x3<f32>, (default_mat3x3.align - 1) * 7, false},
        Params{ty_mat4x4<f32>, (default_mat4x4.align - 1) * 7, false}));

}  // namespace
}  // namespace ArrayStrideTests
}  // namespace resolver
}  // namespace tint
