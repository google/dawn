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

#include "src/inspector/inspector.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/null_literal.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/variable_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type_determiner.h"
#include "tint/tint.h"

namespace tint {
namespace inspector {
namespace {

class InspectorHelper {
 public:
  InspectorHelper()
      : td_(std::make_unique<TypeDeterminer>(&mod_)),
        inspector_(std::make_unique<Inspector>(mod_)),
        sampler_type_(ast::type::SamplerKind::kSampler),
        comparison_sampler_type_(ast::type::SamplerKind::kComparisonSampler) {}

  /// Generates an empty function
  /// @param name name of the function created
  /// @param decorations the function decorations
  /// @returns a function object
  ast::Function* MakeEmptyBodyFunction(
      std::string name,
      ast::FunctionDecorationList decorations = {}) {
    auto* body = create<ast::BlockStatement>();
    body->append(create<ast::ReturnStatement>(Source{}));
    return create<ast::Function>(Source{}, mod()->RegisterSymbol(name), name,
                                 ast::VariableList(), void_type(), body,
                                 decorations);
  }

  /// Generates a function that calls another
  /// @param caller name of the function created
  /// @param callee name of the function to be called
  /// @param decorations the function decorations
  /// @returns a function object
  ast::Function* MakeCallerBodyFunction(
      std::string caller,
      std::string callee,
      ast::FunctionDecorationList decorations = {}) {
    auto* body = create<ast::BlockStatement>();
    auto* ident_expr = create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(callee), callee);
    auto* call_expr = create<ast::CallExpression>(Source{}, ident_expr,
                                                  ast::ExpressionList());
    body->append(create<ast::CallStatement>(call_expr));
    body->append(create<ast::ReturnStatement>(Source{}));
    return create<ast::Function>(Source{}, mod()->RegisterSymbol(caller),
                                 caller, ast::VariableList(), void_type(), body,
                                 decorations);
  }

  /// Add In/Out variables to the global variables
  /// @param inout_vars tuples of {in, out} that will be added as entries to the
  ///                   global variables
  void AddInOutVariables(
      std::vector<std::tuple<std::string, std::string>> inout_vars) {
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      auto* in_var =
          create<ast::Variable>(Source{},                   // source
                                in,                         // name
                                ast::StorageClass::kInput,  // storage_class
                                u32_type(),                 // type
                                false,                      // is_const
                                nullptr,                    // constructor
                                ast::VariableDecorationList{});  // decorations
      auto* out_var =
          create<ast::Variable>(Source{},                    // source
                                out,                         // name
                                ast::StorageClass::kOutput,  // storage_class
                                u32_type(),                  // type
                                false,                       // is_const
                                nullptr,                     // constructor
                                ast::VariableDecorationList{});  // decorations
      mod()->AddGlobalVariable(in_var);
      mod()->AddGlobalVariable(out_var);
    }
  }

  /// Generates a function that references in/out variables
  /// @param name name of the function created
  /// @param inout_vars tuples of {in, out} that will be converted into out = in
  ///                   calls in the function body
  /// @param decorations the function decorations
  /// @returns a function object
  ast::Function* MakeInOutVariableBodyFunction(
      std::string name,
      std::vector<std::tuple<std::string, std::string>> inout_vars,
      ast::FunctionDecorationList decorations = {}) {
    auto* body = create<ast::BlockStatement>();
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      body->append(create<ast::AssignmentStatement>(
          create<ast::IdentifierExpression>(Source{},
                                            mod()->RegisterSymbol(out), out),
          create<ast::IdentifierExpression>(Source{}, mod()->RegisterSymbol(in),
                                            in)));
    }
    body->append(create<ast::ReturnStatement>(Source{}));
    return create<ast::Function>(Source{}, mod()->RegisterSymbol(name), name,
                                 ast::VariableList(), void_type(), body,
                                 decorations);
  }

