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
#include "src/ast/override_decoration.h"
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
  kGroup,
  kLocation,
  kOverride,
  kOffset,
  kSize,
  kStage,
  kStride,
  kStructBlock,
  kWorkgroup,

  kBindingAndGroup,
};

bool IsBindingDecoration(DecorationKind kind) {
  switch (kind) {
    case DecorationKind::kBinding:
    case DecorationKind::kGroup:
    case DecorationKind::kBindingAndGroup:
      return true;
    default:
      return false;
  }
}

struct TestParams {
  DecorationKind kind;
  bool should_pass;
};
struct TestWithParams : ResolverTestWithParam<TestParams> {};

static ast::DecorationList createDecorations(const Source& source,
                                             ProgramBuilder& builder,
                                             DecorationKind kind) {
  switch (kind) {
    case DecorationKind::kAccess:
      return {builder.create<ast::AccessDecoration>(
          source, ast::AccessControl::kReadOnly)};
    case DecorationKind::kAlign:
      return {builder.create<ast::StructMemberAlignDecoration>(source, 4u)};
    case DecorationKind::kBinding:
      return {builder.create<ast::BindingDecoration>(source, 1u)};
    case DecorationKind::kBuiltin:
      return {builder.Builtin(source, ast::Builtin::kPosition)};
    case DecorationKind::kGroup:
      return {builder.create<ast::GroupDecoration>(source, 1u)};
    case DecorationKind::kLocation:
      return {builder.Location(source, 1)};
    case DecorationKind::kOverride:
      return {builder.create<ast::OverrideDecoration>(source, 0u)};
    case DecorationKind::kOffset:
      return {builder.create<ast::StructMemberOffsetDecoration>(source, 4u)};
    case DecorationKind::kSize:
      return {builder.create<ast::StructMemberSizeDecoration>(source, 4u)};
    case DecorationKind::kStage:
      return {builder.Stage(source, ast::PipelineStage::kCompute)};
    case DecorationKind::kStride:
      return {builder.create<ast::StrideDecoration>(source, 4u)};
    case DecorationKind::kStructBlock:
      return {builder.create<ast::StructBlockDecoration>(source)};
    case DecorationKind::kWorkgroup:
      return {
          builder.create<ast::WorkgroupDecoration>(source, builder.Expr(1))};
    case DecorationKind::kBindingAndGroup:
      return {builder.create<ast::BindingDecoration>(source, 1u),
              builder.create<ast::GroupDecoration>(source, 1u)};
  }
  return {};
}

using FunctionReturnTypeDecorationTest = TestWithParams;
TEST_P(FunctionReturnTypeDecorationTest, IsValid) {
  auto& params = GetParam();

  Func("main", ast::VariableList{}, ty.f32(), ast::StatementList{Return(1.f)},
       ast::DecorationList{Stage(ast::PipelineStage::kCompute)},
       createDecorations({}, *this, params.kind));

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
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOverride, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false},
                    TestParams{DecorationKind::kBindingAndGroup, false}));

using ArrayDecorationTest = TestWithParams;
TEST_P(ArrayDecorationTest, IsValid) {
  auto& params = GetParam();

  auto* arr = ty.array(ty.f32(), 0,
                       createDecorations(Source{{12, 34}}, *this, params.kind));
  Structure("mystruct",
            {
                Member("a", arr),
            },
            {create<ast::StructBlockDecoration>()});

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
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOverride, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, true},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false},
                    TestParams{DecorationKind::kBindingAndGroup, false}));

using StructDecorationTest = TestWithParams;
TEST_P(StructDecorationTest, IsValid) {
  auto& params = GetParam();

  Structure("mystruct", {},
            createDecorations(Source{{12, 34}}, *this, params.kind));

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
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOverride, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, true},
                    TestParams{DecorationKind::kWorkgroup, false},
                    TestParams{DecorationKind::kBindingAndGroup, false}));

using StructMemberDecorationTest = TestWithParams;
TEST_P(StructMemberDecorationTest, IsValid) {
  auto& params = GetParam();

  ast::StructMemberList members{Member(
      "a", ty.i32(), createDecorations(Source{{12, 34}}, *this, params.kind))};

  Structure("mystruct", members);

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
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOverride, false},
                    TestParams{DecorationKind::kOffset, true},
                    TestParams{DecorationKind::kSize, true},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false},
                    TestParams{DecorationKind::kBindingAndGroup, false}));

using VariableDecorationTest = TestWithParams;
TEST_P(VariableDecorationTest, IsValid) {
  auto& params = GetParam();

  if (IsBindingDecoration(params.kind)) {
    Global("a", ty.sampler(ast::SamplerKind::kSampler),
           ast::StorageClass::kNone, nullptr,
           createDecorations(Source{{12, 34}}, *this, params.kind));
  } else {
    Global("a", ty.f32(), ast::StorageClass::kInput, nullptr,
           createDecorations(Source{{12, 34}}, *this, params.kind));
  }

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    if (!IsBindingDecoration(params.kind)) {
      EXPECT_EQ(r()->error(),
                "12:34 error: decoration is not valid for variables");
    }
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    VariableDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, true},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOverride, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false},
                    TestParams{DecorationKind::kBindingAndGroup, true}));

