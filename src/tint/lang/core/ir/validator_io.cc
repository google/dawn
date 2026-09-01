// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::ir::validator {
namespace {

/// @returns a human-readable string of all the entries in a EnumSet
template <typename T>
std::string ToString(const EnumSet<T>& values) {
    std::stringstream result;
    result << "[ ";
    bool first = true;
    for (auto v : values) {
        if (!first) {
            result << ", ";
        }
        first = false;
        result << ToString(v);
    }
    result << " ]";
    return result.str();
}

IOAttributeUsage IOAttributeUsageFor(Function::PipelineStage stage, IODirection direction) {
    switch (stage) {
        case Function::PipelineStage::kCompute:
            switch (direction) {
                case IODirection::kInput:
                    return IOAttributeUsage::kComputeInputUsage;
                case IODirection::kOutput:
                    return IOAttributeUsage::kComputeOutputUsage;
                case IODirection::kResource:
                    return IOAttributeUsage::kComputeResourceUsage;
            }
            break;
        case Function::PipelineStage::kFragment:
            switch (direction) {
                case IODirection::kInput:
                    return IOAttributeUsage::kFragmentInputUsage;
                case IODirection::kOutput:
                    return IOAttributeUsage::kFragmentOutputUsage;
                case IODirection::kResource:
                    return IOAttributeUsage::kFragmentResourceUsage;
            }
            break;
        case Function::PipelineStage::kVertex:
            switch (direction) {
                case IODirection::kInput:
                    return IOAttributeUsage::kVertexInputUsage;
                case IODirection::kOutput:
                    return IOAttributeUsage::kVertexOutputUsage;
                case IODirection::kResource:
                    return IOAttributeUsage::kVertexResourceUsage;
            }
            break;
        case Function::PipelineStage::kUndefined:
            return IOAttributeUsage::kUndefinedUsage;
    }
    TINT_ICE() << "Unknown IOAttribute usage " << ToString(direction) << " for a "
               << ToString(stage) << " entry point";
}

IODirection IODirectionFor(AddressSpace address_space) {
    switch (address_space) {
        case AddressSpace::kIn:
            return IODirection::kInput;
        case AddressSpace::kOut:
            return IODirection::kOutput;
        case AddressSpace::kHandle:
            return IODirection::kResource;
        default:
            TINT_ICE() << "Unexpected address_space '" << ToString(address_space)
                       << "' passed to IODirectionFrom()";
    }
}

Result<SuccessType, IOAnnotation> AddIOAnnotationsFromIOAttributes(
    EnumSet<IOAnnotation>& annotations,
    const IOAttributes& attr) {
    if (attr.location.has_value()) {
        if (annotations.Contains(IOAnnotation::kLocation)) {
            return IOAnnotation::kLocation;
        }
        annotations.Add(IOAnnotation::kLocation);
    }

    if (attr.builtin.has_value()) {
        if (annotations.Contains(IOAnnotation::kBuiltin)) {
            return IOAnnotation::kBuiltin;
        }
        annotations.Add(IOAnnotation::kBuiltin);
    }

    if (attr.color.has_value()) {
        if (annotations.Contains(IOAnnotation::kColor)) {
            return IOAnnotation::kColor;
        }
        annotations.Add(IOAnnotation::kColor);
    }

    return Success;
}

/// A BuiltInChecker is the interface used to check that a usage of a builtin attribute meets the
/// basic spec rules, i.e. correct shader stage, data type, and IO direction.
/// It does not test more sophisticated rules like location and builtins being mutually exclusive or
/// that the correct properties are enabled.
struct BuiltInChecker {
    /// What combination of stage and IO direction is this builtin legal for
    EnumSet<IOAttributeUsage> valid_usages;

    /// What values for depth_mode are valid for this builtin.
    /// Currently, kUndefined is the only valid option for non-frag_depth
    EnumSet<BuiltinDepthMode> valid_depth_modes =
        EnumSet<BuiltinDepthMode>{BuiltinDepthMode::kUndefined};

    /// Implements logic for checking if the given type is valid or not. Is not a data entry (i.e. a
    /// type or set of types), because types are part of the IR module and created at runtime.
    using TypeCheckFn = bool(const core::type::Type* type, const Properties& props);

    /// @see #TypeCheckFn
    TypeCheckFn* const type_check;

    /// Message for logging if the type check fails. Cannot be easily generated at runtime, because
    /// the type check is a function, not just a data entry.
    const char* type_error;
};

constexpr BuiltInChecker kPointSizeChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32",
};

/// returns true if the number of elements in @p ty is valid for use in clip_distances without
/// Property::kAllowClipDistancesOnF32.
constexpr auto ClipDistancesElementsCheck = [](const core::type::Type* ty) -> bool {
    const auto elems = ty->Elements();
    return elems.type && elems.type->Is<core::type::F32>() && elems.count <= 8;
};

