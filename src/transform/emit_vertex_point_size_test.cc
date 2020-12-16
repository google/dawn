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
  Transform::Output GetTransform(ast::Module in) {
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
      mod->AddFunction(Func("non_entry_a", ast::VariableList{}, ty.void_,
                            ast::StatementList{},
                            ast::FunctionDecorationList{}));

      auto* entry =
          Func("entry", ast::VariableList{}, ty.void_,
               ast::StatementList{
                   create<ast::VariableDeclStatement>(
                       Var("builtin_assignments_should_happen_before_this",
                           tint::ast::StorageClass::kFunction, ty.f32)),
               },
               ast::FunctionDecorationList{
                   create<ast::StageDecoration>(ast::PipelineStage::kVertex),
               });
      mod->AddFunction(entry);

      mod->AddFunction(Func("non_entry_b", ast::VariableList{}, ty.void_,
                            ast::StatementList{},
                            ast::FunctionDecorationList{}));
    }
  };

  auto result = GetTransform(Builder{}.Module());
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
      mod->AddFunction(Func("non_entry_a", ast::VariableList{}, ty.void_,
                            ast::StatementList{},
                            ast::FunctionDecorationList{}));

      mod->AddFunction(
          Func("entry", ast::VariableList{}, ty.void_, ast::StatementList{},
               ast::FunctionDecorationList{
                   create<ast::StageDecoration>(ast::PipelineStage::kVertex),
               }));

      mod->AddFunction(Func("non_entry_b", ast::VariableList{}, ty.void_,
                            ast::StatementList{},
                            ast::FunctionDecorationList{}));
    }
  };

  auto result = GetTransform(Builder{}.Module());
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
      auto* fragment_entry = Func(
          "fragment_entry", ast::VariableList{}, ty.void_, ast::StatementList{},
          ast::FunctionDecorationList{
              create<ast::StageDecoration>(ast::PipelineStage::kFragment),
          });
      mod->AddFunction(fragment_entry);

      auto* compute_entry = Func(
          "compute_entry", ast::VariableList{}, ty.void_, ast::StatementList{},
          ast::FunctionDecorationList{
              create<ast::StageDecoration>(ast::PipelineStage::kCompute),
          });
      mod->AddFunction(compute_entry);
    }
  };

  auto result = GetTransform(Builder{}.Module());
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
