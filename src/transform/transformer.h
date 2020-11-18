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

#ifndef SRC_TRANSFORM_TRANSFORMER_H_
#define SRC_TRANSFORM_TRANSFORMER_H_

#include <memory>
#include <string>
#include <utility>

#include "src/ast/module.h"
#include "src/context.h"

namespace tint {
namespace transform {

/// Interface class for the transformers
class Transformer {
 public:
  /// Constructor
  /// @param ctx the Tint context
  /// @param mod the module to transform
  Transformer(Context* ctx, ast::Module* mod);
  virtual ~Transformer();

  /// Users of Tint should register the transform with transform manager and
  /// invoke its Run(), instead of directly calling the transform's Run().
  /// Calling Run() directly does not perform module state cleanup operations.
  /// @returns true if the transformation was successful
  virtual bool Run() = 0;

  /// @returns error messages
  const std::string& error() { return error_; }

 protected:
  /// Creates a new `ast::Node` owned by the Module. When the Module is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return mod_->create<T>(std::forward<ARGS>(args)...);
  }

  /// The context
  Context* ctx_ = nullptr;
  /// The module
  ast::Module* mod_ = nullptr;
  /// Any error messages, or blank if no error
  std::string error_;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_TRANSFORMER_H_