constexpr BuiltInChecker kClipDistancesChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::Array>() && ClipDistancesElementsCheck(ty);
    },
    .type_error = "must be an array<f32, N>, where N <= 8",
};

constexpr BuiltInChecker kClipDistancesAllowF32ScalarAndVectorChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ((ty->Is<core::type::Array>() || ty->Is<core::type::Vector>()) &&
                ClipDistancesElementsCheck(ty)) ||
               ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32 or either a vecN<f32> or an array<f32, N>, where N <= 8",
};

constexpr BuiltInChecker kCullDistanceChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::Array>() && ty->DeepestElement()->Is<core::type::F32>();
    },
    .type_error = "must be an array of f32",
};

constexpr BuiltInChecker kFragDepthChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentOutputUsage},
    .valid_depth_modes =
        EnumSet<BuiltinDepthMode>{BuiltinDepthMode::kUndefined, BuiltinDepthMode::kGreater,
                                  BuiltinDepthMode::kLess},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32",
};

constexpr BuiltInChecker kFrontFacingChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::Bool>();
    },
    .type_error = "must be a bool",
};

constexpr BuiltInChecker kGlobalInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kInstanceIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kLocalInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kLocalInvocationIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kNumSubgroupsChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kNumWorkgroupsChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kPositionChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage,
                                              IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 4 && vec->Type()->Is<core::type::F32>();
    },
    .type_error = "must be an vec4<f32>",
};

constexpr BuiltInChecker kSampleIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSampleMaskChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kFragmentOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupSizeChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kVertexIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kWorkgroupIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kPrimitiveIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kBarycentricCoordChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::F32>();
    },
    .type_error = "must be an vec3<f32>",
};

/// @returns an appropriate BuiltInCheck for @p builtin, ICEs when one isn't defined
const BuiltInChecker& BuiltinCheckerFor(BuiltinValue builtin, const Properties& properties) {
    switch (builtin) {
        case BuiltinValue::kPointSize:
            return kPointSizeChecker;
        case BuiltinValue::kClipDistances:
            if (properties.Contains(Property::kAllowClipDistancesOnF32ScalarAndVector)) {
                return kClipDistancesAllowF32ScalarAndVectorChecker;
            }
            return kClipDistancesChecker;
        case BuiltinValue::kCullDistance:
            return kCullDistanceChecker;
        case BuiltinValue::kFragDepth:
            return kFragDepthChecker;
        case BuiltinValue::kFrontFacing:
            return kFrontFacingChecker;
        case BuiltinValue::kGlobalInvocationId:
            return kGlobalInvocationIdChecker;
        case BuiltinValue::kInstanceIndex:
            return kInstanceIndexChecker;
        case BuiltinValue::kLocalInvocationId:
            return kLocalInvocationIdChecker;
        case BuiltinValue::kLocalInvocationIndex:
        case BuiltinValue::kGlobalInvocationIndex:
        case BuiltinValue::kWorkgroupIndex:
            return kLocalInvocationIndexChecker;
        case BuiltinValue::kNumSubgroups:
            return kNumSubgroupsChecker;
        case BuiltinValue::kNumWorkgroups:
            return kNumWorkgroupsChecker;
        case BuiltinValue::kPosition:
            return kPositionChecker;
        case BuiltinValue::kSampleIndex:
            return kSampleIndexChecker;
        case BuiltinValue::kSampleMask:
            return kSampleMaskChecker;
        case BuiltinValue::kSubgroupId:
            return kSubgroupIdChecker;
        case BuiltinValue::kSubgroupInvocationId:
            return kSubgroupInvocationIdChecker;
        case BuiltinValue::kSubgroupSize:
            return kSubgroupSizeChecker;
        case BuiltinValue::kVertexIndex:
            return kVertexIndexChecker;
        case BuiltinValue::kWorkgroupId:
            return kWorkgroupIdChecker;
        case BuiltinValue::kPrimitiveIndex:
            return kPrimitiveIndexChecker;
        case BuiltinValue::kBarycentricCoord:
            return kBarycentricCoordChecker;
        default:
            TINT_ICE() << builtin << " is does not have a checker defined for it";
    }
}

constexpr Validator::IOAttributeChecker kInvariantChecker{
    .kind = IOAttributeKind::kInvariant,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage,
                                              IOAttributeUsage::kFragmentInputUsage},
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kResultValue,
                                            ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*,
                const IOAttributes& attr,
                const Properties&,
                IOAttributeUsage) -> Result<SuccessType, std::string> {
        if (attr.builtin != BuiltinValue::kPosition) {
            return {"invariant can only decorate a value if it is also decorated with position"};
        }
        return Success;
    },
    .type_check = kPositionChecker.type_check,
    .type_error = kPositionChecker.type_error,
};

