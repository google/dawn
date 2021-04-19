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

#include "gtest/gtest.h"
#include "src/ast/call_statement.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type/depth_texture_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "tint/tint.h"

namespace tint {
namespace inspector {
namespace {

class InspectorHelper : public ProgramBuilder {
 public:
  InspectorHelper()
      : sampler_type_(type::SamplerKind::kSampler),
        comparison_sampler_type_(type::SamplerKind::kComparisonSampler) {}

  /// Generates an empty function
  /// @param name name of the function created
  /// @param decorations the function decorations
  void MakeEmptyBodyFunction(std::string name,
                             ast::DecorationList decorations) {
    Func(name, ast::VariableList(), ty.void_(),
         ast::StatementList{create<ast::ReturnStatement>()}, decorations);
  }

  /// Generates a function that calls other functions
  /// @param caller name of the function created
  /// @param callees names of the functions to be called
  /// @param decorations the function decorations
  void MakeCallerBodyFunction(std::string caller,
                              std::vector<std::string> callees,
                              ast::DecorationList decorations) {
    ast::StatementList body;
    body.reserve(callees.size() + 1);
    for (auto callee : callees) {
      body.push_back(create<ast::CallStatement>(Call(callee)));
    }
    body.push_back(create<ast::ReturnStatement>());

    Func(caller, ast::VariableList(), ty.void_(), body, decorations);
  }

  /// Generates a struct that contains user-defined IO members
  /// @param name the name of the generated struct
  /// @param inout_vars tuples of {name, loc} that will be the struct members
  type::Struct* MakeInOutStruct(
      std::string name,
      std::vector<std::tuple<std::string, uint32_t>> inout_vars) {
    ast::StructMemberList members;
    for (auto var : inout_vars) {
      std::string member_name;
      uint32_t location;
      std::tie(member_name, location) = var;
      members.push_back(Member(member_name, ty.u32(), {Location(location)}));
    }
    return Structure(name, members);
  }

  // TODO(crbug.com/tint/697): Remove this.
  /// Add In/Out variables to the global variables
  /// @param inout_vars tuples of {in, out} that will be added as entries to the
  ///                   global variables
  void AddInOutVariables(
      std::vector<std::tuple<std::string, std::string>> inout_vars) {
    uint32_t location = 0;
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;

      Global(in, ty.u32(), ast::StorageClass::kInput, nullptr,
             ast::DecorationList{create<ast::LocationDecoration>(location++)});
      Global(out, ty.u32(), ast::StorageClass::kOutput, nullptr,
             ast::DecorationList{create<ast::LocationDecoration>(location++)});
    }
  }

  // TODO(crbug.com/tint/697): Remove this.
  /// Generates a function that references in/out variables
  /// @param name name of the function created
  /// @param inout_vars tuples of {in, out} that will be converted into out = in
  ///                   calls in the function body
  /// @param decorations the function decorations
  void MakeInOutVariableBodyFunction(
      std::string name,
      std::vector<std::tuple<std::string, std::string>> inout_vars,
      ast::DecorationList decorations) {
    ast::StatementList stmts;
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      stmts.emplace_back(create<ast::AssignmentStatement>(Expr(out), Expr(in)));
    }
    stmts.emplace_back(create<ast::ReturnStatement>());
    Func(name, ast::VariableList(), ty.void_(), stmts, decorations);
  }

  // TODO(crbug.com/tint/697): Remove this.
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
      ast::DecorationList decorations) {
    ast::StatementList stmts;
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      stmts.emplace_back(create<ast::AssignmentStatement>(Expr(out), Expr(in)));
    }
    stmts.emplace_back(create<ast::CallStatement>(Call(callee)));
    stmts.emplace_back(create<ast::ReturnStatement>());

