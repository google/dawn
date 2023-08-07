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

#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"

#include "gmock/gmock.h"

namespace tint::glsl::writer {
namespace {

using ::testing::HasSubstr;
using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

using create_type_func_ptr = ast::Type (*)(const ProgramBuilder::TypesBuilder& ty);

inline ast::Type ty_i32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.i32();
}
inline ast::Type ty_u32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.u32();
}
inline ast::Type ty_f32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.f32();
}
template <typename T>
inline ast::Type ty_vec2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec2<T>();
}
template <typename T>
inline ast::Type ty_vec3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec3<T>();
}
template <typename T>
inline ast::Type ty_vec4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec4<T>();
}
template <typename T>
inline ast::Type ty_mat2x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x2<T>();
}
template <typename T>
inline ast::Type ty_mat2x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x3<T>();
}
template <typename T>
inline ast::Type ty_mat2x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x4<T>();
}
template <typename T>
inline ast::Type ty_mat3x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x2<T>();
}
template <typename T>
inline ast::Type ty_mat3x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x3<T>();
}
template <typename T>
inline ast::Type ty_mat3x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x4<T>();
}
template <typename T>
inline ast::Type ty_mat4x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x2<T>();
}
template <typename T>
inline ast::Type ty_mat4x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x3<T>();
}
template <typename T>
inline ast::Type ty_mat4x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x4<T>();
}

template <typename BASE>
class GlslASTPrinterTest_MemberAccessorBase : public BASE {
  public:
    void SetupStorageBuffer(VectorRef<const ast::StructMember*> members) {
        ProgramBuilder& b = *this;

        auto* s = b.Structure("Data", members);

        b.GlobalVar("data", b.ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
                    b.Group(1_a), b.Binding(0_a));
    }

    void SetupFunction(VectorRef<const ast::Statement*> statements) {
        ProgramBuilder& b = *this;
        b.Func("main", tint::Empty, b.ty.void_(), statements,
               Vector<const ast::Attribute*, 1>{
                   b.Stage(ast::PipelineStage::kFragment),
               });
    }
};

using GlslASTPrinterTest_MemberAccessor = GlslASTPrinterTest_MemberAccessorBase<TestHelper>;

template <typename T>
using GlslASTPrinterTest_MemberAccessorWithParam =
    GlslASTPrinterTest_MemberAccessorBase<TestParamHelper<T>>;

TEST_F(GlslASTPrinterTest_MemberAccessor, EmitExpression_MemberAccessor) {
    auto* s = Structure("Data", Vector{Member("mem", ty.f32())});
    GlobalVar("str", ty.Of(s), core::AddressSpace::kPrivate);

    auto* expr = MemberAccessor("str", "mem");
    WrapInFunction(Var("expr", ty.f32(), expr));

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

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
    return out << c.expected;
}

using GlslASTPrinterTest_MemberAccessor_StorageBufferLoad =
    GlslASTPrinterTest_MemberAccessorWithParam<TypeCase>;
