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

#include "src/ast/entry_point.h"

namespace tint {
namespace ast {

EntryPoint::EntryPoint(PipelineStage stage,
                       const std::string& name,
                       const std::string& fn_name)
    : Node(), stage_(stage), name_(name), fn_name_(fn_name) {}

EntryPoint::EntryPoint(const Source& source,
                       PipelineStage stage,
                       const std::string& name,
                       const std::string& fn_name)
    : Node(source), stage_(stage), name_(name), fn_name_(fn_name) {}

EntryPoint::~EntryPoint() = default;

bool EntryPoint::IsValid() const {
  if (stage_ == PipelineStage::kNone) {
    return false;
  }
  if (fn_name_.length() == 0) {
    return false;
  }
  return true;
}

void EntryPoint::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "EntryPoint{" << stage_;
  if (name_.length() > 0)
    out << " as " << name_;

  out << " = " << fn_name_ << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
