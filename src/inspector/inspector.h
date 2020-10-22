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

#ifndef SRC_INSPECTOR_INSPECTOR_H_
#define SRC_INSPECTOR_INSPECTOR_H_

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/inspector/entry_point.h"
#include "src/inspector/scalar.h"

namespace tint {
namespace inspector {

/// Container for information about how a resource is bound
struct ResourceBinding {
  /// Bind group the binding belongs
  uint32_t bind_group;
  /// Identifier to identify this binding within the bind group
  uint32_t binding;
  /// Minimum size required for this binding, in bytes.
  uint64_t min_buffer_binding_size;
};

/// Extracts information from a module
class Inspector {
 public:
  /// Constructor
  /// @param module Shader module to extract information from.
  explicit Inspector(const ast::Module& module);
  ~Inspector();

  /// @returns error messages from the Inspector
  const std::string& error() { return error_; }
  /// @returns true if an error was encountered
  bool has_error() const { return !error_.empty(); }

  /// @returns vector of entry point information
  std::vector<EntryPoint> GetEntryPoints();

  /// @returns map of const_id to initial value
  std::map<uint32_t, Scalar> GetConstantIDs();

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for Uniform buffers.
  std::vector<ResourceBinding> GetUniformBufferResourceBindings(
      const std::string& entry_point);

 private:
  const ast::Module& module_;
  std::string error_;
};

}  // namespace inspector
}  // namespace tint

#endif  // SRC_INSPECTOR_INSPECTOR_H_
