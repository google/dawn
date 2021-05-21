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

#include "src/ast/override_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/storage_texture_type.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

class ResolverTypeValidationTest : public resolver::TestHelper,
                                   public testing::Test {};

TEST_F(ResolverTypeValidationTest, VariableDeclNoConstructor_Pass) {
  // {
  // var a :i32;
  // a = 2;
  // }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, nullptr);
  auto* lhs = Expr("a");
  auto* rhs = Expr(2);

  auto* body =
      Block(Decl(var), Assign(Source{Source::Location{12, 34}}, lhs, rhs));

  WrapInFunction(body);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
}

TEST_F(ResolverTypeValidationTest, GlobalConstantNoConstructor_Pass) {
  // [[override(0)]] let a :i32;
  GlobalConst(Source{{12, 34}}, "a", ty.i32(), nullptr,
              ast::DecorationList{create<ast::OverrideDecoration>(0)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableWithStorageClass_Pass) {
  // var<in> global_var: f32;
  Global(Source{{12, 34}}, "global_var", ty.f32(), ast::StorageClass::kInput);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalConstantWithStorageClass_Fail) {
  // const<in> global_var: f32;
  AST().AddGlobalVariable(
      create<ast::Variable>(Source{{12, 34}}, Symbols().Register("global_var"),
                            ast::StorageClass::kInput, ty.f32(), true,
                            Expr(1.23f), ast::DecorationList{}));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error v-global01: global constants shouldn't have a storage "
            "class");
}

TEST_F(ResolverTypeValidationTest, GlobalConstNoStorageClass_Pass) {
  // let global_var: f32;
  GlobalConst(Source{{12, 34}}, "global_var", ty.f32(), Construct(ty.f32()));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableUnique_Pass) {
  // var global_var0 : f32 = 0.1;
  // var global_var1 : i32 = 0;

  Global("global_var0", ty.f32(), ast::StorageClass::kPrivate, Expr(0.1f));

  Global(Source{{12, 34}}, "global_var1", ty.f32(), ast::StorageClass::kPrivate,
         Expr(1.0f));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest,
       GlobalVariableFunctionVariableNotUnique_Pass) {
  // fn my_func() {
  //   var a: f32 = 2.0;
  // }
  // var a: f32 = 2.1;

  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  Func("my_func", ast::VariableList{}, ty.void_(), {Decl(var)});

  Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScope_Pass) {
  // {
  // if (true) { var a : f32 = 2.0; }
  // var a : f32 = 3.14;
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = Block(Decl(var));

  auto* var_a_float = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

  auto* outer_body =
      Block(create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
            Decl(Source{{12, 34}}, var_a_float));

  WrapInFunction(outer_body);

  EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScopeBlock_Pass) {
  // {
  //  { var a : f32; }
  //  var a : f32;
  // }
  auto* var_inner = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* inner = Block(Decl(Source{{12, 34}}, var_inner));

  auto* var_outer = Var("a", ty.f32(), ast::StorageClass::kNone);
  auto* outer_body = Block(inner, Decl(var_outer));

  WrapInFunction(outer_body);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest,
       RedeclaredIdentifierDifferentFunctions_Pass) {
  // func0 { var a : f32 = 2.0; return; }
  // func1 { var a : f32 = 3.0; return; }
  auto* var0 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* var1 = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(1.0f));

  Func("func0", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Source{{12, 34}}, var0),
           Return(),
       },
       ast::DecorationList{});

  Func("func1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Source{{13, 34}}, var1),
           Return(),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInFunction_Fail) {
  /// [[stage(vertex)]]
  // fn func() { var a : array<i32>; }

  auto* var =
      Var(Source{{12, 34}}, "a", ty.array<i32>(), ast::StorageClass::kNone);

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(var),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLast_Pass) {
  // [[Block]]
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  Structure("Foo",
            {
                Member("vf", ty.f32()),
                Member("rt", ty.array<f32>()),
            },
            {create<ast::StructBlockDecoration>()});

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLastNoBlock_Fail) {
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  Structure("Foo", {
                       Member("vf", ty.f32()),
                       Member(Source{{12, 34}}, "rt", ty.array<f32>()),
                   });

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error v-0015: a struct containing a runtime-sized array "
            "requires the [[block]] attribute: 'Foo'");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // struct Foo {
  //   rt: array<f32>;
  //   vf: f32;
  // };

  Structure("Foo",
            {
                Member(Source{{12, 34}}, "rt", ty.array<f32>()),
                Member("vf", ty.f32()),
            },
            {create<ast::StructBlockDecoration>()});

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error v-0015: runtime arrays may only appear as the last member of a struct)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsGlobalVariable) {
  Global(Source{{56, 78}}, "g", ty.array<i32>(), ast::StorageClass::kPrivate);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error v-0015: runtime arrays may only appear as the last member of a struct)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsLocalVariable) {
  auto* v = Var(Source{{56, 78}}, "g", ty.array<i32>());
  WrapInFunction(v);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error v-0015: runtime arrays may only appear as the last member of a struct)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsParameter_Fail) {
  // fn func(a : array<u32>) {}
  // [[stage(vertex)]] fn main() {}

  auto* param = Param(Source{{12, 34}}, "a", ty.array<i32>());

  Func("func", ast::VariableList{param}, ty.void_(),
       ast::StatementList{
           Return(),
       },
       ast::DecorationList{});

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Return(),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  b: RTArr;
  //  a: u32;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());
  AST().AddConstructedType(alias);

  Structure("s",
            {
                Member(Source{{12, 34}}, "b", alias),
                Member("a", ty.u32()),
            },
            {create<ast::StructBlockDecoration>()});

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsLast_Pass) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  a: u32;
  //  b: RTArr;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());
  AST().AddConstructedType(alias);

  Structure("s",
            {
                Member("a", ty.u32()),
                Member("b", alias),
            },
            {create<ast::StructBlockDecoration>()});

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

