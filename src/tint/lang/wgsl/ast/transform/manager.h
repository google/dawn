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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_MANAGER_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_MANAGER_H_

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/transform/data.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::ast::transform {

/// A collection of Transforms that act as a single Transform.
/// The inner transforms will execute in the appended order.
/// If any inner transform fails the manager will return immediately and
/// the error can be retrieved with the Output's diagnostics.
class Manager {
  public:
    /// Constructor
    Manager();
    ~Manager();

    /// Add pass to the manager
    /// @param transform the transform to append
    void append(std::unique_ptr<Transform> transform) {
        transforms_.push_back(std::move(transform));
    }

    /// Add pass to the manager of type `T`, constructed with the provided
    /// arguments.
    /// @param args the arguments to forward to the `T` initializer
    template <typename T, typename... ARGS>
    void Add(ARGS&&... args) {
        transforms_.emplace_back(std::make_unique<T>(std::forward<ARGS>(args)...));
    }

    /// Runs the transforms on @p program, returning the transformed clone of @p program.
    /// @param program the source program to transform
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    /// @returns the transformed program
    Program Run(const Program* program, const DataMap& inputs, DataMap& outputs) const;

  private:
    std::vector<std::unique_ptr<Transform>> transforms_;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_MANAGER_H_
