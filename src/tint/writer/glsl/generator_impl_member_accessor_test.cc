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

#include "gmock/gmock.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/glsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using ::testing::HasSubstr;

using create_type_func_ptr = const ast::Type* (*)(const ProgramBuilder::TypesBuilder& ty);

inline const ast::Type* ty_i32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.i32();
}
inline const ast::Type* ty_u32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.u32();
}
inline const ast::Type* ty_f32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.f32();
}
template <typename T>
inline const ast::Type* ty_vec2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec2<T>();
}
template <typename T>
inline const ast::Type* ty_vec3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec3<T>();
}
template <typename T>
inline const ast::Type* ty_vec4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec4<T>();
}
template <typename T>
inline const ast::Type* ty_mat2x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x2<T>();
}
template <typename T>
inline const ast::Type* ty_mat2x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x3<T>();
}
template <typename T>
inline const ast::Type* ty_mat2x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x4<T>();
}
template <typename T>
inline const ast::Type* ty_mat3x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x2<T>();
}
template <typename T>
inline const ast::Type* ty_mat3x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x3<T>();
}
template <typename T>
inline const ast::Type* ty_mat3x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x4<T>();
}
template <typename T>
inline const ast::Type* ty_mat4x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x2<T>();
}
template <typename T>
inline const ast::Type* ty_mat4x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x3<T>();
}
template <typename T>
inline const ast::Type* ty_mat4x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x4<T>();
}

template <typename BASE>
class GlslGeneratorImplTest_MemberAccessorBase : public BASE {
  public:
    void SetupStorageBuffer(ast::StructMemberList members) {
        ProgramBuilder& b = *this;

        auto* s = b.Structure("Data", members);

        b.Global("data", b.ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
                 ast::AttributeList{
                     b.create<ast::BindingAttribute>(0),
                     b.create<ast::GroupAttribute>(1),
                 });
    }

    void SetupFunction(ast::StatementList statements) {
        ProgramBuilder& b = *this;
        b.Func("main", ast::VariableList{}, b.ty.void_(), statements,
               ast::AttributeList{
                   b.Stage(ast::PipelineStage::kFragment),
               });
    }
};

using GlslGeneratorImplTest_MemberAccessor = GlslGeneratorImplTest_MemberAccessorBase<TestHelper>;

template <typename T>
using GlslGeneratorImplTest_MemberAccessorWithParam =
    GlslGeneratorImplTest_MemberAccessorBase<TestParamHelper<T>>;

