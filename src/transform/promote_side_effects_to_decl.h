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

#ifndef SRC_TRANSFORM_PROMOTE_SIDE_EFFECTS_TO_DECL_H_
#define SRC_TRANSFORM_PROMOTE_SIDE_EFFECTS_TO_DECL_H_

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// A transform that hoists expressions with side-effects to a variable
/// declaration just before the statement of usage. This transform may also
/// decompose for-loops into loops so that let declarations can be emitted
/// before loop condition expressions and/or continuing statements. It may also
/// similarly decompose 'else if's to 'else { if }'s for the same reason.
/// @see crbug.com/tint/406
class PromoteSideEffectsToDecl
    : public Castable<PromoteSideEffectsToDecl, Transform> {
 public:
  /// Constructor
  PromoteSideEffectsToDecl();

  /// Destructor
  ~PromoteSideEffectsToDecl() override;

  /// Configuration options for the transform.
  struct Config : public Castable<Config, Data> {
    /// Constructor
    /// @param type_ctor_to_let whether to hoist type constructor expressions
    /// to a let
    /// @param dynamic_index_to_var whether to hoist dynamic indexed
    /// expressions to a var
    Config(bool type_ctor_to_let, bool dynamic_index_to_var);

    /// Destructor
    ~Config() override;

    /// Whether to hoist type constructor expressions to a let
    const bool type_ctor_to_let;

    /// Whether to hoist dynamic indexed expressions to a var
    const bool dynamic_index_to_var;
  };

 protected:
  /// Runs the transform using the CloneContext built for transforming a
  /// program. Run() is responsible for calling Clone() on the CloneContext.
  /// @param ctx the CloneContext primed with the input program and
  /// ProgramBuilder
  /// @param inputs optional extra transform-specific input data
  /// @param outputs optional extra transform-specific output data
  void Run(CloneContext& ctx,
           const DataMap& inputs,
           DataMap& outputs) const override;

 private:
  class State;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_PROMOTE_SIDE_EFFECTS_TO_DECL_H_
