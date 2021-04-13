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

#ifndef SRC_TRANSFORM_CALCULATE_ARRAY_LENGTH_H_
#define SRC_TRANSFORM_CALCULATE_ARRAY_LENGTH_H_

#include <string>

#include "src/ast/internal_decoration.h"
#include "src/transform/transform.h"

namespace tint {

// Forward declarations
class CloneContext;

namespace transform {

/// CalculateArrayLength is a transform used to replace calls to arrayLength()
/// with a value calculated from the size of the storage buffer.
class CalculateArrayLength : public Transform {
 public:
  /// BufferSizeIntrinsic is an InternalDecoration that's applied to intrinsic
  /// functions used to obtain the runtime size of a storage buffer.
  class BufferSizeIntrinsic
      : public Castable<BufferSizeIntrinsic, ast::InternalDecoration> {
   public:
    /// Constructor
    /// @param program_id the identifier of the program that owns this node
    explicit BufferSizeIntrinsic(ProgramID program_id);
    /// Destructor
    ~BufferSizeIntrinsic() override;

    /// @return "buffer_size"
    std::string Name() const override;

    /// Performs a deep clone of this object using the CloneContext `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned object
    BufferSizeIntrinsic* Clone(CloneContext* ctx) const override;
  };

  /// Constructor
  CalculateArrayLength();
  /// Destructor
  ~CalculateArrayLength() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_CALCULATE_ARRAY_LENGTH_H_
