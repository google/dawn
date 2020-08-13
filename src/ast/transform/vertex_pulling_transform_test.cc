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

#include "src/ast/transform/vertex_pulling_transform.h"

#include "gtest/gtest.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/function.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/type_determiner.h"
#include "src/validator.h"

namespace tint {
namespace ast {
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
    mod()->AddEntryPoint(std::make_unique<EntryPoint>(PipelineStage::kVertex,
                                                      "main", "vtx_main"));
    mod()->AddFunction(std::make_unique<Function>(
        "vtx_main", VariableList{},
        ctx_.type_mgr().Get(std::make_unique<type::VoidType>())));
  }

  // Set up the transformation, after building the module
  void InitTransform(VertexStateDescriptor vertex_state) {
    EXPECT_TRUE(mod_->IsValid());

    tint::TypeDeterminer td(&ctx_, mod_.get());
    EXPECT_TRUE(td.Determine());

    transform_->SetVertexState(
        std::make_unique<VertexStateDescriptor>(std::move(vertex_state)));
    transform_->SetEntryPoint("main");
  }

  // Inserts a variable which will be converted to vertex pulling
  void AddVertexInputVariable(uint32_t location,
                              std::string name,
                              type::Type* type) {
    auto var = std::make_unique<DecoratedVariable>(
        std::make_unique<Variable>(name, StorageClass::kInput, type));

    VariableDecorationList decorations;
    decorations.push_back(std::make_unique<LocationDecoration>(location));

    var->set_decorations(std::move(decorations));
    mod_->AddGlobalVariable(std::move(var));
  }

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
  EXPECT_EQ(transform()->GetError(), "SetVertexState not called");
}

