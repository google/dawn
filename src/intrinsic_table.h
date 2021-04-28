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

#ifndef SRC_INTRINSIC_TABLE_H_
#define SRC_INTRINSIC_TABLE_H_

#include <memory>
#include <string>
#include <vector>

#include "src/sem/intrinsic.h"

namespace tint {

// Forward declarations
class ProgramBuilder;

/// IntrinsicTable is a lookup table of all the WGSL intrinsic functions
class IntrinsicTable {
 public:
  /// @return a pointer to a newly created IntrinsicTable
  static std::unique_ptr<IntrinsicTable> Create();

  /// Destructor
  virtual ~IntrinsicTable();

  /// Result is returned by Lookup
  struct Result {
    /// The intrinsic, if the lookup succeeded, otherwise nullptr
    sem::Intrinsic* intrinsic;
    /// Diagnostic messages
    diag::List diagnostics;
  };

  /// Lookup looks for the intrinsic overload with the given signature.
  /// @param builder the program builder
  /// @param type the intrinsic type
  /// @param args the argument types passed to the intrinsic function
  /// @param source the source of the intrinsic call
  /// @return the semantic intrinsic if found, otherwise nullptr
  virtual Result Lookup(ProgramBuilder& builder,
                        sem::IntrinsicType type,
                        const std::vector<const sem::Type*>& args,
                        const Source& source) const = 0;
};

}  // namespace tint

#endif  // SRC_INTRINSIC_TABLE_H_