    return Func(caller, ast::VariableList(), ty.void_(), stmts, decorations);
  }

  /// Add a Constant ID to the global variables.
  /// @param name name of the variable to add
  /// @param id id number for the constant id
  /// @param type type of the variable
  /// @param val value to initialize the variable with, if NULL no initializer
  ///            will be added.
  template <class T>
  void AddConstantID(std::string name, uint32_t id, type::Type* type, T* val) {
    ast::Expression* constructor = nullptr;
    if (val) {
      constructor =
          create<ast::ScalarConstructorExpression>(MakeLiteral(type, val));
    }
    GlobalConst(name, type, constructor,
                ast::DecorationList{
                    create<ast::ConstantIdDecoration>(id),
                });
  }

  /// @param type AST type of the literal, must resolve to BoolLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(type::Type* type, bool* val) {
    return create<ast::BoolLiteral>(type, *val);
  }

  /// @param type AST type of the literal, must resolve to UIntLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(type::Type* type, uint32_t* val) {
    return create<ast::UintLiteral>(type, *val);
  }

  /// @param type AST type of the literal, must resolve to IntLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(type::Type* type, int32_t* val) {
    return create<ast::SintLiteral>(type, *val);
  }

  /// @param type AST type of the literal, must resolve to FloattLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  ast::Literal* MakeLiteral(type::Type* type, float* val) {
    return create<ast::FloatLiteral>(type, *val);
  }

  /// @param vec Vector of StageVariable to be searched
  /// @param name Name to be searching for
  /// @returns true if name is in vec, otherwise false
  bool ContainsName(const std::vector<StageVariable>& vec,
                    const std::string& name) {
    for (auto& s : vec) {
      if (s.name == name) {
        return true;
      }
    }
    return false;
  }

  /// Builds a string for accessing a member in a generated struct
  /// @param idx index of member
  /// @param type type of member
  /// @returns a string for the member
  std::string StructMemberName(size_t idx, type::Type* type) {
    return std::to_string(idx) + type->type_name();
  }

  /// Generates a struct type
  /// @param name name for the type
  /// @param member_types a vector of member types
  /// @param is_block whether or not to decorate as a Block
  /// @returns a struct type
  type::Struct* MakeStructType(const std::string& name,
                               std::vector<type::Type*> member_types,
                               bool is_block) {
    ast::StructMemberList members;
    for (auto* type : member_types) {
      members.push_back(Member(StructMemberName(members.size(), type), type));
    }

    ast::DecorationList decos;
    if (is_block) {
      decos.push_back(create<ast::StructBlockDecoration>());
    }

    auto* str = create<ast::Struct>(members, decos);
    auto* str_ty = ty.struct_(name, str);
    AST().AddConstructedType(str_ty);
    return str_ty;
  }

  /// Generates types appropriate for using in an uniform buffer
  /// @param name name for the type
  /// @param member_types a vector of member types
  /// @returns a struct type that has the layout for an uniform buffer.
  type::Struct* MakeUniformBufferType(const std::string& name,
                                      std::vector<type::Type*> member_types) {
    auto* struct_type = MakeStructType(name, member_types, true);
    return struct_type;
  }

  /// Generates types appropriate for using in a storage buffer
  /// @param name name for the type
  /// @param member_types a vector of member types
  /// @returns a tuple {struct type, access control type}, where the struct has
  ///          the layout for a storage buffer, and the control type wraps the
  ///          struct.
  std::tuple<type::Struct*, type::AccessControl*> MakeStorageBufferTypes(
      const std::string& name,
      std::vector<type::Type*> member_types) {
    auto* struct_type = MakeStructType(name, member_types, true);
    auto* access_type = create<type::AccessControl>(
        ast::AccessControl::kReadWrite, struct_type);
    return {struct_type, std::move(access_type)};
  }

  /// Generates types appropriate for using in a read-only storage buffer
  /// @param name name for the type
  /// @param member_types a vector of member types
  /// @returns a tuple {struct type, access control type}, where the struct has
  ///          the layout for a read-only storage buffer, and the control type
  ///          wraps the struct.
  std::tuple<type::Struct*, type::AccessControl*>
  MakeReadOnlyStorageBufferTypes(const std::string& name,
                                 std::vector<type::Type*> member_types) {
    auto* struct_type = MakeStructType(name, member_types, true);
    auto* access_type =
        create<type::AccessControl>(ast::AccessControl::kReadOnly, struct_type);
    return {struct_type, std::move(access_type)};
  }

  /// Adds a binding variable with a struct type to the program
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param storage_class the storage class to use
  /// @param group the binding and group to use for the uniform buffer
  /// @param binding the binding number to use for the uniform buffer
  void AddBinding(const std::string& name,
                  type::Type* type,
                  ast::StorageClass storage_class,
                  uint32_t group,
                  uint32_t binding) {
    Global(name, type, storage_class, nullptr,
           ast::DecorationList{
               create<ast::BindingDecoration>(binding),
               create<ast::GroupDecoration>(group),
           });
  }

  /// Adds an uniform buffer variable to the program
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param group the binding/group/ to use for the uniform buffer
  /// @param binding the binding number to use for the uniform buffer
  void AddUniformBuffer(const std::string& name,
                        type::Type* type,
                        uint32_t group,
                        uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniform, group, binding);
  }

  /// Adds a storage buffer variable to the program
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param group the binding/group to use for the storage buffer
  /// @param binding the binding number to use for the storage buffer
  void AddStorageBuffer(const std::string& name,
                        type::Type* type,
                        uint32_t group,
                        uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kStorage, group, binding);
  }

  /// Generates a function that references a specific struct variable
  /// @param func_name name of the function created
  /// @param struct_name name of the struct variabler to be accessed
  /// @param members list of members to access, by index and type
  void MakeStructVariableReferenceBodyFunction(
      std::string func_name,
      std::string struct_name,
      std::vector<std::tuple<size_t, type::Type*>> members) {
    ast::StatementList stmts;
    for (auto member : members) {
      size_t member_idx;
      type::Type* member_type;
      std::tie(member_idx, member_type) = member;
      std::string member_name = StructMemberName(member_idx, member_type);

      stmts.emplace_back(create<ast::VariableDeclStatement>(
          Var("local" + member_name, member_type, ast::StorageClass::kNone)));
    }

    for (auto member : members) {
      size_t member_idx;
      type::Type* member_type;
      std::tie(member_idx, member_type) = member;
      std::string member_name = StructMemberName(member_idx, member_type);

      stmts.emplace_back(create<ast::AssignmentStatement>(
          Expr("local" + member_name),
          MemberAccessor(struct_name, member_name)));
    }

    stmts.emplace_back(create<ast::ReturnStatement>());

    Func(func_name, ast::VariableList(), ty.void_(), stmts,
         ast::DecorationList{});
  }

  /// Adds a regular sampler variable to the program
  /// @param name the name of the variable
  /// @param group the binding/group to use for the storage buffer
  /// @param binding the binding number to use for the storage buffer
  void AddSampler(const std::string& name, uint32_t group, uint32_t binding) {
    AddBinding(name, sampler_type(), ast::StorageClass::kUniformConstant, group,
               binding);
  }

  /// Adds a comparison sampler variable to the program
  /// @param name the name of the variable
  /// @param group the binding/group to use for the storage buffer
  /// @param binding the binding number to use for the storage buffer
  void AddComparisonSampler(const std::string& name,
                            uint32_t group,
                            uint32_t binding) {
    AddBinding(name, comparison_sampler_type(),
               ast::StorageClass::kUniformConstant, group, binding);
  }

  /// Generates a SampledTexture appropriate for the params
  /// @param dim the dimensions of the texture
  /// @param type the data type of the sampled texture
  /// @returns the generated SampleTextureType
  type::SampledTexture* MakeSampledTextureType(type::TextureDimension dim,
                                               type::Type* type) {
    return create<type::SampledTexture>(dim, type);
  }

  /// Generates a DepthTexture appropriate for the params
  /// @param dim the dimensions of the texture
  /// @returns the generated DepthTexture
  type::DepthTexture* MakeDepthTextureType(type::TextureDimension dim) {
    return create<type::DepthTexture>(dim);
  }

  /// Generates a MultisampledTexture appropriate for the params
  /// @param dim the dimensions of the texture
  /// @param type the data type of the sampled texture
  /// @returns the generated SampleTextureType
  type::MultisampledTexture* MakeMultisampledTextureType(
      type::TextureDimension dim,
      type::Type* type) {
    return create<type::MultisampledTexture>(dim, type);
  }

  /// Adds a sampled texture variable to the program
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param group the binding/group to use for the sampled texture
  /// @param binding the binding number to use for the sampled texture
  void AddSampledTexture(const std::string& name,
                         type::Type* type,
                         uint32_t group,
                         uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniformConstant, group, binding);
  }

  /// Adds a multi-sampled texture variable to the program
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param group the binding/group to use for the multi-sampled texture
  /// @param binding the binding number to use for the multi-sampled texture
  void AddMultisampledTexture(const std::string& name,
                              type::Type* type,
                              uint32_t group,
                              uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniformConstant, group, binding);
  }

  void AddGlobalVariable(const std::string& name, type::Type* type) {
    Global(name, type, ast::StorageClass::kUniformConstant);
  }

  /// Adds a depth texture variable to the program
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param group the binding/group to use for the depth texture
  /// @param binding the binding number to use for the depth texture
  void AddDepthTexture(const std::string& name,
                       type::Type* type,
                       uint32_t group,
                       uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniformConstant, group, binding);
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
      type::Type* base_type,
      ast::DecorationList decorations) {
    std::string result_name = "sampler_result";

    ast::StatementList stmts;
    stmts.emplace_back(create<ast::VariableDeclStatement>(
        Var("sampler_result", vec_type(base_type, 4),
            ast::StorageClass::kFunction)));

    stmts.emplace_back(create<ast::AssignmentStatement>(
        Expr("sampler_result"),
        Call("textureSample", texture_name, sampler_name, coords_name)));
    stmts.emplace_back(create<ast::ReturnStatement>());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, decorations);
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
      type::Type* base_type,
      ast::DecorationList decorations) {
    std::string result_name = "sampler_result";

    ast::StatementList stmts;

    stmts.emplace_back(create<ast::VariableDeclStatement>(
        Var("sampler_result", vec_type(base_type, 4),
            ast::StorageClass::kFunction)));

    stmts.emplace_back(create<ast::AssignmentStatement>(
        Expr("sampler_result"), Call("textureSample", texture_name,
                                     sampler_name, coords_name, array_index)));
    stmts.emplace_back(create<ast::ReturnStatement>());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, decorations);
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
      type::Type* base_type,
      ast::DecorationList decorations) {
    std::string result_name = "sampler_result";

    ast::StatementList stmts;

    stmts.emplace_back(create<ast::VariableDeclStatement>(
        Var("sampler_result", base_type, ast::StorageClass::kFunction)));
    stmts.emplace_back(create<ast::AssignmentStatement>(
        Expr("sampler_result"), Call("textureSampleCompare", texture_name,
                                     sampler_name, coords_name, depth_name)));
    stmts.emplace_back(create<ast::ReturnStatement>());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, decorations);
  }

  /// Gets an appropriate type for the data in a given texture type.
  /// @param sampled_kind type of in the texture
  /// @returns a pointer to a type appropriate for the coord param
  type::Type* GetBaseType(ResourceBinding::SampledKind sampled_kind) {
    switch (sampled_kind) {
      case ResourceBinding::SampledKind::kFloat:
        return ty.f32();
      case ResourceBinding::SampledKind::kSInt:
        return ty.i32();
      case ResourceBinding::SampledKind::kUInt:
        return ty.u32();
      default:
        return nullptr;
    }
  }

  /// Gets an appropriate type for the coords parameter depending the the
  /// dimensionality of the texture being sampled.
  /// @param dim dimensionality of the texture being sampled
  /// @param scalar the scalar type
  /// @returns a pointer to a type appropriate for the coord param
  type::Type* GetCoordsType(type::TextureDimension dim, type::Type* scalar) {
    switch (dim) {
      case type::TextureDimension::k1d:
        return scalar;
      case type::TextureDimension::k2d:
      case type::TextureDimension::k2dArray:
        return create<type::Vector>(scalar, 2);
      case type::TextureDimension::k3d:
      case type::TextureDimension::kCube:
      case type::TextureDimension::kCubeArray:
        return create<type::Vector>(scalar, 3);
      default:
        [=]() { FAIL() << "Unsupported texture dimension: " << dim; }();
    }
    return nullptr;
  }

  /// Generates appropriate types for a StorageTexture
  /// @param dim the texture dimension of the storage texture
  /// @param format the image format of the storage texture
  /// @returns the storage texture type and subtype
  std::tuple<type::StorageTexture*, type::Type*> MakeStorageTextureTypes(
      type::TextureDimension dim,
      type::ImageFormat format) {
    type::Type* subtype = type::StorageTexture::SubtypeFor(format, Types());
    return {create<type::StorageTexture>(dim, format, subtype), subtype};
  }

  /// Generates appropriate types for a Read-Only StorageTexture
  /// @param dim the texture dimension of the storage texture
  /// @param format the image format of the storage texture
  /// @param read_only should the access type be read only, otherwise write only
  /// @returns the storage texture type, subtype & access control type
  std::tuple<type::StorageTexture*, type::Type*, type::AccessControl*>
  MakeStorageTextureTypes(type::TextureDimension dim,
                          type::ImageFormat format,
                          bool read_only) {
    type::StorageTexture* texture_type;
    type::Type* subtype;
    std::tie(texture_type, subtype) = MakeStorageTextureTypes(dim, format);
    auto* access_control =
        create<type::AccessControl>(read_only ? ast::AccessControl::kReadOnly
                                              : ast::AccessControl::kWriteOnly,
                                    texture_type);
    return {texture_type, subtype, access_control};
  }

  /// Adds a storage texture variable to the program
  /// @param name the name of the variable
  /// @param type the type to use
  /// @param group the binding/group to use for the sampled texture
  /// @param binding the binding number to use for the sampled texture
  void AddStorageTexture(const std::string& name,
                         type::Type* type,
                         uint32_t group,
                         uint32_t binding) {
    AddBinding(name, type, ast::StorageClass::kUniformConstant, group, binding);
  }

  /// Generates a function that references a storage texture variable.
  /// @param func_name name of the function created
  /// @param st_name name of the storage texture to use
  /// @param dim_type type expected by textureDimensons to return
  /// @param decorations the function decorations
  /// @returns a function that references all of the values specified
  ast::Function* MakeStorageTextureBodyFunction(
      const std::string& func_name,
      const std::string& st_name,
      type::Type* dim_type,
      ast::DecorationList decorations) {
    ast::StatementList stmts;

    stmts.emplace_back(create<ast::VariableDeclStatement>(
        Var("dim", dim_type, ast::StorageClass::kFunction)));
    stmts.emplace_back(create<ast::AssignmentStatement>(
        Expr("dim"), Call("textureDimensions", st_name)));
    stmts.emplace_back(create<ast::ReturnStatement>());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, decorations);
  }

  Inspector& Build() {
    if (inspector_) {
      return *inspector_;
    }
    program_ = std::make_unique<Program>(std::move(*this));
    [&]() {
      ASSERT_TRUE(program_->IsValid())
          << diag::Formatter().format(program_->Diagnostics());
    }();
    inspector_ = std::make_unique<Inspector>(program_.get());
    return *inspector_;
  }

  type::ArrayType* u32_array_type(uint32_t count) {
    if (array_type_memo_.find(count) == array_type_memo_.end()) {
      array_type_memo_[count] =
          create<type::ArrayType>(ty.u32(), count,
                                  ast::DecorationList{
                                      create<ast::StrideDecoration>(4),
                                  });
    }
    return array_type_memo_[count];
  }
  type::Vector* vec_type(type::Type* type, uint32_t count) {
    if (vector_type_memo_.find(std::tie(type, count)) ==
        vector_type_memo_.end()) {
      vector_type_memo_[std::tie(type, count)] =
          create<type::Vector>(type, count);
    }
    return vector_type_memo_[std::tie(type, count)];
  }
  type::Sampler* sampler_type() { return &sampler_type_; }
  type::Sampler* comparison_sampler_type() { return &comparison_sampler_type_; }

 private:
  std::unique_ptr<Program> program_;
  std::unique_ptr<Inspector> inspector_;
  type::Sampler sampler_type_;
  type::Sampler comparison_sampler_type_;
  std::map<uint32_t, type::ArrayType*> array_type_memo_;
  std::map<std::tuple<type::Type*, uint32_t>, type::Vector*> vector_type_memo_;
};

