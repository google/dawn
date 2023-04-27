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

#include <string>

#include "src/tint/ast/test_helper.h"
#include "src/tint/clone_context.h"
#include "src/tint/program_builder.h"
#include "src/tint/transform/transform.h"

#include "gtest/gtest.h"

namespace tint::transform {
namespace {

using namespace tint::number_suffixes;  // NOLINT

// Inherit from Transform so we have access to protected methods
struct CreateASTTypeForTest : public testing::Test, public Transform {
    ApplyResult Apply(const Program*, const DataMap&, DataMap&) const override {
        return SkipTransform;
    }

    ast::Type create(std::function<type::Type*(ProgramBuilder&)> create_sem_type) {
        ProgramBuilder sem_type_builder;
        auto* sem_type = create_sem_type(sem_type_builder);
        Program program(std::move(sem_type_builder));
        CloneContext ctx(&ast_type_builder, &program, false);
        return CreateASTTypeFor(ctx, sem_type);
    }

    ProgramBuilder ast_type_builder;
};

TEST_F(CreateASTTypeForTest, Basic) {
    auto check = [&](ast::Type ty, const char* expect) {
        ast::CheckIdentifier(ty->identifier, expect);
    };

    check(create([](ProgramBuilder& b) { return b.create<type::I32>(); }), "i32");
    check(create([](ProgramBuilder& b) { return b.create<type::U32>(); }), "u32");
    check(create([](ProgramBuilder& b) { return b.create<type::F32>(); }), "f32");
    check(create([](ProgramBuilder& b) { return b.create<type::Bool>(); }), "bool");
    EXPECT_EQ(create([](ProgramBuilder& b) { return b.create<type::Void>(); }), nullptr);
}

TEST_F(CreateASTTypeForTest, Matrix) {
    auto mat = create([](ProgramBuilder& b) {
        auto* column_type = b.create<type::Vector>(b.create<type::F32>(), 2u);
        return b.create<type::Matrix>(column_type, 3u);
    });

    ast::CheckIdentifier(mat, ast::Template("mat3x2", "f32"));
}

TEST_F(CreateASTTypeForTest, Vector) {
    auto vec =
        create([](ProgramBuilder& b) { return b.create<type::Vector>(b.create<type::F32>(), 2u); });

    ast::CheckIdentifier(vec, ast::Template("vec2", "f32"));
}

TEST_F(CreateASTTypeForTest, ArrayImplicitStride) {
    auto arr = create([](ProgramBuilder& b) {
        return b.create<type::Array>(b.create<type::F32>(), b.create<type::ConstantArrayCount>(2u),
                                     4u, 4u, 32u, 32u);
    });

    ast::CheckIdentifier(arr, ast::Template("array", "f32", 2_u));
    auto* tmpl_attr = arr->identifier->As<ast::TemplatedIdentifier>();
    ASSERT_NE(tmpl_attr, nullptr);
    EXPECT_TRUE(tmpl_attr->attributes.IsEmpty());
}

TEST_F(CreateASTTypeForTest, ArrayNonImplicitStride) {
    auto arr = create([](ProgramBuilder& b) {
        return b.create<type::Array>(b.create<type::F32>(), b.create<type::ConstantArrayCount>(2u),
                                     4u, 4u, 64u, 32u);
    });
    ast::CheckIdentifier(arr, ast::Template("array", "f32", 2_u));
    auto* tmpl_attr = arr->identifier->As<ast::TemplatedIdentifier>();
    ASSERT_NE(tmpl_attr, nullptr);
    ASSERT_EQ(tmpl_attr->attributes.Length(), 1u);
    ASSERT_TRUE(tmpl_attr->attributes[0]->Is<ast::StrideAttribute>());
    ASSERT_EQ(tmpl_attr->attributes[0]->As<ast::StrideAttribute>()->stride, 64u);
}

// crbug.com/tint/1764
TEST_F(CreateASTTypeForTest, AliasedArrayWithComplexOverrideLength) {
    // override O = 123;
    // type A = array<i32, O*2>;
    //
    // var<workgroup> W : A;
    //
    ProgramBuilder b;
    auto* arr_len = b.Mul("O", 2_a);
    b.Override("O", b.Expr(123_a));
    auto* alias = b.Alias("A", b.ty.array(b.ty.i32(), arr_len));

    Program program(std::move(b));

    auto* arr_ty = program.Sem().Get(alias);

    CloneContext ctx(&ast_type_builder, &program, false);
    auto ast_ty = CreateASTTypeFor(ctx, arr_ty);
    ast::CheckIdentifier(ast_ty, "A");
}

TEST_F(CreateASTTypeForTest, Struct) {
    auto str = create([](ProgramBuilder& b) {
        auto* decl = b.Structure("S", {});
        return b.create<sem::Struct>(decl, decl->name->symbol, utils::Empty, 4u /* align */,
                                     4u /* size */, 4u /* size_no_padding */);
    });

    ast::CheckIdentifier(str, "S");
}

TEST_F(CreateASTTypeForTest, PrivatePointer) {
    auto ptr = create([](ProgramBuilder& b) {
        return b.create<type::Pointer>(b.create<type::I32>(), builtin::AddressSpace::kPrivate,
                                       builtin::Access::kReadWrite);
    });

    ast::CheckIdentifier(ptr, ast::Template("ptr", "private", "i32"));
}

TEST_F(CreateASTTypeForTest, StorageReadWritePointer) {
    auto ptr = create([](ProgramBuilder& b) {
        return b.create<type::Pointer>(b.create<type::I32>(), builtin::AddressSpace::kStorage,
                                       builtin::Access::kReadWrite);
    });

    ast::CheckIdentifier(ptr, ast::Template("ptr", "storage", "i32", "read_write"));
}

}  // namespace
}  // namespace tint::transform
