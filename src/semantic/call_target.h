// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_SEMANTIC_CALL_TARGET_H_
#define SRC_SEMANTIC_CALL_TARGET_H_

#include <utility>
#include <vector>

#include "src/semantic/node.h"
#include "src/type/sampler_type.h"

namespace tint {

// Forward declarations
namespace type {
class Type;
}  // namespace type

namespace semantic {

/// Parameter describes a single parameter of a call target
struct Parameter {
  /// Parameter type
  type::Type* type;
};

using Parameters = std::vector<Parameter>;

/// CallTarget is the base for callable functions
class CallTarget : public Castable<CallTarget, Node> {
 public:
  /// Constructor
  /// @param parameters the parameters for the call target
  explicit CallTarget(const semantic::Parameters& parameters);

  /// Destructor
  ~CallTarget() override;

  /// @return the parameters of the call target
  const Parameters& Parameters() const { return parameters_; }

 private:
  semantic::Parameters parameters_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_CALL_TARGET_H_
