// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_INSPECTOR_TEST_INSPECTOR_BUILDER_H_
#define SRC_TINT_INSPECTOR_TEST_INSPECTOR_BUILDER_H_

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/variable.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "tint/tint.h"

namespace tint::inspector {

/// Utility class for building programs in inspector tests
class InspectorBuilder : public ProgramBuilder {
  public:
    InspectorBuilder();
    ~InspectorBuilder() override;

    /// Generates an empty function
    /// @param name name of the function created
    /// @param attributes the function attributes
    void MakeEmptyBodyFunction(std::string name,
                               utils::VectorRef<const ast::Attribute*> attributes);

    /// Generates a function that calls other functions
    /// @param caller name of the function created
    /// @param callees names of the functions to be called
    /// @param attributes the function attributes
    void MakeCallerBodyFunction(std::string caller,
                                utils::VectorRef<std::string> callees,
                                utils::VectorRef<const ast::Attribute*> attributes);

    /// InOutInfo is a tuple of name and location for a structure member
    using InOutInfo = std::tuple<std::string, uint32_t>;

    /// Generates a struct that contains user-defined IO members
    /// @param name the name of the generated struct
    /// @param inout_vars tuples of {name, loc} that will be the struct members
    /// @returns a structure object
    const ast::Struct* MakeInOutStruct(std::string name, utils::VectorRef<InOutInfo> inout_vars);

    // TODO(crbug.com/tint/697): Remove this.
    /// Add In/Out variables to the global variables
    /// @param inout_vars tuples of {in, out} that will be added as entries to the
    ///                   global variables
    void AddInOutVariables(utils::VectorRef<std::tuple<std::string, std::string>> inout_vars);

    // TODO(crbug.com/tint/697): Remove this.
    /// Generates a function that references in/out variables
    /// @param name name of the function created
    /// @param inout_vars tuples of {in, out} that will be converted into out = in
    ///                   calls in the function body
    /// @param attributes the function attributes
    void MakeInOutVariableBodyFunction(
        std::string name,
        utils::VectorRef<std::tuple<std::string, std::string>> inout_vars,
        utils::VectorRef<const ast::Attribute*> attributes);

    // TODO(crbug.com/tint/697): Remove this.
    /// Generates a function that references in/out variables and calls another
    /// function.
    /// @param caller name of the function created
    /// @param callee name of the function to be called
    /// @param inout_vars tuples of {in, out} that will be converted into out = in
    ///                   calls in the function body
    /// @param attributes the function attributes
    /// @returns a function object
    const ast::Function* MakeInOutVariableCallerBodyFunction(
        std::string caller,
        std::string callee,
        utils::VectorRef<std::tuple<std::string, std::string>> inout_vars,
        utils::VectorRef<const ast::Attribute*> attributes);

    /// Generates a function that references module-scoped, plain-typed constant
    /// or variable.
    /// @param func name of the function created
    /// @param var name of the constant to be reference
    /// @param type type of the const being referenced
    /// @param attributes the function attributes
    /// @returns a function object
    const ast::Function* MakePlainGlobalReferenceBodyFunction(
        std::string func,
        std::string var,
        ast::Type type,
        utils::VectorRef<const ast::Attribute*> attributes);

    /// @param vec Vector of StageVariable to be searched
    /// @param name Name to be searching for
    /// @returns true if name is in vec, otherwise false
    bool ContainsName(utils::VectorRef<StageVariable> vec, const std::string& name);

    /// Builds a string for accessing a member in a generated struct
    /// @param idx index of member
    /// @param type type of member
    /// @returns a string for the member
    std::string StructMemberName(size_t idx, ast::Type type);

    /// Generates a struct type
    /// @param name name for the type
    /// @param member_types a vector of member types
    /// @returns a struct type
    const ast::Struct* MakeStructType(const std::string& name,
                                      utils::VectorRef<ast::Type> member_types);

    /// Generates a struct type from a list of member nodes.
    /// @param name name for the struct type
    /// @param members a vector of members
    /// @returns a struct type
    const ast::Struct* MakeStructTypeFromMembers(
        const std::string& name,
        utils::VectorRef<const ast::StructMember*> members);

