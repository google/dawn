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

#ifndef SRC_SEM_TYPE_CONSTRUCTOR_H_
#define SRC_SEM_TYPE_CONSTRUCTOR_H_

#include "src/sem/call_target.h"

namespace tint {
namespace sem {

/// TypeConstructor is the CallTarget for a type constructor.
class TypeConstructor : public Castable<TypeConstructor, CallTarget> {
 public:
  /// Constructor
  /// @param type the type that's being constructed
  /// @param parameters the type constructor parameters
  TypeConstructor(const sem::Type* type, const ParameterList& parameters);

  /// Destructor
  ~TypeConstructor() override;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_TYPE_CONSTRUCTOR_H_