TEST_F(VertexPullingTransformTest, Error_NoEntryPoint) {
  transform()->SetVertexState(std::make_unique<VertexStateDescriptor>());
  EXPECT_FALSE(transform()->Run());
  EXPECT_EQ(transform()->GetError(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, Error_InvalidEntryPoint) {
  InitBasicModule();
  InitTransform({});
  transform()->SetEntryPoint("_");

  EXPECT_FALSE(transform()->Run());
  EXPECT_EQ(transform()->GetError(), "Vertex stage entry point not found");
}

TEST_F(VertexPullingTransformTest, Error_EntryPointWrongStage) {
  InitBasicModule();
  mod()->entry_points()[0]->set_pipeline_stage(PipelineStage::kFragment);

  InitTransform({});
  EXPECT_FALSE(transform()->Run());
  EXPECT_EQ(transform()->GetError(), "Entry point is not for vertex stage");
}

TEST_F(VertexPullingTransformTest, BasicModule) {
  InitBasicModule();
  InitTransform({});
  EXPECT_TRUE(transform()->Run());
}

TEST_F(VertexPullingTransformTest, EntryPointUsingFunctionName) {
  InitBasicModule();
  mod()->entry_points()[0]->set_name("");
  InitTransform({});
  transform()->SetEntryPoint("vtx_main");
  EXPECT_TRUE(transform()->Run());
}

TEST_F(VertexPullingTransformTest, OneAttribute) {
  InitBasicModule();

  type::F32Type f32;
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
    tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_
  }
  EntryPoint{vertex as main = vtx_main}
  Function vtx_main -> __void
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_vertex_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        As<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{tint_pulling_vertex_buffer_0}
              Identifier{data}
            }
            Binary{
              Identifier{tint_pulling_pos}
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

  type::F32Type f32;
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
    tint_pulling_instance_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_
  }
  EntryPoint{vertex as main = vtx_main}
  Function vtx_main -> __void
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_instance_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        As<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{tint_pulling_vertex_buffer_0}
              Identifier{data}
            }
            Binary{
              Identifier{tint_pulling_pos}
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

  type::F32Type f32;
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
    tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{5}
    }
    tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_
  }
  EntryPoint{vertex as main = vtx_main}
  Function vtx_main -> __void
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_vertex_index}
            multiply
            ScalarConstructor{4}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        As<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{tint_pulling_vertex_buffer_0}
              Identifier{data}
            }
            Binary{
              Identifier{tint_pulling_pos}
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

  type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);
  AddVertexInputVariable(1, "var_b", &f32);

  type::I32Type i32;
  {
    auto vertex_index_var =
        std::make_unique<DecoratedVariable>(std::make_unique<Variable>(
            "custom_vertex_index", StorageClass::kInput, &i32));

    VariableDecorationList decorations;
    decorations.push_back(
        std::make_unique<BuiltinDecoration>(Builtin::kVertexIdx));

    vertex_index_var->set_decorations(std::move(decorations));
    mod()->AddGlobalVariable(std::move(vertex_index_var));
  }

  {
    auto instance_index_var =
        std::make_unique<DecoratedVariable>(std::make_unique<Variable>(
            "custom_instance_index", StorageClass::kInput, &i32));

    VariableDecorationList decorations;
    decorations.push_back(
        std::make_unique<BuiltinDecoration>(Builtin::kInstanceIdx));

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
    tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    tint_pulling_vertex_buffer_1
    storage_buffer
    __struct_
  }
  EntryPoint{vertex as main = vtx_main}
  Function vtx_main -> __void
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
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
        As<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{tint_pulling_vertex_buffer_0}
              Identifier{data}
            }
            Binary{
              Identifier{tint_pulling_pos}
              divide
              ScalarConstructor{4}
            }
          }
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
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
        As<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{tint_pulling_vertex_buffer_1}
              Identifier{data}
            }
            Binary{
              Identifier{tint_pulling_pos}
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

  type::F32Type f32;
  AddVertexInputVariable(0, "var_a", &f32);

  type::ArrayType vec4_f32{&f32, 4u};
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
    tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_
  }
  EntryPoint{vertex as main = vtx_main}
  Function vtx_main -> __void
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_vertex_index}
            multiply
            ScalarConstructor{16}
          }
          add
          ScalarConstructor{0}
        }
      }
      Assignment{
        Identifier{var_a}
        As<__f32>{
          ArrayAccessor{
            MemberAccessor{
              Identifier{tint_pulling_vertex_buffer_0}
              Identifier{data}
            }
            Binary{
              Identifier{tint_pulling_pos}
              divide
              ScalarConstructor{4}
            }
          }
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_vertex_index}
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
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_0}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_0}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{4}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_0}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{8}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_0}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
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

  type::F32Type f32;
  type::ArrayType vec2_f32{&f32, 2u};
  AddVertexInputVariable(0, "var_a", &vec2_f32);

  type::ArrayType vec3_f32{&f32, 3u};
  AddVertexInputVariable(1, "var_b", &vec3_f32);

  type::ArrayType vec4_f32{&f32, 4u};
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
    tint_pulling_vertex_index
    in
    __i32
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{0}
      SetDecoration{4}
    }
    tint_pulling_vertex_buffer_0
    storage_buffer
    __struct_
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{1}
      SetDecoration{4}
    }
    tint_pulling_vertex_buffer_1
    storage_buffer
    __struct_
  }
  DecoratedVariable{
    Decorations{
      BindingDecoration{2}
      SetDecoration{4}
    }
    tint_pulling_vertex_buffer_2
    storage_buffer
    __struct_
  }
  EntryPoint{vertex as main = vtx_main}
  Function vtx_main -> __void
  ()
  {
    Block{
      VariableDeclStatement{
        Variable{
          tint_pulling_pos
          function
          __i32
        }
      }
      Assignment{
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_vertex_index}
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
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_0}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_0}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
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
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_vertex_index}
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
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_1}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_1}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{4}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_1}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
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
        Identifier{tint_pulling_pos}
        Binary{
          Binary{
            Identifier{tint_pulling_vertex_index}
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
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_2}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{0}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_2}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{4}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_2}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
                  add
                  ScalarConstructor{8}
                }
                divide
                ScalarConstructor{4}
              }
            }
          }
          As<__f32>{
            ArrayAccessor{
              MemberAccessor{
                Identifier{tint_pulling_vertex_buffer_2}
                Identifier{data}
              }
              Binary{
                Binary{
                  Identifier{tint_pulling_pos}
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
}  // namespace ast
}  // namespace tint
