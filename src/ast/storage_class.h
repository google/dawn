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

#ifndef SRC_AST_STORAGE_CLASS_H_
#define SRC_AST_STORAGE_CLASS_H_

#include <ostream>

namespace tint {
namespace ast {

/// Storage class of a given pointer.
enum class StorageClass {
  kInvalid = -1,
  kNone,
  kInput,
  kOutput,
  kUniform,
  kWorkgroup,
  kUniformConstant,
  kStorage,
  kImage,
  kPrivate,
  kFunction
};

/// @returns true if the StorageClass is host-shareable
/// @param sc the StorageClass
/// @see https://gpuweb.github.io/gpuweb/wgsl.html#host-shareable
inline bool IsHostShareable(StorageClass sc) {
  return sc == ast::StorageClass::kUniform || sc == ast::StorageClass::kStorage;
}

/// @param sc the StorageClass
/// @return the name of the given storage class
const char* str(StorageClass sc);

/// @param out the std::ostream to write to
/// @param sc the StorageClass
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, StorageClass sc);

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STORAGE_CLASS_H_
