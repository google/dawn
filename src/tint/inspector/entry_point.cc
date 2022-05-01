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
      has_location_decoration(has_location_attribute),
      location_decoration(location_attribute),
      component_type(other.component_type),
      composition_type(other.composition_type),
      interpolation_type(other.interpolation_type),
      interpolation_sampling(other.interpolation_sampling) {}

StageVariable::~StageVariable() = default;

EntryPoint::EntryPoint() = default;
EntryPoint::EntryPoint(EntryPoint&) = default;
EntryPoint::EntryPoint(EntryPoint&&) = default;
EntryPoint::~EntryPoint() = default;

InterpolationType ASTToInspectorInterpolationType(ast::InterpolationType ast_type) {
    switch (ast_type) {
        case ast::InterpolationType::kPerspective:
            return InterpolationType::kPerspective;
        case ast::InterpolationType::kLinear:
            return InterpolationType::kLinear;
        case ast::InterpolationType::kFlat:
            return InterpolationType::kFlat;
    }

    return InterpolationType::kUnknown;
}

InterpolationSampling ASTToInspectorInterpolationSampling(ast::InterpolationSampling sampling) {
    switch (sampling) {
        case ast::InterpolationSampling::kNone:
            return InterpolationSampling::kNone;
        case ast::InterpolationSampling::kCenter:
            return InterpolationSampling::kCenter;
        case ast::InterpolationSampling::kCentroid:
            return InterpolationSampling::kCentroid;
        case ast::InterpolationSampling::kSample:
            return InterpolationSampling::kSample;
    }

    return InterpolationSampling::kUnknown;
}

}  // namespace tint::inspector