constexpr Validator::IOAttributeChecker kBuiltinChecker{
    .kind = IOAttributeKind::kBuiltin,
    .valid_usages =
        EnumSet<IOAttributeUsage>{
            IOAttributeUsage::kComputeInputUsage,
            IOAttributeUsage::kComputeOutputUsage,
            IOAttributeUsage::kFragmentInputUsage,
            IOAttributeUsage::kFragmentOutputUsage,
            IOAttributeUsage::kVertexInputUsage,
            IOAttributeUsage::kVertexOutputUsage,
        },
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kResultValue,
                                            ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type* ty,
                const IOAttributes& attr,
                const Properties& prop,
                IOAttributeUsage usage) -> Result<SuccessType, std::string> {
        if (!attr.builtin.has_value()) {
            return Success;
        }

        const auto builtin = attr.builtin.value();
        const auto& checker = BuiltinCheckerFor(builtin, prop);
        if (usage != IOAttributeUsage::kUndefinedUsage && !checker.valid_usages.Contains(usage)) {
            std::stringstream msg;
            msg << ToString(builtin) << " cannot be used on a " << ToString(usage) << ". ";
            if (checker.valid_usages.Size() == 1) {
                const auto v = *checker.valid_usages.begin();
                msg << "It can only be used on a " << ToString(v) << ".";
            } else {
                msg << "It can only be used on one of " << ToString(checker.valid_usages);
            }
            return msg.str();
        }

        if (!checker.type_check(ty, prop)) {
            std::stringstream msg;
            msg << ToString(builtin) << " " << checker.type_error;
            return msg.str();
        }

        const auto depth_mode = attr.depth_mode.value_or(BuiltinDepthMode::kUndefined);
        if (!checker.valid_depth_modes.Contains(depth_mode)) {
            std::stringstream msg;
            msg << ToString(builtin) << " cannot have a depth mode of " << ToString(depth_mode)
                << ". ";
            if (checker.valid_depth_modes.Size() == 1) {
                const auto v = *checker.valid_depth_modes.begin();
                msg << "It can only be " << ToString(v) << ".";
            } else {
                msg << "It must be one of " << ToString(checker.valid_depth_modes);
            }
            return msg.str();
        }

        if (builtin == BuiltinValue::kPointSize &&
            !prop.Contains(Property::kAllowPointSizeBuiltin)) {
            return std::string{"use of point_size builtin requires kAllowPointSizeBuiltin"};
        }

        return Success;
    },
    .type_check = [](const core::type::Type*, const Properties&) -> bool { return true; },
    .type_error = nullptr,
};

constexpr Validator::IOAttributeChecker kColorChecker{
    .kind = IOAttributeKind::kColor,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .valid_io_kinds =
        EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kModuleScopeVar,
                              ShaderIOKind::kResultValue},
    .check = [](const core::type::Type*, const IOAttributes&, const Properties&, IOAttributeUsage)
        -> Result<SuccessType, std::string> { return Success; },
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->IsNumericScalarOrVector();
    },
    .type_error = "must be a numeric scalar or vector",
};

constexpr Validator::IOAttributeChecker kInputAttachmentIndexChecker{
    .kind = IOAttributeKind::kInputAttachmentIndex,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentResourceUsage},
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*, const IOAttributes&, const Properties&, IOAttributeUsage)
        -> Result<SuccessType, std::string> { return Success; },
    .type_check = [](const core::type::Type* ty, const Properties& props) -> bool {
        return props.Contains(Property::kAllowAnyInputAttachmentIndexType) ||
               ty->Is<core::type::InputAttachment>();
    },
    .type_error = "must be an input_attachment",
};

constexpr Validator::IOAttributeChecker kDepthModeChecker{
    .kind = IOAttributeKind::kDepthMode,
    .valid_usages = kBuiltinChecker.valid_usages,
    .valid_io_kinds = kBuiltinChecker.valid_io_kinds,
    // kBuiltInChecker does the checking of the depth_mode value for the specific builtin.
    .check = [](const core::type::Type*,
                const IOAttributes& attr,
                const Properties&,
                IOAttributeUsage) -> Result<SuccessType, std::string> {
        if (!attr.builtin.has_value()) {
            return {"cannot have a depth_mode without a builtin"};
        }
        return Success;
    },
    .type_check = [](const core::type::Type*, const Properties&) -> bool { return true; },
    .type_error = nullptr,
};

// kBlendSrcChecker, kLocationChecker, kInterpolationChecker, and kBindingPointChecker are
// intentionally not implemented

}  // namespace

