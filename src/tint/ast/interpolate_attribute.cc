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

#include "src/tint/ast/interpolate_attribute.h"

#include <string>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::InterpolateAttribute);

namespace tint::ast {

InterpolateAttribute::InterpolateAttribute(ProgramID pid,
                                           const Source& src,
                                           InterpolationType ty,
                                           InterpolationSampling smpl)
    : Base(pid, src), type(ty), sampling(smpl) {}

InterpolateAttribute::~InterpolateAttribute() = default;

std::string InterpolateAttribute::Name() const {
    return "interpolate";
}

const InterpolateAttribute* InterpolateAttribute::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    return ctx->dst->create<InterpolateAttribute>(src, type, sampling);
}

std::ostream& operator<<(std::ostream& out, InterpolationType type) {
    switch (type) {
        case InterpolationType::kPerspective: {
            out << "perspective";
            break;
        }
        case InterpolationType::kLinear: {
            out << "linear";
            break;
        }
        case InterpolationType::kFlat: {
            out << "flat";
            break;
        }
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, InterpolationSampling sampling) {
    switch (sampling) {
        case InterpolationSampling::kNone: {
            out << "none";
            break;
        }
        case InterpolationSampling::kCenter: {
            out << "center";
            break;
        }
        case InterpolationSampling::kCentroid: {
            out << "centroid";
            break;
        }
        case InterpolationSampling::kSample: {
            out << "sample";
            break;
        }
    }
    return out;
}

}  // namespace tint::ast
