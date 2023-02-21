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

#include "src/tint/inspector/entry_point.h"

namespace tint::inspector {

StageVariable::StageVariable() = default;
StageVariable::StageVariable(const StageVariable& other)
    : name(other.name),
      has_location_attribute(other.has_location_attribute),
      location_attribute(other.location_attribute),
      component_type(other.component_type),
      composition_type(other.composition_type),
      interpolation_type(other.interpolation_type),
      interpolation_sampling(other.interpolation_sampling) {}

StageVariable::~StageVariable() = default;

EntryPoint::EntryPoint() = default;
EntryPoint::EntryPoint(EntryPoint&) = default;
EntryPoint::EntryPoint(EntryPoint&&) = default;
EntryPoint::~EntryPoint() = default;

}  // namespace tint::inspector