class InspectorGetEntryPointTest : public InspectorHelper,
                                   public testing::Test {};
class InspectorGetEntryPointTestWithComponentTypeParam
    : public InspectorHelper,
      public testing::TestWithParam<ComponentType> {};
class InspectorGetRemappedNameForEntryPointTest : public InspectorHelper,
                                                  public testing::Test {};
class InspectorGetConstantIDsTest : public InspectorHelper,
                                    public testing::Test {};
class InspectorGetResourceBindingsTest : public InspectorHelper,
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
  type::TextureDimension type_dim;
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
class InspectorGetStorageTextureResourceBindingsTest : public InspectorHelper,
                                                       public testing::Test {};
struct GetDepthTextureTestParams {
  type::TextureDimension type_dim;
  inspector::ResourceBinding::TextureDimension inspector_dim;
};
class InspectorGetDepthTextureResourceBindingsTestWithParam
    : public InspectorHelper,
      public testing::TestWithParam<GetDepthTextureTestParams> {};

typedef std::tuple<type::TextureDimension, ResourceBinding::TextureDimension>
    DimensionParams;
typedef std::tuple<type::ImageFormat,
                   ResourceBinding::ImageFormat,
                   ResourceBinding::SampledKind>
    ImageFormatParams;
typedef std::tuple<bool, DimensionParams, ImageFormatParams>
    GetStorageTextureTestParams;
