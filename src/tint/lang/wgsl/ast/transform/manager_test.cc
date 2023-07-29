// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/wgsl/ast/transform/manager.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint::ast::transform {
namespace {

using TransformManagerTest = testing::Test;

class AST_NoOp final : public ast::transform::Transform {
    ApplyResult Apply(const Program*, const DataMap&, DataMap&) const override {
        return SkipTransform;
    }
};

class AST_AddFunction final : public ast::transform::Transform {
    ApplyResult Apply(const Program* src, const DataMap&, DataMap&) const override {
        ProgramBuilder b;
        program::CloneContext ctx{&b, src};
        b.Func(b.Sym("ast_func"), {}, b.ty.void_(), {});
        ctx.Clone();
        return Program(std::move(b));
    }
};

Program MakeAST() {
    ProgramBuilder b;
    b.Func(b.Sym("main"), {}, b.ty.void_(), {});
    return Program(std::move(b));
}

// Test that an AST program is always cloned, even if all transforms are skipped.
TEST_F(TransformManagerTest, AST_AlwaysClone) {
    Program ast = MakeAST();

    Manager manager;
    DataMap outputs;
    manager.Add<AST_NoOp>();

    auto result = manager.Run(&ast, {}, outputs);
    EXPECT_TRUE(result.IsValid()) << result.Diagnostics();
    EXPECT_NE(result.ID(), ast.ID());
    ASSERT_EQ(result.AST().Functions().Length(), 1u);
    EXPECT_EQ(result.AST().Functions()[0]->name->symbol.Name(), "main");
}

}  // namespace
}  // namespace tint::ast::transform