    /// Generates a struct member with a specified index and type.
    /// @param index index of the field within the struct
    /// @param type the type of the member field
    /// @param attributes a list of attributes to apply to the member field
    /// @returns a struct member
    const ast::StructMember* MakeStructMember(size_t index,
                                              ast::Type type,
                                              utils::VectorRef<const ast::Attribute*> attributes);

    /// Generates types appropriate for using in an uniform buffer
    /// @param name name for the type
    /// @param member_types a vector of member types
    /// @returns a struct type that has the layout for an uniform buffer.
    const ast::Struct* MakeUniformBufferType(const std::string& name,
                                             utils::VectorRef<ast::Type> member_types);

    /// Generates types appropriate for using in a storage buffer
    /// @param name name for the type
    /// @param member_types a vector of member types
    /// @returns a function that returns the created structure.
    std::function<ast::Type()> MakeStorageBufferTypes(const std::string& name,
                                                      utils::VectorRef<ast::Type> member_types);

    /// Adds an uniform buffer variable to the program
    /// @param name the name of the variable
    /// @param type the type to use
    /// @param group the binding/group/ to use for the uniform buffer
    /// @param binding the binding number to use for the uniform buffer
    void AddUniformBuffer(const std::string& name,
                          ast::Type type,
                          uint32_t group,
                          uint32_t binding);

    /// Adds a workgroup storage variable to the program
    /// @param name the name of the variable
    /// @param type the type of the variable
    void AddWorkgroupStorage(const std::string& name, ast::Type type);

    /// Adds a storage buffer variable to the program
    /// @param name the name of the variable
    /// @param type the type to use
    /// @param access the storage buffer access control
    /// @param group the binding/group to use for the storage buffer
    /// @param binding the binding number to use for the storage buffer
    void AddStorageBuffer(const std::string& name,
                          ast::Type type,
                          builtin::Access access,
                          uint32_t group,
                          uint32_t binding);

    /// MemberInfo is a tuple of member index and type.
    using MemberInfo = std::tuple<size_t, ast::Type>;

    /// Generates a function that references a specific struct variable
    /// @param func_name name of the function created
    /// @param struct_name name of the struct variabler to be accessed
    /// @param members list of members to access, by index and type
    void MakeStructVariableReferenceBodyFunction(std::string func_name,
                                                 std::string struct_name,
                                                 utils::VectorRef<MemberInfo> members);

    /// Adds a regular sampler variable to the program
    /// @param name the name of the variable
    /// @param group the binding/group to use for the storage buffer
    /// @param binding the binding number to use for the storage buffer
    void AddSampler(const std::string& name, uint32_t group, uint32_t binding);

    /// Adds a comparison sampler variable to the program
    /// @param name the name of the variable
    /// @param group the binding/group to use for the storage buffer
    /// @param binding the binding number to use for the storage buffer
    void AddComparisonSampler(const std::string& name, uint32_t group, uint32_t binding);

    /// Adds a sampler or texture variable to the program
    /// @param name the name of the variable
    /// @param type the type to use
    /// @param group the binding/group to use for the resource
    /// @param binding the binding number to use for the resource
    void AddResource(const std::string& name, ast::Type type, uint32_t group, uint32_t binding);

    /// Add a module scope private variable to the progames
    /// @param name the name of the variable
    /// @param type the type to use
    void AddGlobalVariable(const std::string& name, ast::Type type);

    /// Generates a function that references a specific sampler variable
    /// @param func_name name of the function created
    /// @param texture_name name of the texture to be sampled
    /// @param sampler_name name of the sampler to use
    /// @param coords_name name of the coords variable to use
    /// @param base_type sampler base type
    /// @param attributes the function attributes
    /// @returns a function that references all of the values specified
    const ast::Function* MakeSamplerReferenceBodyFunction(
        const std::string& func_name,
        const std::string& texture_name,
        const std::string& sampler_name,
        const std::string& coords_name,
        ast::Type base_type,
        utils::VectorRef<const ast::Attribute*> attributes);