class InspectorGetStorageTextureResourceBindingsTestWithParam
    : public InspectorHelper,
      public testing::TestWithParam<GetStorageTextureTestParams> {};

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kVertex),
             });

  // TODO(dsinclair): Update to run the namer transform when available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPoints) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kVertex),
             });

  MakeEmptyBodyFunction(
      "bar", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

  // TODO(dsinclair): Update to run the namer transform when available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ("bar", result[1].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kCompute, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, MixFunctionsAndEntryPoints) {
  MakeEmptyBodyFunction("func", {});

  MakeCallerBodyFunction(
      "foo", {"func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  MakeCallerBodyFunction(
      "bar", {"func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment),
      });

  // TODO(dsinclair): Update to run the namer transform when available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  EXPECT_FALSE(inspector.has_error());

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ("bar", result[1].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kFragment, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, DefaultWorkgroupSize) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kVertex),
             });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(1u, x);
  EXPECT_EQ(1u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NonDefaultWorkgroupSize) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
                 create<ast::WorkgroupDecoration>(8u, 2u, 1u),
             });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(8u, x);
  EXPECT_EQ(2u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NoInOutVariables) {
  MakeEmptyBodyFunction("func", {});

  MakeCallerBodyFunction(
      "foo", {"func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].input_variables.size());
  EXPECT_EQ(0u, result[0].output_variables.size());
}

TEST_P(InspectorGetEntryPointTestWithComponentTypeParam, InOutVariables) {
  ComponentType inspector_type = GetParam();
  type::Type* tint_type = nullptr;
  switch (inspector_type) {
    case ComponentType::kFloat:
      tint_type = ty.f32();
      break;
    case ComponentType::kSInt:
      tint_type = ty.i32();
      break;
    case ComponentType::kUInt:
      tint_type = ty.u32();
      break;
    case ComponentType::kUnknown:
      return;
  }

  auto* in_var = Param("in_var", tint_type, {Location(0u)});
  Func("foo", {in_var}, tint_type, {Return(Expr("in_var"))},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(inspector_type, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("<retval>", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(inspector_type, result[0].output_variables[0].component_type);
}
INSTANTIATE_TEST_SUITE_P(InspectorGetEntryPointTest,
                         InspectorGetEntryPointTestWithComponentTypeParam,
                         testing::Values(ComponentType::kFloat,
                                         ComponentType::kSInt,
                                         ComponentType::kUInt));

TEST_F(InspectorGetEntryPointTest, MultipleInOutVariables) {
  auto* in_var0 = Param("in_var0", ty.u32(), {Location(0u)});
  auto* in_var1 = Param("in_var1", ty.u32(), {Location(1u)});
  auto* in_var4 = Param("in_var4", ty.u32(), {Location(4u)});
  Func("foo", {in_var0, in_var1, in_var4}, ty.u32(), {Return(Expr("in_var0"))},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(3u, result[0].input_variables.size());
  EXPECT_EQ("in_var0", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_EQ("in_var1", result[0].input_variables[1].name);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);
  EXPECT_EQ("in_var4", result[0].input_variables[2].name);
  EXPECT_TRUE(result[0].input_variables[2].has_location_decoration);
  EXPECT_EQ(4u, result[0].input_variables[2].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[2].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("<retval>", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutVariables) {
  auto* in_var_foo = Param("in_var_foo", ty.u32(), {Location(0u)});
  Func("foo", {in_var_foo}, ty.u32(), {Return(Expr("in_var_foo"))},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});

  auto* in_var_bar = Param("in_var_bar", ty.u32(), {Location(0u)});
  Func("bar", {in_var_bar}, ty.u32(), {Return(Expr("in_var_bar"))},
       {Stage(ast::PipelineStage::kFragment)}, {Location(1u)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var_foo", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("<retval>", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);

  ASSERT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in_var_bar", result[1].input_variables[0].name);
  EXPECT_TRUE(result[1].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[1].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);

  ASSERT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("<retval>", result[1].output_variables[0].name);
  EXPECT_TRUE(result[1].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[1].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, BuiltInsNotStageVariables) {
  auto* in_var0 =
      Param("in_var0", ty.u32(), {Builtin(ast::Builtin::kInstanceIndex)});
  auto* in_var1 = Param("in_var1", ty.u32(), {Location(0u)});
  Func("foo", {in_var0, in_var1}, ty.u32(), {Return(Expr("in_var1"))},
       {Stage(ast::PipelineStage::kFragment)},
       {Builtin(ast::Builtin::kSampleMask)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var1", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

  ASSERT_EQ(0u, result[0].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, InOutStruct) {
  auto* interface = MakeInOutStruct("interface", {{"a", 0u}, {"b", 1u}});
  Func("foo", {Param("param", interface)}, interface, {Return(Expr("param"))},
       {Stage(ast::PipelineStage::kFragment)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_EQ("param.a", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_EQ("param.b", result[0].input_variables[1].name);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutSharedStruct) {
  auto* interface = MakeInOutStruct("interface", {{"a", 0u}, {"b", 1u}});
  Func("foo", {}, interface, {Return(Construct(interface))},
       {Stage(ast::PipelineStage::kFragment)});
  Func("bar", {Param("param", interface)}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ(0u, result[0].input_variables.size());

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);

  ASSERT_EQ(2u, result[1].input_variables.size());
  EXPECT_EQ("param.a", result[1].input_variables[0].name);
  EXPECT_TRUE(result[1].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[1].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);
  EXPECT_EQ("param.b", result[1].input_variables[1].name);
  EXPECT_TRUE(result[1].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[1].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[1].component_type);

  ASSERT_EQ(0u, result[1].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, MixInOutVariablesAndStruct) {
  auto* struct_a = MakeInOutStruct("struct_a", {{"a", 0u}, {"b", 1u}});
  auto* struct_b = MakeInOutStruct("struct_b", {{"a", 2u}});
  Func("foo",
       {Param("param_a", struct_a), Param("param_b", struct_b),
        Param("param_c", ty.f32(), {Location(3u)}),
        Param("param_d", ty.f32(), {Location(4u)})},
       struct_a, {Return(Expr("param_a"))},
       {Stage(ast::PipelineStage::kFragment)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(5u, result[0].input_variables.size());
  EXPECT_EQ("param_a.a", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_EQ("param_a.b", result[0].input_variables[1].name);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);
  EXPECT_EQ("param_b.a", result[0].input_variables[2].name);
  EXPECT_TRUE(result[0].input_variables[2].has_location_decoration);
  EXPECT_EQ(2u, result[0].input_variables[2].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[2].component_type);
  EXPECT_EQ("param_c", result[0].input_variables[3].name);
  EXPECT_TRUE(result[0].input_variables[3].has_location_decoration);
  EXPECT_EQ(3u, result[0].input_variables[3].location_decoration);
  EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[3].component_type);
  EXPECT_EQ("param_d", result[0].input_variables[4].name);
  EXPECT_TRUE(result[0].input_variables[4].has_location_decoration);
  EXPECT_EQ(4u, result[0].input_variables[4].location_decoration);
  EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[4].component_type);

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest, EntryPointInOutVariables_Legacy) {
  AddInOutVariables({{"in_var", "out_var"}});

  MakeInOutVariableBodyFunction(
      "foo", {{"in_var", "out_var"}},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest, FunctionInOutVariables_Legacy) {
  AddInOutVariables({{"in_var", "out_var"}});

  MakeInOutVariableBodyFunction("func", {{"in_var", "out_var"}}, {});

  MakeCallerBodyFunction(
      "foo", {"func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest, RepeatedInOutVariables_Legacy) {
  AddInOutVariables({{"in_var", "out_var"}});

  MakeInOutVariableBodyFunction("func", {{"in_var", "out_var"}}, {});

  MakeInOutVariableCallerBodyFunction(
      "foo", "func", {{"in_var", "out_var"}},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest, EntryPointMultipleInOutVariables_Legacy) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  MakeInOutVariableBodyFunction(
      "foo", {{"in_var", "out_var"}, {"in2_var", "out2_var"}},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsName(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsName(result[0].input_variables, "in2_var"));
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(2u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsName(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsName(result[0].output_variables, "out2_var"));
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(3u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest, FunctionMultipleInOutVariables_Legacy) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  MakeInOutVariableBodyFunction(
      "func", {{"in_var", "out_var"}, {"in2_var", "out2_var"}}, {});

  MakeCallerBodyFunction(
      "foo", {"func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsName(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsName(result[0].input_variables, "in2_var"));
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(2u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsName(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsName(result[0].output_variables, "out2_var"));
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(3u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutVariables_Legacy) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  MakeInOutVariableBodyFunction(
      "foo", {{"in_var", "out2_var"}},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  MakeInOutVariableBodyFunction(
      "bar", {{"in2_var", "out_var"}},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute),
      });

  // TODO(dsinclair): Update to run the namer transform when
  // available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ("foo", result[0].name);
  ASSERT_EQ("foo", result[0].remapped_name);

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out2_var", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(3u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);

  ASSERT_EQ("bar", result[1].name);
  ASSERT_EQ("bar", result[1].remapped_name);

  ASSERT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in2_var", result[1].input_variables[0].name);
  EXPECT_TRUE(result[1].input_variables[0].has_location_decoration);
  EXPECT_EQ(2u, result[1].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);

  ASSERT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("out_var", result[1].output_variables[0].name);
  EXPECT_TRUE(result[1].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[1].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].output_variables[0].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest,
       MultipleEntryPointsSharedInOutVariables_Legacy) {
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  MakeInOutVariableBodyFunction("func", {{"in2_var", "out2_var"}}, {});

  MakeInOutVariableCallerBodyFunction(
      "foo", "func", {{"in_var", "out_var"}},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  MakeCallerBodyFunction(
      "bar", {"func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute),
      });

  // TODO(dsinclair): Update to run the namer transform when
  // available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ("foo", result[0].name);
  ASSERT_EQ("foo", result[0].remapped_name);

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsName(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsName(result[0].input_variables, "in2_var"));
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(2u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsName(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsName(result[0].output_variables, "out2_var"));
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(3u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);

  ASSERT_EQ("bar", result[1].name);
  ASSERT_EQ("bar", result[1].remapped_name);

  ASSERT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in2_var", result[1].input_variables[0].name);
  EXPECT_TRUE(result[1].input_variables[0].has_location_decoration);
  EXPECT_EQ(2u, result[1].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);

  ASSERT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("out2_var", result[1].output_variables[0].name);
  EXPECT_TRUE(result[1].output_variables[0].has_location_decoration);
  EXPECT_EQ(3u, result[1].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].output_variables[0].component_type);
}

// TODO(crbug.com/tint/697): Remove this.
TEST_F(InspectorGetEntryPointTest, BuiltInsNotStageVariables_Legacy) {
  Global("in_var", ty.u32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kPosition)});
  Global("out_var", ty.u32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{create<ast::LocationDecoration>(0)});

  MakeInOutVariableBodyFunction("func", {{"in_var", "out_var"}}, {});

  MakeCallerBodyFunction(
      "foo", {"func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  // TODO(dsinclair): Update to run the namer transform when available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ("foo", result[0].name);
  ASSERT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(0u, result[0].input_variables.size());
  EXPECT_EQ(1u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsName(result[0].output_variables, "out_var"));
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_NoFunctions) {
  Inspector& inspector = Build();

  auto result = inspector.GetRemappedNameForEntryPoint("foo");
  ASSERT_TRUE(inspector.has_error());

  EXPECT_EQ("", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_NoEntryPoints) {
  Inspector& inspector = Build();

  auto result = inspector.GetRemappedNameForEntryPoint("foo");
  ASSERT_TRUE(inspector.has_error());

  EXPECT_EQ("", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_OneEntryPoint) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kVertex),
             });

  // TODO(dsinclair): Update to run the namer transform when
  // available.

  Inspector& inspector = Build();

  auto result = inspector.GetRemappedNameForEntryPoint("foo");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ("foo", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest,
       DISABLED_MultipleEntryPoints) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kVertex),
             });

  // TODO(dsinclair): Update to run the namer transform when
  // available.

  MakeEmptyBodyFunction(
      "bar", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

  Inspector& inspector = Build();

  {
    auto result = inspector.GetRemappedNameForEntryPoint("foo");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    EXPECT_EQ("foo", result);
  }
  {
    auto result = inspector.GetRemappedNameForEntryPoint("bar");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    EXPECT_EQ("bar", result);
  }
}

TEST_F(InspectorGetConstantIDsTest, Bool) {
  bool val_true = true;
  bool val_false = false;
  AddConstantID<bool>("foo", 1, ty.bool_(), nullptr);
  AddConstantID<bool>("bar", 20, ty.bool_(), &val_true);
  AddConstantID<bool>("baz", 300, ty.bool_(), &val_false);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
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
  AddConstantID<uint32_t>("foo", 1, ty.u32(), nullptr);
  AddConstantID<uint32_t>("bar", 20, ty.u32(), &val);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
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
  AddConstantID<int32_t>("foo", 1, ty.i32(), nullptr);
  AddConstantID<int32_t>("bar", 20, ty.i32(), &val_neg);
  AddConstantID<int32_t>("baz", 300, ty.i32(), &val_pos);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
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
  AddConstantID<float>("foo", 1, ty.f32(), nullptr);
  AddConstantID<float>("bar", 20, ty.f32(), &val_zero);
  AddConstantID<float>("baz", 300, ty.f32(), &val_neg);
  AddConstantID<float>("x", 4000, ty.f32(), &val_pos);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
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

TEST_F(InspectorGetResourceBindingsTest, Empty) {
  MakeCallerBodyFunction(
      "ep_func", {},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetResourceBindingsTest, Simple) {
  type::Struct* ub_struct_type = MakeUniformBufferType("ub_type", {ty.i32()});
  AddUniformBuffer("ub_var", ub_struct_type, 0, 0);
  MakeStructVariableReferenceBodyFunction("ub_func", "ub_var", {{0, ty.i32()}});

  type::Struct* sb_struct_type;
  type::AccessControl* sb_control_type;
  std::tie(sb_struct_type, sb_control_type) =
      MakeStorageBufferTypes("sb_type", {ty.i32()});
  AddStorageBuffer("sb_var", sb_control_type, 1, 0);
  MakeStructVariableReferenceBodyFunction("sb_func", "sb_var", {{0, ty.i32()}});

  type::Struct* rosb_struct_type;
  type::AccessControl* rosb_control_type;
  std::tie(rosb_struct_type, rosb_control_type) =
      MakeReadOnlyStorageBufferTypes("rosb_type", {ty.i32()});
  AddStorageBuffer("rosb_var", rosb_control_type, 1, 1);
  MakeStructVariableReferenceBodyFunction("rosb_func", "rosb_var",
                                          {{0, ty.i32()}});

  auto* s_texture_type =
      MakeSampledTextureType(type::TextureDimension::k1d, ty.f32());
  AddSampledTexture("s_texture", s_texture_type, 2, 0);
  AddSampler("s_var", 3, 0);
  AddGlobalVariable("s_coords", ty.f32());
  MakeSamplerReferenceBodyFunction("s_func", "s_texture", "s_var", "s_coords",
                                   ty.f32(), {});

  auto* cs_depth_texture_type =
      MakeDepthTextureType(type::TextureDimension::k2d);
  AddDepthTexture("cs_texture", cs_depth_texture_type, 3, 1);
  AddComparisonSampler("cs_var", 3, 2);
  AddGlobalVariable("cs_coords", ty.vec2<f32>());
  AddGlobalVariable("cs_depth", ty.f32());
  MakeComparisonSamplerReferenceBodyFunction(
      "cs_func", "cs_texture", "cs_var", "cs_coords", "cs_depth", ty.f32(), {});

  type::StorageTexture* st_type;
  type::Type* st_subtype;
  type::AccessControl* st_ac;
  std::tie(st_type, st_subtype, st_ac) = MakeStorageTextureTypes(
      type::TextureDimension::k2d, type::ImageFormat::kR8Uint, false);
  AddStorageTexture("st_var", st_ac, 4, 0);
  MakeStorageTextureBodyFunction("st_func", "st_var", ty.vec2<i32>(), {});

  type::StorageTexture* rost_type;
  type::Type* rost_subtype;
  type::AccessControl* rost_ac;
  std::tie(rost_type, rost_subtype, rost_ac) = MakeStorageTextureTypes(
      type::TextureDimension::k2d, type::ImageFormat::kR8Uint, true);
  AddStorageTexture("rost_var", rost_ac, 4, 1);
  MakeStorageTextureBodyFunction("rost_func", "rost_var", ty.vec2<i32>(), {});

  MakeCallerBodyFunction(
      "ep_func",
      {"ub_func", "sb_func", "rosb_func", "s_func", "cs_func", "st_func",
       "rost_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(9u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[1].resource_type);
  EXPECT_EQ(1u, result[1].bind_group);
  EXPECT_EQ(0u, result[1].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[2].resource_type);
  EXPECT_EQ(1u, result[2].bind_group);
  EXPECT_EQ(1u, result[2].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[3].resource_type);
  EXPECT_EQ(3u, result[3].bind_group);
  EXPECT_EQ(0u, result[3].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler,
            result[4].resource_type);
  EXPECT_EQ(3u, result[4].bind_group);
  EXPECT_EQ(2u, result[4].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture,
            result[5].resource_type);
  EXPECT_EQ(2u, result[5].bind_group);
  EXPECT_EQ(0u, result[5].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageTexture,
            result[6].resource_type);
  EXPECT_EQ(4u, result[6].bind_group);
  EXPECT_EQ(1u, result[6].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kWriteOnlyStorageTexture,
            result[7].resource_type);
  EXPECT_EQ(4u, result[7].bind_group);
  EXPECT_EQ(0u, result[7].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture,
            result[8].resource_type);
  EXPECT_EQ(3u, result[8].bind_group);
  EXPECT_EQ(1u, result[8].binding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MissingEntryPoint) {
  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_TRUE(inspector.has_error());
  std::string error = inspector.error();
  EXPECT_TRUE(error.find("not found") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, NonEntryPointFunc) {
  type::Struct* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
  AddUniformBuffer("foo_ub", foo_struct_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"ub_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ub_func");
  std::string error = inspector.error();
  EXPECT_TRUE(error.find("not an entry point") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MissingBlockDeco) {
  ast::DecorationList decos;
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member(StructMemberName(0, ty.i32()), ty.i32())},
      decos);

  auto* foo_type = ty.struct_("foo_type", str);
  AddUniformBuffer("foo_ub", foo_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"ub_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, Simple) {
  type::Struct* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
  AddUniformBuffer("foo_ub", foo_struct_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"ub_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].size);
  EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleMembers) {
  type::Struct* foo_struct_type =
      MakeUniformBufferType("foo_type", {ty.i32(), ty.u32(), ty.f32()});
  AddUniformBuffer("foo_ub", foo_struct_type, 0, 0);

  MakeStructVariableReferenceBodyFunction(
      "ub_func", "foo_ub", {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});

  MakeCallerBodyFunction(
      "ep_func", {"ub_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, ContainingPadding) {
  type::Struct* foo_struct_type =
      MakeUniformBufferType("foo_type", {ty.vec3<f32>()});
  AddUniformBuffer("foo_ub", foo_struct_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub",
                                          {{0, ty.vec3<f32>()}});

  MakeCallerBodyFunction(
      "ep_func", {"ub_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(16u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleUniformBuffers) {
  type::Struct* ub_struct_type =
      MakeUniformBufferType("ub_type", {ty.i32(), ty.u32(), ty.f32()});
  AddUniformBuffer("ub_foo", ub_struct_type, 0, 0);
  AddUniformBuffer("ub_bar", ub_struct_type, 0, 1);
  AddUniformBuffer("ub_baz", ub_struct_type, 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    MakeStructVariableReferenceBodyFunction(
        func_name, var_name, {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
  };
  AddReferenceFunc("ub_foo_func", "ub_foo");
  AddReferenceFunc("ub_bar_func", "ub_bar");
  AddReferenceFunc("ub_baz_func", "ub_baz");

  auto FuncCall = [&](const std::string& callee) {
    return create<ast::CallStatement>(Call(callee));
  };

  Func("ep_func", ast::VariableList(), ty.void_(),
       ast::StatementList{FuncCall("ub_foo_func"), FuncCall("ub_bar_func"),
                          FuncCall("ub_baz_func"),
                          create<ast::ReturnStatement>()},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[1].resource_type);
  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].size);
  EXPECT_EQ(12u, result[1].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[2].resource_type);
  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].size);
  EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, ContainingArray) {
  // TODO(bclayton) - This is not a legal structure layout for uniform buffer
  // usage. Once crbug.com/tint/628 is implemented, this will fail validation
  // and will need to be fixed.
  type::Struct* foo_struct_type =
      MakeUniformBufferType("foo_type", {ty.i32(), u32_array_type(4)});
  AddUniformBuffer("foo_ub", foo_struct_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"ub_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].size);
  EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, Simple) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].size);
  EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleMembers) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {ty.i32(), ty.u32(), ty.f32()});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction(
      "sb_func", "foo_sb", {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleStorageBuffers) {
  type::Struct* sb_struct_type;
  type::AccessControl* sb_control_type;
  std::tie(sb_struct_type, sb_control_type) =
      MakeStorageBufferTypes("sb_type", {ty.i32(), ty.u32(), ty.f32()});
  AddStorageBuffer("sb_foo", sb_control_type, 0, 0);
  AddStorageBuffer("sb_bar", sb_control_type, 0, 1);
  AddStorageBuffer("sb_baz", sb_control_type, 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    MakeStructVariableReferenceBodyFunction(
        func_name, var_name, {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
  };
  AddReferenceFunc("sb_foo_func", "sb_foo");
  AddReferenceFunc("sb_bar_func", "sb_bar");
  AddReferenceFunc("sb_baz_func", "sb_baz");

  auto FuncCall = [&](const std::string& callee) {
    return create<ast::CallStatement>(Call(callee));
  };

  Func("ep_func", ast::VariableList(), ty.void_(),
       ast::StatementList{
           FuncCall("sb_foo_func"),
           FuncCall("sb_bar_func"),
           FuncCall("sb_baz_func"),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[1].resource_type);
  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].size);
  EXPECT_EQ(12u, result[1].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[2].resource_type);
  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].size);
  EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingArray) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {ty.i32(), u32_array_type(4)});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].size);
  EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingRuntimeArray) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {ty.i32(), u32_array_type(0)});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(8u, result[0].size);
  EXPECT_EQ(8u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingPadding) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {ty.vec3<f32>()});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                          {{0, ty.vec3<f32>()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(16u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, SkipReadOnly) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeReadOnlyStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, Simple) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeReadOnlyStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].size);
  EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest,
       MultipleStorageBuffers) {
  type::Struct* sb_struct_type;
  type::AccessControl* sb_control_type;
  std::tie(sb_struct_type, sb_control_type) =
      MakeReadOnlyStorageBufferTypes("sb_type", {ty.i32(), ty.u32(), ty.f32()});
  AddStorageBuffer("sb_foo", sb_control_type, 0, 0);
  AddStorageBuffer("sb_bar", sb_control_type, 0, 1);
  AddStorageBuffer("sb_baz", sb_control_type, 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    MakeStructVariableReferenceBodyFunction(
        func_name, var_name, {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
  };
  AddReferenceFunc("sb_foo_func", "sb_foo");
  AddReferenceFunc("sb_bar_func", "sb_bar");
  AddReferenceFunc("sb_baz_func", "sb_baz");

  auto FuncCall = [&](const std::string& callee) {
    return create<ast::CallStatement>(Call(callee));
  };

  Func("ep_func", ast::VariableList(), ty.void_(),
       ast::StatementList{
           FuncCall("sb_foo_func"),
           FuncCall("sb_bar_func"),
           FuncCall("sb_baz_func"),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[1].resource_type);
  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].size);
  EXPECT_EQ(12u, result[1].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[2].resource_type);
  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].size);
  EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, ContainingArray) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeReadOnlyStorageBufferTypes("foo_type", {ty.i32(), u32_array_type(4)});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].size);
  EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest,
       ContainingRuntimeArray) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeReadOnlyStorageBufferTypes("foo_type", {ty.i32(), u32_array_type(0)});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(8u, result[0].size);
  EXPECT_EQ(8u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, SkipNonReadOnly) {
  type::Struct* foo_struct_type;
  type::AccessControl* foo_control_type;
  std::tie(foo_struct_type, foo_control_type) =
      MakeStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_control_type, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction(
      "ep_func", {"sb_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, Simple) {
  auto* sampled_texture_type =
      MakeSampledTextureType(type::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", ty.f32(),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, NoSampler) {
  MakeEmptyBodyFunction(
      "ep_func", ast::DecorationList{
                     create<ast::StageDecoration>(ast::PipelineStage::kVertex),
                 });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, InFunction) {
  auto* sampled_texture_type =
      MakeSampledTextureType(type::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("foo_func", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(), {});

  MakeCallerBodyFunction(
      "ep_func", {"foo_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, UnknownEntryPoint) {
  auto* sampled_texture_type =
      MakeSampledTextureType(type::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", ty.f32(),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("foo");
  ASSERT_TRUE(inspector.has_error()) << inspector.error();
}

TEST_F(InspectorGetSamplerResourceBindingsTest, SkipsComparisonSamplers) {
  auto* depth_texture_type = MakeDepthTextureType(type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", ty.f32(),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, Simple) {
  auto* depth_texture_type = MakeDepthTextureType(type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", ty.f32(),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, NoSampler) {
  MakeEmptyBodyFunction(
      "ep_func", ast::DecorationList{
                     create<ast::StageDecoration>(ast::PipelineStage::kVertex),
                 });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, InFunction) {
  auto* depth_texture_type = MakeDepthTextureType(type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction("foo_func", "foo_texture",
                                             "foo_sampler", "foo_coords",
                                             "foo_depth", ty.f32(), {});

  MakeCallerBodyFunction(
      "ep_func", {"foo_func"},
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, UnknownEntryPoint) {
  auto* depth_texture_type = MakeDepthTextureType(type::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", ty.f32(),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("foo");
  ASSERT_TRUE(inspector.has_error()) << inspector.error();
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, SkipsSamplers) {
  auto* sampled_texture_type =
      MakeSampledTextureType(type::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", ty.f32(),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSampledTextureResourceBindingsTest, Empty) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kVertex),
             });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("foo");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetSampledTextureResourceBindingsTestWithParam, textureSample) {
  auto* sampled_texture_type = MakeSampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
  AddGlobalVariable("foo_coords", coord_type);

  MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords",
      GetBaseType(GetParam().sampled_kind),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);

  // Prove that sampled and multi-sampled bindings are accounted
  // for separately.
  auto multisampled_result =
      inspector.GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_TRUE(multisampled_result.empty());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetSampledTextureResourceBindingsTest,
    InspectorGetSampledTextureResourceBindingsTestWithParam,
    testing::Values(
        GetSampledTextureTestParams{
            type::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            type::TextureDimension::k3d,
            inspector::ResourceBinding::TextureDimension::k3d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            type::TextureDimension::kCube,
            inspector::ResourceBinding::TextureDimension::kCube,
            inspector::ResourceBinding::SampledKind::kFloat}));

TEST_P(InspectorGetSampledArrayTextureResourceBindingsTestWithParam,
       textureSample) {
  auto* sampled_texture_type = MakeSampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_array_index", ty.i32());

  MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_array_index",
      GetBaseType(GetParam().sampled_kind),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture,
            result[0].resource_type);
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
            type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            type::TextureDimension::kCubeArray,
            inspector::ResourceBinding::TextureDimension::kCubeArray,
            inspector::ResourceBinding::SampledKind::kFloat}));

TEST_P(InspectorGetMultisampledTextureResourceBindingsTestWithParam,
       textureLoad) {
  auto* multisampled_texture_type = MakeMultisampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddMultisampledTexture("foo_texture", multisampled_texture_type, 0, 0);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.i32());
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_sample_index", ty.i32());

  Func("ep", ast::VariableList(), ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(Call("textureLoad", "foo_texture",
                                           "foo_coords", "foo_sample_index")),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kMultisampledTexture,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);

  // Prove that sampled and multi-sampled bindings are accounted
  // for separately.
  auto single_sampled_result =
      inspector.GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_TRUE(single_sampled_result.empty());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetMultisampledTextureResourceBindingsTest,
    InspectorGetMultisampledTextureResourceBindingsTestWithParam,
    testing::Values(
        GetMultisampledTextureTestParams{
            type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kUInt}));

TEST_F(InspectorGetMultisampledArrayTextureResourceBindingsTest, Empty) {
  MakeEmptyBodyFunction(
      "foo", ast::DecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kVertex),
             });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("foo");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam,
       DISABLED_textureSample) {
  auto* multisampled_texture_type = MakeMultisampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddMultisampledTexture("foo_texture", multisampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_array_index", ty.i32());

  MakeSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_array_index",
      GetBaseType(GetParam().sampled_kind),
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kMultisampledTexture,
            result[0].resource_type);
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
            type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kUInt}));

TEST_F(InspectorGetStorageTextureResourceBindingsTest, Empty) {
  MakeEmptyBodyFunction(
      "ep", ast::DecorationList{
                create<ast::StageDecoration>(ast::PipelineStage::kVertex),
            });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  EXPECT_EQ(0u, result.size());

  result = inspector.GetWriteOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetStorageTextureResourceBindingsTestWithParam, Simple) {
  bool read_only;
  DimensionParams dim_params;
  ImageFormatParams format_params;
  std::tie(read_only, dim_params, format_params) = GetParam();

  type::TextureDimension dim;
  ResourceBinding::TextureDimension expected_dim;
  std::tie(dim, expected_dim) = dim_params;

  type::ImageFormat format;
  ResourceBinding::ImageFormat expected_format;
  ResourceBinding::SampledKind expected_kind;
  std::tie(format, expected_format, expected_kind) = format_params;

  type::StorageTexture* st_type;
  type::Type* st_subtype;
  type::AccessControl* ac;
  std::tie(st_type, st_subtype, ac) =
      MakeStorageTextureTypes(dim, format, read_only);
  AddStorageTexture("st_var", ac, 0, 0);

  type::Type* dim_type = nullptr;
  switch (dim) {
    case type::TextureDimension::k1d:
      dim_type = ty.i32();
      break;
    case type::TextureDimension::k2d:
    case type::TextureDimension::k2dArray:
      dim_type = ty.vec2<i32>();
      break;
    case type::TextureDimension::k3d:
      dim_type = ty.vec3<i32>();
      break;
    default:
      break;
  }

  ASSERT_FALSE(dim_type == nullptr);

  MakeStorageTextureBodyFunction(
      "ep", "st_var", dim_type,
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  Inspector& inspector = Build();

  auto result =
      read_only ? inspector.GetReadOnlyStorageTextureResourceBindings("ep")
                : inspector.GetWriteOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(read_only ? ResourceBinding::ResourceType::kReadOnlyStorageTexture
                      : ResourceBinding::ResourceType::kWriteOnlyStorageTexture,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(expected_dim, result[0].dim);
  EXPECT_EQ(expected_format, result[0].image_format);
  EXPECT_EQ(expected_kind, result[0].sampled_kind);

  result = read_only
               ? inspector.GetWriteOnlyStorageTextureResourceBindings("ep")
               : inspector.GetReadOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetStorageTextureResourceBindingsTest,
    InspectorGetStorageTextureResourceBindingsTestWithParam,
    testing::Combine(
        testing::Bool(),
        testing::Values(
            std::make_tuple(type::TextureDimension::k1d,
                            ResourceBinding::TextureDimension::k1d),
            std::make_tuple(type::TextureDimension::k2d,
                            ResourceBinding::TextureDimension::k2d),
            std::make_tuple(type::TextureDimension::k2dArray,
                            ResourceBinding::TextureDimension::k2dArray),
            std::make_tuple(type::TextureDimension::k3d,
                            ResourceBinding::TextureDimension::k3d)),
        testing::Values(
            std::make_tuple(type::ImageFormat::kR8Uint,
                            ResourceBinding::ImageFormat::kR8Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kR16Uint,
                            ResourceBinding::ImageFormat::kR16Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kRg8Uint,
                            ResourceBinding::ImageFormat::kRg8Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kR32Uint,
                            ResourceBinding::ImageFormat::kR32Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kRg16Uint,
                            ResourceBinding::ImageFormat::kRg16Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kRgba8Uint,
                            ResourceBinding::ImageFormat::kRgba8Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kRg32Uint,
                            ResourceBinding::ImageFormat::kRg32Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kRgba16Uint,
                            ResourceBinding::ImageFormat::kRgba16Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kRgba32Uint,
                            ResourceBinding::ImageFormat::kRgba32Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(type::ImageFormat::kR8Sint,
                            ResourceBinding::ImageFormat::kR8Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kR16Sint,
                            ResourceBinding::ImageFormat::kR16Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kRg8Sint,
                            ResourceBinding::ImageFormat::kRg8Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kR32Sint,
                            ResourceBinding::ImageFormat::kR32Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kRg16Sint,
                            ResourceBinding::ImageFormat::kRg16Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kRgba8Sint,
                            ResourceBinding::ImageFormat::kRgba8Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kRg32Sint,
                            ResourceBinding::ImageFormat::kRg32Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kRgba16Sint,
                            ResourceBinding::ImageFormat::kRgba16Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kRgba32Sint,
                            ResourceBinding::ImageFormat::kRgba32Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(type::ImageFormat::kR8Unorm,
                            ResourceBinding::ImageFormat::kR8Unorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRg8Unorm,
                            ResourceBinding::ImageFormat::kRg8Unorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRgba8Unorm,
                            ResourceBinding::ImageFormat::kRgba8Unorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRgba8UnormSrgb,
                            ResourceBinding::ImageFormat::kRgba8UnormSrgb,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kBgra8Unorm,
                            ResourceBinding::ImageFormat::kBgra8Unorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kBgra8UnormSrgb,
                            ResourceBinding::ImageFormat::kBgra8UnormSrgb,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRgb10A2Unorm,
                            ResourceBinding::ImageFormat::kRgb10A2Unorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kR8Snorm,
                            ResourceBinding::ImageFormat::kR8Snorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRg8Snorm,
                            ResourceBinding::ImageFormat::kRg8Snorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRgba8Snorm,
                            ResourceBinding::ImageFormat::kRgba8Snorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kR16Float,
                            ResourceBinding::ImageFormat::kR16Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kR32Float,
                            ResourceBinding::ImageFormat::kR32Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRg16Float,
                            ResourceBinding::ImageFormat::kRg16Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRg11B10Float,
                            ResourceBinding::ImageFormat::kRg11B10Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRg32Float,
                            ResourceBinding::ImageFormat::kRg32Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRgba16Float,
                            ResourceBinding::ImageFormat::kRgba16Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(type::ImageFormat::kRgba32Float,
                            ResourceBinding::ImageFormat::kRgba32Float,
                            ResourceBinding::SampledKind::kFloat))));

TEST_P(InspectorGetDepthTextureResourceBindingsTestWithParam,
       textureDimensions) {
  auto* depth_texture_type = MakeDepthTextureType(GetParam().type_dim);
  AddDepthTexture("dt", depth_texture_type, 0, 0);
  AddGlobalVariable("dt_level", ty.i32());

  Func("ep", ast::VariableList(), ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(
               Call("textureDimensions", "dt", "dt_level")),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetDepthTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetDepthTextureResourceBindingsTest,
    InspectorGetDepthTextureResourceBindingsTestWithParam,
    testing::Values(
        GetDepthTextureTestParams{
            type::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d},
        GetDepthTextureTestParams{
            type::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray},
        GetDepthTextureTestParams{
            type::TextureDimension::kCube,
            inspector::ResourceBinding::TextureDimension::kCube},
        GetDepthTextureTestParams{
            type::TextureDimension::kCubeArray,
            inspector::ResourceBinding::TextureDimension::kCubeArray}));

}  // namespace
}  // namespace inspector
}  // namespace tint
