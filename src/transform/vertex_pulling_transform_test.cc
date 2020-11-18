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

#include "src/transform/vertex_pulling_transform.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/function.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/transform/manager.h"
#include "src/type_determiner.h"
#include "src/validator/validator.h"

namespace tint {
namespace transform {
namespace {

class VertexPullingTransformHelper {
 public:
  VertexPullingTransformHelper() {
    mod_ = std::make_unique<ast::Module>();
    manager_ = std::make_unique<Manager>(&ctx_, mod_.get());
    auto transform =
        std::make_unique<VertexPullingTransform>(&ctx_, mod_.get());
    transform_ = transform.get();
    manager_->append(std::move(transform));
  }

  // Create basic module with an entry point and vertex function
  void InitBasicModule() {
    auto* func = create<ast::Function>(
        "main", ast::VariableList{},
        ctx_.type_mgr().Get(std::make_unique<ast::type::VoidType>()),
        create<ast::BlockStatement>());
    func->add_decoration(
        create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}));
    mod()->AddFunction(func);
  }

  // Set up the transformation, after building the module
  void InitTransform(VertexStateDescriptor vertex_state) {
    EXPECT_TRUE(mod_->IsValid());

    TypeDeterminer td(&ctx_, mod_.get());
    EXPECT_TRUE(td.Determine());

    transform_->SetVertexState(
        std::make_unique<VertexStateDescriptor>(vertex_state));
    transform_->SetEntryPoint("main");
  }

  // Inserts a variable which will be converted to vertex pulling
  void AddVertexInputVariable(uint32_t location,
                              std::string name,
                              ast::type::Type* type) {
    auto* var = create<ast::DecoratedVariable>(
        create<ast::Variable>(name, ast::StorageClass::kInput, type));

    ast::VariableDecorationList decorations;
    decorations.push_back(create<ast::LocationDecoration>(location, Source{}));

    var->set_decorations(decorations);
    mod_->AddGlobalVariable(var);
  }

  Context* ctx() { return &ctx_; }
  ast::Module* mod() { return mod_.get(); }
  Manager* manager() { return manager_.get(); }
  VertexPullingTransform* transform() { return transform_; }

  /// Creates a new `ast::Node` owned by the Module. When the Module is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return mod_->create<T>(std::forward<ARGS>(args)...);
  }

 private:
  Context ctx_;
  std::unique_ptr<ast::Module> mod_;
  std::unique_ptr<Manager> manager_;
  VertexPullingTransform* transform_;
};

class VertexPullingTransformTest : public VertexPullingTransformHelper,
                                   public testing::Test {};

TEST_F(VertexPullingTransformTest, Error_NoVertexState) {
  EXPECT_FALSE(manager()->Run());
  EXPECT_EQ(manager()->error(), "SetVertexState not called");
}

TEST_F(VertexPullingTransformTest, Error_NoEntryPoint) {
  transform()->SetVertexState(std::make_unique<VertexStateDescriptor>());
  EXPECT_FALSE(manager()->Run());
  EXPECT_EQ(manager()->error(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, Error_InvalidEntryPoint) {
  InitBasicModule();
  InitTransform({});
  transform()->SetEntryPoint("_");

  EXPECT_FALSE(manager()->Run());
  EXPECT_EQ(manager()->error(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, Error_EntryPointWrongStage) {
  auto* func = create<ast::Function>(
      "main", ast::VariableList{},
      ctx()->type_mgr().Get(std::make_unique<ast::type::VoidType>()),
      create<ast::BlockStatement>());
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));
  mod()->AddFunction(func);

  InitTransform({});
  EXPECT_FALSE(manager()->Run());
  EXPECT_EQ(manager()->error(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, BasicModule) {
  InitBasicModule();
  InitTransform({});
  EXPECT_TRUE(manager()->Run());
}

TEST_F(VertexPullingTransformTest, OneAttribute) {
  InitBasicModule();

  ast::type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);

  InitTransform({{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}});

  EXPECT_TRUE(manager()->Run());

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    var_a
    private
    __f32
  }
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
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
            mod()->to_str());
}

TEST_F(VertexPullingTransformTest, OneInstancedAttribute) {
  InitBasicModule();

  ast::type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);

  InitTransform(
      {{{4, InputStepMode::kInstance, {{VertexFormat::kF32, 0, 0}}}}});

  EXPECT_TRUE(manager()->Run());

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    var_a
    private
    __f32
  }
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{instance_idx}
    }
    _tint_pulling_instance_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
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
            mod()->to_str());
}

