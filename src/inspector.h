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

#ifndef SRC_INSPECTOR_H_
#define SRC_INSPECTOR_H_

#include <memory>
#include <string>
#include <vector>

#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"

namespace tint {
namespace inspector {

struct EntryPoint {
  /// The entry point name
  std::string name;
  /// The entry point stage
  ast::PipelineStage stage = ast::PipelineStage::kNone;
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

 private:
  const ast::Module& module_;
  std::string error_;
};

}  // namespace inspector
}  // namespace tint

#endif  // SRC_INSPECTOR_H_
