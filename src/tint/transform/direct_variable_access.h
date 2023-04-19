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

#ifndef SRC_TINT_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
#define SRC_TINT_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// DirectVariableAccess is a transform that allows usage of pointer parameters in the 'storage',
/// 'uniform' and 'workgroup' address space, and passing of pointers to sub-objects. These pointers
/// are only allowed by the resolver when the `chromium_experimental_full_ptr_parameters` extension
/// is enabled.
///
/// DirectVariableAccess works by creating specializations of functions that have pointer
/// parameters, one specialization for each pointer argument's unique access chain 'shape' from a
/// unique variable. Calls to specialized functions are transformed so that the pointer arguments
/// are replaced with an array of access-chain indicies, and if the pointer is in the 'function' or
/// 'private' address space, also with a pointer to the root object. For more information, see the
/// comments in src/tint/transform/direct_variable_access.cc.
///
/// @note DirectVariableAccess requires the transform::Unshadow transform to have been run first.
class DirectVariableAccess final : public utils::Castable<DirectVariableAccess, Transform> {
  public:
    /// Constructor
    DirectVariableAccess();
    /// Destructor
    ~DirectVariableAccess() override;

    /// Options adjusts the behaviour of the transform.
    struct Options {
        /// If true, then 'private' sub-object pointer arguments will be transformed.
        bool transform_private = false;
        /// If true, then 'function' sub-object pointer arguments will be transformed.
        bool transform_function = false;
    };

    /// Config is consumed by the DirectVariableAccess transform.
    /// Config specifies the behavior of the transform.
    struct Config final : public utils::Castable<Data, transform::Data> {
        /// Constructor
        /// @param options behavior of the transform
        explicit Config(const Options& options);
        /// Destructor
        ~Config() override;

        /// The transform behavior options
        const Options options;
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