TEST_F(VertexPullingTransformTest, OneAttributeDifferentOutputSet) {
  InitBasicModule();

  ast::type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);

  InitTransform({{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}});
  transform()->SetPullingBufferBindingSet(5);

  EXPECT_TRUE(manager()->Run());

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
  }
  Variable{
    var_a
    private
    __f32
  }
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{5}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
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
            mod()->to_str());
}

// We expect the transform to use an existing builtin variables if it finds them
TEST_F(VertexPullingTransformTest, ExistingVertexIndexAndInstanceIndex) {
  InitBasicModule();

  ast::type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);
  AddVertexInputVariable(1, "var_b", &f32);

  ast::type::I32Type i32;
  {
    auto* vertex_index_var =
        create<ast::DecoratedVariable>(create<ast::Variable>(
            "custom_vertex_index", ast::StorageClass::kInput, &i32));

    ast::VariableDecorationList decorations;
    decorations.push_back(
        create<ast::BuiltinDecoration>(ast::Builtin::kVertexIdx, Source{}));

    vertex_index_var->set_decorations(decorations);
    mod()->AddGlobalVariable(vertex_index_var);
  }

  {
    auto* instance_index_var =
        create<ast::DecoratedVariable>(create<ast::Variable>(
            "custom_instance_index", ast::StorageClass::kInput, &i32));

    ast::VariableDecorationList decorations;
    decorations.push_back(
        create<ast::BuiltinDecoration>(ast::Builtin::kInstanceIdx, Source{}));

    instance_index_var->set_decorations(decorations);
    mod()->AddGlobalVariable(instance_index_var);
  }

  InitTransform(
      {{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}},
        {4, InputStepMode::kInstance, {{VertexFormat::kF32, 0, 1}}}}});

  EXPECT_TRUE(manager()->Run());

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    custom_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{instance_idx}
    }
    custom_instance_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_1
    storage_buffer
    __struct_TintVertexData
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
            mod()->to_str());
}

TEST_F(VertexPullingTransformTest, TwoAttributesSameBuffer) {
  InitBasicModule();

  ast::type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);

  ast::type::ArrayType vec4_f32{&f32, 4u};
  AddVertexInputVariable(1, "var_b", &vec4_f32);

  InitTransform(
      {{{16,
         InputStepMode::kVertex,
         {{VertexFormat::kF32, 0, 0}, {VertexFormat::kVec4F32, 0, 1}}}}});

  EXPECT_TRUE(manager()->Run());

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
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
            mod()->to_str());
}

TEST_F(VertexPullingTransformTest, FloatVectorAttributes) {
  InitBasicModule();

  ast::type::F32Type f32;
  ast::type::ArrayType vec2_f32{&f32, 2u};
  AddVertexInputVariable(0, "var_a", &vec2_f32);

  ast::type::ArrayType vec3_f32{&f32, 3u};
  AddVertexInputVariable(1, "var_b", &vec3_f32);

  ast::type::ArrayType vec4_f32{&f32, 4u};
  AddVertexInputVariable(2, "var_c", &vec4_f32);

  InitTransform(
      {{{8, InputStepMode::kVertex, {{VertexFormat::kVec2F32, 0, 0}}},
        {12, InputStepMode::kVertex, {{VertexFormat::kVec3F32, 0, 1}}},
        {16, InputStepMode::kVertex, {{VertexFormat::kVec4F32, 0, 2}}}}});

  EXPECT_TRUE(manager()->Run());

  EXPECT_EQ(R"(Module{
  TintVertexData Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    _tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_TintVertexData
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_1
    storage_buffer
    __struct_TintVertexData
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{2}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_2
    storage_buffer
    __struct_TintVertexData
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
            mod()->to_str());
}

}  // namespace
}  // namespace transform
}  // namespace tint