std::string ToString(IOAttributeUsage value) {
    switch (value) {
        case IOAttributeUsage::kComputeInputUsage:
            return "compute shader input";
        case IOAttributeUsage::kComputeOutputUsage:
            return "compute shader output";
        case IOAttributeUsage::kComputeResourceUsage:
            return "compute shader resource";
        case IOAttributeUsage::kFragmentInputUsage:
            return "fragment shader input";
        case IOAttributeUsage::kFragmentOutputUsage:
            return "fragment shader output";
        case IOAttributeUsage::kFragmentResourceUsage:
            return "fragment shader resource";
        case IOAttributeUsage::kVertexInputUsage:
            return "vertex shader input";
        case IOAttributeUsage::kVertexOutputUsage:
            return "vertex shader output";
        case IOAttributeUsage::kVertexResourceUsage:
            return "vertex shader resourcee";
        case IOAttributeUsage::kUndefinedUsage:
            return "non-entry point usage";
    }
    TINT_ICE() << "Unknown enum passed to ToString(IOAttribute)";
}

std::string_view ToString(IODirection value) {
    switch (value) {
        case IODirection::kInput:
            return "input";
        case IODirection::kOutput:
            return "output";
        case IODirection::kResource:
            return "resource";
    }
    TINT_ICE() << "Unknown enum passed to ToString(IODirection)";
}

std::string ToString(IOAnnotation value) {
    switch (value) {
        case IOAnnotation::kBindingPoint:
            return "@group + @binding";
        case IOAnnotation::kLocation:
            return "@location";
        case IOAnnotation::kBuiltin:
            return "built-in";
        case IOAnnotation::kWorkgroup:
            return "<workgroup>";
        case IOAnnotation::kColor:
            return "@color";
    }
    TINT_ICE() << "Unknown enum passed to ToString(IOAnnotation)";
}

std::string ToString(ShaderIOKind value) {
    switch (value) {
        case ShaderIOKind::kInputParam:
            return "input param";
        case ShaderIOKind::kResultValue:
            return "return value";
        case ShaderIOKind::kModuleScopeVar:
            return "module scope variable";
    }
    TINT_ICE() << "Unknown enum passed to ToString(ShaderIOKind)";
}

Vector<const Validator::IOAttributeChecker*, 4> Validator::IOAttributeCheckersFor(
    const IOAttributes& attr,
    bool skip_builtin) {
    Vector<const IOAttributeChecker*, 4> checkers{};
    if (attr.invariant) {
        checkers.Push(&kInvariantChecker);
    }
    if (!skip_builtin && attr.builtin.has_value()) {
        checkers.Push(&kBuiltinChecker);
    }
    if (attr.color.has_value()) {
        checkers.Push(&kColorChecker);
    }
    if (attr.input_attachment_index.has_value()) {
        checkers.Push(&kInputAttachmentIndexChecker);
    }
    if (attr.depth_mode.has_value()) {
        checkers.Push(&kDepthModeChecker);
    }

    // attr.blend_src, attr.location, attr.interpolation, and attr.binding_point are intentionally
    // skipped, because their rules are not amenable to implementation via IOAttributeChecker.
    return checkers;
}