  /// Generates a function that references in/out variables and calls another
  /// function.
  /// @param caller name of the function created
  /// @param callee name of the function to be called
  /// @param inout_vars tuples of {in, out} that will be converted into out = in
  ///                   calls in the function body
  /// @param decorations the function decorations
  /// @returns a function object
  ast::Function* MakeInOutVariableCallerBodyFunction(
      std::string caller,
      std::string callee,
      std::vector<std::tuple<std::string, std::string>> inout_vars,
      ast::FunctionDecorationList decorations = {}) {
    auto* body = create<ast::BlockStatement>();
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      body->append(create<ast::AssignmentStatement>(
          create<ast::IdentifierExpression>(Source{},
                                            mod()->RegisterSymbol(out), out),
          create<ast::IdentifierExpression>(Source{}, mod()->RegisterSymbol(in),
                                            in)));
    }
    auto* ident_expr = create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(callee), callee);
    auto* call_expr = create<ast::CallExpression>(Source{}, ident_expr,
                                                  ast::ExpressionList());
    body->append(create<ast::CallStatement>(call_expr));
    body->append(create<ast::ReturnStatement>(Source{}));
    return create<ast::Function>(Source{}, mod()->RegisterSymbol(caller),
                                 caller, ast::VariableList(), void_type(), body,
                                 decorations);
  }

  /// Add a Constant ID to the global variables.
  /// @param name name of the variable to add
  /// @param id id number for the constant id
  /// @param type type of the variable
  /// @param val value to initialize the variable with, if NULL no initializer
  ///            will be added.
  template <class T>
  void AddConstantID(std::string name,
                     uint32_t id,
                     ast::type::Type* type,
                     T* val) {
    ast::Expression* constructor = nullptr;
    if (val) {
      constructor = create<ast::ScalarConstructorExpression>(
          Source{}, MakeLiteral(type, val));
    }
    auto* var = create<ast::Variable>(
        Source{},                  // source
        name,                      // name
        ast::StorageClass::kNone,  // storage_class
        type,                      // type
        true,                      // is_const
        constructor,               // constructor
        ast::VariableDecorationList{
            // decorations
            create<ast::ConstantIdDecoration>(id, Source{}),
        });
    mod()->AddGlobalVariable(var);
  }

  /// @param type AST type of the literal, must resolve to BoolLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(ast::type::Type* type, bool* val) {
    return create<ast::BoolLiteral>(Source{}, type, *val);
  }

  /// @param type AST type of the literal, must resolve to UIntLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(ast::type::Type* type, uint32_t* val) {
    return create<ast::UintLiteral>(Source{}, type, *val);
  }

  /// @param type AST type of the literal, must resolve to IntLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(ast::type::Type* type, int32_t* val) {
    return create<ast::SintLiteral>(Source{}, type, *val);
  }

  /// @param type AST type of the literal, must resolve to FloattLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(ast::type::Type* type, float* val) {
    return create<ast::FloatLiteral>(Source{}, type, *val);
  }

  /// @param vec Vector of strings to be searched
  /// @param str String to be searching for
  /// @returns true if str is in vec, otherwise false
  bool ContainsString(const std::vector<std::string>& vec,
                      const std::string& str) {
    for (auto& s : vec) {
      if (s == str) {
        return true;
      }
    }
    return false;
  }

  /// Builds a string for accessing a member in a generated struct
  /// @param idx index of member
  /// @param type type of member
  /// @returns a string for the member
  std::string StructMemberName(size_t idx, ast::type::Type* type) {
    return std::to_string(idx) + type->type_name();
  }

  /// Generates a struct type
  /// @param name name for the type
  /// @param members_info a vector of {type, offset} where each entry is the
  ///                     type and offset of a member of the struct
  /// @param is_block whether or not to decorate as a Block
  /// @returns a struct type
  std::unique_ptr<ast::type::Struct> MakeStructType(
      const std::string& name,
      std::vector<std::tuple<ast::type::Type*, uint32_t>> members_info,
      bool is_block) {
    ast::StructMemberList members;
    for (auto& member_info : members_info) {
      ast::type::Type* type;
      uint32_t offset;
      std::tie(type, offset) = member_info;

      ast::StructMemberDecorationList deco;
      deco.push_back(
          create<ast::StructMemberOffsetDecoration>(offset, Source{}));

      members.push_back(create<ast::StructMember>(
          StructMemberName(members.size(), type), type, deco));
    }

    ast::StructDecorationList decos;
    if (is_block) {
      decos.push_back(create<ast::StructBlockDecoration>(Source{}));
    }

    auto* str = create<ast::Struct>(decos, members);

    return std::make_unique<ast::type::Struct>(name, str);
  }

  /// Generates types appropriate for using in an uniform buffer
  /// @param name name for the type
  /// @param members_info a vector of {type, offset} where each entry is the
  ///                     type and offset of a member of the struct
  /// @returns a tuple {struct type, access control type}, where the struct has
  ///          the layout for an uniform buffer, and the control type wraps the
  ///          struct.
  std::tuple<std::unique_ptr<ast::type::Struct>,
             std::unique_ptr<ast::type::AccessControl>>
  MakeUniformBufferTypes(
      const std::string& name,
      std::vector<std::tuple<ast::type::Type*, uint32_t>> members_info) {
    auto struct_type = MakeStructType(name, members_info, true);
    auto access_type = std::make_unique<ast::type::AccessControl>(
        ast::AccessControl::kReadOnly, struct_type.get());
    return {std::move(struct_type), std::move(access_type)};
  }

  /// Generates types appropriate for using in a storage buffer
  /// @param name name for the type
  /// @param members_info a vector of {type, offset} where each entry is the
  ///                     type and offset of a member of the struct
  /// @returns a tuple {struct type, access control type}, where the struct has
  ///          the layout for a storage buffer, and the control type wraps the
  ///          struct.
  std::tuple<std::unique_ptr<ast::type::Struct>,
             std::unique_ptr<ast::type::AccessControl>>
  MakeStorageBufferTypes(
      const std::string& name,
      std::vector<std::tuple<ast::type::Type*, uint32_t>> members_info) {
    auto struct_type = MakeStructType(name, members_info, false);
    auto access_type = std::make_unique<ast::type::AccessControl>(
        ast::AccessControl::kReadWrite, struct_type.get());
    return {std::move(struct_type), std::move(access_type)};
  }

  /// Generates types appropriate for using in a read-only storage buffer
  /// @param name name for the type
  /// @param members_info a vector of {type, offset} where each entry is the
  ///                     type and offset of a member of the struct
  /// @returns a tuple {struct type, access control type}, where the struct has
  ///          the layout for a read-only storage buffer, and the control type
  ///          wraps the struct.
  std::tuple<std::unique_ptr<ast::type::Struct>,
             std::unique_ptr<ast::type::AccessControl>>
  MakeReadOnlyStorageBufferTypes(
      const std::string& name,
      std::vector<std::tuple<ast::type::Type*, uint32_t>> members_info) {
    auto struct_type = MakeStructType(name, members_info, false);
    auto access_type = std::make_unique<ast::type::AccessControl>(
        ast::AccessControl::kReadOnly, struct_type.get());
    return {std::move(struct_type), std::move(access_type)};
  }

  /// Adds a binding variable with a struct type to the module
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param storage_class the storage class to use
  /// @param set the binding group/set to use for the uniform buffer
  /// @param binding the binding number to use for the uniform buffer
  void AddBinding(const std::string& name,
                  ast::type::Type* type,
                  ast::StorageClass storage_class,
                  uint32_t set,
                  uint32_t binding) {
    auto* var = create<ast::Variable>(
        Source{},       // source
        name,           // name
        storage_class,  // storage_class
        type,           // type
        false,          // is_const
        nullptr,        // constructor
        ast::VariableDecorationList{
            // decorations
            create<ast::BindingDecoration>(binding, Source{}),
            create<ast::SetDecoration>(set, Source{}),
        });

    mod()->AddGlobalVariable(var);
  }

  /// Adds an uniform buffer variable to the module
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param set the binding group/set to use for the uniform buffer
  /// @param binding the binding number to use for the uniform buffer
  void AddUniformBuffer(const std::string& name,
                        ast::type::Type* type,
                        uint32_t set,
                        uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniform, set, binding);
  }

  /// Adds a storage buffer variable to the module
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param set the binding group/set to use for the storage buffer
  /// @param binding the binding number to use for the storage buffer
  void AddStorageBuffer(const std::string& name,
                        ast::type::Type* type,
                        uint32_t set,
                        uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kStorageBuffer, set, binding);
  }

  /// Generates a function that references a specific struct variable
  /// @param func_name name of the function created
  /// @param struct_name name of the struct variabler to be accessed
  /// @param members list of members to access, by index and type
  /// @returns a function that references all of the members specified
  ast::Function* MakeStructVariableReferenceBodyFunction(
      std::string func_name,
      std::string struct_name,
      std::vector<std::tuple<size_t, ast::type::Type*>> members) {
    auto* body = create<ast::BlockStatement>();

    for (auto member : members) {
      size_t member_idx;
      ast::type::Type* member_type;
      std::tie(member_idx, member_type) = member;
      std::string member_name = StructMemberName(member_idx, member_type);
      body->append(create<ast::VariableDeclStatement>(create<ast::Variable>(
          Source{},                          // source
          "local" + member_name,             // name
          ast::StorageClass::kNone,          // storage_class
          member_type,                       // type
          false,                             // is_const
          nullptr,                           // constructor
          ast::VariableDecorationList{})));  // decorations
    }

    for (auto member : members) {
      size_t member_idx;
      ast::type::Type* member_type;
      std::tie(member_idx, member_type) = member;
      std::string member_name = StructMemberName(member_idx, member_type);
      body->append(create<ast::AssignmentStatement>(
          create<ast::IdentifierExpression>(
              Source{}, mod()->RegisterSymbol("local" + member_name),
              "local" + member_name),
          create<ast::MemberAccessorExpression>(Source{},
              create<ast::IdentifierExpression>(
                  Source{}, mod()->RegisterSymbol(struct_name), struct_name),
              create<ast::IdentifierExpression>(
                  Source{}, mod()->RegisterSymbol(member_name), member_name))));
    }

    body->append(create<ast::ReturnStatement>(Source{}));
    return create<ast::Function>(Source{}, mod()->RegisterSymbol(func_name),
                                 func_name, ast::VariableList(), void_type(),
                                 body, ast::FunctionDecorationList{});
  }

  /// Adds a regular sampler variable to the module
  /// @param name the name of the variable
  /// @param set the binding group/set to use for the storage buffer
  /// @param binding the binding number to use for the storage buffer
  void AddSampler(const std::string& name, uint32_t set, uint32_t binding) {
    AddBinding(name, sampler_type(), ast::StorageClass::kUniformConstant, set,
               binding);
  }

  /// Adds a comparison sampler variable to the module
  /// @param name the name of the variable
  /// @param set the binding group/set to use for the storage buffer
  /// @param binding the binding number to use for the storage buffer
  void AddComparisonSampler(const std::string& name,
                            uint32_t set,
                            uint32_t binding) {
    AddBinding(name, comparison_sampler_type(),
               ast::StorageClass::kUniformConstant, set, binding);
  }

  /// Generates a SampledTexture appropriate for the params
  /// @param dim the dimensions of the texture
  /// @param type the data type of the sampled texture
  /// @returns the generated SampleTextureType
  std::unique_ptr<ast::type::SampledTexture> MakeSampledTextureType(
      ast::type::TextureDimension dim,
      ast::type::Type* type) {
    return std::make_unique<ast::type::SampledTexture>(dim, type);
  }

  /// Generates a DepthTexture appropriate for the params
  /// @param dim the dimensions of the texture
  /// @returns the generated DepthTexture
  std::unique_ptr<ast::type::DepthTexture> MakeDepthTextureType(
      ast::type::TextureDimension dim) {
    return std::make_unique<ast::type::DepthTexture>(dim);
  }

  /// Generates a MultisampledTexture appropriate for the params
  /// @param dim the dimensions of the texture
  /// @param type the data type of the sampled texture
  /// @returns the generated SampleTextureType
  std::unique_ptr<ast::type::MultisampledTexture> MakeMultisampledTextureType(
      ast::type::TextureDimension dim,
      ast::type::Type* type) {
    return std::make_unique<ast::type::MultisampledTexture>(dim, type);
  }

  /// Adds a sampled texture variable to the module
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param set the binding group/set to use for the sampled texture
  /// @param binding the binding number to use for the sampled texture
  void AddSampledTexture(const std::string& name,
                         ast::type::Type* type,
                         uint32_t set,
                         uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniformConstant, set, binding);
  }

  /// Adds a multi-sampled texture variable to the module
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param set the binding group/set to use for the multi-sampled texture
  /// @param binding the binding number to use for the multi-sampled texture
  void AddMultisampledTexture(const std::string& name,
                              ast::type::Type* type,
                              uint32_t set,
                              uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniformConstant, set, binding);
  }

  void AddGlobalVariable(const std::string& name, ast::type::Type* type) {
    mod()->AddGlobalVariable(create<ast::Variable>(
        Source{},                             // source
        name,                                 // name
        ast::StorageClass::kUniformConstant,  // storage_class
        type,                                 // type
        false,                                // is_const
        nullptr,                              // constructor
        ast::VariableDecorationList{}));      // decorations
  }

  /// Adds a depth texture variable to the module
  /// @param name the name of the variable
  /// @param type the type to use
  void AddDepthTexture(const std::string& name, ast::type::Type* type) {
    mod()->AddGlobalVariable(create<ast::Variable>(
        Source{},                             // source
        name,                                 // name
        ast::StorageClass::kUniformConstant,  // storage_class
        type,                                 // type
        false,                                // is_const
        nullptr,                              // constructor
        ast::VariableDecorationList{}));      // decorations
  }

  /// Generates a function that references a specific sampler variable
  /// @param func_name name of the function created
  /// @param texture_name name of the texture to be sampled
  /// @param sampler_name name of the sampler to use
  /// @param coords_name name of the coords variable to use
  /// @param base_type sampler base type
  /// @param decorations the function decorations
  /// @returns a function that references all of the values specified
  ast::Function* MakeSamplerReferenceBodyFunction(
      const std::string& func_name,
      const std::string& texture_name,
      const std::string& sampler_name,
      const std::string& coords_name,
      ast::type::Type* base_type,
      ast::FunctionDecorationList decorations = {}) {
    std::string result_name = "sampler_result";

    auto* body = create<ast::BlockStatement>();

    auto* call_result =
        create<ast::Variable>(Source{},                        // source
                              "sampler_result",                // name
                              ast::StorageClass::kFunction,    // storage_class
                              vec_type(base_type, 4),          // type
                              false,                           // is_const
                              nullptr,                         // constructor
                              ast::VariableDecorationList{});  // decorations
    body->append(create<ast::VariableDeclStatement>(call_result));

    ast::ExpressionList call_params;
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(texture_name), texture_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(sampler_name), sampler_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(coords_name), coords_name));
    auto* call_expr = create<ast::CallExpression>(
        Source{},
        create<ast::IdentifierExpression>(
            Source{}, mod()->RegisterSymbol("textureSample"), "textureSample"),
        call_params);

    body->append(create<ast::AssignmentStatement>(
        create<ast::IdentifierExpression>(
            Source{}, mod()->RegisterSymbol("sampler_result"),
            "sampler_result"),
        call_expr));
    body->append(create<ast::ReturnStatement>(Source{}));

    return create<ast::Function>(Source{}, mod()->RegisterSymbol(func_name),
                                 func_name, ast::VariableList(), void_type(),
                                 body, decorations);
  }

  /// Generates a function that references a specific sampler variable
  /// @param func_name name of the function created
  /// @param texture_name name of the texture to be sampled
  /// @param sampler_name name of the sampler to use
  /// @param coords_name name of the coords variable to use
  /// @param array_index name of the array index variable to use
  /// @param base_type sampler base type
  /// @param decorations the function decorations
  /// @returns a function that references all of the values specified
  ast::Function* MakeSamplerReferenceBodyFunction(
      const std::string& func_name,
      const std::string& texture_name,
      const std::string& sampler_name,
      const std::string& coords_name,
      const std::string& array_index,
      ast::type::Type* base_type,
      ast::FunctionDecorationList decorations = {}) {
    std::string result_name = "sampler_result";

    auto* body = create<ast::BlockStatement>();

    auto* call_result =
        create<ast::Variable>(Source{},                        // source
                              "sampler_result",                // name
                              ast::StorageClass::kFunction,    // storage_class
                              vec_type(base_type, 4),          // type
                              false,                           // is_const
                              nullptr,                         // constructor
                              ast::VariableDecorationList{});  // decorations
    body->append(create<ast::VariableDeclStatement>(call_result));

    ast::ExpressionList call_params;
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(texture_name), texture_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(sampler_name), sampler_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(coords_name), coords_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(array_index), array_index));
    auto* call_expr = create<ast::CallExpression>(
        Source{},
        create<ast::IdentifierExpression>(
            Source{}, mod()->RegisterSymbol("textureSample"), "textureSample"),
        call_params);

    body->append(create<ast::AssignmentStatement>(
        create<ast::IdentifierExpression>(
            Source{}, mod()->RegisterSymbol("sampler_result"),
            "sampler_result"),
        call_expr));
    body->append(create<ast::ReturnStatement>(Source{}));

    return create<ast::Function>(Source{}, mod()->RegisterSymbol(func_name),
                                 func_name, ast::VariableList(), void_type(),
                                 body, decorations);
  }

  /// Generates a function that references a specific comparison sampler
  /// variable.
  /// @param func_name name of the function created
  /// @param texture_name name of the depth texture to  use
  /// @param sampler_name name of the sampler to use
  /// @param coords_name name of the coords variable to use
  /// @param depth_name name of the depth reference to use
  /// @param base_type sampler base type
  /// @param decorations the function decorations
  /// @returns a function that references all of the values specified
  ast::Function* MakeComparisonSamplerReferenceBodyFunction(
      const std::string& func_name,
      const std::string& texture_name,
      const std::string& sampler_name,
      const std::string& coords_name,
      const std::string& depth_name,
      ast::type::Type* base_type,
      ast::FunctionDecorationList decorations = {}) {
    std::string result_name = "sampler_result";

    auto* body = create<ast::BlockStatement>();

    auto* call_result =
        create<ast::Variable>(Source{},                        // source
                              "sampler_result",                // name
                              ast::StorageClass::kFunction,    // storage_class
                              base_type,                       // type
                              false,                           // is_const
                              nullptr,                         // constructor
                              ast::VariableDecorationList{});  // decorations
    body->append(create<ast::VariableDeclStatement>(call_result));

    ast::ExpressionList call_params;
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(texture_name), texture_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(sampler_name), sampler_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(coords_name), coords_name));
    call_params.push_back(create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(depth_name), depth_name));
    auto* call_expr = create<ast::CallExpression>(
        Source{},
        create<ast::IdentifierExpression>(
            Source{}, mod()->RegisterSymbol("textureSampleCompare"),
            "textureSampleCompare"),
        call_params);

    body->append(create<ast::AssignmentStatement>(
        create<ast::IdentifierExpression>(
            Source{}, mod()->RegisterSymbol("sampler_result"),
            "sampler_result"),
        call_expr));
    body->append(create<ast::ReturnStatement>(Source{}));

    return create<ast::Function>(Source{}, mod()->RegisterSymbol(func_name),
                                 func_name, ast::VariableList(), void_type(),
                                 body, decorations);
  }

  /// Gets an appropriate type for the data in a given texture type.
  /// @param sampled_kind type of in the texture
  /// @returns a pointer to a type appropriate for the coord param
  ast::type::Type* GetBaseType(ResourceBinding::SampledKind sampled_kind) {
    switch (sampled_kind) {
      case ResourceBinding::SampledKind::kFloat:
        return f32_type();
      case ResourceBinding::SampledKind::kSInt:
        return i32_type();
      case ResourceBinding::SampledKind::kUInt:
        return u32_type();
      default:
        return nullptr;
    }
  }

  /// Gets an appropriate type for the coords parameter depending the the
  /// dimensionality of the texture being sampled.
  /// @param dim dimensionality of the texture being sampled
  /// @param sampled_kind type of data in the texture
  /// @returns a pointer to a type appropriate for the coord param
  ast::type::Type* GetCoordsType(ast::type::TextureDimension dim,
                                 ResourceBinding::SampledKind sampled_kind) {
    ast::type::Type* base_type = GetBaseType(sampled_kind);
    if (dim == ast::type::TextureDimension::k1d) {
      return base_type;
    } else if (dim == ast::type::TextureDimension::k1dArray ||
               dim == ast::type::TextureDimension::k2d) {
      return vec_type(base_type, 2);
    } else if (dim == ast::type::TextureDimension::kCubeArray) {
      return vec_type(base_type, 4);
    }
    return vec_type(base_type, 3);
  }

  ast::Module* mod() { return &mod_; }
  TypeDeterminer* td() { return td_.get(); }
  Inspector* inspector() { return inspector_.get(); }

  ast::type::Bool* bool_type() { return &bool_type_; }
  ast::type::F32* f32_type() { return &f32_type_; }
  ast::type::I32* i32_type() { return &i32_type_; }
  ast::type::U32* u32_type() { return &u32_type_; }
  ast::type::Array* u32_array_type(uint32_t count) {
    if (array_type_memo_.find(count) == array_type_memo_.end()) {
      array_type_memo_[count] = create<ast::type::Array>(
          u32_type(), count,
          ast::ArrayDecorationList{
              create<ast::StrideDecoration>(4, Source{}),
          });
    }
    return array_type_memo_[count];
  }
  ast::type::Vector* vec_type(ast::type::Type* type, uint32_t count) {
    if (vector_type_memo_.find(std::tie(type, count)) ==
        vector_type_memo_.end()) {
      vector_type_memo_[std::tie(type, count)] =
          std::make_unique<ast::type::Vector>(u32_type(), count);
    }
    return vector_type_memo_[std::tie(type, count)].get();
  }
  ast::type::Void* void_type() { return &void_type_; }
  ast::type::Sampler* sampler_type() { return &sampler_type_; }
  ast::type::Sampler* comparison_sampler_type() {
    return &comparison_sampler_type_;
  }

  /// Creates a new `ast::Node` owned by the Module. When the Module is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return mod_.create<T>(std::forward<ARGS>(args)...);
  }

 private:
  ast::Module mod_;
  std::unique_ptr<TypeDeterminer> td_;
  std::unique_ptr<Inspector> inspector_;

  ast::type::Bool bool_type_;
  ast::type::F32 f32_type_;
  ast::type::I32 i32_type_;
  ast::type::U32 u32_type_;
  ast::type::Void void_type_;
  ast::type::Sampler sampler_type_;
  ast::type::Sampler comparison_sampler_type_;
  std::map<uint32_t, ast::type::Array*> array_type_memo_;
  std::map<std::tuple<ast::type::Type*, uint32_t>,
           std::unique_ptr<ast::type::Vector>>
      vector_type_memo_;
};

