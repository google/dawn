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

#include "src/tint/ast/storage_class.h"

namespace tint::ast {

const char* ToString(StorageClass sc) {
    switch (sc) {
        case StorageClass::kInvalid:
            return "invalid";
        case StorageClass::kNone:
            return "none";
        case StorageClass::kInput:
            return "in";
        case StorageClass::kOutput:
            return "out";
        case StorageClass::kUniform:
            return "uniform";
        case StorageClass::kWorkgroup:
            return "workgroup";
        case StorageClass::kHandle:
            return "handle";
        case StorageClass::kStorage:
            return "storage";
        case StorageClass::kPrivate:
            return "private";
        case StorageClass::kFunction:
            return "function";
    }
    return "<unknown>";
}
std::ostream& operator<<(std::ostream& out, StorageClass sc) {
    out << ToString(sc);
    return out;
}

}  // namespace tint::ast