void Validator::ValidateShaderIOAnnotations(const CastableBase* msg_anchor,
                                            const core::type::Type* ty,
                                            const std::optional<BindingPoint>& binding_point,
                                            const IOAttributes& attr,
                                            ShaderIOKind kind) {
    EnumSet<IOAnnotation> annotations;

    // Since there is no entries in the set at this point, this should never fail.
    TINT_ASSERT(AddIOAnnotationsFromIOAttributes(annotations, attr) == Success);

    if (binding_point.has_value()) {
        annotations.Add(IOAnnotation::kBindingPoint);
    }

    if (auto* mv = ty->As<core::type::MemoryView>()) {
        if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
            annotations.Add(IOAnnotation::kWorkgroup);
        }
    }

    if (ty->Is<core::type::Void>()) {
        if (!annotations.Empty()) {
            AddError(msg_anchor) << ToString(kind) << " with void type should never be annotated";
        }
        return;  // Early return because later rules assume non-void types.
    }

    if (attr.location.has_value()) {
        if (ir_.properties.Contains(Property::kAllowLocationForNumericComposites)) {
            std::function<bool(const core::type::Type*)> is_numeric =
                [&is_numeric](const core::type::Type* t) -> bool {
                t = t->UnwrapPtrOrRef();
                bool result = false;
                tint::Switch(
                    t,
                    [&](const core::type::Struct* s) {
                        for (auto* m : s->Members()) {
                            if (!is_numeric(m->Type())) {
                                return;
                            }
                        }
                        result = true;
                    },
                    [&](Default) {
                        auto* e = t->DeepestElement()->UnwrapPtrOrRef();
                        tint::Switch(
                            e,  //
                            [&](const core::type::Struct* s) { result = is_numeric(s); },
                            [&](Default) { result = e->IsNumericScalarOrVector(); });
                    });
                return result;
            };
            if (!is_numeric(ty)) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " with a location attribute must contain only numeric elements "
                    << ty->FriendlyName();
                return;
            }
        } else {
            if (!ty->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " with a location attribute must be a numeric scalar or "
                                        "vector, but has type "
                                     << ty->FriendlyName();
                return;
            }
        }
    }

    if (auto* ty_struct = ty->UnwrapPtrOrRef()->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            EnumSet<IOAnnotation> mem_annotations = annotations;
            auto add_result = AddIOAnnotationsFromIOAttributes(mem_annotations, mem->Attributes());
            if (add_result != Success) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " struct member has same IO annotation, as top-level struct, '"
                    << ToString(add_result.Failure()) << "'";
                return;
            }

            if (!CheckStructMemberAttributes(mem, [&]() -> diag::Diagnostic& {
                    return AddError(msg_anchor) << ToString(kind) << " ";
                })) {
                return;
            }

            if (ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
                if (auto* mv = mem->Type()->As<core::type::MemoryView>()) {
                    if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
                        mem_annotations.Add(IOAnnotation::kWorkgroup);
                    }
                }
            }

            if (mem_annotations.Empty()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " struct members must have at least one IO annotation, "
                                        "e.g. a binding point, a location, etc";
            } else if (mem_annotations.Size() > 1) {
                AddError(msg_anchor)
                    << ToString(kind) << " struct member has more than one IO annotation, "
                    << ToString(mem_annotations);
            }
        }
    } else {
        if (annotations.Empty()) {
            if (!(ir_.properties.Contains(Property::kAllowUnannotatedModuleIOVariables) &&
                  kind == ShaderIOKind::kModuleScopeVar)) {
                AddError(msg_anchor) << ToString(kind)
                                     << " must have at least one IO annotation, e.g. a binding "
                                        "point, a location, etc";
            }
        } else if (annotations.Size() > 1) {
            AddError(msg_anchor) << ToString(kind) << " has more than one IO annotation, "
                                 << ToString(annotations);
        }
    }
}

void Validator::ValidateIOAttributes(const Function* func) {
    const auto stage = func->Stage();
    struct Task {
        const CastableBase* anchor;
        const core::type::Type* type;
        const IOAttributes& attr;
        IODirection dir;
        ShaderIOKind io_kind;
    };
    Vector<Task, 16> tasks;

    // Gather parameters.
    for (auto* param : func->Params()) {
        tasks.Push({param, param->Type(), param->Attributes(), IODirection::kInput,
                    ShaderIOKind::kInputParam});
    }

    // Gather return value.
    tasks.Push({func, func->ReturnType(), func->ReturnAttributes(), IODirection::kOutput,
                ShaderIOKind::kResultValue});

    // Gather referenced module variables.
    for (auto* var : referenced_module_vars_.TransitiveReferences(func)) {
        auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        if (mv == nullptr) {
            continue;
        }
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut ||
            mv->AddressSpace() == AddressSpace::kHandle) {
            tasks.Push({var, mv->StoreType(), var->Attributes(),
                        validator::IODirectionFor(mv->AddressSpace()),
                        ShaderIOKind::kModuleScopeVar});
        }
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Shared context for blend_src and location validation
        BlendSrcContext input_ctx{func->Stage(), {}, {}, nullptr, IODirection::kInput};
        BlendSrcContext output_ctx{func->Stage(), {}, {}, nullptr, IODirection::kOutput};

        // First pass: pre-populate location hashes for blend_src.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            WalkTypeAndMembers(
                ctx, task.type, task.attr,
                [task](BlendSrcContext& c, const core::type::Type*, const IOAttributes& a) {
                    if (a.blend_src.has_value() && a.location.has_value()) {
                        c.locations.Add(a.location.value(), task.anchor);
                    }
                });
        }

        // Second pass: validate blend_src usages.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            CheckBlendSrc(ctx, task.anchor, task.type, task.attr);
        }

        if (!output_ctx.blend_srcs.IsEmpty()) {
            if (output_ctx.blend_srcs.Count() != 2) {
                AddError(func) << "if any @blend_src is used on an output, then @blend_src(0) and "
                                  "@blend_src(1) must be used";
            }
        }

        // Third pass: validate all non-blend_src location usages.
        for (const auto& task : tasks) {
            if (task.dir == IODirection::kInput) {
                CheckLocation(input_ctx.locations, task.anchor, task.attr, func->Stage(), task.type,
                              task.dir);
            } else if (task.dir == IODirection::kOutput) {
                CheckLocation(output_ctx.locations, task.anchor, task.attr, func->Stage(),
                              task.type, task.dir);
            }
        }
    }

    // Validate all the interpolation usages.
    for (const auto& task : tasks) {
        CheckInterpolation(task.anchor, task.type, task.attr, stage, task.dir);
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Validate all the binding_point usages, and ensure things that require binding_point have
        // them.
        for (const auto& task : tasks) {
            CheckBindingPoint(task.anchor, task.type, task.attr, task.io_kind);
        }
    }

    IOAttributeContext impl_ctx{.input_builtins = {}, .output_builtins = {}};
    // Validate all remaining attributes on IO objects
    for (const auto& task : tasks) {
        ValidateIOAttributesImpl(impl_ctx, task.anchor, task.type, task.attr, stage, task.dir,
                                 task.io_kind);
    }
}

