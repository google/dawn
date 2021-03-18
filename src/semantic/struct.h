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

#ifndef SRC_SEMANTIC_STRUCT_H_
#define SRC_SEMANTIC_STRUCT_H_

#include <stdint.h>

#include <unordered_set>
#include <vector>

#include "src/ast/storage_class.h"
#include "src/semantic/node.h"

namespace tint {

// Forward declarations
namespace ast {
class StructMember;
}  // namespace ast
namespace type {
class Struct;
}  // namespace type

namespace semantic {

class StructMember;

/// A vector of StructMember pointers.
using StructMemberList = std::vector<StructMember*>;

/// Struct holds the semantic information for structures.
class Struct : public Castable<Struct, Node> {
 public:
  /// Constructor
  /// @param type the structure type
  /// @param members the structure members
  /// @param align the byte alignment of the structure
  /// @param size the byte size of the structure
  /// @param size_no_padding size of the members without the end of structure
  /// alignment padding
  /// @param storage_class_usage a set of all the storage class usages
  Struct(type::Struct* type,
         StructMemberList members,
         uint32_t align,
         uint32_t size,
         uint32_t size_no_padding,
         std::unordered_set<ast::StorageClass> storage_class_usage);

  /// Destructor
  ~Struct() override;

  /// @returns the structure type
  type::Struct* Type() const { return type_; }

  /// @returns the members of the structure
  const StructMemberList& Members() const { return members_; }

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
  /// host-sharable.
  bool IsHostSharable() const {
    for (auto sc : storage_class_usage_) {
      if (ast::IsHostSharable(sc)) {
        return true;
      }
    }
    return false;
  }

 private:
  type::Struct* const type_;
  StructMemberList const members_;
  uint32_t const align_;
  uint32_t const size_;
  uint32_t const size_no_padding_;
  std::unordered_set<ast::StorageClass> const storage_class_usage_;
};

/// StructMember holds the semantic information for structure members.
class StructMember : public Castable<StructMember, Node> {
 public:
  /// Constructor
  /// @param declaration the AST declaration node
  /// @param offset the byte offset from the base of the structure
  /// @param align the byte alignment of the member
  /// @param size the byte size of the member
  StructMember(ast::StructMember* declaration,
               uint32_t offset,
               uint32_t align,
               uint32_t size);

  /// Destructor
  ~StructMember() override;

  /// @returns the AST declaration node
  ast::StructMember* Declaration() const { return declaration_; }

  /// @returns byte offset from base of structure
  uint32_t Offset() const { return offset_; }

  /// @returns the alignment of the member in bytes
  uint32_t Align() const { return align_; }

  /// @returns byte size
  uint32_t Size() const { return size_; }

 private:
  ast::StructMember* const declaration_;
  uint32_t const offset_;  // Byte offset from base of structure
  uint32_t const align_;   // Byte alignment of the member
  uint32_t const size_;    // Byte size of the member
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_STRUCT_H_
