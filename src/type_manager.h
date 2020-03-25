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

#ifndef SRC_TYPE_MANAGER_H_
#define SRC_TYPE_MANAGER_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "src/ast/type/type.h"

namespace tint {

/// The type manager holds all the pointers to the known types.
///
/// Note, the type manager is a singleton. Any synchronization for the manager
/// must be done by the caller.
class TypeManager {
 public:
  /// @returns a pointer to the type manager
  static TypeManager* Instance();
  /// Frees the type manager and any associated types. The types should not be
  /// used after the manager is freed.
  static void Destroy();

  /// Get the given type from the type manager
  /// @param type The type to register
  /// @return the pointer to the registered type
  ast::type::Type* Get(std::unique_ptr<ast::type::Type> type);

 private:
  TypeManager();
  ~TypeManager();

  std::unordered_map<std::string, std::unique_ptr<ast::type::Type>> types_;
};

}  // namespace tint

#endif  // SRC_TYPE_MANAGER_H_