void Validator::ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                         const CastableBase* msg_anchor,
                                         const core::type::Type* ty,
                                         const IOAttributes& attr,
                                         Function::PipelineStage stage,
                                         IODirection dir,
                                         ShaderIOKind io_kind) {
    bool skip_builtins = ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO) &&
                         io_kind == ShaderIOKind::kModuleScopeVar;
    const IOAttributeUsage usage = IOAttributeUsageFor(stage, dir);
    WalkTypeAndMembers(
        *this, ty, attr,
        [&ctx, msg_anchor, usage, io_kind, skip_builtins, dir, this](
            Validator& v, const core::type::Type* t, const IOAttributes& a) {
            const auto checkers = IOAttributeCheckersFor(a, skip_builtins);
            if (checkers.IsEmpty()) {
                return;
            }

            if (a.builtin.has_value() && !skip_builtins &&
                usage != IOAttributeUsage::kUndefinedUsage) {
                const auto& builtin = a.builtin.value();

                uint32_t count = 0;
                switch (dir) {
                    case IODirection::kInput:
                        count = ++(ctx.input_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    case IODirection::kOutput:
                        count = ++(ctx.output_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    default:
                        // This shouldn't ever happen, but this will get caught later in the
                        // checker, so just ignoring
                        break;
                }
                if (v.ir_.properties.Contains(Property::kAllowClipDistancesOnF32ScalarAndVector) &&
                    builtin == BuiltinValue::kClipDistances) {
                    if (count > 2) {
                        v.AddError(msg_anchor)
                            << "too many instances of builtin 'clip_distances' on entry point "
                            << ToString(dir)
                            << ", only two allowed with 'kAllowClipDistancesOnF32ScalarAndVector' "
                               "property enabled";
                    }
                } else {
                    if (count > 1) {
                        v.AddError(msg_anchor)
                            << "duplicate instance of builtin '" << ToString(builtin)
                            << "' on entry point " << ToString(dir)
                            << ", must be unique per entry point i/o direction";
                    }
                }
            }

            auto failed = tint::Hashset<const IOAttributeChecker*, 4>();

            if (usage != IOAttributeUsage::kUndefinedUsage) {
                for (const auto* checker : checkers) {
                    if (!checker->valid_usages.Contains(usage)) {
                        failed.Add(checker);

                        std::stringstream msg;
                        msg << ToString(checker->kind) << " IO attributes cannot be declared for a "
                            << ToString(usage) << ". ";
                        if (checker->valid_usages.Size() == 1) {
                            const auto& u = *checker->valid_usages.begin();
                            msg << "They can only be used for a " << ToString(u) << ".";
                        } else {
                            msg << "They can only be used for " << ToString(checker->valid_usages);
                        }
                        v.AddError(msg_anchor) << msg.str();
                    }
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->valid_io_kinds.Contains(io_kind)) {
                    failed.Add(checker);

                    std::stringstream msg;
                    msg << ToString(checker->kind) << " IO attributes cannot be declared on a "
                        << ToString(io_kind) << ". ";
                    if (checker->valid_io_kinds.Size() == 1) {
                        const auto& k = *checker->valid_io_kinds.begin();
                        msg << "They can only be used on a " << ToString(k) << ".";
                    } else {
                        msg << "They can only be used on " << ToString(checker->valid_io_kinds);
                    }
                    v.AddError(msg_anchor) << msg.str();
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->type_check(t, v.ir_.properties)) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << ToString(checker->kind) << " " << checker->type_error;
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (auto res = checker->check(t, a, v.ir_.properties, usage); res != Success) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << res.Failure();
                }
            }
        });
}

void Validator::CheckNotBool(const CastableBase* msg_anchor,
                             const core::type::Type* ty,
                             const std::string& err) {
    if (ty->Is<core::type::Bool>()) {
        AddError(msg_anchor) << err;
    }
}

