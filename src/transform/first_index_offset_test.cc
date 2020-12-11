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

#include "src/transform/first_index_offset.h"

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/block_statement.h"
#include "src/ast/builder.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/ast/storage_class.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/diagnostic/formatter.h"
#include "src/source.h"
#include "src/transform/manager.h"

namespace tint {
namespace transform {
namespace {

class FirstIndexOffsetTest : public testing::Test {};

struct ModuleBuilder : public ast::BuilderWithModule {
  ast::Module Module() {
    Build();
    return std::move(*mod);
  }

 protected:
  void AddBuiltinInput(const std::string& name, ast::Builtin builtin) {
    mod->AddGlobalVariable(
        Var(name, ast::StorageClass::kInput, ty.u32, nullptr,
            {create<ast::BuiltinDecoration>(builtin, Source{})}));
  }

  ast::Function* AddFunction(const std::string& name,
                             ast::VariableList params = {}) {
    auto* func = create<ast::Function>(Source{}, name, std::move(params),
                                       ty.u32, create<ast::BlockStatement>(),
                                       ast::FunctionDecorationList());
    mod->AddFunction(func);
    return func;
  }

  virtual void Build() = 0;
};

TEST_F(FirstIndexOffsetTest, Error_AlreadyTransformed) {
  struct Builder : public ModuleBuilder {
    void Build() override {
      AddBuiltinInput("vert_idx", ast::Builtin::kVertexIdx);
      AddFunction("test")->body()->append(create<ast::ReturnStatement>(
          Source{}, create<ast::IdentifierExpression>("vert_idx")));
    }
  };

  Manager manager;
  manager.append(std::make_unique<FirstIndexOffset>(0, 0));
  manager.append(std::make_unique<FirstIndexOffset>(1, 1));

  auto module = Builder{}.Module();
  auto result = manager.Run(&module);

  // Release the source module to ensure there's no uncloned data in result
  { auto tmp = std::move(module); }

  ASSERT_EQ(diag::Formatter().format(result.diagnostics),
            "error: First index offset transform has already been applied.");
}

TEST_F(FirstIndexOffsetTest, EmptyModule) {
  Manager manager;
  manager.append(std::make_unique<FirstIndexOffset>(0, 0));

  ast::Module module;
  auto result = manager.Run(&module);

  // Release the source module to ensure there's no uncloned data in result
  { auto tmp = std::move(module); }

  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto got = result.module.to_str();
  auto* expected = "Module{\n}\n";
  EXPECT_EQ(got, expected);
}

TEST_F(FirstIndexOffsetTest, BasicModuleVertexIndex) {
  struct Builder : public ModuleBuilder {
    void Build() override {
      AddBuiltinInput("vert_idx", ast::Builtin::kVertexIdx);
      AddFunction("test")->body()->append(create<ast::ReturnStatement>(
          Source{}, create<ast::IdentifierExpression>("vert_idx")));
    }
  };

  Manager manager;
  manager.append(std::make_unique<FirstIndexOffset>(1, 2));

  auto module = Builder{}.Module();
  auto result = manager.Run(&module);

  // Release the source module to ensure there's no uncloned data in result
  { auto tmp = std::move(module); }

  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto got = result.module.to_str();
  auto* expected =
      R"(Module{
  TintFirstIndexOffsetData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] tint_first_vertex_index: __u32}
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    tint_first_index_offset_vert_idx
    in
    __u32
  }
  Variable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{2}
    }
    tint_first_index_data
    uniform
    __struct_TintFirstIndexOffsetData
  }
  Function test -> __u32
  ()
  {
    VariableDeclStatement{
      VariableConst{
        vert_idx
        none
        __u32
        {
          Binary[__u32]{
            Identifier[__ptr_in__u32]{tint_first_index_offset_vert_idx}
            add
            MemberAccessor[__ptr_uniform__u32]{
              Identifier[__ptr_uniform__struct_TintFirstIndexOffsetData]{tint_first_index_data}
              Identifier[not set]{tint_first_vertex_index}
            }
          }
        }
      }
    }
    Return{
      {
        Identifier[__u32]{vert_idx}
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected);
}

TEST_F(FirstIndexOffsetTest, BasicModuleInstanceIndex) {
  struct Builder : public ModuleBuilder {
    void Build() override {
      AddBuiltinInput("inst_idx", ast::Builtin::kInstanceIdx);
      AddFunction("test")->body()->append(create<ast::ReturnStatement>(
          Source{}, create<ast::IdentifierExpression>("inst_idx")));
    }
  };

  Manager manager;
  manager.append(std::make_unique<FirstIndexOffset>(1, 7));

  auto module = Builder{}.Module();
  auto result = manager.Run(&module);

  // Release the source module to ensure there's no uncloned data in result
  { auto tmp = std::move(module); }

  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto got = result.module.to_str();
  auto* expected = R"(Module{
  TintFirstIndexOffsetData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] tint_first_instance_index: __u32}
  }
  Variable{
    Decorations{
      BuiltinDecoration{instance_idx}
    }
    tint_first_index_offset_inst_idx
    in
    __u32
  }
  Variable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{7}
    }
    tint_first_index_data
    uniform
    __struct_TintFirstIndexOffsetData
  }
  Function test -> __u32
  ()
  {
    VariableDeclStatement{
      VariableConst{
        inst_idx
        none
        __u32
        {
          Binary[__u32]{
            Identifier[__ptr_in__u32]{tint_first_index_offset_inst_idx}
            add
            MemberAccessor[__ptr_uniform__u32]{
              Identifier[__ptr_uniform__struct_TintFirstIndexOffsetData]{tint_first_index_data}
              Identifier[not set]{tint_first_instance_index}
            }
          }
        }
      }
    }
    Return{
      {
        Identifier[__u32]{inst_idx}
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected);
}

TEST_F(FirstIndexOffsetTest, BasicModuleBothIndex) {
  struct Builder : public ModuleBuilder {
    void Build() override {
      AddBuiltinInput("inst_idx", ast::Builtin::kInstanceIdx);
      AddBuiltinInput("vert_idx", ast::Builtin::kVertexIdx);
      AddFunction("test")->body()->append(
          create<ast::ReturnStatement>(Source{}, Expr(1u)));
    }
  };

  auto transform = std::make_unique<FirstIndexOffset>(1, 7);
  auto* transform_ptr = transform.get();

  Manager manager;
  manager.append(std::move(transform));

  auto module = Builder{}.Module();
  auto result = manager.Run(&module);

  // Release the source module to ensure there's no uncloned data in result
  { auto tmp = std::move(module); }

  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto got = result.module.to_str();
  auto* expected = R"(Module{
  TintFirstIndexOffsetData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] tint_first_vertex_index: __u32}
    StructMember{[[ offset 4 ]] tint_first_instance_index: __u32}
  }
  Variable{
    Decorations{
      BuiltinDecoration{instance_idx}
    }
    tint_first_index_offset_inst_idx
    in
    __u32
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    tint_first_index_offset_vert_idx
    in
    __u32
  }
  Variable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{7}
    }
    tint_first_index_data
    uniform
    __struct_TintFirstIndexOffsetData
  }
  Function test -> __u32
  ()
  {
    Return{
      {
        ScalarConstructor[__u32]{1}
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected);

  EXPECT_TRUE(transform_ptr->HasVertexIndex());
  EXPECT_EQ(transform_ptr->GetFirstVertexOffset(), 0u);

  EXPECT_TRUE(transform_ptr->HasInstanceIndex());
  EXPECT_EQ(transform_ptr->GetFirstInstanceOffset(), 4u);
}

TEST_F(FirstIndexOffsetTest, NestedCalls) {
  struct Builder : public ModuleBuilder {
    void Build() override {
      AddBuiltinInput("vert_idx", ast::Builtin::kVertexIdx);
      ast::Function* func1 = AddFunction("func1");
      func1->body()->append(create<ast::ReturnStatement>(
          Source{}, create<ast::IdentifierExpression>("vert_idx")));
      ast::Function* func2 = AddFunction("func2");
      func2->body()->append(create<ast::ReturnStatement>(
          Source{}, create<ast::CallExpression>(
                        create<ast::IdentifierExpression>("func1"),
                        ast::ExpressionList{})));
    }
  };

  auto transform = std::make_unique<FirstIndexOffset>(2, 2);

  Manager manager;
  manager.append(std::move(transform));

  auto module = Builder{}.Module();
  auto result = manager.Run(&module);

  // Release the source module to ensure there's no uncloned data in result
  { auto tmp = std::move(module); }

  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  auto got = result.module.to_str();
  auto* expected = R"(Module{
  TintFirstIndexOffsetData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] tint_first_vertex_index: __u32}
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    tint_first_index_offset_vert_idx
    in
    __u32
  }
  Variable{
    Decorations{
      BindingDecoration{2}
      SetDecoration{2}
    }
    tint_first_index_data
    uniform
    __struct_TintFirstIndexOffsetData
  }
  Function func1 -> __u32
  ()
  {
    VariableDeclStatement{
      VariableConst{
        vert_idx
        none
        __u32
        {
          Binary[__u32]{
            Identifier[__ptr_in__u32]{tint_first_index_offset_vert_idx}
            add
            MemberAccessor[__ptr_uniform__u32]{
              Identifier[__ptr_uniform__struct_TintFirstIndexOffsetData]{tint_first_index_data}
              Identifier[not set]{tint_first_vertex_index}
            }
          }
        }
      }
    }
    Return{
      {
        Identifier[__u32]{vert_idx}
      }
    }
  }
  Function func2 -> __u32
  ()
  {
    Return{
      {
        Call[__u32]{
          Identifier[__u32]{func1}
          (
          )
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected);
}

}  // namespace
}  // namespace transform
}  // namespace tint