using ConstantDecorationTest = TestWithParams;
TEST_P(ConstantDecorationTest, IsValid) {
  auto& params = GetParam();

  GlobalConst("a", ty.f32(), Expr(1.23f),
              createDecorations(Source{{12, 34}}, *this, params.kind));

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for constants");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    ConstantDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOverride, true},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false},
                    TestParams{DecorationKind::kBindingAndGroup, false}));

using FunctionDecorationTest = TestWithParams;
TEST_P(FunctionDecorationTest, IsValid) {
  auto& params = GetParam();

  ast::DecorationList decos =
      createDecorations(Source{{12, 34}}, *this, params.kind);
  Func("foo", ast::VariableList{}, ty.void_(), ast::StatementList{}, decos);

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
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOverride, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    // Skip kStage as we do not apply it in this test
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    // Skip kWorkgroup as this is a different error
                    TestParams{DecorationKind::kBindingAndGroup, false}));

}  // namespace
}  // namespace DecorationTests

namespace ArrayStrideTests {
namespace {

struct Params {
  create_ast_type_func_ptr create_el_type;
  uint32_t stride;
  bool should_pass;
};

struct TestWithParams : ResolverTestWithParam<Params> {};

using ArrayStrideTest = TestWithParams;
TEST_P(ArrayStrideTest, All) {
  auto& params = GetParam();
  auto* el_ty = params.create_el_type(ty);

  std::stringstream ss;
  ss << "el_ty: " << FriendlyName(el_ty) << ", stride: " << params.stride
     << ", should_pass: " << params.should_pass;
  SCOPED_TRACE(ss.str());

  auto* arr = ty.array(Source{{12, 34}}, el_ty, 4, params.stride);

  Global("myarray", arr, ast::StorageClass::kInput);

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
        Params{ast_u32, default_u32.size, true},
        Params{ast_i32, default_i32.size, true},
        Params{ast_f32, default_f32.size, true},
        Params{ast_vec2<f32>, default_vec2.size, true},
        // vec3's default size is not a multiple of its alignment
        // Params{ast_vec3<f32>, default_vec3.size, true},
        Params{ast_vec4<f32>, default_vec4.size, true},
        Params{ast_mat2x2<f32>, default_mat2x2.size, true},
        Params{ast_mat3x3<f32>, default_mat3x3.size, true},
        Params{ast_mat4x4<f32>, default_mat4x4.size, true},

        // Fail because stride is < element size
        Params{ast_u32, default_u32.size - 1, false},
        Params{ast_i32, default_i32.size - 1, false},
        Params{ast_f32, default_f32.size - 1, false},
        Params{ast_vec2<f32>, default_vec2.size - 1, false},
        Params{ast_vec3<f32>, default_vec3.size - 1, false},
        Params{ast_vec4<f32>, default_vec4.size - 1, false},
        Params{ast_mat2x2<f32>, default_mat2x2.size - 1, false},
        Params{ast_mat3x3<f32>, default_mat3x3.size - 1, false},
        Params{ast_mat4x4<f32>, default_mat4x4.size - 1, false},

        // Succeed because stride equals multiple of element alignment
        Params{ast_u32, default_u32.align * 7, true},
        Params{ast_i32, default_i32.align * 7, true},
        Params{ast_f32, default_f32.align * 7, true},
        Params{ast_vec2<f32>, default_vec2.align * 7, true},
        Params{ast_vec3<f32>, default_vec3.align * 7, true},
        Params{ast_vec4<f32>, default_vec4.align * 7, true},
        Params{ast_mat2x2<f32>, default_mat2x2.align * 7, true},
        Params{ast_mat3x3<f32>, default_mat3x3.align * 7, true},
        Params{ast_mat4x4<f32>, default_mat4x4.align * 7, true},

        // Fail because stride is not multiple of element alignment
        Params{ast_u32, (default_u32.align - 1) * 7, false},
        Params{ast_i32, (default_i32.align - 1) * 7, false},
        Params{ast_f32, (default_f32.align - 1) * 7, false},
        Params{ast_vec2<f32>, (default_vec2.align - 1) * 7, false},
        Params{ast_vec3<f32>, (default_vec3.align - 1) * 7, false},
        Params{ast_vec4<f32>, (default_vec4.align - 1) * 7, false},
        Params{ast_mat2x2<f32>, (default_mat2x2.align - 1) * 7, false},
        Params{ast_mat3x3<f32>, (default_mat3x3.align - 1) * 7, false},
        Params{ast_mat4x4<f32>, (default_mat4x4.align - 1) * 7, false}));

TEST_F(ArrayStrideTest, MultipleDecorations) {
  auto* arr = ty.array(Source{{12, 34}}, ty.i32(), 4,
                       {
                           create<ast::StrideDecoration>(4),
                           create<ast::StrideDecoration>(4),
                       });

  Global("myarray", arr, ast::StorageClass::kInput);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: array must have at most one [[stride]] decoration");
}

}  // namespace
}  // namespace ArrayStrideTests

