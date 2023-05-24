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

#include "src/tint/transform/manager.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/ast/transform/transform.h"
#include "src/tint/program_builder.h"

#if TINT_BUILD_IR
#include "src/tint/ir/builder.h"              // nogncheck
#include "src/tint/ir/transform/transform.h"  // nogncheck
#endif                                        // TINT_BUILD_IR

namespace tint::transform {
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
        CloneContext ctx{&b, src};
        b.Func(b.Sym("ast_func"), {}, b.ty.void_(), {});
        ctx.Clone();
        return Program(std::move(b));
    }
};

#if TINT_BUILD_IR
class IR_AddFunction final : public ir::transform::Transform {
    void Run(ir::Module* mod, const DataMap&, DataMap&) const override {
        ir::Builder builder(*mod);
        auto* func =
            builder.CreateFunction(mod->symbols.New("ir_func"), mod->Types().Get<type::Void>());
        func->StartTarget()->SetInstructions(utils::Vector{builder.Branch(func->EndTarget())});
        mod->functions.Push(func);
    }
};
#endif  // TINT_BUILD_IR

Program MakeAST() {
    ProgramBuilder b;
    b.Func(b.Sym("main"), {}, b.ty.void_(), {});
    return Program(std::move(b));
}

#if TINT_BUILD_IR
ir::Module MakeIR() {
    ir::Module mod;
    ir::Builder builder(mod);
    auto* func =
        builder.CreateFunction(builder.ir.symbols.New("main"), mod.Types().Get<type::Void>());
    func->StartTarget()->SetInstructions(utils::Vector{builder.Branch(func->EndTarget())});
    builder.ir.functions.Push(func);
    return mod;
}
#endif  // TINT_BUILD_IR

// Test that an AST program is always cloned, even if all transforms are skipped.
TEST_F(TransformManagerTest, AST_AlwaysClone) {
    Program ast = MakeAST();

    transform::Manager manager;
    transform::DataMap outputs;
    manager.Add<AST_NoOp>();

    auto result = manager.Run(&ast, {}, outputs);
    EXPECT_TRUE(result.IsValid()) << result.Diagnostics();
    EXPECT_NE(result.ID(), ast.ID());
    ASSERT_EQ(result.AST().Functions().Length(), 1u);
    EXPECT_EQ(result.AST().Functions()[0]->name->symbol.Name(), "main");
}

#if TINT_BUILD_IR

// Test that an IR module is mutated in place.
TEST_F(TransformManagerTest, IR_MutateInPlace) {
    ir::Module ir = MakeIR();

    transform::Manager manager;
    transform::DataMap outputs;
    manager.Add<IR_AddFunction>();

    manager.Run(&ir, {}, outputs);

    ASSERT_EQ(ir.functions.Length(), 2u);
    EXPECT_EQ(ir.functions[0]->Name().Name(), "main");
    EXPECT_EQ(ir.functions[1]->Name().Name(), "ir_func");
}

TEST_F(TransformManagerTest, AST_MixedTransforms_AST_Before_IR) {
    Program ast = MakeAST();

    transform::Manager manager;
    transform::DataMap outputs;
    manager.Add<AST_AddFunction>();
    manager.Add<IR_AddFunction>();

    auto result = manager.Run(&ast, {}, outputs);
    ASSERT_TRUE(result.IsValid()) << result.Diagnostics();
    ASSERT_EQ(result.AST().Functions().Length(), 3u);
    EXPECT_EQ(result.AST().Functions()[0]->name->symbol.Name(), "ast_func");
    EXPECT_EQ(result.AST().Functions()[1]->name->symbol.Name(), "main");
    EXPECT_EQ(result.AST().Functions()[2]->name->symbol.Name(), "ir_func");
}

TEST_F(TransformManagerTest, AST_MixedTransforms_IR_Before_AST) {
    Program ast = MakeAST();

    transform::Manager manager;
    transform::DataMap outputs;
    manager.Add<IR_AddFunction>();
    manager.Add<AST_AddFunction>();

    auto result = manager.Run(&ast, {}, outputs);
    ASSERT_TRUE(result.IsValid()) << result.Diagnostics();
    ASSERT_EQ(result.AST().Functions().Length(), 3u);
    EXPECT_EQ(result.AST().Functions()[0]->name->symbol.Name(), "ast_func");
    EXPECT_EQ(result.AST().Functions()[1]->name->symbol.Name(), "main");
    EXPECT_EQ(result.AST().Functions()[2]->name->symbol.Name(), "ir_func");
}

TEST_F(TransformManagerTest, IR_MixedTransforms_AST_Before_IR) {
    ir::Module ir = MakeIR();

    transform::Manager manager;
    transform::DataMap outputs;
    manager.Add<AST_AddFunction>();
    manager.Add<IR_AddFunction>();

    manager.Run(&ir, {}, outputs);
    ASSERT_EQ(ir.functions.Length(), 3u);
    EXPECT_EQ(ir.functions[0]->Name().Name(), "ast_func");
    EXPECT_EQ(ir.functions[1]->Name().Name(), "main");
    EXPECT_EQ(ir.functions[2]->Name().Name(), "ir_func");
}

TEST_F(TransformManagerTest, IR_MixedTransforms_IR_Before_AST) {
    ir::Module ir = MakeIR();

    transform::Manager manager;
    transform::DataMap outputs;
    manager.Add<IR_AddFunction>();
    manager.Add<AST_AddFunction>();

    manager.Run(&ir, {}, outputs);
    ASSERT_EQ(ir.functions.Length(), 3u);
    EXPECT_EQ(ir.functions[0]->Name().Name(), "ast_func");
    EXPECT_EQ(ir.functions[1]->Name().Name(), "main");
    EXPECT_EQ(ir.functions[2]->Name().Name(), "ir_func");
}
#endif  // TINT_BUILD_IR

}  // namespace
}  // namespace tint::transform
