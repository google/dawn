// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/msl/writer/common/printer_support.h"

#include <cmath>
#include <limits>

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/strconv/float_to_string.h"

namespace tint::msl::writer {

std::string BuiltinToAttribute(builtin::BuiltinValue builtin) {
    switch (builtin) {
        case builtin::BuiltinValue::kPosition:
            return "position";
        case builtin::BuiltinValue::kVertexIndex:
            return "vertex_id";
        case builtin::BuiltinValue::kInstanceIndex:
            return "instance_id";
        case builtin::BuiltinValue::kFrontFacing:
            return "front_facing";
        case builtin::BuiltinValue::kFragDepth:
            return "depth(any)";
        case builtin::BuiltinValue::kLocalInvocationId:
            return "thread_position_in_threadgroup";
        case builtin::BuiltinValue::kLocalInvocationIndex:
            return "thread_index_in_threadgroup";
        case builtin::BuiltinValue::kGlobalInvocationId:
            return "thread_position_in_grid";
        case builtin::BuiltinValue::kWorkgroupId:
            return "threadgroup_position_in_grid";
        case builtin::BuiltinValue::kNumWorkgroups:
            return "threadgroups_per_grid";
        case builtin::BuiltinValue::kSampleIndex:
            return "sample_id";
        case builtin::BuiltinValue::kSampleMask:
            return "sample_mask";
        case builtin::BuiltinValue::kPointSize:
            return "point_size";
        default:
            break;
    }
    return "";
}

std::string InterpolationToAttribute(builtin::InterpolationType type,
                                     builtin::InterpolationSampling sampling) {
    std::string attr;
    switch (sampling) {
        case builtin::InterpolationSampling::kCenter:
            attr = "center_";
            break;
        case builtin::InterpolationSampling::kCentroid:
            attr = "centroid_";
            break;
        case builtin::InterpolationSampling::kSample:
            attr = "sample_";
            break;
        case builtin::InterpolationSampling::kUndefined:
            break;
    }
    switch (type) {
        case builtin::InterpolationType::kPerspective:
            attr += "perspective";
            break;
        case builtin::InterpolationType::kLinear:
            attr += "no_perspective";
            break;
        case builtin::InterpolationType::kFlat:
            attr += "flat";
            break;
        case builtin::InterpolationType::kUndefined:
            break;
    }
    return attr;
}

SizeAndAlign MslPackedTypeSizeAndAlign(const type::Type* ty) {
    return tint::Switch(
        ty,

        // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
        // 2.1 Scalar Data Types
        [&](const type::U32*) {
            return SizeAndAlign{4, 4};
        },
        [&](const type::I32*) {
            return SizeAndAlign{4, 4};
        },
        [&](const type::F32*) {
            return SizeAndAlign{4, 4};
        },
        [&](const type::F16*) {
            return SizeAndAlign{2, 2};
        },

        [&](const type::Vector* vec) {
            auto num_els = vec->Width();
            auto* el_ty = vec->type();
            SizeAndAlign el_size_align = MslPackedTypeSizeAndAlign(el_ty);
            if (el_ty->IsAnyOf<type::U32, type::I32, type::F32, type::F16>()) {
                // Use a packed_vec type for 3-element vectors only.
                if (num_els == 3) {
                    // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
                    // 2.2.3 Packed Vector Types
                    return SizeAndAlign{num_els * el_size_align.size, el_size_align.align};
                } else {
                    // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
                    // 2.2 Vector Data Types
                    // Vector data types are aligned to their size.
                    return SizeAndAlign{num_els * el_size_align.size, num_els * el_size_align.size};
                }
            }
            TINT_UNREACHABLE() << "Unhandled vector element type " << el_ty->TypeInfo().name;
            return SizeAndAlign{};
        },

        [&](const type::Matrix* mat) {
            // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
            // 2.3 Matrix Data Types
            auto cols = mat->columns();
            auto rows = mat->rows();
            auto* el_ty = mat->type();
            // Metal only support half and float matrix.
            if (el_ty->IsAnyOf<type::F32, type::F16>()) {
                static constexpr SizeAndAlign table_f32[] = {
                    /* float2x2 */ {16, 8},
                    /* float2x3 */ {32, 16},
                    /* float2x4 */ {32, 16},
                    /* float3x2 */ {24, 8},
                    /* float3x3 */ {48, 16},
                    /* float3x4 */ {48, 16},
                    /* float4x2 */ {32, 8},
                    /* float4x3 */ {64, 16},
                    /* float4x4 */ {64, 16},
                };
                static constexpr SizeAndAlign table_f16[] = {
                    /* half2x2 */ {8, 4},
                    /* half2x3 */ {16, 8},
                    /* half2x4 */ {16, 8},
                    /* half3x2 */ {12, 4},
                    /* half3x3 */ {24, 8},
                    /* half3x4 */ {24, 8},
                    /* half4x2 */ {16, 4},
                    /* half4x3 */ {32, 8},
                    /* half4x4 */ {32, 8},
                };
                if (cols >= 2 && cols <= 4 && rows >= 2 && rows <= 4) {
                    if (el_ty->Is<type::F32>()) {
                        return table_f32[(3 * (cols - 2)) + (rows - 2)];
                    } else {
                        return table_f16[(3 * (cols - 2)) + (rows - 2)];
                    }
                }
            }

            TINT_UNREACHABLE() << "Unhandled matrix element type " << el_ty->TypeInfo().name;
            return SizeAndAlign{};
        },

        [&](const type::Array* arr) {
            if (TINT_UNLIKELY(!arr->IsStrideImplicit())) {
                TINT_ICE()
                    << "arrays with explicit strides should not exist past the SPIR-V reader";
                return SizeAndAlign{};
            }
            if (arr->Count()->Is<type::RuntimeArrayCount>()) {
                return SizeAndAlign{arr->Stride(), arr->Align()};
            }
            if (auto count = arr->ConstantCount()) {
                return SizeAndAlign{arr->Stride() * count.value(), arr->Align()};
            }
            TINT_ICE() << type::Array::kErrExpectedConstantCount;
            return SizeAndAlign{};
        },

        [&](const type::Struct* str) {
            // TODO(crbug.com/tint/650): There's an assumption here that MSL's
            // default structure size and alignment matches WGSL's. We need to
            // confirm this.
            return SizeAndAlign{str->Size(), str->Align()};
        },

        [&](const type::Atomic* atomic) { return MslPackedTypeSizeAndAlign(atomic->Type()); },

        [&](Default) {
            TINT_UNREACHABLE() << "Unhandled type " << ty->TypeInfo().name;
            return SizeAndAlign{};
        });
}

void PrintF32(StringStream& out, float value) {
    // Note: Currently inf and nan should not be constructable, but this is implemented for the day
    // we support them.
    if (std::isinf(value)) {
        out << (value >= 0 ? "INFINITY" : "-INFINITY");
    } else if (std::isnan(value)) {
        out << "NAN";
    } else {
        out << tint::writer::FloatToString(value) << "f";
    }
}

void PrintF16(StringStream& out, float value) {
    // Note: Currently inf and nan should not be constructable, but this is implemented for the day
    // we support them.
    if (std::isinf(value)) {
        // HUGE_VALH evaluates to +infinity.
        out << (value >= 0 ? "HUGE_VALH" : "-HUGE_VALH");
    } else if (std::isnan(value)) {
        // There is no NaN expr for half in MSL, "NAN" is of float type.
        out << "NAN";
    } else {
        out << tint::writer::FloatToString(value) << "h";
    }
}

void PrintI32(StringStream& out, int32_t value) {
    // MSL (and C++) parse `-2147483648` as a `long` because it parses unary minus and `2147483648`
    // as separate tokens, and the latter doesn't fit into an (32-bit) `int`.
    // WGSL, on the other hand, parses this as an `i32`.
    // To avoid issues with `long` to `int` casts, emit `(-2147483647 - 1)` instead, which ensures
    // the expression type is `int`.
    if (auto int_min = std::numeric_limits<int32_t>::min(); value == int_min) {
        out << "(" << int_min + 1 << " - 1)";
    } else {
        out << value;
    }
}

}  // namespace tint::msl::writer
