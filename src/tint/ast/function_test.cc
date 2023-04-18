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

#include "gtest/gtest-spi.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/ast/workgroup_attribute.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using FunctionTest = TestHelper;

TEST_F(FunctionTest, Creation_i32ReturnType) {
    utils::Vector params{Param("var", ty.i32())};
    auto i32 = ty.i32();
    auto* var = params[0];

    auto* f = Func("func", params, i32, utils::Empty);
    EXPECT_EQ(f->name->symbol, Symbols().Get("func"));
    ASSERT_EQ(f->params.Length(), 1u);
    CheckIdentifier(f->return_type, "i32");
    EXPECT_EQ(f->params[0], var);
}

TEST_F(FunctionTest, Creation_NoReturnType) {
    utils::Vector params{Param("var", ty.i32())};
    auto* var = params[0];

    auto* f = Func("func", params, ty.void_(), utils::Empty);
    EXPECT_EQ(f->name->symbol, Symbols().Get("func"));
    ASSERT_EQ(f->params.Length(), 1u);
    EXPECT_EQ(f->return_type, nullptr);
    EXPECT_EQ(f->params[0], var);
}

TEST_F(FunctionTest, Creation_SingleParam) {
    utils::Vector params{Param("var", ty.i32())};
    auto* var = params[0];

    auto* f = Func("func", params, ty.void_(), utils::Empty);
    EXPECT_EQ(f->name->symbol, Symbols().Get("func"));
    ASSERT_EQ(f->params.Length(), 1u);
    EXPECT_EQ(f->return_type, nullptr);
    EXPECT_EQ(f->params[0], var);
    ASSERT_NE(f->body, nullptr);
    ASSERT_EQ(f->body->statements.Length(), 0u);
}

TEST_F(FunctionTest, Creation_Body_Vector) {
    auto* f = Func("func", utils::Empty, ty.void_(), utils::Vector{Discard(), Return()});
    ASSERT_NE(f->body, nullptr);
    ASSERT_EQ(f->body->statements.Length(), 2u);
    EXPECT_TRUE(f->body->statements[0]->Is<DiscardStatement>());
    EXPECT_TRUE(f->body->statements[1]->Is<ReturnStatement>());
}

TEST_F(FunctionTest, Creation_Body_Block) {
    auto* f = Func("func", utils::Empty, ty.void_(), Block(Discard(), Return()));
    ASSERT_NE(f->body, nullptr);
    ASSERT_EQ(f->body->statements.Length(), 2u);
    EXPECT_TRUE(f->body->statements[0]->Is<DiscardStatement>());
    EXPECT_TRUE(f->body->statements[1]->Is<ReturnStatement>());
}

TEST_F(FunctionTest, Creation_Body_Stmt) {
    auto* f = Func("func", utils::Empty, ty.void_(), Return());
    ASSERT_NE(f->body, nullptr);
    ASSERT_EQ(f->body->statements.Length(), 1u);
    EXPECT_TRUE(f->body->statements[0]->Is<ReturnStatement>());
}

TEST_F(FunctionTest, Creation_Body_Nullptr) {
    auto* f = Func("func", utils::Empty, ty.void_(), nullptr);
    EXPECT_EQ(f->body, nullptr);
}

TEST_F(FunctionTest, Creation_WithSource) {
    utils::Vector params{Param("var", ty.i32())};

    auto* f = Func(Source{Source::Location{20, 2}}, "func", params, ty.void_(), utils::Empty);
    auto src = f->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(FunctionTest, Assert_NullName) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Func(static_cast<Identifier*>(nullptr), utils::Empty, b.ty.void_(), utils::Empty);
        },
        "internal compiler error");
}

TEST_F(FunctionTest, Assert_TemplatedName) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Func(b.Ident("a", "b"), utils::Empty, b.ty.void_(), utils::Empty);
        },
        "internal compiler error");
}

TEST_F(FunctionTest, Assert_NullParam) {
    using ParamList = utils::Vector<const Parameter*, 2>;
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            ParamList params;
            params.Push(b.Param("var", b.ty.i32()));
            params.Push(nullptr);
            b.Func("f", params, b.ty.void_(), utils::Empty);
        },
        "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_Symbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Func(b2.Sym("func"), utils::Empty, b1.ty.void_(), utils::Empty);
        },
        "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_Param) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Func("func",
                    utils::Vector{
                        b2.Param("var", b2.ty.i32()),
                    },
                    b1.ty.void_(), utils::Empty);
        },
        "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_Attr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Func("func", utils::Empty, b1.ty.void_(), utils::Empty,
                    utils::Vector{
                        b2.WorkgroupSize(2_i, 4_i, 6_i),
                    });
        },
        "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_ReturnType) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Func("func", utils::Empty, b2.ty.i32(), utils::Empty);
        },
        "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_ReturnAttr) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Func("func", utils::Empty, b1.ty.void_(), utils::Empty, utils::Empty,
                    utils::Vector{
                        b2.WorkgroupSize(2_i, 4_i, 6_i),
                    });
        },
        "internal compiler error");
}

using FunctionListTest = TestHelper;

TEST_F(FunctionListTest, FindSymbol) {
    auto* func = Func("main", utils::Empty, ty.f32(), utils::Empty);
    FunctionList list;
    list.Add(func);
    EXPECT_EQ(func, list.Find(Symbols().Register("main")));
}

TEST_F(FunctionListTest, FindSymbolMissing) {
    FunctionList list;
    EXPECT_EQ(nullptr, list.Find(Symbols().Register("Missing")));
}

TEST_F(FunctionListTest, FindSymbolStage) {
    auto* fs = Func("main", utils::Empty, ty.f32(), utils::Empty,
                    utils::Vector{
                        Stage(PipelineStage::kFragment),
                    });
    auto* vs = Func("main", utils::Empty, ty.f32(), utils::Empty,
                    utils::Vector{
                        Stage(PipelineStage::kVertex),
                    });
    FunctionList list;
    list.Add(fs);
    list.Add(vs);
    EXPECT_EQ(fs, list.Find(Symbols().Register("main"), PipelineStage::kFragment));
    EXPECT_EQ(vs, list.Find(Symbols().Register("main"), PipelineStage::kVertex));
}

TEST_F(FunctionListTest, FindSymbolStageMissing) {
    FunctionList list;
    list.Add(Func("main", utils::Empty, ty.f32(), utils::Empty,
                  utils::Vector{
                      Stage(PipelineStage::kFragment),
                  }));
    EXPECT_EQ(nullptr, list.Find(Symbols().Register("main"), PipelineStage::kVertex));
}

TEST_F(FunctionListTest, HasStage) {
    FunctionList list;
    list.Add(Func("main", utils::Empty, ty.f32(), utils::Empty,
                  utils::Vector{
                      Stage(PipelineStage::kFragment),
                  }));
    EXPECT_TRUE(list.HasStage(PipelineStage::kFragment));
    EXPECT_FALSE(list.HasStage(PipelineStage::kVertex));
}

}  // namespace
}  // namespace tint::ast