namespace StructBlockTests {
namespace {

using StructBlockTest = ResolverTest;
TEST_F(StructBlockTest, StructUsedAsArrayElement) {
  auto* s = Structure("S", {Member("x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.array(s, 4);
  Global("G", a, ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: A structure type with a [[block]] decoration cannot be "
            "used as an element of an array");
}

}  // namespace
}  // namespace StructBlockTests

namespace ResourceTests {
namespace {

using ResourceDecorationTest = ResolverTest;
TEST_F(ResourceDecorationTest, UniformBufferMissingBinding) {
  auto* s = Structure("S", {Member("x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  Global(Source{{12, 34}}, "G", s, ast::StorageClass::kUniform);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: resource variables require [[group]] and [[binding]] "
            "decorations");
}

TEST_F(ResourceDecorationTest, StorageBufferMissingBinding) {
  auto* s = Structure("S", {Member("x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* ac = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{12, 34}}, "G", ac, ast::StorageClass::kStorage);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: resource variables require [[group]] and [[binding]] "
            "decorations");
}

TEST_F(ResourceDecorationTest, TextureMissingBinding) {
  Global(Source{{12, 34}}, "G", ty.depth_texture(ast::TextureDimension::k2d),
         ast::StorageClass::kNone);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: resource variables require [[group]] and [[binding]] "
            "decorations");
}

TEST_F(ResourceDecorationTest, SamplerMissingBinding) {
  Global(Source{{12, 34}}, "G", ty.sampler(ast::SamplerKind::kSampler),
         ast::StorageClass::kNone);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: resource variables require [[group]] and [[binding]] "
            "decorations");
}

TEST_F(ResourceDecorationTest, BindingPairMissingBinding) {
  Global(Source{{12, 34}}, "G", ty.sampler(ast::SamplerKind::kSampler),
         ast::StorageClass::kNone, nullptr,
         {
             create<ast::GroupDecoration>(1),
         });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: resource variables require [[group]] and [[binding]] "
            "decorations");
}

TEST_F(ResourceDecorationTest, BindingPairMissingGroup) {
  Global(Source{{12, 34}}, "G", ty.sampler(ast::SamplerKind::kSampler),
         ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(1),
         });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: resource variables require [[group]] and [[binding]] "
            "decorations");
}

TEST_F(ResourceDecorationTest, BindingPointUsedTwiceByEntryPoint) {
  Global(Source{{12, 34}}, "A",
         ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });
  Global(Source{{56, 78}}, "B",
         ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("F", {}, ty.void_(),
       {
           Decl(Var("a", ty.vec4<f32>(), ast::StorageClass::kNone,
                    Call("textureLoad", "A", vec2<i32>(1, 2), 0))),
           Decl(Var("b", ty.vec4<f32>(), ast::StorageClass::kNone,
                    Call("textureLoad", "B", vec2<i32>(1, 2), 0))),
       },
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: entry point 'F' references multiple variables that use the same resource binding [[group(2), binding(1)]]
12:34 note: first resource binding usage declared here)");
}

TEST_F(ResourceDecorationTest, BindingPointUsedTwiceByDifferentEntryPoints) {
  Global(Source{{12, 34}}, "A",
         ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });
  Global(Source{{56, 78}}, "B",
         ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         ast::StorageClass::kNone, nullptr,
         {
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("F_A", {}, ty.void_(),
       {
           Decl(Var("a", ty.vec4<f32>(), ast::StorageClass::kNone,
                    Call("textureLoad", "A", vec2<i32>(1, 2), 0))),
       },
       {Stage(ast::PipelineStage::kFragment)});
  Func("F_B", {}, ty.void_(),
       {
           Decl(Var("b", ty.vec4<f32>(), ast::StorageClass::kNone,
                    Call("textureLoad", "B", vec2<i32>(1, 2), 0))),
       },
       {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResourceDecorationTest, BindingPointOnNonResource) {
  Global(Source{{12, 34}}, "G", ty.f32(), ast::StorageClass::kPrivate, nullptr,
         {
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: non-resource variables must not have [[group]] or "
            "[[binding]] decorations");
}

}  // namespace
}  // namespace ResourceTests

namespace WorkgroupDecorationTests {
namespace {

using WorkgroupDecoration = ResolverTest;

TEST_F(WorkgroupDecoration, NotAnEntryPoint) {
  Func("main", {}, ty.void_(), {},
       {create<ast::WorkgroupDecoration>(Source{{12, 34}}, Expr(1))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: the workgroup_size attribute is only valid for "
            "compute stages");
}

TEST_F(WorkgroupDecoration, NotAComputeShader) {
  Func("main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment),
        create<ast::WorkgroupDecoration>(Source{{12, 34}}, Expr(1))});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: the workgroup_size attribute is only valid for "
            "compute stages");
}

TEST_F(WorkgroupDecoration, MultipleAttributes) {
  Func(Source{{12, 34}}, "main", {}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1),
        WorkgroupSize(2)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: only one workgroup_size attribute permitted per "
            "entry point");
}

}  // namespace
}  // namespace WorkgroupDecorationTests

}  // namespace resolver
}  // namespace tint
