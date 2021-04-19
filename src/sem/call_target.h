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

#ifndef SRC_SEM_CALL_TARGET_H_
#define SRC_SEM_CALL_TARGET_H_

#include <vector>

#include "src/sem/node.h"
#include "src/sem/sampler_type.h"

namespace tint {

namespace sem {
// Forward declarations
class Type;

/// Parameter describes a single parameter of a call target
struct Parameter {
  /// Usage is extra metadata for identifying a parameter based on its overload
  /// position
  enum class Usage {
    kNone,
    kArrayIndex,
    kBias,
    kCoords,
    kDepthRef,
    kDdx,
    kDdy,
    kLevel,
    kOffset,
    kSampler,
    kSampleIndex,
    kTexture,
    kValue,
  };

  /// Parameter type
  sem::Type* const type;
  /// Parameter usage
  Usage const usage = Usage::kNone;
};

std::ostream& operator<<(std::ostream& out, Parameter parameter);

/// Comparison operator for Parameters
static inline bool operator==(const Parameter& a, const Parameter& b) {
  return a.type == b.type && a.usage == b.usage;
}

/// @returns a string representation of the given parameter usage.
const char* str(Parameter::Usage usage);

/// ParameterList is a list of Parameter
using ParameterList = std::vector<Parameter>;

/// @param parameters the list of parameters
/// @param usage the parameter usage to find
/// @returns the index of the parameter with the given usage, or -1 if no
/// parameter with the given usage exists.
int IndexOf(const ParameterList& parameters, Parameter::Usage usage);

/// CallTarget is the base for callable functions
class CallTarget : public Castable<CallTarget, Node> {
 public:
  /// Constructor
  /// @param return_type the return type of the call target
  /// @param parameters the parameters for the call target
  CallTarget(sem::Type* return_type, const ParameterList& parameters);

  /// @return the return type of the call target
  sem::Type* ReturnType() const { return return_type_; }

  /// Destructor
  ~CallTarget() override;

  /// @return the parameters of the call target
  const ParameterList& Parameters() const { return parameters_; }

 private:
  sem::Type* const return_type_;
  ParameterList const parameters_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_CALL_TARGET_H_
