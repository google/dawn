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

#ifndef SRC_TINT_SEM_CALL_TARGET_H_
#define SRC_TINT_SEM_CALL_TARGET_H_

#include <vector>

#include "src/tint/sem/node.h"
#include "src/tint/sem/sampler.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/hash.h"

// Forward declarations
namespace tint::sem {
class Type;
}  // namespace tint::sem

namespace tint::sem {

/// CallTargetSignature holds the return type and parameters for a call target
struct CallTargetSignature {
  /// Constructor
  /// @param ret_ty the call target return type
  /// @param params the call target parameters
  CallTargetSignature(const sem::Type* ret_ty, const ParameterList& params);

  /// Copy constructor
  CallTargetSignature(const CallTargetSignature&);

  /// Destructor
  ~CallTargetSignature();

  /// The type of the call target return value
  const sem::Type* const return_type = nullptr;
  /// The parameters of the call target
  const ParameterList parameters;

  /// Equality operator
  /// @param other the signature to compare this to
  /// @returns true if this signature is equal to other
  bool operator==(const CallTargetSignature& other) const;

  /// @param usage the parameter usage to find
  /// @returns the index of the parameter with the given usage, or -1 if no
  /// parameter with the given usage exists.
  int IndexOf(ParameterUsage usage) const;
};

/// CallTarget is the base for callable functions, builtins, type constructors
/// and type casts.
class CallTarget : public Castable<CallTarget, Node> {
 public:
  /// Constructor
  /// @param return_type the return type of the call target
  /// @param parameters the parameters for the call target
  CallTarget(const sem::Type* return_type, const ParameterList& parameters);

  /// Copy constructor
  CallTarget(const CallTarget&);

  /// Destructor
  ~CallTarget() override;

  /// @return the return type of the call target
  const sem::Type* ReturnType() const { return signature_.return_type; }

  /// @return the parameters of the call target
  const ParameterList& Parameters() const { return signature_.parameters; }

  /// @return the signature of the call target
  const CallTargetSignature& Signature() const { return signature_; }

 private:
  CallTargetSignature signature_;
};

}  // namespace tint::sem

namespace std {

/// Custom std::hash specialization for tint::sem::CallTargetSignature so
/// CallTargetSignature can be used as keys for std::unordered_map and
/// std::unordered_set.
template <>
class hash<tint::sem::CallTargetSignature> {
 public:
  /// @param sig the CallTargetSignature to hash
  /// @return the hash value
  std::size_t operator()(const tint::sem::CallTargetSignature& sig) const;
};

}  // namespace std

#endif  // SRC_TINT_SEM_CALL_TARGET_H_