class InspectorGetEntryPointTest : public InspectorHelper,
                                   public testing::Test {};
class InspectorGetRemappedNameForEntryPointTest : public InspectorHelper,
                                                  public testing::Test {};
class InspectorGetConstantIDsTest : public InspectorHelper,
                                    public testing::Test {};
class InspectorGetUniformBufferResourceBindingsTest : public InspectorHelper,
                                                      public testing::Test {};
class InspectorGetStorageBufferResourceBindingsTest : public InspectorHelper,
                                                      public testing::Test {};
class InspectorGetReadOnlyStorageBufferResourceBindingsTest
    : public InspectorHelper,
      public testing::Test {};
class InspectorGetSamplerResourceBindingsTest : public InspectorHelper,
                                                public testing::Test {};
class InspectorGetComparisonSamplerResourceBindingsTest
    : public InspectorHelper,
      public testing::Test {};
class InspectorGetSampledTextureResourceBindingsTest : public InspectorHelper,
                                                       public testing::Test {};
class InspectorGetSampledArrayTextureResourceBindingsTest
    : public InspectorHelper,
      public testing::Test {};
struct GetSampledTextureTestParams {
  ast::type::TextureDimension type_dim;
  inspector::ResourceBinding::TextureDimension inspector_dim;
  inspector::ResourceBinding::SampledKind sampled_kind;
};
class InspectorGetSampledTextureResourceBindingsTestWithParam
    : public InspectorHelper,
      public testing::TestWithParam<GetSampledTextureTestParams> {};
