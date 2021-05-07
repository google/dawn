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

#ifndef SRC_SEM_STRUCT_H_
#define SRC_SEM_STRUCT_H_

#include <stdint.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "src/ast/storage_class.h"
#include "src/ast/struct.h"
#include "src/sem/node.h"
#include "src/sem/type.h"
#include "src/symbol.h"

namespace tint {

// Forward declarations
namespace ast {
class StructMember;
}  // namespace ast

namespace sem {

// Forward declarations
class StructMember;
class Type;

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
class Struct : public Castable<Struct, Type> {
 public:
  /// Constructor
  /// @param declaration the AST structure declaration
  /// @param members the structure members
  /// @param align the byte alignment of the structure
  /// @param size the byte size of the structure
  /// @param size_no_padding size of the members without the end of structure
  /// alignment padding
  Struct(const ast::Struct* declaration,
         StructMemberList members,
         uint32_t align,
         uint32_t size,
         uint32_t size_no_padding);

  /// Destructor
  ~Struct() override;

  /// @returns the struct
  const ast::Struct* Declaration() const { return declaration_; }

  /// @returns the members of the structure
  const StructMemberList& Members() const { return members_; }

  /// @param name the member name to look for
  /// @returns the member with the given name, or nullptr if it was not found.
  const StructMember* FindMember(Symbol name) const;

  /// @returns the byte alignment of the structure
  /// @note this may differ from the alignment of a structure member of this
  /// structure type, if the member is annotated with the `[[align(n)]]`
  /// decoration.
  uint32_t Align() const { return align_; }

  /// @returns the byte size of the structure
  /// @note this may differ from the size of a structure member of this
  /// structure type, if the member is annotated with the `[[size(n)]]`
  /// decoration.
  uint32_t Size() const { return size_; }

  /// @returns the byte size of the members without the end of structure
  /// alignment padding
  uint32_t SizeNoPadding() const { return size_no_padding_; }

  /// Adds the StorageClass usage to the structure.
  /// @param usage the storage usage
  void AddUsage(ast::StorageClass usage) {
    storage_class_usage_.emplace(usage);
  }

  /// @returns the set of storage class uses of this structure
  const std::unordered_set<ast::StorageClass>& StorageClassUsage() const {
    return storage_class_usage_;
  }

  /// @param usage the ast::StorageClass usage type to query
  /// @returns true iff this structure has been used as the given storage class
  bool UsedAs(ast::StorageClass usage) const {
    return storage_class_usage_.count(usage) > 0;
  }

  /// @returns true iff this structure has been used by storage class that's
  /// host-shareable.
  bool IsHostShareable() const {
    for (auto sc : storage_class_usage_) {
      if (ast::IsHostShareable(sc)) {
        return true;
      }
    }
    return false;
  }

  /// Adds the pipeline stage usage to the structure.
  /// @param usage the storage usage
  void AddUsage(PipelineStageUsage usage) {
    pipeline_stage_uses_.emplace(usage);
  }

  /// @returns the set of entry point uses of this structure
  const std::unordered_set<PipelineStageUsage>& PipelineStageUses() const {
    return pipeline_stage_uses_;
  }

  /// @returns true if the struct has a block decoration
  bool IsBlockDecorated() const { return declaration_->IsBlockDecorated(); }

  /// @returns the name for the type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  uint64_t LargestMemberBaseAlignment(MemoryLayout mem_layout) const;

  ast::Struct const* const declaration_;
  StructMemberList const members_;
  uint32_t const align_;
  uint32_t const size_;
  uint32_t const size_no_padding_;
  std::unordered_set<ast::StorageClass> storage_class_usage_;
  std::unordered_set<PipelineStageUsage> pipeline_stage_uses_;
};

/// StructMember holds the semantic information for structure members.
class StructMember : public Castable<StructMember, Node> {
 public:
  /// Constructor
  /// @param declaration the AST declaration node
  /// @param type the type of the member
  /// @param index the index of the member in the structure
  /// @param offset the byte offset from the base of the structure
  /// @param align the byte alignment of the member
  /// @param size the byte size of the member
  StructMember(ast::StructMember* declaration,
               sem::Type* type,
               uint32_t index,
               uint32_t offset,
               uint32_t align,
               uint32_t size);

  /// Destructor
  ~StructMember() override;

  /// @returns the AST declaration node
  ast::StructMember* Declaration() const { return declaration_; }

  /// @returns the type of the member
  sem::Type* Type() const { return type_; }

  /// @returns the member index
  uint32_t Index() const { return index_; }

  /// @returns byte offset from base of structure
  uint32_t Offset() const { return offset_; }

  /// @returns the alignment of the member in bytes
  uint32_t Align() const { return align_; }

  /// @returns byte size
  uint32_t Size() const { return size_; }

 private:
  ast::StructMember* const declaration_;
  sem::Type* const type_;
  uint32_t const index_;
  uint32_t const offset_;
  uint32_t const align_;
  uint32_t const size_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_STRUCT_H_