void Validator::CheckFrontFacingIfBool(const CastableBase* msg_anchor,
                                       const IOAttributes& attr,
                                       const core::type::Type* ty,
                                       const std::string& err) {
    if (ty->Is<core::type::Bool>() && attr.builtin != BuiltinValue::kFrontFacing) {
        AddError(msg_anchor) << err;
    }
}

void Validator::CheckBlendSrc(BlendSrcContext& ctx,
                              const CastableBase* target,
                              const core::type::Type* ty,
                              const IOAttributes& attr) {
    if (attr.blend_src.has_value()) {
        if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
            AddError(target) << "blend_src cannot be used on non-struct-member types";
        }
        CheckBlendSrcImpl(ctx, target, ty, attr);
    }

    if (auto* s = ty->As<core::type::Struct>()) {
        if (s->Members().Any([](auto* m) { return m->Attributes().blend_src.has_value(); })) {
            auto location_count = 0u;
            for (const auto* mem : s->Members()) {
                auto& mem_attr = mem->Attributes();
                if (mem_attr.location.has_value()) {
                    location_count++;
                }
                CheckBlendSrcImpl(ctx, target, mem->Type(), mem_attr);
            }

            if (location_count != 2) {
                AddError(target)
                    << "structs with blend_src members must have exactly 2 members with "
                       "location annotations";
            }
            return;
        }
    }

    // Reject blend_src on nested members
    if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
        WalkTypeAndMembers(
            ctx, ty, attr,
            [&target, this](BlendSrcContext&, const core::type::Type*, const IOAttributes& a) {
                if (a.blend_src.has_value()) {
                    AddError(target)
                        << "blend_src cannot be used on members of non-top level structs";
                }
            });
    }
}

void Validator::CheckBlendSrcImpl(BlendSrcContext& ctx,
                                  const CastableBase* target,
                                  const core::type::Type* ty,
                                  const IOAttributes& attr) {
    if (!attr.blend_src.has_value()) {
        return;
    }

    auto bs_val = attr.blend_src.value();
    if (bs_val != 0 && bs_val != 1) {
        AddError(target) << "blend_src value must be 0 or 1";
    }
    if (!ctx.blend_srcs.Add(bs_val)) {
        AddError(target) << "duplicate blend_src(" << bs_val << ") on entry point "
                         << ToString(ctx.dir);
    }

    if (ctx.dir != IODirection::kOutput || ctx.stage != Function::PipelineStage::kFragment) {
        AddError(target) << "blend_src can only be used on fragment shader outputs";
        return;
    }
    if (!attr.location.has_value() || attr.location.value() != 0) {
        AddError(target) << "struct members with blend_src must be located at 0";
    }

    if (!ctx.blend_src_type) {
        if (!ty->IsNumericScalarOrVector()) {
            AddError(target) << "blend_src must be a numeric scalar or vector, but has type "
                             << ty->FriendlyName();
        }
        ctx.blend_src_type = ty;
    } else if (ctx.blend_src_type != ty) {
        AddError(target) << "blend_src type " << ty->FriendlyName()
                         << " does not match other blend_src type "
                         << ctx.blend_src_type->FriendlyName();
    }
}

void Validator::CheckLocation(Hashmap<uint32_t, const CastableBase*, 4>& locations,
                              const CastableBase* target,
                              const IOAttributes& attr,
                              const Function::PipelineStage stage,
                              const core::type::Type* type,
                              const IODirection dir) {
    struct WalkContext {
        Validator* validator;
        Hashmap<uint32_t, const CastableBase*, 4>& locations;
        const CastableBase* target;
        const Function::PipelineStage stage;
        const IODirection dir;
    };
    WalkContext ctx{this, locations, target, stage, dir};

    WalkTypeAndMembers(
        ctx, type, attr,
        [](WalkContext& context, const core::type::Type* ty, const IOAttributes& attribute) {
            if (ty->Is<core::type::Struct>()) {
                return;
            }

            if (attribute.blend_src) {
                // locations associated with a blend_src usage should already be
                // pre-populated in locations
                return;
            }

            if (attribute.location.has_value()) {
                if (context.stage == Function::PipelineStage::kCompute &&
                    context.dir == IODirection::kInput) {
                    context.validator->AddError(context.target)
                        << "location attribute is not valid for compute shader inputs";
                }

                auto loc = attribute.location.value();
                if (const auto conflict = context.locations.Get(loc)) {
                    context.validator->AddError(context.target)
                        << "duplicate location(" << loc << ") on entry point "
                        << ToString(context.dir);
                    context.validator->AddDeclarationNote(*conflict.value);
                } else {
                    context.locations.Add(loc, context.target);
                }
            }
        });
}

