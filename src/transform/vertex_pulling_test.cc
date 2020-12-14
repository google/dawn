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

#include "src/transform/vertex_pulling.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/ast/function.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/demangler.h"
#include "src/diagnostic/formatter.h"
#include "src/transform/manager.h"
#include "src/type_determiner.h"
#include "src/validator/validator.h"

namespace tint {
namespace transform {
namespace {

class VertexPullingHelper {
 public:
  VertexPullingHelper() {
    mod_ = std::make_unique<ast::Module>();
    manager_ = std::make_unique<Manager>();
    auto transform = std::make_unique<VertexPulling>();
    transform_ = transform.get();
    manager_->append(std::move(transform));
  }

  // Create basic module with an entry point and vertex function
  void InitBasicModule() {
    auto* func = create<ast::Function>(
        Source{}, mod()->RegisterSymbol("main"), "main", ast::VariableList{},
        mod_->create<ast::type::Void>(),
        create<ast::BlockStatement>(Source{}, ast::StatementList{}),
        ast::FunctionDecorationList{create<ast::StageDecoration>(
            Source{}, ast::PipelineStage::kVertex)});
    mod()->AddFunction(func);
  }

  // Set up the transformation, after building the module
  void InitTransform(VertexStateDescriptor vertex_state) {
    EXPECT_TRUE(mod_->IsValid());

    TypeDeterminer td(mod_.get());
    EXPECT_TRUE(td.Determine());

    transform_->SetVertexState(vertex_state);
    transform_->SetEntryPoint("main");
  }

  // Inserts a variable which will be converted to vertex pulling
  void AddVertexInputVariable(uint32_t location,
                              std::string name,
                              ast::type::Type* type) {
    auto* var = create<ast::Variable>(
        Source{},                   // source
        name,                       // name
        ast::StorageClass::kInput,  // storage_class
        type,                       // type
        false,                      // is_const
        nullptr,                    // constructor
        ast::VariableDecorationList{
            // decorations
            create<ast::LocationDecoration>(Source{}, location),
        });

    mod_->AddGlobalVariable(var);
  }

  ast::Module* mod() { return mod_.get(); }

  Manager* manager() { return manager_.get(); }
  VertexPulling* transform() { return transform_; }

  /// Creates a new `ast::Node` owned by the Module. When the Module is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return mod_->create<T>(std::forward<ARGS>(args)...);
  }

 private:
  std::unique_ptr<ast::Module> mod_;
  std::unique_ptr<Manager> manager_;
  VertexPulling* transform_;
};

class VertexPullingTest : public VertexPullingHelper, public testing::Test {};

TEST_F(VertexPullingTest, Error_NoVertexState) {
  auto result = manager()->Run(mod());
  EXPECT_TRUE(result.diagnostics.contains_errors());
  EXPECT_EQ(diag::Formatter().format(result.diagnostics),
            "error: SetVertexState not called");
}

TEST_F(VertexPullingTest, Error_NoEntryPoint) {
  transform()->SetVertexState({});
  auto result = manager()->Run(mod());
  EXPECT_TRUE(result.diagnostics.contains_errors());
  EXPECT_EQ(diag::Formatter().format(result.diagnostics),
            "error: Vertex stage entry point not found");
}

TEST_F(VertexPullingTest, Error_InvalidEntryPoint) {
  InitBasicModule();
  InitTransform({});
  transform()->SetEntryPoint("_");

  auto result = manager()->Run(mod());
  EXPECT_TRUE(result.diagnostics.contains_errors());
  EXPECT_EQ(diag::Formatter().format(result.diagnostics),
            "error: Vertex stage entry point not found");
}

TEST_F(VertexPullingTest, Error_EntryPointWrongStage) {
  auto* func = create<ast::Function>(
      Source{}, mod()->RegisterSymbol("main"), "main", ast::VariableList{},
      mod()->create<ast::type::Void>(),
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kFragment),
      });
  mod()->AddFunction(func);

  InitTransform({});
  auto result = manager()->Run(mod());
  EXPECT_TRUE(result.diagnostics.contains_errors());
  EXPECT_EQ(diag::Formatter().format(result.diagnostics),
            "error: Vertex stage entry point not found");
}

TEST_F(VertexPullingTest, BasicModule) {
  InitBasicModule();
  InitTransform({});
  auto result = manager()->Run(mod());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);
}

TEST_F(VertexPullingTest, OneAttribute) {
  InitBasicModule();

  ast::type::F32 f32;
  AddVertexInputVariable(0, "var_a", &f32);

  InitTransform({{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}});

  auto result = manager()->Run(mod());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  Variable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    var_a
    private
    __f32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          _tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor[__u32]{4}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__f32]{var_a}
        Bitcast[__f32]<__f32>{
          ArrayAccessor[__ptr_storage_buffer__u32]{
            MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
              Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
              Identifier[not set]{_tint_vertex_data}
            }
            Binary[__i32]{
              Identifier[__ptr_function__i32]{_tint_pulling_pos}
              divide
              ScalarConstructor[__u32]{4}
            }
          }
        }
      }
    }
  }
}
)",
            Demangler().Demangle(result.module, result.module.to_str()));
}

