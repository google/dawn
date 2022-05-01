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

#ifndef SRC_TINT_TRANSFORM_WRAP_ARRAYS_IN_STRUCTS_H_
#define SRC_TINT_TRANSFORM_WRAP_ARRAYS_IN_STRUCTS_H_

#include <string>
#include <unordered_map>

#include "src/tint/transform/transform.h"

// Forward declarations
namespace tint::ast {
class Type;
}  // namespace tint::ast

namespace tint::transform {

/// WrapArraysInStructs is a transform that replaces all array types with a
/// structure holding a single field of that array type.
/// Array index expressions and constructors are also adjusted to deal with this
/// wrapping.
/// This transform helps with backends that cannot directly return arrays or use
/// them as parameters.
class WrapArraysInStructs : public Castable<WrapArraysInStructs, Transform> {
  public:
    /// Constructor
    WrapArraysInStructs();

    /// Destructor
    ~WrapArraysInStructs() override;

    /// @param program the program to inspect
    /// @param data optional extra transform-specific input data
    /// @returns true if this transform should be run for the given program
    bool ShouldRun(const Program* program, const DataMap& data = {}) const override;

  protected:
    /// Runs the transform using the CloneContext built for transforming a
    /// program. Run() is responsible for calling Clone() on the CloneContext.
    /// @param ctx the CloneContext primed with the input program and
    /// ProgramBuilder
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const override;

  private:
    struct WrappedArrayInfo {
        WrappedArrayInfo();
        WrappedArrayInfo(const WrappedArrayInfo&);
        ~WrappedArrayInfo();

        Symbol wrapper_name;
        std::function<const ast::Type*(CloneContext&)> array_type;

        operator bool() { return wrapper_name.IsValid(); }
    };

    /// WrapArray wraps the fixed-size array type in a new structure (if it hasn't
    /// already been wrapped). WrapArray will recursively wrap arrays-of-arrays.
    /// The new structure will be added to module-scope type declarations of
    /// `ctx.dst`.
    /// @param ctx the CloneContext
    /// @param wrapped_arrays a map of src array type to the wrapped structure
    /// name
    /// @param array the array type
    /// @return the name of the structure that wraps the array, or an invalid
    /// Symbol if this array should not be wrapped
    WrappedArrayInfo WrapArray(
        CloneContext& ctx,
        std::unordered_map<const sem::Array*, WrappedArrayInfo>& wrapped_arrays,
        const sem::Array* array) const;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_WRAP_ARRAYS_IN_STRUCTS_H_