namespace GetCanonicalTests {
struct Params {
  create_ast_type_func_ptr create_ast_type;
  create_sem_type_func_ptr create_sem_type;
};

static constexpr Params cases[] = {
    Params{ast_bool, sem_bool},
    Params{ast_alias<ast_bool>, sem_bool},
    Params{ast_alias<ast_alias<ast_bool>>, sem_bool},

    Params{ast_vec3<ast_f32>, sem_vec3<sem_f32>},
    Params{ast_alias<ast_vec3<ast_f32>>, sem_vec3<sem_f32>},
    Params{ast_alias<ast_alias<ast_vec3<ast_f32>>>, sem_vec3<sem_f32>},

    Params{ast_vec3<ast_alias<ast_f32>>, sem_vec3<sem_f32>},
    Params{ast_alias<ast_vec3<ast_alias<ast_f32>>>, sem_vec3<sem_f32>},
    Params{ast_alias<ast_alias<ast_vec3<ast_alias<ast_f32>>>>,
           sem_vec3<sem_f32>},
    Params{ast_alias<ast_alias<ast_vec3<ast_alias<ast_alias<ast_f32>>>>>,
           sem_vec3<sem_f32>},

    Params{ast_mat3x3<ast_alias<ast_f32>>, sem_mat3x3<sem_f32>},
    Params{ast_alias<ast_mat3x3<ast_alias<ast_f32>>>, sem_mat3x3<sem_f32>},
    Params{ast_alias<ast_alias<ast_mat3x3<ast_alias<ast_f32>>>>,
           sem_mat3x3<sem_f32>},
    Params{ast_alias<ast_alias<ast_mat3x3<ast_alias<ast_alias<ast_f32>>>>>,
           sem_mat3x3<sem_f32>},

    Params{ast_alias<ast_access<ast_alias<ast_bool>>>, sem_bool},
    Params{ast_alias<ast_access<ast_alias<ast_vec3<ast_access<ast_f32>>>>>,
           sem_vec3<sem_f32>},
    Params{ast_alias<ast_access<ast_alias<ast_mat3x3<ast_access<ast_f32>>>>>,
           sem_mat3x3<sem_f32>},
};

using CanonicalTest = ResolverTestWithParam<Params>;
TEST_P(CanonicalTest, All) {
  auto& params = GetParam();

  auto* type = params.create_ast_type(ty);

  auto* var = Var("v", type);
  auto* expr = Expr("v");
  WrapInFunction(var, expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* got = TypeOf(expr)->UnwrapRef();
  auto* expected = params.create_sem_type(ty);

  EXPECT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                           << "expected: " << FriendlyName(expected) << "\n";
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         CanonicalTest,
                         testing::ValuesIn(cases));

}  // namespace GetCanonicalTests

namespace MultisampledTextureTests {
struct DimensionParams {
  ast::TextureDimension dim;
  bool is_valid;
};

static constexpr DimensionParams dimension_cases[] = {
    DimensionParams{ast::TextureDimension::k1d, false},
    DimensionParams{ast::TextureDimension::k2d, true},
    DimensionParams{ast::TextureDimension::k2dArray, false},
    DimensionParams{ast::TextureDimension::k3d, false},
    DimensionParams{ast::TextureDimension::kCube, false},
    DimensionParams{ast::TextureDimension::kCubeArray, false}};

using MultisampledTextureDimensionTest = ResolverTestWithParam<DimensionParams>;
TEST_P(MultisampledTextureDimensionTest, All) {
  auto& params = GetParam();
  Global(Source{{12, 34}}, "a", ty.multisampled_texture(params.dim, ty.i32()),
         ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: only 2d multisampled textures are supported");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         MultisampledTextureDimensionTest,
                         testing::ValuesIn(dimension_cases));

struct TypeParams {
  create_ast_type_func_ptr type_func;
  bool is_valid;
};

static constexpr TypeParams type_cases[] = {
    TypeParams{ast_bool, false},
    TypeParams{ast_i32, true},
    TypeParams{ast_u32, true},
    TypeParams{ast_f32, true},

    TypeParams{ast_alias<ast_bool>, false},
    TypeParams{ast_alias<ast_i32>, true},
    TypeParams{ast_alias<ast_u32>, true},
    TypeParams{ast_alias<ast_f32>, true},

    TypeParams{ast_vec3<ast_f32>, false},
    TypeParams{ast_mat3x3<ast_f32>, false},

    TypeParams{ast_alias<ast_vec3<ast_f32>>, false},
    TypeParams{ast_alias<ast_mat3x3<ast_f32>>, false}};

using MultisampledTextureTypeTest = ResolverTestWithParam<TypeParams>;
TEST_P(MultisampledTextureTypeTest, All) {
  auto& params = GetParam();
  Global(
      Source{{12, 34}}, "a",
      ty.multisampled_texture(ast::TextureDimension::k2d, params.type_func(ty)),
      ast::StorageClass::kNone, nullptr,
      ast::DecorationList{
          create<ast::BindingDecoration>(0),
          create<ast::GroupDecoration>(0),
      });

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: texture_multisampled_2d<type>: type must be f32, "
              "i32 or u32");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         MultisampledTextureTypeTest,
                         testing::ValuesIn(type_cases));

}  // namespace MultisampledTextureTests

namespace StorageTextureTests {
struct DimensionParams {
  ast::TextureDimension dim;
  bool is_valid;
};

static constexpr DimensionParams Dimension_cases[] = {
    DimensionParams{ast::TextureDimension::k1d, true},
    DimensionParams{ast::TextureDimension::k2d, true},
    DimensionParams{ast::TextureDimension::k2dArray, true},
    DimensionParams{ast::TextureDimension::k3d, true},
    DimensionParams{ast::TextureDimension::kCube, false},
    DimensionParams{ast::TextureDimension::kCubeArray, false}};

using StorageTextureDimensionTest = ResolverTestWithParam<DimensionParams>;
TEST_P(StorageTextureDimensionTest, All) {
  // [[group(0), binding(0)]]
  // var a : [[access(read)]] texture_storage_*<ru32int>;
  auto& params = GetParam();

  auto* st = ty.storage_texture(params.dim, ast::ImageFormat::kR32Uint);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, st);