TEST_P(GlslASTPrinterTest_MemberAccessor_StorageBufferLoad, Test) {
    // struct Data {
    //   a : i32;
    //   b : <type>;
    // };
    // var<storage> data : Data;
    // data.b;

    auto p = GetParam();

    SetupStorageBuffer(Vector{
        Member("a", ty.i32()),
        Member("b", p.member_type(ty)),
    });

    SetupFunction(Vector{
        Decl(Var("x", MemberAccessor("data", "b"))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_MemberAccessor,
                         GlslASTPrinterTest_MemberAccessor_StorageBufferLoad,
                         testing::Values(TypeCase{ty_u32, "data.inner.b"},
                                         TypeCase{ty_f32, "data.inner.b"},
                                         TypeCase{ty_i32, "data.inner.b"},
                                         TypeCase{ty_vec2<u32>, "data.inner.b"},
                                         TypeCase{ty_vec2<f32>, "data.inner.b"},
                                         TypeCase{ty_vec2<i32>, "data.inner.b"},
                                         TypeCase{ty_vec3<u32>, "data.inner.b"},
                                         TypeCase{ty_vec3<f32>, "data.inner.b"},
                                         TypeCase{ty_vec3<i32>, "data.inner.b"},
                                         TypeCase{ty_vec4<u32>, "data.inner.b"},
                                         TypeCase{ty_vec4<f32>, "data.inner.b"},
                                         TypeCase{ty_vec4<i32>, "data.inner.b"},
                                         TypeCase{ty_mat2x2<f32>, "data.inner.b"},
                                         TypeCase{ty_mat2x3<f32>, "data.inner.b"},
                                         TypeCase{ty_mat2x4<f32>, "data.inner.b"},
                                         TypeCase{ty_mat3x2<f32>, "data.inner.b"},
                                         TypeCase{ty_mat3x3<f32>, "data.inner.b"},
                                         TypeCase{ty_mat3x4<f32>, "data.inner.b"},
                                         TypeCase{ty_mat4x2<f32>, "data.inner.b"},
                                         TypeCase{ty_mat4x3<f32>, "data.inner.b"},
                                         TypeCase{ty_mat4x4<f32>, "data.inner.b"}));

using GlslASTPrinterTest_MemberAccessor_StorageBufferStore =
    GlslASTPrinterTest_MemberAccessorWithParam<TypeCase>;
TEST_P(GlslASTPrinterTest_MemberAccessor_StorageBufferStore, Test) {
    // struct Data {
    //   a : i32;
    //   b : <type>;
    // };
    // var<storage> data : Data;
    // data.b = <type>();

    auto p = GetParam();

    SetupStorageBuffer(Vector{
        Member("a", ty.i32()),
        Member("b", p.member_type(ty)),
    });

    SetupFunction(Vector{
        Decl(Var("value", p.member_type(ty), Call(p.member_type(ty)))),
        Assign(MemberAccessor("data", "b"), Expr("value")),
    });

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_MemberAccessor,
                         GlslASTPrinterTest_MemberAccessor_StorageBufferStore,
                         testing::Values(TypeCase{ty_u32, "data.inner.b = value"},
                                         TypeCase{ty_f32, "data.inner.b = value"},
                                         TypeCase{ty_i32, "data.inner.b = value"},
                                         TypeCase{ty_vec2<u32>, "data.inner.b = value"},
                                         TypeCase{ty_vec2<f32>, "data.inner.b = value"},
                                         TypeCase{ty_vec2<i32>, "data.inner.b = value"},
                                         TypeCase{ty_vec3<u32>, "data.inner.b = value"},
                                         TypeCase{ty_vec3<f32>, "data.inner.b = value"},
                                         TypeCase{ty_vec3<i32>, "data.inner.b = value"},
                                         TypeCase{ty_vec4<u32>, "data.inner.b = value"},
                                         TypeCase{ty_vec4<f32>, "data.inner.b = value"},
                                         TypeCase{ty_vec4<i32>, "data.inner.b = value"},
                                         TypeCase{ty_mat2x2<f32>, "data.inner.b = value"},
                                         TypeCase{ty_mat2x3<f32>, R"(
  data.inner.b[0] = value[0u];
  data.inner.b[1] = value[1u];)"},
                                         TypeCase{ty_mat2x4<f32>, "data.inner.b = value"},
                                         TypeCase{ty_mat3x2<f32>, "data.inner.b = value"},
                                         TypeCase{ty_mat3x3<f32>, R"(
  data.inner.b[0] = value[0u];
  data.inner.b[1] = value[1u];
  data.inner.b[2] = value[2u];)"},
                                         TypeCase{ty_mat3x4<f32>, "data.inner.b = value"},
                                         TypeCase{ty_mat4x2<f32>, "data.inner.b = value"},
                                         TypeCase{ty_mat4x3<f32>, R"(
  data.inner.b[0] = value[0u];
  data.inner.b[1] = value[1u];
  data.inner.b[2] = value[2u];
  data.inner.b[3] = value[3u];)"},
                                         TypeCase{ty_mat4x4<f32>, "data.inner.b = value"}));

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Store_Matrix_Empty) {
    // struct Data {
    //   z : f32;
    //   a : mat2x3<f32>;
    // };
    // var<storage> data : Data;
    // data.a = mat2x3<f32>();

    SetupStorageBuffer(Vector{
        Member("a", ty.i32()),
        Member("b", ty.mat2x3<f32>()),
    });

    SetupFunction(Vector{
        Assign(MemberAccessor("data", "b"), Call<mat2x3<f32>>()),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto* expected =
        R"(#version 310 es
precision highp float;

struct Data {
  int a;
  uint pad;
  uint pad_1;
  uint pad_2;
  mat2x3 b;
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void assign_and_preserve_padding_data_b(mat2x3 value) {
  data.inner.b[0] = value[0u];
  data.inner.b[1] = value[1u];
}

void tint_symbol() {
  assign_and_preserve_padding_data_b(mat2x3(vec3(0.0f), vec3(0.0f)));
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Load_Matrix_Single_Element) {
    // struct Data {
    //   z : f32;
    //   a : mat4x3<f32>;
    // };
    // var<storage> data : Data;
    // data.a[2i][1i];

    SetupStorageBuffer(Vector{
        Member("z", ty.f32()),
        Member("a", ty.mat4x3<f32>()),
    });

    SetupFunction(Vector{
        Decl(Var("x", IndexAccessor(IndexAccessor(MemberAccessor("data", "a"), 2_i), 1_i))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Data {
  float z;
  uint pad;
  uint pad_1;
  uint pad_2;
  mat4x3 a;
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  float x = data.inner.a[2][1];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor,
       EmitExpression_IndexAccessor_StorageBuffer_Load_Int_FromArray) {
    // struct Data {
    //   a : array<i32, 5>;
    // };
    // var<storage> data : Data;
    // data.a[2];

    SetupStorageBuffer(Vector{
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>()),
    });

    SetupFunction(Vector{
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), 2_i))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Data {
  float z;
  int a[5];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  int x = data.inner.a[2];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor,
       EmitExpression_IndexAccessor_StorageBuffer_Load_Int_FromArray_ExprIdx) {
    // struct Data {
    //   a : array<i32, 5u>;
    // };
    // var<storage> data : Data;
    // data.a[(2i + 4i) - 3i];

    SetupStorageBuffer(Vector{
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>()),
    });

    SetupFunction(Vector{
        Decl(Var("a", Expr(2_i))),
        Decl(Var("b", Expr(4_i))),
        Decl(Var("c", Expr(3_i))),
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), Sub(Add("a", "b"), "c")))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Data {
  float z;
  int a[5];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  int a = 2;
  int b = 4;
  int c = 3;
  int x = data.inner.a[((a + b) - c)];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Store_ToArray) {
    // struct Data {
    //   a : array<i32, 5u>;
    // };
    // var<storage> data : Data;
    // data.a[2i] = 2i;

    SetupStorageBuffer(Vector{
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>()),
    });

    SetupFunction(Vector{
        Assign(IndexAccessor(MemberAccessor("data", "a"), 2_i), 2_i),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Data {
  float z;
  int a[5];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  data.inner.a[2] = 2;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Load_MultiLevel) {
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

    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(Vector{
        Decl(Var("x", MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Inner {
  vec3 a;
  uint pad;
  vec3 b;
  uint pad_1;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  vec3 x = data.inner.c[2].b;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Load_MultiLevel_Swizzle) {
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

    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(Vector{
        Decl(Var("x",
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "xy"))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Inner {
  vec3 a;
  uint pad;
  vec3 b;
  uint pad_1;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  vec2 x = data.inner.c[2].b.xy;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor,
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

    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(Vector{
        Decl(Var("x",
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "g"))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Inner {
  vec3 a;
  uint pad;
  vec3 b;
  uint pad_1;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  float x = data.inner.c[2].b.g;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Load_MultiLevel_Index) {
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

    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(Vector{
        Decl(Var("x",
                 IndexAccessor(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
                               1_i))),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Inner {
  vec3 a;
  uint pad;
  vec3 b;
  uint pad_1;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  float x = data.inner.c[2].b[1];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Store_MultiLevel) {
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

    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.vec3<f32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(Vector{
        Assign(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
               Call<vec3<f32>>(1_f, 2_f, 3_f)),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Inner {
  vec3 a;
  uint pad;
  vec3 b;
  uint pad_1;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  data.inner.c[2].b = vec3(1.0f, 2.0f, 3.0f);
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, StorageBuffer_Store_Swizzle_SingleLetter) {
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

    auto* inner = Structure("Inner", Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(Vector{
        Assign(MemberAccessor(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
                              "y"),
               Expr(1_f)),
    });

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    auto* expected =
        R"(#version 310 es
precision highp float;

struct Inner {
  ivec3 a;
  uint pad;
  vec3 b;
  uint pad_1;
};

struct Data {
  Inner c[4];
};

layout(binding = 0, std430) buffer data_block_ssbo {
  Data inner;
} data;

void tint_symbol() {
  data.inner.c[2].b.y = 1.0f;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(gen.Result(), expected);
}

TEST_F(GlslASTPrinterTest_MemberAccessor, Swizzle_xyz) {
    auto* var = Var("my_vec", ty.vec4<f32>(), Call<vec4<f32>>(1_f, 2_f, 3_f, 4_f));
    auto* expr = MemberAccessor("my_vec", "xyz");
    WrapInFunction(var, expr);

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("my_vec.xyz"));
}

TEST_F(GlslASTPrinterTest_MemberAccessor, Swizzle_gbr) {
    auto* var = Var("my_vec", ty.vec4<f32>(), Call<vec4<f32>>(1_f, 2_f, 3_f, 4_f));
    auto* expr = MemberAccessor("my_vec", "gbr");
    WrapInFunction(var, expr);

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("my_vec.gbr"));
}

}  // namespace
}  // namespace tint::glsl::writer
