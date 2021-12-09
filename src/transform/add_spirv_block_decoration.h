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

#ifndef SRC_TRANSFORM_ADD_SPIRV_BLOCK_DECORATION_H_
#define SRC_TRANSFORM_ADD_SPIRV_BLOCK_DECORATION_H_

#include <string>

#include "src/ast/internal_decoration.h"
#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// AddSpirvBlockDecoration is a transform that adds an
/// [[internal(spirv_block)]] attribute to any structure that is used as the
/// store type of a buffer. If that structure is nested inside another structure
/// or an array, then it is wrapped inside another structure which gets the
/// [[internal(spirv_block)]] attribute instead.
class AddSpirvBlockDecoration
    : public Castable<AddSpirvBlockDecoration, Transform> {
 public:
  /// SpirvBlockDecoration is an InternalDecoration that is used to decorate a
  // structure that needs a SPIR-V block decoration.
  class SpirvBlockDecoration
      : public Castable<SpirvBlockDecoration, ast::InternalDecoration> {
   public:
    /// Constructor
    /// @param program_id the identifier of the program that owns this node
    explicit SpirvBlockDecoration(ProgramID program_id);
    /// Destructor
    ~SpirvBlockDecoration() override;

    /// @return a short description of the internal decoration which will be
    /// displayed as `[[internal(<name>)]]`
    std::string InternalName() const override;

    /// Performs a deep clone of this object using the CloneContext `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned object
    const SpirvBlockDecoration* Clone(CloneContext* ctx) const override;
  };

  /// Constructor
  AddSpirvBlockDecoration();

  /// Destructor
  ~AddSpirvBlockDecoration() override;

 protected:
  /// Runs the transform using the CloneContext built for transforming a
  /// program. Run() is responsible for calling Clone() on the CloneContext.
  /// @param ctx the CloneContext primed with the input program and
  /// ProgramBuilder
  /// @param inputs optional extra transform-specific input data
  /// @param outputs optional extra transform-specific output data
  void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_ADD_SPIRV_BLOCK_DECORATION_H_