  Global(Source{{12, 34}}, "a", ac, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: cube dimensions for storage textures are not supported");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         StorageTextureDimensionTest,
                         testing::ValuesIn(Dimension_cases));

struct FormatParams {
  ast::ImageFormat format;
  bool is_valid;
};

static constexpr FormatParams format_cases[] = {
    FormatParams{ast::ImageFormat::kBgra8Unorm, false},
    FormatParams{ast::ImageFormat::kBgra8UnormSrgb, false},
    FormatParams{ast::ImageFormat::kR16Float, false},
    FormatParams{ast::ImageFormat::kR16Sint, false},
    FormatParams{ast::ImageFormat::kR16Uint, false},
    FormatParams{ast::ImageFormat::kR32Float, true},
    FormatParams{ast::ImageFormat::kR32Sint, true},
    FormatParams{ast::ImageFormat::kR32Uint, true},
    FormatParams{ast::ImageFormat::kR8Sint, false},
    FormatParams{ast::ImageFormat::kR8Snorm, false},
    FormatParams{ast::ImageFormat::kR8Uint, false},
    FormatParams{ast::ImageFormat::kR8Unorm, false},
    FormatParams{ast::ImageFormat::kRg11B10Float, false},
    FormatParams{ast::ImageFormat::kRg16Float, false},
    FormatParams{ast::ImageFormat::kRg16Sint, false},
    FormatParams{ast::ImageFormat::kRg16Uint, false},
    FormatParams{ast::ImageFormat::kRg32Float, true},
    FormatParams{ast::ImageFormat::kRg32Sint, true},
    FormatParams{ast::ImageFormat::kRg32Uint, true},
    FormatParams{ast::ImageFormat::kRg8Sint, false},
    FormatParams{ast::ImageFormat::kRg8Snorm, false},
    FormatParams{ast::ImageFormat::kRg8Uint, false},
    FormatParams{ast::ImageFormat::kRg8Unorm, false},
    FormatParams{ast::ImageFormat::kRgb10A2Unorm, false},
    FormatParams{ast::ImageFormat::kRgba16Float, true},
    FormatParams{ast::ImageFormat::kRgba16Sint, true},
    FormatParams{ast::ImageFormat::kRgba16Uint, true},
    FormatParams{ast::ImageFormat::kRgba32Float, true},
    FormatParams{ast::ImageFormat::kRgba32Sint, true},
    FormatParams{ast::ImageFormat::kRgba32Uint, true},
    FormatParams{ast::ImageFormat::kRgba8Sint, true},
    FormatParams{ast::ImageFormat::kRgba8Snorm, true},
    FormatParams{ast::ImageFormat::kRgba8Uint, true},
    FormatParams{ast::ImageFormat::kRgba8Unorm, true},
    FormatParams{ast::ImageFormat::kRgba8UnormSrgb, false}};

using StorageTextureFormatTest = ResolverTestWithParam<FormatParams>;
TEST_P(StorageTextureFormatTest, All) {
  auto& params = GetParam();
  // [[group(0), binding(0)]]
  // var a : [[access(read)]] texture_storage_1d<*>;
  // [[group(0), binding(1)]]
  // var b : [[access(read)]] texture_storage_2d<*>;
  // [[group(0), binding(2)]]
  // var c : [[access(read)]] texture_storage_2d_array<*>;
  // [[group(0), binding(3)]]
  // var d : [[access(read)]] texture_storage_3d<*>;

  auto* st_a = ty.storage_texture(ast::TextureDimension::k1d, params.format);
  auto* ac_a = ty.access(ast::AccessControl::kReadOnly, st_a);
  Global(Source{{12, 34}}, "a", ac_a, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  auto* st_b = ty.storage_texture(ast::TextureDimension::k2d, params.format);
  auto* ac_b = ty.access(ast::AccessControl::kReadOnly, st_b);
  Global("b", ac_b, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* st_c =
      ty.storage_texture(ast::TextureDimension::k2dArray, params.format);
  auto* ac_c = ty.access(ast::AccessControl::kReadOnly, st_c);
  Global("c", ac_c, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(2),
         });

  auto* st_d = ty.storage_texture(ast::TextureDimension::k3d, params.format);
  auto* ac_d = ty.access(ast::AccessControl::kReadOnly, st_d);
  Global("d", ac_d, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(3),
         });

  if (params.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: image format must be one of the texel formats "
              "specified for storage textues in "
              "https://gpuweb.github.io/gpuweb/wgsl/#texel-formats");
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         StorageTextureFormatTest,
                         testing::ValuesIn(format_cases));

using StorageTextureAccessControlTest = ResolverTest;

TEST_F(StorageTextureAccessControlTest, MissingAccessControl_Fail) {
  // [[group(0), binding(0)]]
  // var a : texture_storage_1d<ru32int>;

  auto* st = ty.storage_texture(Source{{12, 34}}, ast::TextureDimension::k1d,
                                ast::ImageFormat::kR32Uint);

  Global("a", st, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: storage textures must have access control");
}

TEST_F(StorageTextureAccessControlTest, RWAccessControl_Fail) {
  // [[group(0), binding(0)]]
  // var a : [[access(readwrite)]] texture_storage_1d<ru32int>;

  auto* st = ty.storage_texture(ast::TextureDimension::k1d,
                                ast::ImageFormat::kR32Uint);
  auto* ac = ty.access(ast::AccessControl::kReadWrite, st);

  Global(Source{{12, 34}}, "a", ac, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: storage textures only support read-only and "
            "write-only access");
}

TEST_F(StorageTextureAccessControlTest, ReadOnlyAccessControl_Pass) {
  // [[group(0), binding(0)]]
  // var a : [[access(read)]] texture_storage_1d<ru32int>;

  auto* st = ty.storage_texture(ast::TextureDimension::k1d,
                                ast::ImageFormat::kR32Uint);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, st);

  Global("a", ac, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StorageTextureAccessControlTest, WriteOnlyAccessControl_Pass) {
  // [[group(0), binding(0)]]
  // var a : [[access(write)]] texture_storage_1d<ru32int>;

  auto* st = ty.storage_texture(ast::TextureDimension::k1d,
                                ast::ImageFormat::kR32Uint);
  auto* ac = ty.access(ast::AccessControl::kWriteOnly, st);

  Global("a", ac, ast::StorageClass::kNone, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace StorageTextureTests

}  // namespace
}  // namespace resolver
}  // namespace tint
