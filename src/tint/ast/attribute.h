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

#ifndef SRC_TINT_AST_ATTRIBUTE_H_
#define SRC_TINT_AST_ATTRIBUTE_H_

#include <string>
#include <vector>

#include "src/tint/ast/node.h"

namespace tint::ast {

/// The base class for all attributes
class Attribute : public utils::Castable<Attribute, Node> {
  public:
    ~Attribute() override;

    /// @returns the WGSL name for the attribute
    virtual std::string Name() const = 0;

  protected:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    Attribute(ProgramID pid, NodeID nid, const Source& src) : Base(pid, nid, src) {}
};

/// @param attributes the list of attributes to search
/// @returns true if `attributes` includes a attribute of type `T`
template <typename... Ts>
bool HasAttribute(utils::VectorRef<const Attribute*> attributes) {
    for (auto* attr : attributes) {
        if (attr->IsAnyOf<Ts...>()) {
            return true;
        }
    }
    return false;
}

/// @param attributes the list of attributes to search
/// @returns a pointer to `T` from `attributes` if found, otherwise nullptr.
template <typename T>
const T* GetAttribute(utils::VectorRef<const Attribute*> attributes) {
    for (auto* attr : attributes) {
        if (attr->Is<T>()) {
            return attr->As<T>();
        }
    }
    return nullptr;
}

}  // namespace tint::ast

#endif  // SRC_TINT_AST_ATTRIBUTE_H_