TEST_F(VertexPullingTest, OneInstancedAttribute) {
  InitBasicModule();

  ast::type::F32 f32;
  AddVertexInputVariable(0, "var_a", &f32);

  InitTransform(
      {{{4, InputStepMode::kInstance, {{VertexFormat::kF32, 0, 0}}}}});

  auto result = manager()->Run(mod());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    Decorations{
      BuiltinDecoration{instance_idx}
    }
    _tint_pulling_instance_index
    in
    __i32
  }
  Variable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    var_a
    private
    __f32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          _tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_instance_index}
            multiply
            ScalarConstructor[__u32]{4}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__f32]{var_a}
        Bitcast[__f32]<__f32>{
          ArrayAccessor[__ptr_storage_buffer__u32]{
            MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
              Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
              Identifier[not set]{_tint_vertex_data}
            }
            Binary[__i32]{
              Identifier[__ptr_function__i32]{_tint_pulling_pos}
              divide
              ScalarConstructor[__u32]{4}
            }
          }
        }
      }
    }
  }
}
)",
            Demangler().Demangle(result.module, result.module.to_str()));
}

TEST_F(VertexPullingTest, OneAttributeDifferentOutputSet) {
  InitBasicModule();

  ast::type::F32 f32;
  AddVertexInputVariable(0, "var_a", &f32);

  InitTransform({{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}});
  transform()->SetPullingBufferBindingSet(5);

  auto result = manager()->Run(mod());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  Variable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{5}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    var_a
    private
    __f32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          _tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor[__u32]{4}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__f32]{var_a}
        Bitcast[__f32]<__f32>{
          ArrayAccessor[__ptr_storage_buffer__u32]{
            MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
              Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
              Identifier[not set]{_tint_vertex_data}
            }
            Binary[__i32]{
              Identifier[__ptr_function__i32]{_tint_pulling_pos}
              divide
              ScalarConstructor[__u32]{4}
            }
          }
        }
      }
    }
  }
}
)",
            Demangler().Demangle(result.module, result.module.to_str()));
}

// We expect the transform to use an existing builtin variables if it finds them
TEST_F(VertexPullingTest, ExistingVertexIndexAndInstanceIndex) {
  InitBasicModule();

  ast::type::F32 f32;
  AddVertexInputVariable(0, "var_a", &f32);
  AddVertexInputVariable(1, "var_b", &f32);

  ast::type::I32 i32;

  mod()->AddGlobalVariable(create<ast::Variable>(
      Source{},                   // source
      "custom_vertex_index",      // name
      ast::StorageClass::kInput,  // storage_class
      &i32,                       // type
      false,                      // is_const
      nullptr,                    // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(Source{}, ast::Builtin::kVertexIdx),
      }));

  mod()->AddGlobalVariable(create<ast::Variable>(
      Source{},                   // source
      "custom_instance_index",    // name
      ast::StorageClass::kInput,  // storage_class
      &i32,                       // type
      false,                      // is_const
      nullptr,                    // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(Source{}, ast::Builtin::kInstanceIdx),
      }));

  InitTransform(
      {{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}},
        {4, InputStepMode::kInstance, {{VertexFormat::kF32, 0, 1}}}}});

  auto result = manager()->Run(mod());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_1
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    var_a
    private
    __f32
  }
  Variable{
    var_b
    private
    __f32
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    custom_vertex_index
    in
    __i32
  }
  Variable{
    Decorations{
      BuiltinDecoration{instance_idx}
    }
    custom_instance_index
    in
    __i32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          _tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{custom_vertex_index}
            multiply
            ScalarConstructor[__u32]{4}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__f32]{var_a}
        Bitcast[__f32]<__f32>{
          ArrayAccessor[__ptr_storage_buffer__u32]{
            MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
              Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
              Identifier[not set]{_tint_vertex_data}
            }
            Binary[__i32]{
              Identifier[__ptr_function__i32]{_tint_pulling_pos}
              divide
              ScalarConstructor[__u32]{4}
            }
          }
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{custom_instance_index}
            multiply
            ScalarConstructor[__u32]{4}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__f32]{var_b}
        Bitcast[__f32]<__f32>{
          ArrayAccessor[__ptr_storage_buffer__u32]{
            MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
              Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_1}
              Identifier[not set]{_tint_vertex_data}
            }
            Binary[__i32]{
              Identifier[__ptr_function__i32]{_tint_pulling_pos}
              divide
              ScalarConstructor[__u32]{4}
            }
          }
        }
      }
    }
  }
}
)",
            Demangler().Demangle(result.module, result.module.to_str()));
}