void Validator::CheckInterpolation(const CastableBase* anchor,
                                   const core::type::Type* ty,
                                   const IOAttributes& attr,
                                   const Function::PipelineStage stage,
                                   const IODirection dir) {
    if (!ty) {
        return;
    }

    bool ctx = false;

    WalkTypeAndMembers(
        ctx, ty, attr,
        [this, anchor, stage, dir](bool& in_location_composite, const core::type::Type* t,
                                   const IOAttributes& a) {
            bool has_location = a.location.has_value() || in_location_composite;
            if (!has_location) {
                if (auto* str = t->As<core::type::Struct>()) {
                    has_location |= str->Members().All(
                        [](const auto* mem) { return mem->Attributes().location.has_value(); });
                }
            }

            if (a.interpolation.has_value()) {
                has_location |= (ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO) &&
                                 a.builtin.has_value());

                if (!ir_.properties.Contains(Property::kAllowLocationForNumericComposites) &&
                    t->As<core::type::Struct>()) {
                    AddError(anchor) << "interpolation cannot be applied to a struct without "
                                        "'kAllowLocationForNumericComposites' property";
                }

                if (t->IsIntegerScalarOrVector()) {
                    if (a.interpolation.value().type != InterpolationType::kFlat) {
                        AddError(anchor)
                            << "interpolation attribute type must be flat for integral types";
                    }
                }

                auto interp_type = a.interpolation.value().type;
                auto interp_sampling = a.interpolation.value().sampling;
                if (interp_sampling != InterpolationSampling::kUndefined) {
                    switch (interp_type) {
                        case InterpolationType::kFlat:
                            if (interp_sampling != InterpolationSampling::kFirst &&
                                interp_sampling != InterpolationSampling::kEither) {
                                AddError(anchor) << "flat interpolation can only use 'first', "
                                                    "'either' or undefined sampling parameters";
                            }
                            break;
                        case InterpolationType::kLinear:
                        case InterpolationType::kPerspective:
                            if (interp_sampling != InterpolationSampling::kCenter &&
                                interp_sampling != InterpolationSampling::kCentroid &&
                                interp_sampling != InterpolationSampling::kSample) {
                                AddError(anchor) << "linear and perspective interpolation can only "
                                                    "use 'center', 'centroid', 'sample', or "
                                                    "undefined sampling parameters";
                            }
                            break;
                        case InterpolationType::kUndefined:
                            AddError(anchor) << "undefined interpolation should on have an "
                                                "undefined sampling parameter";
                            break;
                        default:
                            TINT_UNREACHABLE();
                    }
                }

                if (!has_location) {
                    if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
                        AddError(anchor) << "interpolation attribute requires a location attribute";
                    } else {
                        AddError(anchor) << "interpolation attribute requires a location attribute "
                                            "(or location-like shader I/O annotation)";
                    }
                }
            } else if (has_location && t->IsIntegerScalarOrVector()) {
                // Integral vertex outputs and fragment inputs require flat interpolation.
                const bool needs_flat =
                    (stage == Function::PipelineStage::kVertex && dir == IODirection::kOutput) ||
                    (stage == Function::PipelineStage::kFragment && dir == IODirection::kInput);
                if (needs_flat) {
                    AddError(anchor) << "integral user-defined inputs and outputs must have an "
                                        "@interpolate(flat) attribute";
                }
            }

            if (t->IsAnyOf<core::type::Array, core::type::Struct>()) {
                in_location_composite |= a.location.has_value();
            }
        });
}

void Validator::CheckBindingPoint(const CastableBase* anchor,
                                  const core::type::Type* ty,
                                  const IOAttributes& attr,
                                  const ShaderIOKind& io_kind) {
    const auto& binding_point = attr.binding_point;
    auto address_space = AddressSpace::kUndefined;
    if (const auto* mv = ty->As<core::type::MemoryView>()) {
        address_space = mv->AddressSpace();
    } else {
        // ModuleScopeVars transform in MSL backends unwraps pointers to handles
        if (ty->IsHandle()) {
            address_space = AddressSpace::kHandle;
        }
    }

    if (binding_point.has_value() && io_kind != ShaderIOKind::kModuleScopeVar &&
        !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
        AddError(anchor) << "binding_points are only valid on resource variables";
    }

    switch (address_space) {
        case AddressSpace::kHandle:
            if (!binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " resource requires a binding point";
            }
            break;
        case AddressSpace::kStorage:
        case AddressSpace::kUniform:
            if (!binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " resource requires a binding point";
            }
            break;
        default:
            if (binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " non-resource cannot have a binding point";
            }
            break;
    }
}

}  // namespace tint::core::ir::validator
