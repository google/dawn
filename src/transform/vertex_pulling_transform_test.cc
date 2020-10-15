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

#include "gtest/gtest.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/function.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/type_determiner.h"
#include "src/validator.h"

namespace tint {
namespace transform {
namespace {

class VertexPullingTransformHelper {
 public:
  VertexPullingTransformHelper() {
    mod_ = std::make_unique<ast::Module>();
    transform_ = std::make_unique<VertexPullingTransform>(&ctx_, mod_.get());
  }

  // Create basic module with an entry point and vertex function
  void InitBasicModule() {
    auto func = std::make_unique<ast::Function>(
        "main", ast::VariableList{},
        ctx_.type_mgr().Get(std::make_unique<ast::type::VoidType>()));
    func->add_decoration(
        std::make_unique<ast::StageDecoration>(ast::PipelineStage ::kVertex));
    mod()->AddFunction(std::move(func));
  }

  // Set up the transformation, after building the module
  void InitTransform(VertexStateDescriptor vertex_state) {
    EXPECT_TRUE(mod_->IsValid());

    TypeDeterminer td(&ctx_, mod_.get());
    EXPECT_TRUE(td.Determine());

    transform_->SetVertexState(
        std::make_unique<VertexStateDescriptor>(std::move(vertex_state)));
    transform_->SetEntryPoint("main");
  }

  // Inserts a variable which will be converted to vertex pulling
  void AddVertexInputVariable(uint32_t location,
                              std::string name,
                              ast::type::Type* type) {
    auto var = std::make_unique<ast::DecoratedVariable>(
        std::make_unique<ast::Variable>(name, ast::StorageClass::kInput, type));

    ast::VariableDecorationList decorations;
    decorations.push_back(std::make_unique<ast::LocationDecoration>(location));

    var->set_decorations(std::move(decorations));
    mod_->AddGlobalVariable(std::move(var));
  }

  Context* ctx() { return &ctx_; }
  ast::Module* mod() { return mod_.get(); }
  VertexPullingTransform* transform() { return transform_.get(); }

 private:
  Context ctx_;
  std::unique_ptr<ast::Module> mod_;
  std::unique_ptr<VertexPullingTransform> transform_;
};

class VertexPullingTransformTest : public VertexPullingTransformHelper,
                                   public testing::Test {};

TEST_F(VertexPullingTransformTest, Error_NoVertexState) {
  EXPECT_FALSE(transform()->Run());
  EXPECT_EQ(transform()->error(), "SetVertexState not called");
}

TEST_F(VertexPullingTransformTest, Error_NoEntryPoint) {
  transform()->SetVertexState(std::make_unique<VertexStateDescriptor>());
  EXPECT_FALSE(transform()->Run());
  EXPECT_EQ(transform()->error(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, Error_InvalidEntryPoint) {
  InitBasicModule();
  InitTransform({});
  transform()->SetEntryPoint("_");

  EXPECT_FALSE(transform()->Run());
  EXPECT_EQ(transform()->error(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, Error_EntryPointWrongStage) {
  auto func = std::make_unique<ast::Function>(
      "main", ast::VariableList{},
      ctx()->type_mgr().Get(std::make_unique<ast::type::VoidType>()));
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));
  mod()->AddFunction(std::move(func));

  InitTransform({});
  EXPECT_FALSE(transform()->Run());
  EXPECT_EQ(transform()->error(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, BasicModule) {
  InitBasicModule();
  InitTransform({});
  EXPECT_TRUE(transform()->Run());
}

TEST_F(VertexPullingTransformTest, OneAttribute) {
  InitBasicModule();

  ast::type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);

  InitTransform({{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}}}});

  EXPECT_TRUE(transform()->Run());

  EXPECT_EQ(R"(Module{
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
    __alias_TintVertexData__struct_TintVertexData
  }
  TintVertexData -> __struct_TintVertexData
  Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        Bitcast<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{_tint_pulling_vertex_buffer_0}
              Identifier{_tint_vertex_data}
            }
            Binary{
              Identifier{_tint_pulling_pos}
              divide
              ScalarConstructor{4}
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

  EXPECT_TRUE(transform()->Run());

  EXPECT_EQ(R"(Module{
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
    __alias_TintVertexData__struct_TintVertexData
  }
  TintVertexData -> __struct_TintVertexData
  Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_instance_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        Bitcast<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{_tint_pulling_vertex_buffer_0}
              Identifier{_tint_vertex_data}
            }
            Binary{
              Identifier{_tint_pulling_pos}
              divide
              ScalarConstructor{4}
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

  EXPECT_TRUE(transform()->Run());

  EXPECT_EQ(R"(Module{
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
    __alias_TintVertexData__struct_TintVertexData
  }
  TintVertexData -> __struct_TintVertexData
  Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        Bitcast<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{_tint_pulling_vertex_buffer_0}
              Identifier{_tint_vertex_data}
            }
            Binary{
              Identifier{_tint_pulling_pos}
              divide
              ScalarConstructor{4}
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
    auto vertex_index_var = std::make_unique<ast::DecoratedVariable>(
        std::make_unique<ast::Variable>("custom_vertex_index",
                                        ast::StorageClass::kInput, &i32));

    ast::VariableDecorationList decorations;
    decorations.push_back(
        std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kVertexIdx));

    vertex_index_var->set_decorations(std::move(decorations));
    mod()->AddGlobalVariable(std::move(vertex_index_var));
  }