TEST_F(VertexPullingTest, TwoAttributesSameBuffer) {
  InitBasicModule();

  ast::type::F32 f32;
  AddVertexInputVariable(0, "var_a", &f32);

  ast::type::Array vec4_f32{&f32, 4u, ast::ArrayDecorationList{}};
  AddVertexInputVariable(1, "var_b", &vec4_f32);

  InitTransform(
      {{{16,
         InputStepMode::kVertex,
         {{VertexFormat::kF32, 0, 0}, {VertexFormat::kVec4F32, 0, 1}}}}});

  auto result = manager()->Run(mod());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  Variable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    var_a
    private
    __f32
  }
  Variable{
    var_b
    private
    __array__f32_4
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          _tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor[__u32]{16}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__f32]{var_a}
        Bitcast[__f32]<__f32>{
          ArrayAccessor[__ptr_storage_buffer__u32]{
            MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
              Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
              Identifier[not set]{_tint_vertex_data}
            }
            Binary[__i32]{
              Identifier[__ptr_function__i32]{_tint_pulling_pos}
              divide
              ScalarConstructor[__u32]{4}
            }
          }
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor[__u32]{16}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__array__f32_4]{var_b}
        TypeConstructor[__vec_4__f32]{
          __vec_4__f32
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{0}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{4}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{8}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{12}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
        }
      }
    }
  }
}
)",
            Demangler().Demangle(result.module, result.module.to_str()));
}

TEST_F(VertexPullingTest, FloatVectorAttributes) {
  InitBasicModule();

  ast::type::F32 f32;
  ast::type::Array vec2_f32{&f32, 2u, ast::ArrayDecorationList{}};
  AddVertexInputVariable(0, "var_a", &vec2_f32);

  ast::type::Array vec3_f32{&f32, 3u, ast::ArrayDecorationList{}};
  AddVertexInputVariable(1, "var_b", &vec3_f32);

  ast::type::Array vec4_f32{&f32, 4u, ast::ArrayDecorationList{}};
  AddVertexInputVariable(2, "var_c", &vec4_f32);

  InitTransform(
      {{{8, InputStepMode::kVertex, {{VertexFormat::kVec2F32, 0, 0}}},
        {12, InputStepMode::kVertex, {{VertexFormat::kVec3F32, 0, 1}}},
        {16, InputStepMode::kVertex, {{VertexFormat::kVec4F32, 0, 2}}}}});

  auto result = manager()->Run(mod());
  ASSERT_FALSE(result.diagnostics.contains_errors())
      << diag::Formatter().format(result.diagnostics);

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  Variable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_1
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    Decorations{
      BindingDecoration{2}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_2
    storage_buffer
    __struct_TintVertexData
  }
  Variable{
    var_a
    private
    __array__f32_2
  }
  Variable{
    var_b
    private
    __array__f32_3
  }
  Variable{
    var_c
    private
    __array__f32_4
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          _tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor[__u32]{8}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__array__f32_2]{var_a}
        TypeConstructor[__vec_2__f32]{
          __vec_2__f32
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{0}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_0}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{4}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor[__u32]{12}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__array__f32_3]{var_b}
        TypeConstructor[__vec_3__f32]{
          __vec_3__f32
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_1}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{0}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_1}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{4}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_1}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{8}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
        }
      }
      Assignment{
        Identifier[__ptr_function__i32]{_tint_pulling_pos}
        Binary[__i32]{
          Binary[__i32]{
            Identifier[__ptr_in__i32]{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor[__u32]{16}
          }
          add
          ScalarConstructor[__u32]{0}
        }
      }
      Assignment{
        Identifier[__ptr_private__array__f32_4]{var_c}
        TypeConstructor[__vec_4__f32]{
          __vec_4__f32
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_2}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{0}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_2}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{4}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_2}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{8}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
          Bitcast[__f32]<__f32>{
            ArrayAccessor[__ptr_storage_buffer__u32]{
              MemberAccessor[__ptr_storage_buffer__array__u32_stride_4]{
                Identifier[__ptr_storage_buffer__struct_TintVertexData]{_tint_pulling_vertex_buffer_2}
                Identifier[not set]{_tint_vertex_data}
              }
              Binary[__i32]{
                Binary[__i32]{
                  Identifier[__ptr_function__i32]{_tint_pulling_pos}
                  add
                  ScalarConstructor[__u32]{12}
                }
                divide
                ScalarConstructor[__u32]{4}
              }
            }
          }
        }
      }
    }
  }
}
)",
            Demangler().Demangle(result.module, result.module.to_str()));
}

}  // namespace
}  // namespace transform
}  // namespace tint