TEST_F(GlslGeneratorImplTest_MemberAccessor, EmitExpression_MemberAccessor) {
    auto* s = Structure("Data", {Member("mem", ty.f32())});
    Global("str", ty.Of(s), ast::StorageClass::kPrivate);

    auto* expr = MemberAccessor("str", "mem");
    WrapInFunction(Var("expr", ty.f32(), ast::StorageClass::kNone, expr));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

struct Data {
  float mem;
};

Data str = Data(0.0f);
void test_function() {
  float expr = str.mem;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

struct TypeCase {
    create_type_func_ptr member_type;
    std::string expected;
};
inline std::ostream& operator<<(std::ostream& out, TypeCase c) {
    ProgramBuilder b;
    auto* ty = c.member_type(b.ty);
    out << ty->FriendlyName(b.Symbols());
    return out;
}

using GlslGeneratorImplTest_MemberAccessor_StorageBufferLoad =
    GlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;
TEST_P(GlslGeneratorImplTest_MemberAccessor_StorageBufferLoad, Test) {
    // struct Data {
    //   a : i32;
    //   b : <type>;
    // };
    // var<storage> data : Data;
    // data.b;

    auto p = GetParam();

    SetupStorageBuffer({
        Member("a", ty.i32()),
        Member("b", p.member_type(ty)),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone, MemberAccessor("data", "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(GlslGeneratorImplTest_MemberAccessor,
                         GlslGeneratorImplTest_MemberAccessor_StorageBufferLoad,
                         testing::Values(TypeCase{ty_u32, "data.b"},
                                         TypeCase{ty_f32, "data.b"},
                                         TypeCase{ty_i32, "data.b"},
                                         TypeCase{ty_vec2<u32>, "data.b"},
                                         TypeCase{ty_vec2<f32>, "data.b"},
                                         TypeCase{ty_vec2<i32>, "data.b"},
                                         TypeCase{ty_vec3<u32>, "data.b"},
                                         TypeCase{ty_vec3<f32>, "data.b"},
                                         TypeCase{ty_vec3<i32>, "data.b"},
                                         TypeCase{ty_vec4<u32>, "data.b"},
                                         TypeCase{ty_vec4<f32>, "data.b"},
                                         TypeCase{ty_vec4<i32>, "data.b"},
                                         TypeCase{ty_mat2x2<f32>, "data.b"},
                                         TypeCase{ty_mat2x3<f32>, "data.b"},
                                         TypeCase{ty_mat2x4<f32>, "data.b"},
                                         TypeCase{ty_mat3x2<f32>, "data.b"},
                                         TypeCase{ty_mat3x3<f32>, "data.b"},
                                         TypeCase{ty_mat3x4<f32>, "data.b"},
                                         TypeCase{ty_mat4x2<f32>, "data.b"},
                                         TypeCase{ty_mat4x3<f32>, "data.b"},
                                         TypeCase{ty_mat4x4<f32>, "data.b"}));

using GlslGeneratorImplTest_MemberAccessor_StorageBufferStore =
    GlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;
TEST_P(GlslGeneratorImplTest_MemberAccessor_StorageBufferStore, Test) {
    // struct Data {
    //   a : i32;
    //   b : <type>;
    // };
    // var<storage> data : Data;
    // data.b = <type>();

    auto p = GetParam();

    SetupStorageBuffer({
        Member("a", ty.i32()),
        Member("b", p.member_type(ty)),
    });

    SetupFunction({
        Decl(Var("value", p.member_type(ty), ast::StorageClass::kNone,
                 Construct(p.member_type(ty)))),
        Assign(MemberAccessor("data", "b"), Expr("value")),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(GlslGeneratorImplTest_MemberAccessor,
                         GlslGeneratorImplTest_MemberAccessor_StorageBufferStore,
                         testing::Values(TypeCase{ty_u32, "data.b = value"},
                                         TypeCase{ty_f32, "data.b = value"},
                                         TypeCase{ty_i32, "data.b = value"},
                                         TypeCase{ty_vec2<u32>, "data.b = value"},
                                         TypeCase{ty_vec2<f32>, "data.b = value"},
                                         TypeCase{ty_vec2<i32>, "data.b = value"},
                                         TypeCase{ty_vec3<u32>, "data.b = value"},
                                         TypeCase{ty_vec3<f32>, "data.b = value"},
                                         TypeCase{ty_vec3<i32>, "data.b = value"},
                                         TypeCase{ty_vec4<u32>, "data.b = value"},
                                         TypeCase{ty_vec4<f32>, "data.b = value"},
                                         TypeCase{ty_vec4<i32>, "data.b = value"},
                                         TypeCase{ty_mat2x2<f32>, "data.b = value"},
                                         TypeCase{ty_mat2x3<f32>, "data.b = value"},
                                         TypeCase{ty_mat2x4<f32>, "data.b = value"},
                                         TypeCase{ty_mat3x2<f32>, "data.b = value"},
                                         TypeCase{ty_mat3x3<f32>, "data.b = value"},
                                         TypeCase{ty_mat3x4<f32>, "data.b = value"},
                                         TypeCase{ty_mat4x2<f32>, "data.b = value"},
                                         TypeCase{ty_mat4x3<f32>, "data.b = value"},
                                         TypeCase{ty_mat4x4<f32>, "data.b = value"}));

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_Matrix_Empty) {
    // struct Data {
    //   z : f32;
    //   a : mat2x3<f32>;
    // };
    // var<storage> data : Data;
    // data.a = mat2x3<f32>();

    SetupStorageBuffer({
        Member("a", ty.i32()),
        Member("b", ty.mat2x3<f32>()),
    });

    SetupFunction({
        Assign(MemberAccessor("data", "b"), Construct(ty.mat2x3<f32>(), ast::ExpressionList{})),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Data {
  int a;
  mat2x3 b;
};

layout(binding = 0, std430) buffer Data_1 {
  int a;
  mat2x3 b;
} data;
void tint_symbol() {
  data.b = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_Matrix_Single_Element) {
    // struct Data {
    //   z : f32;
    //   a : mat4x3<f32>;
    // };
    // var<storage> data : Data;
    // data.a[2i][1i];

    SetupStorageBuffer({
        Member("z", ty.f32()),
        Member("a", ty.mat4x3<f32>()),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone,
                 IndexAccessor(IndexAccessor(MemberAccessor("data", "a"), 2_i), 1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Data {
  float z;
  mat4x3 a;
};

layout(binding = 0, std430) buffer Data_1 {
  float z;
  mat4x3 a;
} data;
void tint_symbol() {
  float x = data.a[2][1];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_StorageBuffer_Load_Int_FromArray) {
    // struct Data {
    //   a : array<i32, 5>;
    // };
    // var<storage> data : Data;
    // data.a[2];

    SetupStorageBuffer({
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>(4)),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone,
                 IndexAccessor(MemberAccessor("data", "a"), 2_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Data {
  float z;
  int a[5];
};

layout(binding = 0, std430) buffer Data_1 {
  float z;
  int a[5];
} data;
void tint_symbol() {
  int x = data.a[2];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_StorageBuffer_Load_Int_FromArray_ExprIdx) {
    // struct Data {
    //   a : array<i32, 5u>;
    // };
    // var<storage> data : Data;
    // data.a[(2i + 4i) - 3i];

    SetupStorageBuffer({
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>(4)),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone,
                 IndexAccessor(MemberAccessor("data", "a"), Sub(Add(2_i, 4_i), 3_i)))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Data {
  float z;
  int a[5];
};

layout(binding = 0, std430) buffer Data_1 {
  float z;
  int a[5];
} data;
void tint_symbol() {
  int x = data.a[((2 + 4) - 3)];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_ToArray) {
    // struct Data {
    //   a : array<i32, 5u>;
    // };
    // var<storage> data : Data;
    // data.a[2i] = 2i;

    SetupStorageBuffer({
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>(4)),
    });

    SetupFunction({
        Assign(IndexAccessor(MemberAccessor("data", "a"), 2_i), 2_i),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Data {
  float z;
  int a[5];
};

layout(binding = 0, std430) buffer Data_1 {
  float z;
  int a[5];
} data;
void tint_symbol() {
  data.a[2] = 2;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_MultiLevel) {
    // struct Inner {
    //   a : vec3<i32>;
    //   b : vec3<f32>;
    // };
    // struct Data {
    //   var c : array<Inner, 4u>;
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b

    auto* inner = Structure("Inner", {
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer({
        Member("c", ty.array(ty.Of(inner), 4_u, 32)),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone,
                 MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Inner {
  vec3 a;
  vec3 b;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer Data_1 {
  Inner c[4];
} data;
void tint_symbol() {
  vec3 x = data.c[2].b;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_MultiLevel_Swizzle) {
    // struct Inner {
    //   a : vec3<i32>;
    //   b : vec3<f32>;
    // };
    // struct Data {
    //   var c : array<Inner, 4u>;
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b.xy

    auto* inner = Structure("Inner", {
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer({
        Member("c", ty.array(ty.Of(inner), 4_u, 32)),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone,
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "xy"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Inner {
  vec3 a;
  vec3 b;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer Data_1 {
  Inner c[4];
} data;
void tint_symbol() {
  vec2 x = data.c[2].b.xy;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor,
       StorageBuffer_Load_MultiLevel_Swizzle_SingleLetter) {  // NOLINT
    // struct Inner {
    //   a : vec3<i32>;
    //   b : vec3<f32>;
    // };
    // struct Data {
    //   var c : array<Inner, 4u>;
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b.g

    auto* inner = Structure("Inner", {
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer({
        Member("c", ty.array(ty.Of(inner), 4_u, 32)),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone,
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "g"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Inner {
  vec3 a;
  vec3 b;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer Data_1 {
  Inner c[4];
} data;
void tint_symbol() {
  float x = data.c[2].b.g;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_MultiLevel_Index) {
    // struct Inner {
    //   a : vec3<i32>;
    //   b : vec3<f32>;
    // };
    // struct Data {
    //   var c : array<Inner, 4u>;
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b[1i]

    auto* inner = Structure("Inner", {
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer({
        Member("c", ty.array(ty.Of(inner), 4_u, 32)),
    });

    SetupFunction({
        Decl(Var("x", nullptr, ast::StorageClass::kNone,
                 IndexAccessor(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
                               1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Inner {
  vec3 a;
  vec3 b;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer Data_1 {
  Inner c[4];
} data;
void tint_symbol() {
  float x = data.c[2].b[1];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_MultiLevel) {
    // struct Inner {
    //   a : vec3<i32>;
    //   b : vec3<f32>;
    // };
    // struct Data {
    //   var c : array<Inner, 4u>;
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b = vec3<f32>(1.f, 2.f, 3.f);

    auto* inner = Structure("Inner", {
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer({
        Member("c", ty.array(ty.Of(inner), 4_u, 32)),
    });

    SetupFunction({
        Assign(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
               vec3<f32>(1.f, 2.f, 3.f)),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Inner {
  vec3 a;
  vec3 b;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer Data_1 {
  Inner c[4];
} data;
void tint_symbol() {
  data.c[2].b = vec3(1.0f, 2.0f, 3.0f);
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_Swizzle_SingleLetter) {
    // struct Inner {
    //   a : vec3<i32>;
    //   b : vec3<f32>;
    // };
    // struct Data {
    //   var c : array<Inner, 4u>;
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b.y = 1.f;

    auto* inner = Structure("Inner", {
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer({
        Member("c", ty.array(ty.Of(inner), 4_u, 32)),
    });

    SetupFunction({
        Assign(MemberAccessor(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
                              "y"),
               Expr(1.f)),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(#version 310 es
precision mediump float;

struct Inner {
  ivec3 a;
  vec3 b;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer Data_1 {
  Inner c[4];
} data;
void tint_symbol() {
  data.c[2].b.y = 1.0f;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, Swizzle_xyz) {
    auto* var =
        Var("my_vec", ty.vec4<f32>(), ast::StorageClass::kNone, vec4<f32>(1.f, 2.f, 3.f, 4.f));
    auto* expr = MemberAccessor("my_vec", "xyz");
    WrapInFunction(var, expr);

    GeneratorImpl& gen = SanitizeAndBuild();
    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("my_vec.xyz"));
}

TEST_F(GlslGeneratorImplTest_MemberAccessor, Swizzle_gbr) {
    auto* var =
        Var("my_vec", ty.vec4<f32>(), ast::StorageClass::kNone, vec4<f32>(1.f, 2.f, 3.f, 4.f));
    auto* expr = MemberAccessor("my_vec", "gbr");
    WrapInFunction(var, expr);

    GeneratorImpl& gen = SanitizeAndBuild();
    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("my_vec.gbr"));
}

}  // namespace
}  // namespace tint::writer::glsl
