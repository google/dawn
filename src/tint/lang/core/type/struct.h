// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_TYPE_STRUCT_H_
#define SRC_TINT_LANG_CORE_TYPE_STRUCT_H_

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/interpolation.h"
#include "src/tint/lang/core/type/node.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations
namespace tint::type {
class StructMember;
}  // namespace tint::type

namespace tint::type {

/// Metadata to capture how a structure is used in a shader module.
enum class PipelineStageUsage {
    kVertexInput,
    kVertexOutput,
    kFragmentInput,
    kFragmentOutput,
    kComputeInput,
    kComputeOutput,
};

enum StructFlag {
    /// The structure is a block-decorated structure (for SPIR-V or GLSL).
    kBlock,
};

/// An alias to tint::EnumSet<StructFlag>
using StructFlags = tint::EnumSet<StructFlag>;

/// Struct holds the Type information for structures.
class Struct : public Castable<Struct, Type> {
  public:
    /// Constructor
    /// @param name the name of the structure
    /// @param members the structure members
    /// @param align the byte alignment of the structure
    /// @param size the byte size of the structure
    /// @param size_no_padding size of the members without the end of structure
    /// alignment padding
    Struct(Symbol name,
           VectorRef<const StructMember*> members,
           uint32_t align,
           uint32_t size,
           uint32_t size_no_padding);

    /// Destructor
    ~Struct() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the name of the structure
    Symbol Name() const { return name_; }

    /// Renames the structure
    /// @param name the new name for the structure
    void SetName(Symbol name) { name_ = name; }

    /// @returns the members of the structure
    VectorRef<const StructMember*> Members() const { return members_; }

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

    /// @returns the structure flags
    type::StructFlags StructFlags() const { return struct_flags_; }

    /// Set a structure flag.
    /// @param flag the flag to set
    void SetStructFlag(StructFlag flag) { struct_flags_.Add(flag); }

    /// Adds the AddressSpace usage to the structure.
    /// @param usage the storage usage
    void AddUsage(core::AddressSpace usage) { address_space_usage_.emplace(usage); }

    /// @returns the set of address space uses of this structure
    const std::unordered_set<core::AddressSpace>& AddressSpaceUsage() const {
        return address_space_usage_;
    }

    /// @param usage the AddressSpace usage type to query
    /// @returns true iff this structure has been used as the given address space
    bool UsedAs(core::AddressSpace usage) const { return address_space_usage_.count(usage) > 0; }

    /// @returns true iff this structure has been used by address space that's
    /// host-shareable.
    bool IsHostShareable() const {
        for (auto sc : address_space_usage_) {
            if (core::IsHostShareable(sc)) {
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

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @returns a multiline string that describes the layout of this struct,
    /// including size and alignment information.
    std::string Layout() const;

    /// @param concrete the conversion-rank ordered concrete versions of this abstract structure.
    void SetConcreteTypes(VectorRef<const Struct*> concrete) { concrete_types_ = concrete; }

    /// @returns the conversion-rank ordered concrete versions of this abstract structure, or an
    /// empty vector if this structure is not abstract.
    /// @note only structures returned by builtins may be abstract (e.g. modf, frexp)
    VectorRef<const Struct*> ConcreteTypes() const { return concrete_types_; }

    /// @copydoc Type::Elements
    TypeAndCount Elements(const Type* type_if_invalid = nullptr,
                          uint32_t count_if_invalid = 0) const override;

    /// @copydoc Type::Element
    const Type* Element(uint32_t index) const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    Struct* Clone(CloneContext& ctx) const override;

  private:
    Symbol name_;
    const tint::Vector<const StructMember*, 4> members_;
    const uint32_t align_;
    const uint32_t size_;
    const uint32_t size_no_padding_;
    type::StructFlags struct_flags_;
    std::unordered_set<core::AddressSpace> address_space_usage_;
    std::unordered_set<PipelineStageUsage> pipeline_stage_uses_;
    tint::Vector<const Struct*, 2> concrete_types_;
};

/// Attributes that can be applied to the StructMember
struct StructMemberAttributes {
    /// The value of a `@location` attribute
    std::optional<uint32_t> location;
    /// The value of a `@index` attribute
    std::optional<uint32_t> index;
    /// The value of a `@builtin` attribute
    std::optional<core::BuiltinValue> builtin;
    /// The values of a `@interpolate` attribute
    std::optional<core::Interpolation> interpolation;
    /// True if the member was annotated with `@invariant`
    bool invariant = false;
};

/// StructMember holds the type information for structure members.
class StructMember : public Castable<StructMember, Node> {
  public:
    /// Constructor
    /// @param name the name of the structure member
    /// @param type the type of the member
    /// @param index the index of the member in the structure
    /// @param offset the byte offset from the base of the structure
    /// @param align the byte alignment of the member
    /// @param size the byte size of the member
    /// @param attributes the optional attributes
    StructMember(Symbol name,
                 const type::Type* type,
                 uint32_t index,
                 uint32_t offset,
                 uint32_t align,
                 uint32_t size,
                 const StructMemberAttributes& attributes);

    /// Destructor
    ~StructMember() override;

    /// @returns the name of the structure member
    Symbol Name() const { return name_; }

    /// Sets the owning structure to `s`
    /// @param s the new structure owner
    void SetStruct(const Struct* s) { struct_ = s; }

    /// @returns the structure that owns this member
    const type::Struct* Struct() const { return struct_; }

    /// @returns the type of the member
    const type::Type* Type() const { return type_; }

    /// @returns the member index
    uint32_t Index() const { return index_; }

    /// @returns byte offset from base of structure
    uint32_t Offset() const { return offset_; }

    /// @returns the alignment of the member in bytes
    uint32_t Align() const { return align_; }

    /// @returns byte size
    uint32_t Size() const { return size_; }

    /// @returns the optional attributes
    const StructMemberAttributes& Attributes() const { return attributes_; }

    /// Set the attributes of the struct member.
    /// @param attributes the new attributes
    void SetAttributes(StructMemberAttributes&& attributes) { attributes_ = std::move(attributes); }

    /// @param ctx the clone context
    /// @returns a clone of this struct member
    StructMember* Clone(CloneContext& ctx) const;

  private:
    const Symbol name_;
    const type::Struct* struct_;
    const type::Type* type_;
    const uint32_t index_;
    const uint32_t offset_;
    const uint32_t align_;
    const uint32_t size_;
    StructMemberAttributes attributes_;
};

}  // namespace tint::type

#endif  // SRC_TINT_LANG_CORE_TYPE_STRUCT_H_