class InspectorGetSampledArrayTextureResourceBindingsTestWithParam
    : public InspectorHelper,
      public testing::TestWithParam<GetSampledTextureTestParams> {};
class InspectorGetMultisampledTextureResourceBindingsTest
    : public InspectorHelper,
      public testing::Test {};
class InspectorGetMultisampledArrayTextureResourceBindingsTest
    : public InspectorHelper,
      public testing::Test {};
typedef GetSampledTextureTestParams GetMultisampledTextureTestParams;
class InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam
    : public InspectorHelper,
      public testing::TestWithParam<GetMultisampledTextureTestParams> {};
class InspectorGetMultisampledTextureResourceBindingsTestWithParam
    : public InspectorHelper,
      public testing::TestWithParam<GetMultisampledTextureTestParams> {};

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
  mod()->AddFunction(MakeEmptyBodyFunction("foo"));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
  auto* foo = MakeEmptyBodyFunction(
      "foo",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  // TODO(dsinclair): Update to run the namer transform when available.

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPoints) {
  auto* foo = MakeEmptyBodyFunction(
      "foo",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto* bar = MakeEmptyBodyFunction(
      "bar",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });
  mod()->AddFunction(bar);

  // TODO(dsinclair): Update to run the namer transform when available.

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ("bar", result[1].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kCompute, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, MixFunctionsAndEntryPoints) {
  auto* func = MakeEmptyBodyFunction("func");
  mod()->AddFunction(func);

  auto* foo = MakeCallerBodyFunction(
      "foo", "func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto* bar = MakeCallerBodyFunction(
      "bar", "func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });
  mod()->AddFunction(bar);

  // TODO(dsinclair): Update to run the namer transform when available.

  auto result = inspector()->GetEntryPoints();
  EXPECT_FALSE(inspector()->has_error());

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ("bar", result[1].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kFragment, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, DefaultWorkgroupSize) {
  auto* foo = MakeCallerBodyFunction(
      "foo", "func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(1u, x);
  EXPECT_EQ(1u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NonDefaultWorkgroupSize) {
  auto* foo = MakeEmptyBodyFunction(
      "foo",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
          create<ast::WorkgroupDecoration>(8u, 2u, 1u, Source{}),
      });
  mod()->AddFunction(foo);

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(8u, x);
  EXPECT_EQ(2u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NoInOutVariables) {
  auto* func = MakeEmptyBodyFunction("func");
  mod()->AddFunction(func);

  auto* foo = MakeCallerBodyFunction(
      "foo", "func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].input_variables.size());
  EXPECT_EQ(0u, result[0].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, EntryPointInOutVariables) {
  AddInOutVariables({{"in_var", "out_var"}});

  auto* foo = MakeInOutVariableBodyFunction(
      "foo", {{"in_var", "out_var"}},
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, FunctionInOutVariables) {
  AddInOutVariables({{"in_var", "out_var"}});

  auto* func = MakeInOutVariableBodyFunction("func", {{"in_var", "out_var"}});
  mod()->AddFunction(func);

  auto* foo = MakeCallerBodyFunction(
      "foo", "func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, RepeatedInOutVariables) {
  AddInOutVariables({{"in_var", "out_var"}});

  auto* func = MakeInOutVariableBodyFunction("func", {{"in_var", "out_var"}});
  mod()->AddFunction(func);

  auto* foo = MakeInOutVariableCallerBodyFunction(
      "foo", "func", {{"in_var", "out_var"}},
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, EntryPointMultipleInOutVariables) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto* foo = MakeInOutVariableBodyFunction(
      "foo", {{"in_var", "out_var"}, {"in2_var", "out2_var"}},
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in2_var"));
  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out2_var"));
}

TEST_F(InspectorGetEntryPointTest, FunctionMultipleInOutVariables) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto* func = MakeInOutVariableBodyFunction(
      "func", {{"in_var", "out_var"}, {"in2_var", "out2_var"}});
  mod()->AddFunction(func);

  auto* foo = MakeCallerBodyFunction(
      "foo", "func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in2_var"));
  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out2_var"));
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutVariables) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto* foo = MakeInOutVariableBodyFunction(
      "foo", {{"in_var", "out2_var"}},
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto* bar = MakeInOutVariableBodyFunction(
      "bar", {{"in2_var", "out_var"}},
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });
  mod()->AddFunction(bar);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  // TODO(dsinclair): Update to run the namer transform when available.

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ("foo", result[0].name);
  ASSERT_EQ("foo", result[0].remapped_name);
  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out2_var", result[0].output_variables[0]);

  ASSERT_EQ("bar", result[1].name);
  ASSERT_EQ("bar", result[1].remapped_name);
  ASSERT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in2_var", result[1].input_variables[0]);
  ASSERT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("out_var", result[1].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsSharedInOutVariables) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto* func = MakeInOutVariableBodyFunction("func", {{"in2_var", "out2_var"}});
  mod()->AddFunction(func);

  auto* foo = MakeInOutVariableCallerBodyFunction(
      "foo", "func", {{"in_var", "out_var"}},
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto* bar = MakeCallerBodyFunction(
      "bar", "func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });
  mod()->AddFunction(bar);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  // TODO(dsinclair): Update to run the namer transform when available.

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ("foo", result[0].name);
  ASSERT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in2_var"));
  EXPECT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out2_var"));

  ASSERT_EQ("bar", result[1].name);
  ASSERT_EQ("bar", result[1].remapped_name);
  EXPECT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in2_var", result[1].input_variables[0]);
  EXPECT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("out2_var", result[1].output_variables[0]);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_NoFunctions) {
  auto result = inspector()->GetRemappedNameForEntryPoint("foo");
  ASSERT_TRUE(inspector()->has_error());

  EXPECT_EQ("", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_NoEntryPoints) {
  mod()->AddFunction(MakeEmptyBodyFunction("foo"));

  auto result = inspector()->GetRemappedNameForEntryPoint("foo");
  ASSERT_TRUE(inspector()->has_error());

  EXPECT_EQ("", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_OneEntryPoint) {
  auto* foo = MakeEmptyBodyFunction(
      "foo",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  // TODO(dsinclair): Update to run the namer transform when available.

  auto result = inspector()->GetRemappedNameForEntryPoint("foo");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  EXPECT_EQ("foo", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest,
       DISABLED_MultipleEntryPoints) {
  auto* foo = MakeEmptyBodyFunction(
      "foo",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  // TODO(dsinclair): Update to run the namer transform when available.

  auto* bar = MakeEmptyBodyFunction(
      "bar",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });
  mod()->AddFunction(bar);

  {
    auto result = inspector()->GetRemappedNameForEntryPoint("foo");
    ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
    EXPECT_EQ("foo", result);
  }
  {
    auto result = inspector()->GetRemappedNameForEntryPoint("bar");
    ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
    EXPECT_EQ("bar", result);
  }
}

TEST_F(InspectorGetConstantIDsTest, Bool) {
  bool val_true = true;
  bool val_false = false;
  AddConstantID<bool>("foo", 1, bool_type(), nullptr);
  AddConstantID<bool>("bar", 20, bool_type(), &val_true);
  AddConstantID<bool>("baz", 300, bool_type(), &val_false);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(3u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsBool());
  EXPECT_TRUE(result[20].AsBool());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsBool());
  EXPECT_FALSE(result[300].AsBool());
}

TEST_F(InspectorGetConstantIDsTest, U32) {
  uint32_t val = 42;
  AddConstantID<uint32_t>("foo", 1, u32_type(), nullptr);
  AddConstantID<uint32_t>("bar", 20, u32_type(), &val);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(2u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsU32());
  EXPECT_EQ(42u, result[20].AsU32());
}

TEST_F(InspectorGetConstantIDsTest, I32) {
  int32_t val_neg = -42;
  int32_t val_pos = 42;
  AddConstantID<int32_t>("foo", 1, i32_type(), nullptr);
  AddConstantID<int32_t>("bar", 20, i32_type(), &val_neg);
  AddConstantID<int32_t>("baz", 300, i32_type(), &val_pos);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(3u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsI32());
  EXPECT_EQ(-42, result[20].AsI32());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsI32());
  EXPECT_EQ(42, result[300].AsI32());
}

TEST_F(InspectorGetConstantIDsTest, Float) {
  float val_zero = 0.0f;
  float val_neg = -10.0f;
  float val_pos = 15.0f;
  AddConstantID<float>("foo", 1, f32_type(), nullptr);
  AddConstantID<float>("bar", 20, f32_type(), &val_zero);
  AddConstantID<float>("baz", 300, f32_type(), &val_neg);
  AddConstantID<float>("x", 4000, f32_type(), &val_pos);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(4u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsFloat());
  EXPECT_FLOAT_EQ(0.0, result[20].AsFloat());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsFloat());
  EXPECT_FLOAT_EQ(-10.0, result[300].AsFloat());

  ASSERT_TRUE(result.find(4000) != result.end());
  EXPECT_TRUE(result[4000].IsFloat());
  EXPECT_FLOAT_EQ(15.0, result[4000].AsFloat());
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MissingEntryPoint) {
  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_TRUE(inspector()->has_error());
  std::string error = inspector()->error();
  EXPECT_TRUE(error.find("not found") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, NonEntryPointFunc) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeUniformBufferTypes("foo_type", {{i32_type(), 0}});
  AddUniformBuffer("foo_ub", foo_control_type.get(), 0, 0);

  auto* ub_func = MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub",
                                                          {{0, i32_type()}});
  mod()->AddFunction(ub_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "ub_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ub_func");
  std::string error = inspector()->error();
  EXPECT_TRUE(error.find("not an entry point") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MissingBlockDeco) {
  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));

  members.push_back(create<ast::StructMember>(
      StructMemberName(members.size(), i32_type()), i32_type(), deco));

  ast::StructDecorationList decos;

  auto* str = create<ast::Struct>(decos, members);
  auto foo_type = std::make_unique<ast::type::Struct>("foo_type", str);

  AddUniformBuffer("foo_ub", foo_type.get(), 0, 0);

  auto* ub_func = MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub",
                                                          {{0, i32_type()}});
  mod()->AddFunction(ub_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "ub_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, Simple) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeUniformBufferTypes("foo_type", {{i32_type(), 0}});
  AddUniformBuffer("foo_ub", foo_control_type.get(), 0, 0);

  auto* ub_func = MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub",
                                                          {{0, i32_type()}});
  mod()->AddFunction(ub_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "ub_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(16u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleMembers) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) = MakeUniformBufferTypes(
      "foo_type", {{i32_type(), 0}, {u32_type(), 4}, {f32_type(), 8}});
  AddUniformBuffer("foo_ub", foo_control_type.get(), 0, 0);

  auto* ub_func = MakeStructVariableReferenceBodyFunction(
      "ub_func", "foo_ub", {{0, i32_type()}, {1, u32_type()}, {2, f32_type()}});
  mod()->AddFunction(ub_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "ub_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(16u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleUniformBuffers) {
  std::unique_ptr<ast::type::Struct> ub_struct_type;
  std::unique_ptr<ast::type::AccessControl> ub_control_type;
  std::tie(ub_struct_type, ub_control_type) = MakeUniformBufferTypes(
      "ub_type", {{i32_type(), 0}, {u32_type(), 4}, {f32_type(), 8}});
  AddUniformBuffer("ub_foo", ub_control_type.get(), 0, 0);
  AddUniformBuffer("ub_bar", ub_control_type.get(), 0, 1);
  AddUniformBuffer("ub_baz", ub_control_type.get(), 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    auto* ub_func = MakeStructVariableReferenceBodyFunction(
        func_name, var_name,
        {{0, i32_type()}, {1, u32_type()}, {2, f32_type()}});
    mod()->AddFunction(ub_func);
  };
  AddReferenceFunc("ub_foo_func", "ub_foo");
  AddReferenceFunc("ub_bar_func", "ub_bar");
  AddReferenceFunc("ub_baz_func", "ub_baz");

  auto AddFuncCall = [&](ast::BlockStatement* body, const std::string& callee) {
    auto* ident_expr = create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(callee), callee);
    auto* call_expr = create<ast::CallExpression>(Source{}, ident_expr,
                                                  ast::ExpressionList());
    body->append(create<ast::CallStatement>(call_expr));
  };
  auto* body = create<ast::BlockStatement>();

  AddFuncCall(body, "ub_foo_func");
  AddFuncCall(body, "ub_bar_func");
  AddFuncCall(body, "ub_baz_func");

  body->append(create<ast::ReturnStatement>(Source{}));
  ast::Function* func = create<ast::Function>(
      Source{}, mod()->RegisterSymbol("ep_func"), "ep_func",
      ast::VariableList(), void_type(), body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(16u, result[0].min_buffer_binding_size);

  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(16u, result[1].min_buffer_binding_size);

  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(16u, result[2].min_buffer_binding_size);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, ContainingArray) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) = MakeUniformBufferTypes(
      "foo_type", {{i32_type(), 0}, {u32_array_type(4), 4}});
  AddUniformBuffer("foo_ub", foo_control_type.get(), 0, 0);

  auto* ub_func = MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub",
                                                          {{0, i32_type()}});
  mod()->AddFunction(ub_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "ub_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(32u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, Simple) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {{i32_type(), 0}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleMembers) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) = MakeStorageBufferTypes(
      "foo_type", {{i32_type(), 0}, {u32_type(), 4}, {f32_type(), 8}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction(
      "sb_func", "foo_sb", {{0, i32_type()}, {1, u32_type()}, {2, f32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleStorageBuffers) {
  std::unique_ptr<ast::type::Struct> sb_struct_type;
  std::unique_ptr<ast::type::AccessControl> sb_control_type;
  std::tie(sb_struct_type, sb_control_type) = MakeStorageBufferTypes(
      "sb_type", {{i32_type(), 0}, {u32_type(), 4}, {f32_type(), 8}});
  AddStorageBuffer("sb_foo", sb_control_type.get(), 0, 0);
  AddStorageBuffer("sb_bar", sb_control_type.get(), 0, 1);
  AddStorageBuffer("sb_baz", sb_control_type.get(), 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    auto* sb_func = MakeStructVariableReferenceBodyFunction(
        func_name, var_name,
        {{0, i32_type()}, {1, u32_type()}, {2, f32_type()}});
    mod()->AddFunction(sb_func);
  };
  AddReferenceFunc("sb_foo_func", "sb_foo");
  AddReferenceFunc("sb_bar_func", "sb_bar");
  AddReferenceFunc("sb_baz_func", "sb_baz");

  auto AddFuncCall = [&](ast::BlockStatement* body, const std::string& callee) {
    auto* ident_expr = create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(callee), callee);
    auto* call_expr = create<ast::CallExpression>(Source{}, ident_expr,
                                                  ast::ExpressionList());
    body->append(create<ast::CallStatement>(call_expr));
  };
  auto* body = create<ast::BlockStatement>();

  AddFuncCall(body, "sb_foo_func");
  AddFuncCall(body, "sb_bar_func");
  AddFuncCall(body, "sb_baz_func");

  body->append(create<ast::ReturnStatement>(Source{}));
  ast::Function* func = create<ast::Function>(
      Source{}, mod()->RegisterSymbol("ep_func"), "ep_func",
      ast::VariableList(), void_type(), body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].min_buffer_binding_size);

  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].min_buffer_binding_size);

  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].min_buffer_binding_size);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingArray) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) = MakeStorageBufferTypes(
      "foo_type", {{i32_type(), 0}, {u32_array_type(4), 4}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingRuntimeArray) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) = MakeStorageBufferTypes(
      "foo_type", {{i32_type(), 0}, {u32_array_type(0), 4}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(8u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, SkipReadOnly) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeReadOnlyStorageBufferTypes("foo_type", {{i32_type(), 0}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, Simple) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeReadOnlyStorageBufferTypes("foo_type", {{i32_type(), 0}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result =
      inspector()->GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest,
       MultipleStorageBuffers) {
  std::unique_ptr<ast::type::Struct> sb_struct_type;
  std::unique_ptr<ast::type::AccessControl> sb_control_type;
  std::tie(sb_struct_type, sb_control_type) = MakeReadOnlyStorageBufferTypes(
      "sb_type", {{i32_type(), 0}, {u32_type(), 4}, {f32_type(), 8}});
  AddStorageBuffer("sb_foo", sb_control_type.get(), 0, 0);
  AddStorageBuffer("sb_bar", sb_control_type.get(), 0, 1);
  AddStorageBuffer("sb_baz", sb_control_type.get(), 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    auto* sb_func = MakeStructVariableReferenceBodyFunction(
        func_name, var_name,
        {{0, i32_type()}, {1, u32_type()}, {2, f32_type()}});
    mod()->AddFunction(sb_func);
  };
  AddReferenceFunc("sb_foo_func", "sb_foo");
  AddReferenceFunc("sb_bar_func", "sb_bar");
  AddReferenceFunc("sb_baz_func", "sb_baz");

  auto AddFuncCall = [&](ast::BlockStatement* body, const std::string& callee) {
    auto* ident_expr = create<ast::IdentifierExpression>(
        Source{}, mod()->RegisterSymbol(callee), callee);
    auto* call_expr = create<ast::CallExpression>(Source{}, ident_expr,
                                                  ast::ExpressionList());
    body->append(create<ast::CallStatement>(call_expr));
  };
  auto* body = create<ast::BlockStatement>();

  AddFuncCall(body, "sb_foo_func");
  AddFuncCall(body, "sb_bar_func");
  AddFuncCall(body, "sb_baz_func");

  body->append(create<ast::ReturnStatement>(Source{}));
  ast::Function* func = create<ast::Function>(
      Source{}, mod()->RegisterSymbol("ep_func"), "ep_func",
      ast::VariableList(), void_type(), body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result =
      inspector()->GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].min_buffer_binding_size);

  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].min_buffer_binding_size);

  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].min_buffer_binding_size);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, ContainingArray) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) = MakeReadOnlyStorageBufferTypes(
      "foo_type", {{i32_type(), 0}, {u32_array_type(4), 4}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result =
      inspector()->GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest,
       ContainingRuntimeArray) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) = MakeReadOnlyStorageBufferTypes(
      "foo_type", {{i32_type(), 0}, {u32_array_type(0), 4}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result =
      inspector()->GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(8u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, SkipNonReadOnly) {
  std::unique_ptr<ast::type::Struct> foo_struct_type;
  std::unique_ptr<ast::type::AccessControl> foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {{i32_type(), 0}});
  AddStorageBuffer("foo_sb", foo_control_type.get(), 0, 0);

  auto* sb_func = MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                                          {{0, i32_type()}});
  mod()->AddFunction(sb_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "sb_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result =
      inspector()->GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, Simple) {
  auto sampled_texture_type =
      MakeSampledTextureType(ast::type::TextureDimension::k1d, f32_type());
  AddSampledTexture("foo_texture", sampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());

  auto* func = MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", f32_type(),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, NoSampler) {
  auto* func = MakeEmptyBodyFunction(
      "ep_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, InFunction) {
  auto sampled_texture_type =
      MakeSampledTextureType(ast::type::TextureDimension::k1d, f32_type());
  AddSampledTexture("foo_texture", sampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());

  auto* foo_func = MakeSamplerReferenceBodyFunction(
      "foo_func", "foo_texture", "foo_sampler", "foo_coords", f32_type());
  mod()->AddFunction(foo_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "foo_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, UnknownEntryPoint) {
  auto sampled_texture_type =
      MakeSampledTextureType(ast::type::TextureDimension::k1d, f32_type());
  AddSampledTexture("foo_texture", sampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());

  auto* func = MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", f32_type(),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSamplerResourceBindings("foo");
  ASSERT_TRUE(inspector()->has_error()) << inspector()->error();
}

TEST_F(InspectorGetSamplerResourceBindingsTest, SkipsComparisonSamplers) {
  auto depth_texture_type =
      MakeDepthTextureType(ast::type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type.get());
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());
  AddGlobalVariable("foo_depth", f32_type());

  auto* func = MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", f32_type(),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, Simple) {
  auto depth_texture_type =
      MakeDepthTextureType(ast::type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type.get());
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());
  AddGlobalVariable("foo_depth", f32_type());

  auto* func = MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", f32_type(),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetComparisonSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, NoSampler) {
  auto* func = MakeEmptyBodyFunction(
      "ep_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetComparisonSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, InFunction) {
  auto depth_texture_type =
      MakeDepthTextureType(ast::type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type.get());
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());
  AddGlobalVariable("foo_depth", f32_type());

  auto* foo_func = MakeComparisonSamplerReferenceBodyFunction(
      "foo_func", "foo_texture", "foo_sampler", "foo_coords", "foo_depth",
      f32_type());
  mod()->AddFunction(foo_func);

  auto* ep_func = MakeCallerBodyFunction(
      "ep_func", "foo_func",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(ep_func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetComparisonSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, UnknownEntryPoint) {
  auto depth_texture_type =
      MakeDepthTextureType(ast::type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type.get());
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());
  AddGlobalVariable("foo_depth", f32_type());

  auto* func = MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", f32_type(),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSamplerResourceBindings("foo");
  ASSERT_TRUE(inspector()->has_error()) << inspector()->error();
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, SkipsSamplers) {
  auto sampled_texture_type =
      MakeSampledTextureType(ast::type::TextureDimension::k1d, f32_type());
  AddSampledTexture("foo_texture", sampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", f32_type());

  auto* func = MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", f32_type(),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetComparisonSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSampledTextureResourceBindingsTest, Empty) {
  auto* foo = MakeEmptyBodyFunction(
      "foo",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto result = inspector()->GetSampledTextureResourceBindings("foo");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetSampledTextureResourceBindingsTestWithParam, textureSample) {
  auto sampled_texture_type = MakeSampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddSampledTexture("foo_texture", sampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type =
      GetCoordsType(GetParam().type_dim, GetParam().sampled_kind);
  AddGlobalVariable("foo_coords", coord_type);

  auto* func = MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords",
      GetBaseType(GetParam().sampled_kind),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetSampledTextureResourceBindingsTest,
    InspectorGetSampledTextureResourceBindingsTestWithParam,
    testing::Values(
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kUInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kUInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k3d,
            inspector::ResourceBinding::TextureDimension::k3d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k3d,
            inspector::ResourceBinding::TextureDimension::k3d,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k3d,
            inspector::ResourceBinding::TextureDimension::k3d,
            inspector::ResourceBinding::SampledKind::kUInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::kCube,
            inspector::ResourceBinding::TextureDimension::kCube,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::kCube,
            inspector::ResourceBinding::TextureDimension::kCube,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::kCube,
            inspector::ResourceBinding::TextureDimension::kCube,
            inspector::ResourceBinding::SampledKind::kUInt}));

TEST_P(InspectorGetSampledArrayTextureResourceBindingsTestWithParam,
       textureSample) {
  auto sampled_texture_type = MakeSampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddSampledTexture("foo_texture", sampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type =
      GetCoordsType(GetParam().type_dim, GetParam().sampled_kind);
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_array_index", u32_type());

  auto* func = MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_array_index",
      GetBaseType(GetParam().sampled_kind),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetSampledArrayTextureResourceBindingsTest,
    InspectorGetSampledArrayTextureResourceBindingsTestWithParam,
    testing::Values(
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k1dArray,
            inspector::ResourceBinding::TextureDimension::k1dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k1dArray,
            inspector::ResourceBinding::TextureDimension::k1dArray,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k1dArray,
            inspector::ResourceBinding::TextureDimension::k1dArray,
            inspector::ResourceBinding::SampledKind::kUInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kUInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::kCubeArray,
            inspector::ResourceBinding::TextureDimension::kCubeArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::kCubeArray,
            inspector::ResourceBinding::TextureDimension::kCubeArray,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetSampledTextureTestParams{
            ast::type::TextureDimension::kCubeArray,
            inspector::ResourceBinding::TextureDimension::kCubeArray,
            inspector::ResourceBinding::SampledKind::kUInt}));

TEST_P(InspectorGetMultisampledTextureResourceBindingsTestWithParam,
       textureSample) {
  auto multisampled_texture_type = MakeMultisampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddMultisampledTexture("foo_texture", multisampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type =
      GetCoordsType(GetParam().type_dim, GetParam().sampled_kind);
  AddGlobalVariable("foo_coords", coord_type);

  auto* func = MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords",
      GetBaseType(GetParam().sampled_kind),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetMultisampledTextureResourceBindingsTest,
    InspectorGetMultisampledTextureResourceBindingsTestWithParam,
    testing::Values(
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kUInt},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kUInt}));

TEST_F(InspectorGetMultisampledArrayTextureResourceBindingsTest, Empty) {
  auto* foo = MakeEmptyBodyFunction(
      "foo",
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(foo);

  auto result = inspector()->GetSampledTextureResourceBindings("foo");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam,
       textureSample) {
  auto multisampled_texture_type = MakeMultisampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddMultisampledTexture("foo_texture", multisampled_texture_type.get(), 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type =
      GetCoordsType(GetParam().type_dim, GetParam().sampled_kind);
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_array_index", u32_type());

  auto* func = MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_array_index",
      GetBaseType(GetParam().sampled_kind),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });
  mod()->AddFunction(func);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector()->has_error()) << inspector()->error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetMultisampledArrayTextureResourceBindingsTest,
    InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam,
    testing::Values(
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k1dArray,
            inspector::ResourceBinding::TextureDimension::k1dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k1dArray,
            inspector::ResourceBinding::TextureDimension::k1dArray,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k1dArray,
            inspector::ResourceBinding::TextureDimension::k1dArray,
            inspector::ResourceBinding::SampledKind::kUInt},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            ast::type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kUInt}));

}  // namespace
}  // namespace inspector
}  // namespace tint
