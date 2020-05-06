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

#include "src/ast/storage_class.h"

namespace tint {
namespace ast {

std::ostream& operator<<(std::ostream& out, StorageClass sc) {
  switch (sc) {
    case StorageClass::kNone: {
      out << "none";
      break;
    }
    case StorageClass::kInput: {
      out << "in";
      break;
    }
    case StorageClass::kOutput: {
      out << "out";
      break;
    }
    case StorageClass::kUniform: {
      out << "uniform";
      break;
    }
    case StorageClass::kWorkgroup: {
      out << "workgroup";
      break;
    }
    case StorageClass::kUniformConstant: {
      out << "uniform_constant";
      break;
    }
    case StorageClass::kStorageBuffer: {
      out << "storage_buffer";
      break;
    }
    case StorageClass::kImage: {
      out << "image";
      break;
    }
    case StorageClass::kPrivate: {
      out << "private";
      break;
    }
    case StorageClass::kFunction: {
      out << "function";
      break;
    }
  }
  return out;
}

}  // namespace ast
}  // namespace tint
