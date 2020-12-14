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

#include "src/transform/emit_vertex_point_size.h"

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/builder.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/demangler.h"
#include "src/diagnostic/formatter.h"
#include "src/transform/manager.h"

namespace tint {
namespace transform {
namespace {

class EmitVertexPointSizeTest : public testing::Test {
 public:
  Transform::Output Transform(ast::Module in) {
    Manager manager;
    manager.append(std::make_unique<EmitVertexPointSize>());
    return manager.Run(&in);
  }
};

struct ModuleBuilder : public ast::BuilderWithModule {
  ModuleBuilder() {}

  ast::Module Module() {
    Build();
    return std::move(*mod);
  }

 protected:
  virtual void Build() = 0;
};

TEST_F(EmitVertexPointSizeTest, VertexStageBasic) {
  struct Builder : ModuleBuilder {
    void Build() override {
      auto* block = create<ast::BlockStatement>(
          Source{},
          ast::StatementList{
              create<ast::VariableDeclStatement>(
                  Source{}, Var("builtin_assignments_should_happen_before_this",
                                tint::ast::StorageClass::kFunction, ty.f32)),
          });

      auto a_sym = mod->RegisterSymbol("non_entry_a");
      mod->AddFunction(create<ast::Function>(
          Source{}, a_sym, "non_entry_a", ast::VariableList{}, ty.void_,
          create<ast::BlockStatement>(Source{}, ast::StatementList{}),
          ast::FunctionDecorationList{}));

      auto entry_sym = mod->RegisterSymbol("entry");
      auto* entry = create<ast::Function>(
          Source{}, entry_sym, "entry", ast::VariableList{}, ty.void_, block,
          ast::FunctionDecorationList{
              create<ast::StageDecoration>(Source{},
                                           ast::PipelineStage::kVertex),
          });
      mod->AddFunction(entry);

      auto b_sym = mod->RegisterSymbol("non_entry_b");
      mod->AddFunction(create<ast::Function>(
          Source{}, b_sym, "non_entry_b", ast::VariableList{}, ty.void_,
          create<ast::BlockStatement>(Source{}, ast::StatementList{}),
          ast::FunctionDecorationList{}));
    }
  };

  auto result = Transform(Builder{}.Module());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto* expected = R"(Module{
  Variable{
    Decorations{
      BuiltinDecoration{pointsize}
    }
    tint_pointsize
    out
    __f32
  }
  Function non_entry_a -> __void
  ()
  {
  }
  Function entry -> __void
  StageDecoration{vertex}
  ()
  {
    Assignment{
      Identifier[__ptr_out__f32]{tint_pointsize}
      ScalarConstructor[__f32]{1.000000}
    }
    VariableDeclStatement{
      Variable{
        builtin_assignments_should_happen_before_this
        function
        __f32
      }
    }
  }
  Function non_entry_b -> __void
  ()
  {
  }
}
)";
  EXPECT_EQ(expected,
            Demangler().Demangle(result.module, result.module.to_str()));
}

TEST_F(EmitVertexPointSizeTest, VertexStageEmpty) {
  struct Builder : ModuleBuilder {
    void Build() override {
      auto a_sym = mod->RegisterSymbol("non_entry_a");
      mod->AddFunction(create<ast::Function>(
          Source{}, a_sym, "non_entry_a", ast::VariableList{}, ty.void_,
          create<ast::BlockStatement>(Source{}, ast::StatementList{}),
          ast::FunctionDecorationList{}));

      auto entry_sym = mod->RegisterSymbol("entry");
      mod->AddFunction(create<ast::Function>(
          Source{}, entry_sym, "entry", ast::VariableList{}, ty.void_,
          create<ast::BlockStatement>(Source{}, ast::StatementList{}),
          ast::FunctionDecorationList{
              create<ast::StageDecoration>(Source{},
                                           ast::PipelineStage::kVertex),
          }));

      auto b_sym = mod->RegisterSymbol("non_entry_b");
      mod->AddFunction(create<ast::Function>(
          Source{}, b_sym, "non_entry_b", ast::VariableList{}, ty.void_,
          create<ast::BlockStatement>(Source{}, ast::StatementList{}),
          ast::FunctionDecorationList{}));
    }
  };

  auto result = Transform(Builder{}.Module());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto* expected = R"(Module{
  Variable{
    Decorations{
      BuiltinDecoration{pointsize}
    }
    tint_pointsize
    out
    __f32
  }
  Function non_entry_a -> __void
  ()
  {
  }
  Function entry -> __void
  StageDecoration{vertex}
  ()
  {
    Assignment{
      Identifier[__ptr_out__f32]{tint_pointsize}
      ScalarConstructor[__f32]{1.000000}
    }
  }
  Function non_entry_b -> __void
  ()
  {
  }
}
)";
  EXPECT_EQ(expected,
            Demangler().Demangle(result.module, result.module.to_str()));
}

TEST_F(EmitVertexPointSizeTest, NonVertexStage) {
  struct Builder : ModuleBuilder {
    void Build() override {
      auto frag_sym = mod->RegisterSymbol("fragment_entry");
      auto* fragment_entry = create<ast::Function>(
          Source{}, frag_sym, "fragment_entry", ast::VariableList{}, ty.void_,
          create<ast::BlockStatement>(Source{}, ast::StatementList{}),
          ast::FunctionDecorationList{
              create<ast::StageDecoration>(Source{},
                                           ast::PipelineStage::kFragment),
          });
      mod->AddFunction(fragment_entry);

      auto comp_sym = mod->RegisterSymbol("compute_entry");
      auto* compute_entry = create<ast::Function>(
          Source{}, comp_sym, "compute_entry", ast::VariableList{}, ty.void_,
          create<ast::BlockStatement>(Source{}, ast::StatementList{}),
          ast::FunctionDecorationList{
              create<ast::StageDecoration>(Source{},
                                           ast::PipelineStage::kCompute),
          });
      mod->AddFunction(compute_entry);
    }
  };

  auto result = Transform(Builder{}.Module());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto* expected = R"(Module{
  Function fragment_entry -> __void
  StageDecoration{fragment}
  ()
  {
  }
  Function compute_entry -> __void
  StageDecoration{compute}
  ()
  {
  }
}
)";
  EXPECT_EQ(expected,
            Demangler().Demangle(result.module, result.module.to_str()));
}

}  // namespace
}  // namespace transform
}  // namespace tint
