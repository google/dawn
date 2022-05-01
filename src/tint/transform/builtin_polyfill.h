// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TRANSFORM_BUILTIN_POLYFILL_H_
#define SRC_TINT_TRANSFORM_BUILTIN_POLYFILL_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Implements builtins for backends that do not have a native implementation.
class BuiltinPolyfill final : public Castable<BuiltinPolyfill, Transform> {
  public:
    /// Constructor
    BuiltinPolyfill();
    /// Destructor
    ~BuiltinPolyfill() override;

    /// Enumerator of polyfill levels
    enum class Level {
        /// No polyfill needed, supported by the backend.
        kNone,
        /// Clamp the parameters to the inner implementation.
        kClampParameters,
        /// Polyfill the entire function
        kFull,
    };

    /// Specifies the builtins that should be polyfilled by the transform.
    struct Builtins {
        /// Should `countLeadingZeros()` be polyfilled?
        bool count_leading_zeros = false;
        /// Should `countTrailingZeros()` be polyfilled?
        bool count_trailing_zeros = false;
        /// What level should `extractBits()` be polyfilled?
        Level extract_bits = Level::kNone;
        /// Should `firstLeadingBit()` be polyfilled?
        bool first_leading_bit = false;
        /// Should `firstTrailingBit()` be polyfilled?
        bool first_trailing_bit = false;
        /// Should `insertBits()` be polyfilled?
        Level insert_bits = Level::kNone;
    };

    /// Config is consumed by the BuiltinPolyfill transform.
    /// Config specifies the builtins that should be polyfilled.
    struct Config final : public Castable<Data, transform::Data> {
        /// Constructor
        /// @param b the list of builtins to polyfill
        explicit Config(const Builtins& b);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The builtins to polyfill
        const Builtins builtins;
    };

    /// @param program the program to inspect
    /// @param data optional extra transform-specific input data
    /// @returns true if this transform should be run for the given program
    bool ShouldRun(const Program* program, const DataMap& data = {}) const override;

  protected:
    struct State;

    /// Runs the transform using the CloneContext built for transforming a
    /// program. Run() is responsible for calling Clone() on the CloneContext.
    /// @param ctx the CloneContext primed with the input program and
    /// ProgramBuilder
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_BUILTIN_POLYFILL_H_
