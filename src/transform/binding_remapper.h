// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TRANSFORM_BINDING_REMAPPER_H_
#define SRC_TRANSFORM_BINDING_REMAPPER_H_

#include <unordered_map>

#include "src/ast/access_control.h"
#include "src/transform/binding_point.h"
#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// BindingRemapper is a transform used to remap resource binding points and
/// access controls.
class BindingRemapper : public Transform {
 public:
  /// BindingPoints is a map of old binding point to new binding point
  using BindingPoints = std::unordered_map<BindingPoint, BindingPoint>;

  /// AccessControls is a map of old binding point to new access control
  using AccessControls = std::unordered_map<BindingPoint, ast::AccessControl>;

  /// Remappings is consumed by the BindingRemapper transform.
  /// Data holds information about shader usage and constant buffer offsets.
  struct Remappings : public Castable<Data, transform::Data> {
    /// Constructor
    /// @param bp a map of new binding points
    /// @param ac a map of new access controls
    Remappings(BindingPoints bp, AccessControls ac);

    /// Copy constructor
    Remappings(const Remappings&);

    /// Destructor
    ~Remappings() override;

    /// A map of old binding point to new binding point
    BindingPoints const binding_points;

    /// A map of old binding point to new access controls
    AccessControls const access_controls;
  };

  /// Constructor
  BindingRemapper();
  ~BindingRemapper() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_BINDING_REMAPPER_H_