  {
    auto instance_index_var = std::make_unique<ast::DecoratedVariable>(
        std::make_unique<ast::Variable>("custom_instance_index",
                                        ast::StorageClass::kInput, &i32));

    ast::VariableDecorationList decorations;
    decorations.push_back(
        std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kInstanceIdx));

    instance_index_var->set_decorations(std::move(decorations));
    mod()->AddGlobalVariable(std::move(instance_index_var));
  }

  InitTransform(
      {{{4, InputStepMode::kVertex, {{VertexFormat::kF32, 0, 0}}},
        {4, InputStepMode::kInstance, {{VertexFormat::kF32, 0, 1}}}}});

  EXPECT_TRUE(transform()->Run());

  EXPECT_EQ(R"(Module{
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
    __alias_TintVertexData__struct_TintVertexData
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_1
    storage_buffer
    __alias_TintVertexData__struct_TintVertexData
  }
  TintVertexData -> __struct_TintVertexData
  Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{custom_vertex_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        Bitcast<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{_tint_pulling_vertex_buffer_0}
              Identifier{_tint_vertex_data}
            }
            Binary{
              Identifier{_tint_pulling_pos}
              divide
              ScalarConstructor{4}
            }
          }
        }
      }
      Assignment{
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{custom_instance_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_b}
        Bitcast<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{_tint_pulling_vertex_buffer_1}
              Identifier{_tint_vertex_data}
            }
            Binary{
              Identifier{_tint_pulling_pos}
              divide
              ScalarConstructor{4}
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

  EXPECT_TRUE(transform()->Run());

  EXPECT_EQ(R"(Module{
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
    __alias_TintVertexData__struct_TintVertexData
  }
  TintVertexData -> __struct_TintVertexData
  Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor{16}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        Bitcast<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{_tint_pulling_vertex_buffer_0}
              Identifier{_tint_vertex_data}
            }
            Binary{
              Identifier{_tint_pulling_pos}
              divide
              ScalarConstructor{4}
            }
          }
        }
      }
      Assignment{
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor{16}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_b}
        TypeConstructor{
          __vec_4__f32
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_0}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_0}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{4}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_0}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{8}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_0}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{12}
                }
                divide
                ScalarConstructor{4}
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

  EXPECT_TRUE(transform()->Run());

  EXPECT_EQ(R"(Module{
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
    __alias_TintVertexData__struct_TintVertexData
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_1
    storage_buffer
    __alias_TintVertexData__struct_TintVertexData
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{2}
      SetDecoration{4}
    }
    _tint_pulling_vertex_buffer_2
    storage_buffer
    __alias_TintVertexData__struct_TintVertexData
  }
  TintVertexData -> __struct_TintVertexData
  Struct{
    [[block]]
    StructMember{[[ offset 0 ]] _tint_vertex_data: __array__u32_stride_4}
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
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor{8}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        TypeConstructor{
          __vec_2__f32
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_0}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_0}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{4}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
        }
      }
      Assignment{
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor{12}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_b}
        TypeConstructor{
          __vec_3__f32
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_1}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_1}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{4}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_1}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{8}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
        }
      }
      Assignment{
        Identifier{_tint_pulling_pos}
        Binary{
          Binary{
            Identifier{_tint_pulling_vertex_index}
            multiply
            ScalarConstructor{16}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_c}
        TypeConstructor{
          __vec_4__f32
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_2}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_2}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{4}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_2}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{8}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          Bitcast<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{_tint_pulling_vertex_buffer_2}
                Identifier{_tint_vertex_data}
              }
              Binary{
                Binary{
                  Identifier{_tint_pulling_pos}
                  add
                  ScalarConstructor{12}
                }
                divide
                ScalarConstructor{4}
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
