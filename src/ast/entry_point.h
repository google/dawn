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

#ifndef SRC_AST_ENTRY_POINT_H_
#define SRC_AST_ENTRY_POINT_H_

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "src/ast/node.h"
#include "src/ast/pipeline_stage.h"

namespace tint {
namespace ast {

/// An entry point statement.
class EntryPoint : public Node {
 public:
  /// Constructor
  EntryPoint() = default;
  /// Constructor
  /// @param stage the entry point stage
  /// @param name the entry point name
  /// @param fn_name the function name
  EntryPoint(PipelineStage stage,
             const std::string& name,
             const std::string& fn_name);
  /// Constructor
  /// @param source the source of the entry point
  /// @param stage the entry point stage
  /// @param name the entry point name
  /// @param fn_name the function name
  EntryPoint(const Source& source,
             PipelineStage stage,
             const std::string& name,
             const std::string& fn_name);
  /// Move constructor
  EntryPoint(EntryPoint&&) = default;

  ~EntryPoint() override;

  /// Sets the entry point name
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the entry points name
  const std::string& name() const { return name_; }
  /// Sets the entry point function name
  /// @param name the function name
  void set_function_name(const std::string& name) { fn_name_ = name; }
  /// @returns the function name for the entry point
  const std::string& function_name() const { return fn_name_; }
  /// Sets the piepline stage
  /// @param stage the stage to set
  void set_pipeline_stage(PipelineStage stage) { stage_ = stage; }
  /// @returns the pipeline stage for the entry point
  PipelineStage stage() const { return stage_; }

  /// @returns true if the entry point is valid
  bool IsValid() const override;

  /// Writes a representation of the entry point to the output stream
  /// @param out the stream to write too
  /// @param indent number of spaces to ident the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  EntryPoint(const EntryPoint&) = delete;

  Source source_;
  PipelineStage stage_;
  std::string name_;
  std::string fn_name_;
};

/// A list of unique entry points.
using EntryPointList = std::vector<std::unique_ptr<EntryPoint>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ENTRY_POINT_H_
