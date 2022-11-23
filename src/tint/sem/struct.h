// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_STRUCT_H_
#define SRC_TINT_SEM_STRUCT_H_

#include <stdint.h>

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "src/tint/ast/address_space.h"
#include "src/tint/ast/struct.h"
#include "src/tint/sem/node.h"
#include "src/tint/sem/type.h"
#include "src/tint/symbol.h"

// Forward declarations
namespace tint::ast {
class StructMember;
}  // namespace tint::ast
namespace tint::sem {
class StructMember;
class Type;
}  // namespace tint::sem

namespace tint::sem {

/// A vector of StructMember pointers.
using StructMemberList = std::vector<const StructMember*>;

/// Metadata to capture how a structure is used in a shader module.
enum class PipelineStageUsage {
    kVertexInput,
    kVertexOutput,
    kFragmentInput,
    kFragmentOutput,
    kComputeInput,
    kComputeOutput,
};

/// Struct holds the semantic information for structures.
class Struct final : public Castable<Struct, Type> {
  public:
    /// Constructor
    /// @param declaration the AST structure declaration
    /// @param name the name of the structure
    /// @param members the structure members
    /// @param align the byte alignment of the structure
    /// @param size the byte size of the structure
    /// @param size_no_padding size of the members without the end of structure
    /// alignment padding
    Struct(const ast::Struct* declaration,
           Symbol name,
           StructMemberList members,
           uint32_t align,
           uint32_t size,
           uint32_t size_no_padding);

    /// Destructor
    ~Struct() override;

    /// @returns a hash of the type.
    size_t Hash() const override;

    /// @param other the other type to compare against
    /// @returns true if the this type is equal to the given type
    bool Equals(const Type& other) const override;

    /// @returns the struct
    const ast::Struct* Declaration() const { return declaration_; }

    /// @returns the name of the structure
    Symbol Name() const { return name_; }

    /// @returns the members of the structure
    const StructMemberList& Members() const { return members_; }

    /// @param name the member name to look for
    /// @returns the member with the given name, or nullptr if it was not found.
    const StructMember* FindMember(Symbol name) const;

    /// @returns the byte alignment of the structure
    /// @note this may differ from the alignment of a structure member of this
    /// structure type, if the member is annotated with the `@align(n)`
    /// attribute.
    uint32_t Align() const override;

    /// @returns the byte size of the structure
    /// @note this may differ from the size of a structure member of this
    /// structure type, if the member is annotated with the `@size(n)`
    /// attribute.
    uint32_t Size() const override;

    /// @returns the byte size of the members without the end of structure
    /// alignment padding
    uint32_t SizeNoPadding() const { return size_no_padding_; }

    /// Adds the AddressSpace usage to the structure.
    /// @param usage the storage usage
    void AddUsage(ast::AddressSpace usage) { address_space_usage_.emplace(usage); }

    /// @returns the set of address space uses of this structure
    const std::unordered_set<ast::AddressSpace>& AddressSpaceUsage() const {
        return address_space_usage_;
    }

    /// @param usage the ast::AddressSpace usage type to query
    /// @returns true iff this structure has been used as the given address space
    bool UsedAs(ast::AddressSpace usage) const { return address_space_usage_.count(usage) > 0; }

    /// @returns true iff this structure has been used by address space that's
    /// host-shareable.
    bool IsHostShareable() const {
        for (auto sc : address_space_usage_) {
            if (ast::IsHostShareable(sc)) {
                return true;
            }
        }
        return false;
    }

    /// Adds the pipeline stage usage to the structure.
    /// @param usage the storage usage
    void AddUsage(PipelineStageUsage usage) { pipeline_stage_uses_.emplace(usage); }

    /// @returns the set of entry point uses of this structure
    const std::unordered_set<PipelineStageUsage>& PipelineStageUses() const {
        return pipeline_stage_uses_;
    }

    /// @param symbols the program's symbol table
    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const SymbolTable& symbols) const override;

    /// @param symbols the program's symbol table
    /// @returns a multiline string that describes the layout of this struct,
    /// including size and alignment information.
    std::string Layout(const tint::SymbolTable& symbols) const;

    /// @param concrete the conversion-rank ordered concrete versions of this abstract structure.
    void SetConcreteTypes(utils::VectorRef<const Struct*> concrete) { concrete_types_ = concrete; }

    /// @returns the conversion-rank ordered concrete versions of this abstract structure, or an
    /// empty vector if this structure is not abstract.
    /// @note only structures returned by builtins may be abstract (e.g. modf, frexp)
    const utils::Vector<const Struct*, 2>& ConcreteTypes() const { return concrete_types_; }

  private:
    ast::Struct const* const declaration_;
    const Symbol name_;
    const StructMemberList members_;
    const uint32_t align_;
    const uint32_t size_;
    const uint32_t size_no_padding_;
    std::unordered_set<ast::AddressSpace> address_space_usage_;
    std::unordered_set<PipelineStageUsage> pipeline_stage_uses_;
    utils::Vector<const Struct*, 2> concrete_types_;
};

/// StructMember holds the semantic information for structure members.
class StructMember final : public Castable<StructMember, Node> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param name the name of the structure
    /// @param type the type of the member
    /// @param index the index of the member in the structure
    /// @param offset the byte offset from the base of the structure
    /// @param align the byte alignment of the member
    /// @param size the byte size of the member
    /// @param location the location attribute, if present
    StructMember(const ast::StructMember* declaration,
                 Symbol name,
                 const sem::Type* type,
                 uint32_t index,
                 uint32_t offset,
                 uint32_t align,
                 uint32_t size,
                 std::optional<uint32_t> location);

    /// Destructor
    ~StructMember() override;

    /// @returns the AST declaration node
    const ast::StructMember* Declaration() const { return declaration_; }

    /// @returns the name of the structure member
    Symbol Name() const { return name_; }

    /// Sets the owning structure to `s`
    /// @param s the new structure owner
    void SetStruct(const sem::Struct* s) { struct_ = s; }

    /// @returns the structure that owns this member
    const sem::Struct* Struct() const { return struct_; }

    /// @returns the type of the member
    const sem::Type* Type() const { return type_; }

    /// @returns the member index
    uint32_t Index() const { return index_; }

    /// @returns byte offset from base of structure
    uint32_t Offset() const { return offset_; }

    /// @returns the alignment of the member in bytes
    uint32_t Align() const { return align_; }

    /// @returns byte size
    uint32_t Size() const { return size_; }

    /// @returns the location, if set
    std::optional<uint32_t> Location() const { return location_; }

  private:
    const ast::StructMember* const declaration_;
    const Symbol name_;
    const sem::Struct* struct_;
    const sem::Type* type_;
    const uint32_t index_;
    const uint32_t offset_;
    const uint32_t align_;
    const uint32_t size_;
    const std::optional<uint32_t> location_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_STRUCT_H_