    /// Generates a function that references a specific sampler variable
    /// @param func_name name of the function created
    /// @param texture_name name of the texture to be sampled
    /// @param sampler_name name of the sampler to use
    /// @param coords_name name of the coords variable to use
    /// @param array_index name of the array index variable to use
    /// @param base_type sampler base type
    /// @param attributes the function attributes
    /// @returns a function that references all of the values specified
    const ast::Function* MakeSamplerReferenceBodyFunction(
        const std::string& func_name,
        const std::string& texture_name,
        const std::string& sampler_name,
        const std::string& coords_name,
        const std::string& array_index,
        ast::Type base_type,
        utils::VectorRef<const ast::Attribute*> attributes);

    /// Generates a function that references a specific comparison sampler
    /// variable.
    /// @param func_name name of the function created
    /// @param texture_name name of the depth texture to  use
    /// @param sampler_name name of the sampler to use
    /// @param coords_name name of the coords variable to use
    /// @param depth_name name of the depth reference to use
    /// @param base_type sampler base type
    /// @param attributes the function attributes
    /// @returns a function that references all of the values specified
    const ast::Function* MakeComparisonSamplerReferenceBodyFunction(
        const std::string& func_name,
        const std::string& texture_name,
        const std::string& sampler_name,
        const std::string& coords_name,
        const std::string& depth_name,
        ast::Type base_type,
        utils::VectorRef<const ast::Attribute*> attributes);

    /// Gets an appropriate type for the data in a given texture type.
    /// @param sampled_kind type of in the texture
    /// @returns a pointer to a type appropriate for the coord param
    ast::Type GetBaseType(ResourceBinding::SampledKind sampled_kind);

    /// Gets an appropriate type for the coords parameter depending the the
    /// dimensionality of the texture being sampled.
    /// @param dim dimensionality of the texture being sampled
    /// @param scalar the scalar type
    /// @returns a pointer to a type appropriate for the coord param
    ast::Type GetCoordsType(type::TextureDimension dim, ast::Type scalar);

    /// Generates appropriate types for a Read-Only StorageTexture
    /// @param dim the texture dimension of the storage texture
    /// @param format the texel format of the storage texture
    /// @returns the storage texture type
    ast::Type MakeStorageTextureTypes(type::TextureDimension dim, builtin::TexelFormat format);

    /// Adds a storage texture variable to the program
    /// @param name the name of the variable
    /// @param type the type to use
    /// @param group the binding/group to use for the sampled texture
    /// @param binding the binding57 number to use for the sampled texture
    void AddStorageTexture(const std::string& name,
                           ast::Type type,
                           uint32_t group,
                           uint32_t binding);

    /// Generates a function that references a storage texture variable.
    /// @param func_name name of the function created
    /// @param st_name name of the storage texture to use
    /// @param dim_type type expected by textureDimensons to return
    /// @param attributes the function attributes
    /// @returns a function that references all of the values specified
    const ast::Function* MakeStorageTextureBodyFunction(
        const std::string& func_name,
        const std::string& st_name,
        ast::Type dim_type,
        utils::VectorRef<const ast::Attribute*> attributes);

    /// Get a generator function that returns a type appropriate for a stage
    /// variable with the given combination of component and composition type.
    /// @param component component type of the stage variable
    /// @param composition composition type of the stage variable
    /// @returns a generator function for the stage variable's type.
    std::function<ast::Type()> GetTypeFunction(ComponentType component,
                                               CompositionType composition);

    /// Build the Program given all of the previous methods called and return an
    /// Inspector for it.
    /// Should only be called once per test.
    /// @returns a reference to the Inspector for the built Program.
    Inspector& Build();

  protected:
    /// Program built by this builder.
    std::unique_ptr<Program> program_;
    /// Inspector for |program_|
    std::unique_ptr<Inspector> inspector_;
};

}  // namespace tint::inspector

#endif  // SRC_TINT_INSPECTOR_TEST_INSPECTOR_BUILDER_H_
