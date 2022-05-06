// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_RESOLVER_INTRINSIC_TABLE_H_
#define SRC_TINT_RESOLVER_INTRINSIC_TABLE_H_

#include <memory>
#include <string>
#include <vector>

#include "src/tint/sem/builtin.h"

// Forward declarations
namespace tint {
class ProgramBuilder;
}  // namespace tint

namespace tint {

/// IntrinsicTable is a lookup table of all the WGSL builtin functions
class IntrinsicTable {
  public:
    /// @param builder the program builder
    /// @return a pointer to a newly created IntrinsicTable
    static std::unique_ptr<IntrinsicTable> Create(ProgramBuilder& builder);

    /// Destructor
    virtual ~IntrinsicTable();

    /// Lookup looks for the builtin overload with the given signature, raising
    /// an error diagnostic if the builtin was not found.
    /// @param type the builtin type
    /// @param args the argument types passed to the builtin function
    /// @param source the source of the builtin call
    /// @return the semantic builtin if found, otherwise nullptr
    virtual const sem::Builtin* Lookup(sem::BuiltinType type,
                                       const std::vector<const sem::Type*>& args,
                                       const Source& source) = 0;
};

}  // namespace tint

#endif  // SRC_TINT_RESOLVER_INTRINSIC_TABLE_H_
