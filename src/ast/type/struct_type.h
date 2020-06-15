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

#ifndef SRC_AST_TYPE_STRUCT_TYPE_H_
#define SRC_AST_TYPE_STRUCT_TYPE_H_

#include <memory>
#include <string>

#include "src/ast/struct.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// A structure type
class StructType : public Type {
 public:
  /// Constructor
  /// @param impl the struct data
  explicit StructType(std::unique_ptr<Struct> impl);
  /// Move constructor
  StructType(StructType&&);
  ~StructType() override;

  /// Sets the name of the struct
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the struct name
  const std::string& name() const { return name_; }

  /// @returns true if the struct has a block decoration
  bool IsBlockDecorated() const {
    return struct_->decoration() == StructDecoration::kBlock;
  }

  /// @returns true if the type is a struct type
  bool IsStruct() const override;

  /// @returns the struct name
  Struct* impl() const { return struct_.get(); }

  /// @returns the name for th type
  std::string type_name() const override;

 private:
  std::string name_;
  std::unique_ptr<Struct> struct_;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_STRUCT_TYPE_H_
