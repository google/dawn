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

#include "src/tint/lang/core/ir/structural_validator.h"

#include <algorithm>
#include <string_view>
#include <utility>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/referenced_functions.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/function.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/text_style.h"

namespace tint::core::ir::validator {
namespace {

/// @returns a human-readable string for an IODirection
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

/// @returns text describing the shader IO kind for error logging
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

/// @returns the parent block of @p block
const Block* ParentBlockOf(const Block* block) {
    if (auto* parent = block->Parent()) {
        return parent->Block();
    }
    return nullptr;
}

/// @returns true if @p block directly or transitively holds the instruction @p inst
bool TransitivelyHolds(const Block* block, const Instruction* inst) {
    for (auto* b = inst->Block(); b; b = ParentBlockOf(b)) {
        if (b == block) {
            return true;
        }
    }
    return false;
}

/// @returns true if @p type is in the core namespace
bool IsCoreType(const core::type::Type* type) {
    return std::string_view(type->TypeInfo().name).starts_with("tint::core");
}

/// @returns true if @p ty meets the basic function parameter rules (i.e. one of constructible,
///          pointer, handle).
///
/// Note: Does not handle corner cases like if certain properties are present.
bool IsValidFunctionParamType(const core::type::Type* ty) {
    if (ty->IsConstructible() || ty->IsHandle()) {
        return true;
    }

    if (auto* ptr = ty->As<core::type::Pointer>()) {
        return ptr->AddressSpace() != core::AddressSpace::kHandle;
    }
    return false;
}

/// @returns true if @p ty is a non-struct and decorated with @builtin(position), or if it is a
/// struct and one of its members is decorated, otherwise false.
/// @param attr attributes attached to data
/// @param ty type of the data being tested
bool IsPositionPresent(const IOAttributes& attr, const core::type::Type* ty) {
    if (auto* ty_struct = ty->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            if (mem->Attributes().builtin == BuiltinValue::kPosition) {
                return true;
            }
        }
        return false;
    }

    return attr.builtin == BuiltinValue::kPosition;
}

template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl);
/// Helper that walks the members of a struct, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param str the struct to walk the members of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkStructMembers(CTX& ctx, const core::type::Struct* str, IMPL&& impl) {
    for (auto* member : str->Members()) {
        WalkTypeAndMembers(ctx, member->Type(), member->Attributes(), impl);
    }
}

/// Helper that walks an array's element type, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param arr the array to walk the element type of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkArrayElements(CTX& ctx, const core::type::Array* arr, IMPL&& impl) {
    WalkTypeAndMembers(ctx, arr->ElemType(), IOAttributes{}, impl);
}

/// Helper for walking a type that maybe a struct, calling an impl function for the type and each of
/// its members.
/// @param ctx a context object to pass to the implementation function
/// @param type the type to walk
/// @param attr the attributes for @p type
/// @param impl a function with the signature `void(const core::type::Type*, const IOAttributes&,
///             CTX&)` that is called for each type.
template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl) {
    impl(ctx, type, attr);
    tint::Switch(
        type, [&](const core::type::Struct* s) { WalkStructMembers(ctx, s, impl); },
        [&](const core::type::Array* a) { WalkArrayElements(ctx, a, impl); });
}

/// How an attribute is being used, a tuple of the shader stage and IO direction
enum class IOAttributeUsage : uint8_t {
    kComputeInputUsage,
    kComputeOutputUsage,
    kComputeResourceUsage,
    kFragmentInputUsage,
    kFragmentOutputUsage,
    kFragmentResourceUsage,
    kVertexInputUsage,
    kVertexOutputUsage,
    kVertexResourceUsage,
    kUndefinedUsage,
};

/// @returns a human-readable string for an IOAttributeUsage
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

/// @returns the IOAttributeUsage for a given Function::PipelineStage + IODirection tuple
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

/// @returns the Function::PipelineStage for an IOAttributeUsage
[[maybe_unused]] Function::PipelineStage PipelineStageFor(IOAttributeUsage usage) {
    switch (usage) {
        case IOAttributeUsage::kComputeInputUsage:
        case IOAttributeUsage::kComputeOutputUsage:
        case IOAttributeUsage::kComputeResourceUsage:
            return Function::PipelineStage::kCompute;
        case IOAttributeUsage::kFragmentInputUsage:
        case IOAttributeUsage::kFragmentOutputUsage:
        case IOAttributeUsage::kFragmentResourceUsage:
            return Function::PipelineStage::kFragment;
        case IOAttributeUsage::kVertexInputUsage:
        case IOAttributeUsage::kVertexOutputUsage:
        case IOAttributeUsage::kVertexResourceUsage:
            return Function::PipelineStage::kVertex;
        case IOAttributeUsage::kUndefinedUsage:
            return Function::PipelineStage::kUndefined;
    }
    TINT_ICE() << "Unknown IOAttribute usage " << ToString(usage);
}

/// @returns the IODirection for an IOAttributeUsage
[[maybe_unused]] IODirection IODirectionFor(IOAttributeUsage usage) {
    switch (usage) {
        case IOAttributeUsage::kComputeInputUsage:
        case IOAttributeUsage::kFragmentInputUsage:
        case IOAttributeUsage::kVertexInputUsage:
            return IODirection::kInput;
        case IOAttributeUsage::kComputeOutputUsage:
        case IOAttributeUsage::kFragmentOutputUsage:
        case IOAttributeUsage::kVertexOutputUsage:
            return IODirection::kOutput;
        case IOAttributeUsage::kComputeResourceUsage:
        case IOAttributeUsage::kFragmentResourceUsage:
        case IOAttributeUsage::kVertexResourceUsage:
        case IOAttributeUsage::kUndefinedUsage:
            return IODirection::kResource;  // Technically it could also be a kInput or kOutput, but
                                            // no validation depends on differentiate between these
                                            // cases currently.
    }
    TINT_ICE() << "Unknown IOAttribute usage " << ToString(usage);
}

/// A BuiltInChecker is the interface used to check that a usage of a builtin attribute meets the
/// basic spec rules, i.e. correct shader stage, data type, and IO direction.
/// It does not test more sophisticated rules like location and builtins being mutually exclusive or
/// that the correct capabilities are enabled.
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
/// Capability::kAllowClipDistancesOnF32.
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
        EnumSet<BuiltinDepthMode>{BuiltinDepthMode::kUndefined, BuiltinDepthMode::kAny,
                                  BuiltinDepthMode::kGreater, BuiltinDepthMode::kLess},
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

/// IOAttributeChecker is the interface used to check that a usage of an IO attribute
/// meets the spec rules for a given context.
struct IOAttributeChecker {
    /// What kinda of IO attribute is being checked
    IOAttributeKind kind;

    /// What combination of stage and IO direction is this attribute legal for.
    EnumSet<IOAttributeUsage> valid_usages;

    /// What type of shader IO values is this attribute legal for.
    EnumSet<ShaderIOKind> valid_io_kinds;

    /// Implements the validation logic for a specific attribute.
    using CheckFn = Result<SuccessType, std::string>(const core::type::Type* ty,
                                                     const IOAttributes& attr,
                                                     const Properties& prop,
                                                     IOAttributeUsage usage);

    /// The validation function.
    CheckFn* const check;

    /// Implements logic for checking if the given type is valid or not. Is not a data entry (i.e. a
    /// type or set of types), because types are part of the IR module and created at runtime.
    using TypeCheckFn = bool(const core::type::Type* type, const Properties& props);

    /// @see #TypeCheckFn
    TypeCheckFn* const type_check;

    /// Message for logging if the type check fails. Cannot be easily generated at runtime, because
    /// the type check is a function, not just a data entry.
    const char* type_error;
};

constexpr IOAttributeChecker kInvariantChecker{
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

constexpr IOAttributeChecker kBuiltinChecker{
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

constexpr IOAttributeChecker kColorChecker{
    .kind = IOAttributeKind::kColor,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .valid_io_kinds =
        EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*, const IOAttributes&, const Properties&, IOAttributeUsage)
        -> Result<SuccessType, std::string> { return Success; },
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->IsNumericScalarOrVector();
    },
    .type_error = "must be a numeric scalar or vector",
};

constexpr IOAttributeChecker kInputAttachmentIndexChecker{
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

constexpr IOAttributeChecker kDepthModeChecker{
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

/// @returns all the appropriate IOAttributeCheckers for @p attr
Vector<const IOAttributeChecker*, 4> IOAttributeCheckersFor(const IOAttributes& attr,
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

/// Annotations that can be associated with a value that are used for shader IO,
/// e.g. binding_points, @location, being in workgroup address space, etc.
/// These are a subset of IOAttributes.
enum class IOAnnotation : uint8_t {
    /// @group + @binding
    kBindingPoint,
    /// @location
    kLocation,
    /// @builtin(...)
    kBuiltin,
    /// Pointer to Workgroup address space
    kWorkgroup,
    /// @color
    kColor,
};

/// @returns text describing the annotation for error logging
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

/// Adds appropriate entries to annotations, based on what values are present in attributes
/// @param annotations the set to updated
/// @param attr the attributes to be examined
/// @returns Success if none of the values being added where already present, otherwise returns the
/// first non-unique value as a Failure
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

}  // namespace

Structural::Structural(const Module& ir, diag::List& diagnostics, Capabilities capabilities)
    : ir_(ir), diag_(diagnostics), capabilities_(capabilities), referenced_module_vars_(ir) {}

Structural::~Structural() = default;

Disassembler& Structural::Disassemble() {
    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(ir_));
    }
    return *disassembler_;
}

void Structural::Validate() {
    RunStructuralSoundnessChecks();

    CheckForRecursion();
    CheckForOrphanedInstructions();
    CheckStageRestrictedInstructions();
}

void Structural::CheckForRecursion() {
    if (diag_.ContainsErrors()) {
        return;
    }

    ReferencedFunctions<const Module> referenced_functions(ir_);
    for (auto& func : ir_.functions) {
        auto& refs = referenced_functions.TransitiveReferences(func);
        if (refs.Contains(func)) {
            // TODO(434684891): Consider improving this error with more information.
            AddError(func) << "recursive function calls are not allowed";
            return;
        }
    }
}

void Structural::CheckForOrphanedInstructions() {
    if (diag_.ContainsErrors()) {
        return;
    }

    // Check for orphaned instructions.
    for (auto* inst : ir_.Instructions()) {
        if (!visited_instructions_.Contains(inst)) {
            AddError(inst) << "orphaned instruction: " << inst->FriendlyName();
        }
    }
}

void Structural::CheckStageRestrictedInstructions() {
    if (diag_.ContainsErrors()) {
        return;
    }

    // Check for instructions being used in stages that do not support them.
    for (const auto& i : stage_restricted_instructions_) {
        const auto& inst = i.key;
        const auto& stages = i.value;
        const auto* f = ContainingFunction(inst);
        if (f == nullptr) {
            continue;
        }

        if (f->IsEntryPoint() && !stages.Contains(f->Stage())) {
            AddError(inst) << "cannot be used in a " << f->Stage() << " shader";
        } else {
            for (const Function* ep : ContainingEndPoints(f)) {
                if (!stages.Contains(ep->Stage())) {
                    AddError(inst) << "cannot be used in a " << ep->Stage() << " shader";
                }
            }
        }
    }
}

void Structural::RunStructuralSoundnessChecks() {
    scope_stack_.Push();
    TINT_DEFER({
        scope_stack_.Pop();
        TINT_ASSERT(scope_stack_.IsEmpty());
        TINT_ASSERT(tasks_.IsEmpty());
        TINT_ASSERT(control_stack_.IsEmpty());
        TINT_ASSERT(block_stack_.IsEmpty());
    });
    CheckRootBlock(ir_.root_block);

    for (auto& func : ir_.functions) {
        if (!all_functions_.Add(func)) {
            AddError(func) << "function " << NameOf(func) << " added to module multiple times";
        }
        scope_stack_.Add(func);
    }

    for (auto& func : ir_.functions) {
        block_to_function_.Add(func->Block(), func);
        CheckFunction(func);
    }
}

diag::Diagnostic& Structural::AddError(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Structural::AddError(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Structural::AddResultError(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Structural::AddError(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const CastableBase* base) {
    diag::Diagnostic* diag = nullptr;
    tint::Switch(
        base,  //
        [&](const Block* block) { diag = &AddError(block); },
        [&](const BlockParam* param) { diag = &AddError(param); },
        [&](const Function* fn) { diag = &AddError(fn); },
        [&](const FunctionParam* param) { diag = &AddError(param); },
        [&](const Instruction* inst) { diag = &AddError(inst); },
        [&](const InstructionResult* res) { diag = &AddError(res); });
    TINT_ASSERT(diag);
    return *diag;
}

diag::Diagnostic& Structural::AddNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    return AddNote(src);
}

diag::Diagnostic& Structural::AddNote(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddNote(src);
}

diag::Diagnostic& Structural::AddOperandNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Structural::AddResultNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Structural::AddNote(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Structural::AddError(Source src) {
    auto& diag = diag_.AddError(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

diag::Diagnostic& Structural::AddNote(Source src) {
    auto& diag = diag_.AddNote(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

void Structural::AddDeclarationNote(const CastableBase* decl) {
    tint::Switch(
        decl,  //
        [&](const Block* block) { AddDeclarationNote(block); },
        [&](const BlockParam* param) { AddDeclarationNote(param); },
        [&](const Function* fn) { AddDeclarationNote(fn); },
        [&](const FunctionParam* param) { AddDeclarationNote(param); },
        [&](const Instruction* inst) { AddDeclarationNote(inst); },
        [&](const InstructionResult* res) { AddDeclarationNote(res); });
}

void Structural::AddDeclarationNote(const Block* block) {
    auto src = Disassemble().BlockSource(block);
    if (src.file) {
        AddNote(src) << NameOf(block) << " declared here";
    }
}

void Structural::AddDeclarationNote(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Structural::AddDeclarationNote(const Function* fn) {
    AddNote(fn) << NameOf(fn) << " declared here";
}

void Structural::AddDeclarationNote(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Structural::AddDeclarationNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    if (src.file) {
        AddNote(src) << NameOf(inst) << " declared here";
    }
}

void Structural::AddDeclarationNote(const InstructionResult* res) {
    if (auto* inst = res->Instruction()) {
        auto results = inst->Results();
        for (size_t i = 0; i < results.Length(); i++) {
            if (results[i] == res) {
                AddResultNote(res->Instruction(), i) << NameOf(res) << " declared here";
                return;
            }
        }
    }
}

StyledText Structural::NameOf(const CastableBase* decl) {
    return tint::Switch(
        decl,  //
        [&](const core::type::Type* ty) { return NameOf(ty); },
        [&](const Value* value) { return NameOf(value); },
        [&](const Instruction* inst) { return NameOf(inst); },
        [&](const Block* block) { return NameOf(block); },  //
        TINT_ICE_ON_NO_MATCH);
}

StyledText Structural::NameOf(const core::type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

StyledText Structural::NameOf(const Value* value) {
    return Disassemble().NameOf(value);
}

StyledText Structural::NameOf(const Instruction* inst) {
    auto name = inst ? inst->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(name);
}

StyledText Structural::NameOf(const Block* block) {
    auto parent_name = block->Parent() ? block->Parent()->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(parent_name) << " block "
                        << Disassemble().NameOf(block);
}

bool Structural::CheckResult(const Instruction* inst, size_t idx) {
    auto* result = inst->Result(idx);
    if (DAWN_UNLIKELY(result == nullptr)) {
        AddResultError(inst, idx) << "result is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Type() == nullptr)) {
        AddResultError(inst, idx) << "result type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Instruction() == nullptr)) {
        AddResultError(inst, idx) << "result instruction is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Instruction() != inst)) {
        AddResultError(inst, idx)
            << "result instruction does not match instruction (possible double usage)";
        return false;
    }

    if (!inst->Is<core::ir::Call>() && result->Type()->Is<core::type::Void>()) {
        AddResultError(inst, idx) << "result type cannot be void";
        return false;
    }

    if (inst->Is<core::ir::ControlInstruction>()) {
        if (result->Type()->Is<core::type::Pointer>()) {
            AddResultError(inst, idx) << "result type cannot be a pointer";
            return false;
        }
        if (!result->Type()->IsConstructible()) {
            AddResultError(inst, idx) << "result type must be constructable";
            return false;
        }
    }

    if (result->Type()->Is<core::type::Void>() && ir_.NameOf(result)) {
        AddResultError(inst, idx) << "void results must not have names";
        return false;
    }

    return true;
}

bool Structural::CheckResults(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Results().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " results, got "
                           << inst->Results().Length();
            return false;
        }
    }

    bool passed = true;
    Hashset<const InstructionResult*, 4> seen_instruction_results;
    for (size_t i = 0; i < inst->Results().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckResult(inst, i))) {
            passed = false;
        }

        if (!seen_instruction_results.Add(inst->Result(i))) {
            AddResultError(inst, i) << "result was seen previously as a result";
            passed = false;
        }
    }
    return passed;
}

bool Structural::CheckOperand(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operand(idx);

    if (DAWN_UNLIKELY(operand == nullptr)) {
        // var instructions are allowed to have a nullptr initializers.
        // terminator instructions use nullptr operands to signal 'undef'.
        if (inst->IsAnyOf<Terminator, Var>()) {
            return true;
        }

        AddError(inst, idx) << "operand is undefined";
        return false;
    }

    // ir::Unused is a internal value used by some transforms to track unused entries, and is
    // removed as part of generating an output shader.
    if (DAWN_UNLIKELY(operand->Is<ir::Unused>())) {
        return true;
    }

    if (DAWN_UNLIKELY(operand->Type() == nullptr)) {
        AddError(inst, idx) << "operand type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Alive())) {
        AddError(inst, idx) << "operand is not alive";
        return false;
    }

    if (DAWN_UNLIKELY(operand->Is<Constant>() &&
                      operand->Type()->Is<core::type::SubgroupMatrix>())) {
        AddError(inst, idx) << "subgroup_matrix values cannot be constant";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->HasUsage(inst, idx))) {
        AddError(inst, idx) << "operand missing usage";
        return false;
    }

    if (auto fn = operand->As<Function>(); fn && !all_functions_.Contains(fn)) {
        AddError(inst, idx) << NameOf(operand) << " is not part of the module";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Is<ir::Unused>() && !operand->Is<Constant>() &&
                      !scope_stack_.Contains(operand))) {
        AddError(inst, idx) << NameOf(operand) << " is not in scope";
        AddDeclarationNote(operand);
        return false;
    }

    return true;
}

bool Structural::CheckOperands(const ir::Instruction* inst,
                               size_t min_count,
                               std::optional<size_t> max_count) {
    if (DAWN_UNLIKELY(inst->Operands().Length() < min_count)) {
        if (max_count.has_value()) {
            AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                           << " operands, got " << inst->Operands().Length();
        } else {
            AddError(inst) << "expected at least " << min_count << " operands, got "
                           << inst->Operands().Length();
        }
        return false;
    }

    if (DAWN_UNLIKELY(max_count.has_value() && inst->Operands().Length() > max_count.value())) {
        AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                       << " operands, got " << inst->Operands().Length();
        return false;
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Structural::CheckOperands(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Operands().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " operands, got "
                           << inst->Operands().Length();
            return false;
        }
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Structural::CheckResultsAndOperandRange(const ir::Instruction* inst,
                                             size_t num_results,
                                             size_t min_operands,
                                             std::optional<size_t> max_operands = {}) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, min_operands, max_operands);
    return results_passed && operands_passed;
}

bool Structural::CheckResultsAndOperands(const ir::Instruction* inst,
                                         size_t num_results,
                                         size_t num_operands) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, num_operands);
    return results_passed && operands_passed;
}

void Structural::CheckType(const core::type::Type* root,
                           std::function<diag::Diagnostic&()> diag,
                           Capabilities allow_caps) {
    if (root == nullptr) {
        return;
    }

    if (!ir_.properties.Contains(Property::kAllowNonCoreTypes)) {
        if (!IsCoreType(root)) {
            diag() << "non-core types not allowed in core IR";
            return;
        }
    }

    if (!validated_types_.Add(root)) {
        return;
    }

    AddressSpace addrspace = AddressSpace::kUndefined;
    if (auto* mv = root->As<core::type::MemoryView>()) {
        addrspace = mv->AddressSpace();
    }

    auto visit = [&](const core::type::Type* type) {
        if (type->IsAbstract()) {
            diag() << "abstracts are not permitted";
            return false;
        }

        return tint::Switch(
            type,
            [&](const core::type::Struct* str) {
                uint32_t cur_offset = 0;
                for (auto* member : str->Members()) {
                    if (member->Type()->Is<core::type::Void>()) {
                        diag() << "struct member " << member->Index() << " cannot have void type";
                        return false;
                    }

                    if (!CheckStructMemberAttributes(member, diag)) {
                        return false;
                    }

                    if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
                        if (member->Type()->Is<core::type::Pointer>()) {
                            diag() << "struct member " << member->Index()
                                   << " cannot be a pointer type";
                            return false;
                        }

                        if (member->Type()->Is<core::type::Texture>()) {
                            diag() << "struct member " << member->Index()
                                   << " cannot be a texture type";
                            return false;
                        }

                        if (member->Type()->Is<core::type::Sampler>()) {
                            diag() << "struct member " << member->Index()
                                   << " cannot be a sampler type";
                            return false;
                        }
                    }

                    if (auto* arr = member->Type()->As<core::type::Array>();
                        arr && arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                        if (member != str->Members().Back()) {
                            diag() << "runtime-sized arrays can only be the last member of a "
                                      "struct";
                            return false;
                        }
                    }

                    if (member->Align() == 0) {
                        diag() << "struct member must not have an alignment of 0";
                        return false;
                    }
                    if (!tint::IsPowerOfTwo(member->Align())) {
                        diag() << "struct member alignment must be a power of 2";
                        return false;
                    }

                    if (member->Type()->Align() == 0) {
                        diag() << "struct member type must not have an alignment of 0";
                        return false;
                    }
                    if (!tint::IsPowerOfTwo(member->Type()->Align())) {
                        diag() << "struct member type alignment must be a power of 2";
                        return false;
                    }
                    if (!ir_.properties.Contains(Property::kAllowStructMatrixDecorations)) {
                        if (member->RowMajor()) {
                            diag() << "Row major annotation not allowed on structures";
                            return false;
                        }
                        if (member->HasMatrixStride()) {
                            diag() << "Matrix stride annotation not allowed on structures";
                            return false;
                        }
                    }

                    // TODO(448608979): Remove guard once updated to handle RowMajor correctly
                    if (!member->RowMajor()) {
                        if (member->Size() < member->Type()->Size()) {
                            diag() << "struct member " << member->Index()
                                   << " with size=" << member->Size()
                                   << " must be at least as large as the type with size "
                                   << member->Type()->Size();
                            return false;
                        }

                        if (member->Align() % member->Type()->Align() != 0) {
                            diag() << "struct member alignment (" << member->Align()
                                   << ") must be divisible by type alignment ("
                                   << member->Type()->Align() << ")";
                            return false;
                        }
                    }

                    cur_offset += (member->Offset() - cur_offset) + member->MinimumRequiredSize();
                }
                if (str->Size() < cur_offset) {
                    diag() << "struct size (" << str->Size()
                           << ") is smaller than the end of the last member (" << cur_offset << ")";
                    return false;
                }

                return true;
            },
            [&](const core::type::Reference* ref) {
                if (ref->StoreType()->Is<core::type::Void>()) {
                    diag() << "references to void are not permitted";
                    return false;
                }

                // Reference types are guarded by the AllowRefTypes property.
                if (!ir_.properties.Contains(Property::kAllowRefTypes)) {
                    diag() << "reference types are not permitted here";
                    return false;
                } else if (type != root) {
                    // If they are allowed, reference types still cannot be nested.
                    diag() << "nested reference types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::Pointer* ptr) {
                if (ptr->StoreType()->Is<core::type::Void>()) {
                    diag() << "pointers to void are not permitted";
                    return false;
                }

                if (ptr->AddressSpace() == AddressSpace::kUniform ||
                    ptr->AddressSpace() == AddressSpace::kHandle) {
                    if (ptr->Access() != core::Access::kRead) {
                        diag() << "uniform and handle pointers must be read access";
                        return false;
                    }
                }

                if (ptr->AddressSpace() == AddressSpace::kWorkgroup) {
                    if (ptr->Access() != core::Access::kReadWrite) {
                        diag() << "workgroup pointers must be read_write access";
                        return false;
                    }
                }

                if (ptr->AddressSpace() == AddressSpace::kHandle) {
                    if (!ptr->StoreType()->IsHandle()) {
                        diag() << "the 'handle' address space can only be used for handle types";
                        return false;
                    }
                } else {
                    if (ptr->StoreType()->IsHandle()) {
                        diag() << "handle types can only be declared in the 'handle' address space";
                        return false;
                    }
                }

                if (ptr->AddressSpace() == core::AddressSpace::kImmediate) {
                    if (ptr->Access() != core::Access::kRead) {
                        diag() << "immediate pointers must be read access";
                        return false;
                    }
                }

                if (ptr->StoreType()->Is<core::type::Pointer>()) {
                    diag() << "pointers to pointers are not allowed";
                    return false;
                }
                return true;
            },
            [&](const core::type::U64*) {
                // u64 types are guarded by the Allow64BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow64BitIntegers) &&
                    !allow_caps.Contains(Capability::kAllow64BitIntegers)) {
                    diag() << "64-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::I8*) {
                // i8 types are guarded by the Allow8BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow8BitIntegers)) {
                    diag() << "8-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::U8*) {
                // u8 types are guarded by the Allow8BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow8BitIntegers)) {
                    diag() << "8-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::U16*) {
                // u16 types are guarded by the Allow16BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow16BitIntegers)) {
                    diag() << "16-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::Array* arr) {
                if (!arr->ElemType()->HasCreationFixedFootprint()) {
                    diag() << "array elements, " << NameOf(type)
                           << ", must have creation-fixed footprint";
                    return false;
                }
                if (auto* count = arr->Count()->As<core::type::ConstantArrayCount>()) {
                    if (count->value == 0) {
                        diag() << "array requires a constant array size > 0";
                        return false;
                    }
                }
                return true;
            },
            [&](const core::type::Vector* v) {
                if (!v->Type()->IsScalar()) {
                    diag() << "vector elements, " << NameOf(type) << ", must be scalars";
                    return false;
                }
                return true;
            },
            [&](const core::type::Matrix* m) {
                if (!m->Type()->IsFloatScalar()) {
                    diag() << "matrix elements, " << NameOf(type) << ", must be float scalars";
                    return false;
                }
                return true;
            },
            [&](const core::type::Atomic* a) {
                // Prior to lowering we allow for atomic operations on vec2u to support the
                // AtomicVec2UMinMax feature.
                if (auto* vec = a->Type()->As<core::type::Vector>()) {
                    if (vec->Width() == 2 && vec->Type()->Is<core::type::U32>()) {
                        return true;
                    }
                }

                if (!a->Type()->IsAnyOf<core::type::I32, core::type::U32, core::type::U64>()) {
                    diag() << "atomic subtype must be i32, u32 or u64 type is "
                           << NameOf(a->Type());
                    return false;
                }
                return true;
            },
            [&](const core::type::SampledTexture* s) {
                if (!s->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
                    diag() << "invalid sampled texture sample type: " << NameOf(s->Type());
                    return false;
                }
                return true;
            },
            [&](const core::type::MultisampledTexture* ms) {
                if (!ms->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
                    diag() << "invalid multisampled texture sample type: " << NameOf(ms->Type());
                    return false;
                }

                switch (ms->Dim()) {
                    case core::type::TextureDimension::k2d:
                        break;
                    default:
                        diag() << "invalid multisampled texture dimension: "
                               << style::Literal(ToString(ms->Dim()));
                        return false;
                }
                return true;
            },
            [&](const core::type::StorageTexture* s) {
                switch (s->Dim()) {
                    case core::type::TextureDimension::kCube:
                    case core::type::TextureDimension::kCubeArray:
                        diag() << "dimension " << style::Literal(ToString(s->Dim()))
                               << " for storage textures does not in WGSL yet";
                        return false;
                    case core::type::TextureDimension::kNone:
                        diag() << "invalid texture dimension "
                               << style::Literal(ToString(s->Dim()));
                        return false;
                    default:
                        return true;
                }
            },
            [&](const core::type::InputAttachment* i) {
                if (!i->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
                    diag() << "invalid input attachment component type: " << NameOf(i->Type());
                    return false;
                }
                return true;
            },
            [&](const core::type::SubgroupMatrix* m) {
                if (!m->Type()
                         ->IsAnyOf<core::type::F16, core::type::F32, core::type::I8,
                                   core::type::I32, core::type::U8, core::type::U32>()) {
                    diag() << "invalid subgroup matrix component type: " << NameOf(m->Type());
                    return false;
                }
                if (!(addrspace == AddressSpace::kUndefined ||
                      addrspace == AddressSpace::kFunction)) {
                    diag() << "invalid address space for subgroup matrix : " << addrspace;
                    return false;
                }
                return true;
            },
            [&](const core::type::BindingArray* t) {
                if (!t->Count()->Is<core::type::ConstantArrayCount>()) {
                    diag() << "binding_array count must be a constant expression";
                    return false;
                }

                auto count = t->Count()->As<core::type::ConstantArrayCount>()->value;
                if (count == 0) {
                    diag() << "binding array requires a constant array size > 0";
                    return false;
                }

                if (!(addrspace == AddressSpace::kUndefined ||
                      addrspace == AddressSpace::kHandle) &&
                    !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
                    diag() << "invalid address space for binding_array : " << addrspace;
                    return false;
                }

                if (!ir_.properties.Contains(Property::kAllowNonCoreTypes)) {
                    if (!t->ElemType()->Is<core::type::SampledTexture>()) {
                        diag() << "binding_array element type must be a sampled texture type";
                        return false;
                    }
                }
                return true;
            },
            [](Default) { return true; });
    };

    Vector<const core::type::Type*, 8> stack{root};
    Hashset<const core::type::Type*, 8> seen{};
    while (!stack.IsEmpty()) {
        auto* ty = stack.Pop();
        if (!ty) {
            continue;
        }
        if (!visit(ty)) {
            return;
        }

        if (auto* view = ty->As<core::type::MemoryView>(); view && seen.Add(view)) {
            stack.Push(view->StoreType());
            continue;
        }

        // Visit the elements of a composite type.
        auto type_count = ty->Elements();
        if (type_count.type) {
            // Every element has the same type (e.g. array, vector, matrix, ...), so validate that
            // type once if it has not been seen before.
            if (seen.Add(type_count.type)) {
                stack.Push(type_count.type);
            }
        } else {
            // Different elements have different types (e.g. a struct), so we need to validate each
            // of them if they have not been seen before.
            for (uint32_t i = 0; i < type_count.count; i++) {
                if (auto* subtype = ty->Element(i); subtype && seen.Add(subtype)) {
                    stack.Push(subtype);
                }
            }
        }
    }
}

void Structural::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    Hashset<const core::ir::Value*, 8> pipeline_evaluatable{};

    auto add_evaluatable = [&](const Instruction* inst, const bool is_creatable) {
        if (auto* res = inst->Result(0); res != nullptr && is_creatable) {
            pipeline_evaluatable.Add(res);
        }
    };

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "instruction in root block does not have root block as parent";
            continue;
        }

        auto is_pipeline_creatable = true;
        for (auto* op : inst->Operands()) {
            if (!op) {
                continue;
            }
            if (op->Is<core::ir::Constant>()) {
                continue;
            }
            if (pipeline_evaluatable.Contains(op)) {
                continue;
            }
            is_pipeline_creatable = false;
            break;
        }

        if (!is_pipeline_creatable) {
            AddError(inst) << "instruction is not evaluatable at pipeline creation time";
        }

        tint::Switch(
            inst,  //
            [&](const core::ir::Override* o) {
                if (ir_.properties.Contains(Property::kAllowOverrides)) {
                    CheckInstruction(o);
                    add_evaluatable(o, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Var* var) { CheckInstruction(var); },
            [&](const core::ir::Let* let) {
                if (ir_.properties.Contains(Property::kAllowModuleScopeLets)) {
                    CheckInstruction(let);
                    add_evaluatable(let, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Construct* c) {
                if (ir_.properties.Contains(Property::kAllowModuleScopeLets) ||
                    ir_.properties.Contains(Property::kAllowOverrides)) {
                    CheckInstruction(c);
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(c, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](Default) {
                // Note, this validation around kAllowOverrides is looser than it could be. There
                // are only certain expressions and builtins which can be used in an override, which
                // currently isn't checked.
                if (ir_.properties.Contains(Property::kAllowOverrides) &&
                    inst->IsAnyOf<core::ir::Unary, core::ir::Binary, core::ir::BuiltinCall,
                                  core::ir::Convert, core::ir::Swizzle, core::ir::Access,
                                  core::ir::ConstExprIf>()) {
                    CheckInstruction(inst);
                    // If overrides are allowed we can have certain regular instructions in the root
                    // block, with the caveat that those instructions can _only_ be used in the root
                    // block.
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(inst, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            });

        // Process tasks queued by CheckInstruction (like AddResults) before moving to next
        // instruction.
        ProcessTasks();
    }
}

void Structural::CheckOnlyUsedInRootBlock(const Instruction* inst) {
    if (inst->Result(0)) {
        for (auto& usage : inst->Result(0)->UsagesSorted()) {
            if (usage.instruction->Block() != ir_.root_block) {
                AddError(inst) << "root block: instruction used outside of root block "
                               << inst->TypeInfo().name;
            }
        }
    }

    CheckInstruction(inst);
}

void Structural::CheckFunction(const Function* func) {
    // Scope holds the parameters and block
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    // The recursion checks require this to be true as it will be asserted by the
    // referenced_functions helper.
    func->ForEachUseUnsorted([&](const Usage& use) {
        if (use.instruction->As<UserCall>() || use.instruction->As<Return>()) {
            return;
        }
        AddError(use.instruction, use.operand_index) << "function may not be used as a operand";
    });

    if (func->IsEntryPoint()) {
        // Check that there is at most one entry point unless we allow multiple entry points.
        if (!ir_.properties.Contains(Property::kAllowMultipleEntryPoints)) {
            if (!entry_point_names_.IsEmpty()) {
                AddError(func) << "a module with multiple entry points requires the "
                                  "AllowMultipleEntryPoints property";
                return;
            }
        }

        // Checking the name early, so its usage can be recorded, even if the function is malformed.
        const auto name = ir_.NameOf(func).Name();
        if (!entry_point_names_.Add(name)) {
            AddError(func) << "entry point name " << style::Function(name) << " is not unique";
        }
    }

    if (!func->Type() || !func->Type()->Is<core::type::Function>()) {
        AddError(func) << "functions must have type '<function>'";
        return;
    }

    // Note: This is not a validator error because Function::SetBlock() asserts that the block is
    // not null, and the disassembler will crash if this is not null. This should only be hit due
    // to some sort of corruption, not a bad shader/programmer error.
    TINT_ASSERT(func->Block()) << "root block for function is undefined";

    if (func->Block()->Is<ir::MultiInBlock>()) {
        AddError(func) << "root block for function cannot be a multi-in block";
        return;
    }

    Hashset<const FunctionParam*, 4> param_set{};
    for (auto* param : func->Params()) {
        if (!param->Alive()) {
            AddError(param) << "destroyed parameter found in function parameter list";
            return;
        }
        if (!param->Function()) {
            AddError(param) << "function parameter has nullptr parent function";
            return;
        } else if (param->Function() != func) {
            AddError(param) << "function parameter has incorrect parent function";
            AddNote(param->Function()) << "parent function declared here";
            return;
        }

        if (!param->Type()) {
            AddError(param) << "function parameter has nullptr type";
            return;
        }

        if (!param_set.Add(param)) {
            AddError(param) << "function parameter is not unique";
            continue;
        }

        CheckType(param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); });

        if (!IsValidFunctionParamType(param->Type())) {
            auto ptr_ty = param->Type()->As<core::type::Pointer>();
            bool allowed_ptr_to_handle = ir_.properties.Contains(Property::kAllowPointerToHandle) &&
                                         ptr_ty != nullptr && ptr_ty->StoreType()->IsHandle();

            auto struct_ty = param->Type()->As<core::type::Struct>();
            if (!allowed_ptr_to_handle &&
                (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface) ||
                 (struct_ty == nullptr) ||
                 struct_ty->Members().Any([](const core::type::StructMember* m) {
                     return !IsValidFunctionParamType(m->Type());
                 }))) {
                AddError(param) << "function parameter type, " << NameOf(param->Type())
                                << ", must be constructible, a pointer, or a handle";
            }
        }
        if (func->IsEntryPoint() &&
            !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
            if (param->Type()->Is<core::type::Pointer>()) {
                AddError(param) << "entry point parameters cannot be pointers";
            }
        }

        if (func->IsFragment()) {
            WalkTypeAndMembers(param, param->Type(), param->Attributes(),
                               [this](const auto* p, const auto* t, const auto& a) {
                                   CheckFrontFacingIfBool(
                                       p, a, t,
                                       "fragment entry point params can only be a bool if "
                                       "decorated with @builtin(front_facing)");
                               });
        } else if (func->IsEntryPoint()) {
            WalkTypeAndMembers(
                param, param->Type(), param->Attributes(),
                [this](const auto* p, const auto* t, const auto&) {
                    CheckNotBool(p, t,
                                 "entry point params can only be a bool for fragment shaders");
                });
        }

        AddressSpace address_space = AddressSpace::kUndefined;
        auto* mv = param->Type()->As<core::type::MemoryView>();
        if (mv) {
            address_space = mv->AddressSpace();
        } else {
            // ModuleScopeVars transform in MSL backends unwraps pointers to handles
            if (param->Type()->IsHandle()) {
                address_space = AddressSpace::kHandle;
            }
        }

        if (address_space == AddressSpace::kPixelLocal) {
            if (!mv->StoreType()->Is<core::type::Struct>()) {
                AddError(param) << "pixel_local param must be of type struct";
            }
        }

        if (func->IsEntryPoint()) {
            ValidateShaderIOAnnotations(param, param->Type(), param->BindingPoint(),
                                        param->Attributes(), ShaderIOKind::kInputParam);
        } else {
            if (param->BindingPoint().has_value()) {
                AddError(param)
                    << "input param to non-entry point function has a binding point set";
            }

            if (param->Builtin().has_value()) {
                AddError(param) << "builtins can only be decorated on entry point params";
            }
        }

        if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface) &&
            func->IsEntryPoint()) {
            if (mv && mv->Is<core::type::Pointer>() && address_space == AddressSpace::kWorkgroup) {
                AddError(param) << "input param to entry point cannot be a ptr in the 'workgroup' "
                                   "address space";
            }
        }

        scope_stack_.Add(param);
    }

    CheckType(func->ReturnType(), [&]() -> diag::Diagnostic& { return AddError(func); });

    // void needs to be filtered out, since it isn't constructible, but used in the IR when no
    // return is specified.
    if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>() &&
                      !func->ReturnType()->IsConstructible())) {
        AddError(func) << "function return type must be constructible";
    }

    ValidateIOAttributes(func);

    if (func->IsEntryPoint()) {
        if (DAWN_UNLIKELY(ir_.NameOf(func).Name().empty())) {
            AddError(func) << "entry points must have names";
        }
    }

    CheckWorkgroupSize(func);

    CheckSubgroupSize(func);

    if (func->Stage() == Function::PipelineStage::kCompute) {
        if (DAWN_UNLIKELY(func->ReturnType() && !func->ReturnType()->Is<core::type::Void>())) {
            AddError(func) << "compute entry point must not have a return type, found "
                           << NameOf(func->ReturnType());
        }
    }

    if (func->IsEntryPoint()) {
        ValidateShaderIOAnnotations(func, func->ReturnType(), std::nullopt,
                                    func->ReturnAttributes(), ShaderIOKind::kResultValue);

        WalkTypeAndMembers(
            func, func->ReturnType(), func->ReturnAttributes(),
            [this](const Function* f, const core::type::Type* t, const IOAttributes&) {
                CheckNotBool(f, t, "entry point returns can not be 'bool'");
            });

        Hashset<BindingPoint, 4> binding_points{};
        const Var* user_declared_immediate = nullptr;

        for (auto var : referenced_module_vars_.TransitiveReferences(func)) {
            if (!ir_.properties.Contains(Property::kAllowDuplicateBindings) &&
                var->BindingPoint().has_value()) {
                auto bp = var->BindingPoint().value();
                if (!binding_points.Add(bp)) {
                    AddError(var) << "found non-unique binding point, " << bp
                                  << ", being referenced in entry point, " << NameOf(func);
                }
            }

            const auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
            const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
            const auto attr = var->Attributes();
            if (!mv || !ty) {
                continue;
            }

            auto address_space = mv->AddressSpace();
            switch (address_space) {
                case AddressSpace::kImmediate:
                    if (user_declared_immediate) {
                        AddError(var)
                            << "multiple user-declared immediate data variables referenced "
                               "by entry point "
                            << NameOf(func);
                    }
                    user_declared_immediate = var;
                    continue;
                case AddressSpace::kWorkgroup:
                    if (!func->IsCompute()) {
                        AddError(var) << "workgroup variable cannot be used in a " << func->Stage()
                                      << " shader";
                    }
                    continue;
                case AddressSpace::kPixelLocal:
                    if (!func->IsFragment()) {
                        AddError(var) << "pixel_local variable cannot be used in a "
                                      << func->Stage() << " shader";
                    }
                    continue;
                case AddressSpace::kIn:
                case AddressSpace::kOut:
                    break;
                default:
                    continue;
            }

            if (func->IsFragment() && address_space == AddressSpace::kIn) {
                WalkTypeAndMembers(
                    var, ty, attr, [this](const auto* v, const auto* t, const auto& a) {
                        CheckFrontFacingIfBool(
                            v, a, t,
                            "input address space values referenced by fragment shaders "
                            "can only be 'bool' if decorated with "
                            "@builtin(front_facing)");
                    });
            } else {
                WalkTypeAndMembers(
                    var, ty, attr, [this](const auto* v, const auto* t, const auto&) {
                        CheckNotBool(
                            v, t,
                            "IO address space values referenced by shader entry points can "
                            "only be 'bool' if in the input space, used only by fragment "
                            "shaders and decorated with @builtin(front_facing)");
                    });
            }
        }
    }

    if (func->IsVertex()) {
        CheckPositionPresentForVertexOutput(func);
    }

    QueueBlock(func->Block());
    ProcessTasks();
}

void Structural::ValidateIOAttributes(const Function* func) {
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
            tasks.Push({var, mv->StoreType(), var->Attributes(), IODirectionFor(mv->AddressSpace()),
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

void Structural::ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                          const CastableBase* msg_anchor,
                                          const core::type::Type* ty,
                                          const IOAttributes& attr,
                                          Function::PipelineStage stage,
                                          IODirection dir,
                                          ShaderIOKind io_kind) {
    bool skip_builtins = capabilities_.Contains(Capability::kLoosenValidationForShaderIO) &&
                         io_kind == ShaderIOKind::kModuleScopeVar;
    const IOAttributeUsage usage = IOAttributeUsageFor(stage, dir);
    WalkTypeAndMembers(
        *this, ty, attr,
        [&ctx, msg_anchor, usage, io_kind, skip_builtins, dir](
            Structural& v, const core::type::Type* t, const IOAttributes& a) {
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

void Structural::CheckFrontFacingIfBool(const CastableBase* msg_anchor,
                                        const IOAttributes& attr,
                                        const core::type::Type* ty,
                                        const std::string& err) {
    if (ty->Is<core::type::Bool>() && attr.builtin != BuiltinValue::kFrontFacing) {
        AddError(msg_anchor) << err;
    }
}

void Structural::CheckNotBool(const CastableBase* msg_anchor,
                              const core::type::Type* ty,
                              const std::string& err) {
    if (ty->Is<core::type::Bool>()) {
        AddError(msg_anchor) << err;
    }
}

void Structural::CheckWorkgroupSize(const Function* func) {
    if (!func->IsCompute()) {
        if (func->WorkgroupSize().has_value()) {
            AddError(func) << "@workgroup_size only valid on compute entry point";
        }
        return;
    }

    if (!func->WorkgroupSize().has_value()) {
        AddError(func) << "compute entry point requires @workgroup_size";
        return;
    }

    auto workgroup_sizes = func->WorkgroupSize().value();
    // The number parameters cannot be checked here, since it is stored internally as a 3 element
    // array, so will always have 3 elements at this point.
    TINT_ASSERT(workgroup_sizes.size() == 3);

    uint64_t total_size = 1;

    std::optional<const core::type::Type*> sizes_ty;
    for (auto* size : workgroup_sizes) {
        if (!size || !size->Type()) {
            AddError(func) << "a @workgroup_size param is undefined or missing a type";
            return;
        }

        auto* ty = size->Type();
        if (!ty->IsAnyOf<core::type::I32, core::type::U32>()) {
            AddError(func) << "@workgroup_size params must be an 'i32' or 'u32', received "
                           << NameOf(ty);
            return;
        }

        if (!sizes_ty.has_value()) {
            sizes_ty = ty;
        }

        if (sizes_ty != ty) {
            AddError(func) << "@workgroup_size params must be all 'i32's or all 'u32's";
            return;
        }

        if (auto* c = size->As<ir::Constant>()) {
            if (c->Value()->ValueAs<int64_t>() <= 0) {
                AddError(func) << "@workgroup_size params must be greater than 0";
                return;
            }
            total_size *= c->Value()->ValueAs<uint64_t>();

            constexpr uint64_t kMaxGridSize = 0xffffffff;
            if (total_size > kMaxGridSize) {
                AddError(func) << "workgroup grid size cannot exceed 0x" << std::hex
                               << kMaxGridSize;
            }
            continue;
        }

        if (!ir_.properties.Contains(Property::kAllowOverrides)) {
            AddError(func) << "@workgroup_size param is not a constant value, and IR property "
                              "'AllowOverrides' is not enabled";
            return;
        }

        if (auto* r = size->As<ir::InstructionResult>()) {
            if (!r->Instruction()) {
                AddError(func) << "instruction for @workgroup_size param is not defined";
                return;
            }

            if (r->Instruction()->Block() != ir_.root_block) {
                AddError(func) << "@workgroup_size param defined by non-module scope value";
                return;
            }

            // Since above, it is already checked if the value is in the root block, it is assumed
            // to be pipeline creatable here, i.e. const/override or derived from consts and
            // overrides.
            // If that is not true, that indicates an issue in CheckRootBlock().
            continue;
        }

        AddError(func) << "@workgroup_size must be an InstructionResult or a Constant";
    }
}

void Structural::CheckSubgroupSize(const Function* func) {
    // @subgroup_size is optional
    if (!func->SubgroupSize().has_value()) {
        return;
    }

    if (!func->IsCompute()) {
        AddError(func) << "@subgroup_size only valid on compute entry point";
        return;
    }

    auto subgroup_size = func->SubgroupSize().value();
    if (subgroup_size == nullptr) {
        AddError(func) << "a @subgroup_size param must have a value";
        return;
    }

    if (!subgroup_size->Type()) {
        AddError(func) << "a @subgroup_size param is missing a type";
        return;
    }

    auto* ty = subgroup_size->Type();
    if (!ty->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError(func) << "@subgroup_size param must be an 'i32' or 'u32', received " << NameOf(ty);
        return;
    }

    if (auto* c = subgroup_size->As<ir::Constant>()) {
        int64_t value = c->Value()->ValueAs<int64_t>();
        if (value <= 0) {
            AddError(func) << "@subgroup_size param must be greater than 0";
            return;
        }

        if (!IsPowerOfTwo<int64_t>(value)) {
            AddError(func) << "@subgroup_size param must be a power of 2";
            return;
        }

        return;
    }

    if (!ir_.properties.Contains(Property::kAllowOverrides)) {
        AddError(func) << "@subgroup_size param is not a constant value, and IR property "
                          "'AllowOverrides' is not enabled";
        return;
    }

    if (auto* r = subgroup_size->As<ir::InstructionResult>()) {
        if (!r->Instruction()) {
            AddError(func) << "instruction for @subgroup_size param is not defined";
            return;
        }

        if (r->Instruction()->Block() != ir_.root_block) {
            AddError(func) << "@subgroup_size param defined by non-module scope value";
            return;
        }

        if (r->Instruction()->Is<core::ir::Override>()) {
            return;
        }
    }

    AddError(func) << "@subgroup_size must be an InstructionResult or a Constant";
}

void Structural::CheckPositionPresentForVertexOutput(const Function* ep) {
    if (IsPositionPresent(ep->ReturnAttributes(), ep->ReturnType())) {
        return;
    }

    for (const auto& var : referenced_module_vars_.TransitiveReferences(ep)) {
        const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
        if (!ty) {
            continue;
        }

        const auto attr = var->Attributes();
        if (IsPositionPresent(attr, ty)) {
            return;
        }
    }
    AddError(ep) << "position must be declared for vertex entry point output";
}

void Structural::ProcessTasks() {
    while (!tasks_.IsEmpty()) {
        tasks_.Pop()();
    }
}

void Structural::QueueBlock(const Block* blk) {
    tasks_.Push([this] { EndBlock(); });
    tasks_.Push([this, blk] { BeginBlock(blk); });
}

void Structural::BeginBlock(const Block* blk) {
    scope_stack_.Push();
    block_stack_.Push(blk);

    if (auto* mb = blk->As<MultiInBlock>()) {
        for (auto* param : mb->Params()) {
            if (!param->Alive()) {
                AddError(param) << "destroyed parameter found in block parameter list";
                return;
            }
            if (!param->Block()) {
                AddError(param) << "block parameter has nullptr parent block";
                return;
            } else if (param->Block() != mb) {
                AddError(param) << "block parameter has incorrect parent block";
                AddNote(param->Block()) << "parent block declared here";
                return;
            }

            CheckType(param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); });

            if (param->Type()->Is<core::type::Void>()) {
                AddError(param) << "block parameter type cannot be void";
            }
            if (param->Type()->Is<core::type::Reference>()) {
                AddError(param) << "block parameter type cannot be a reference";
            }

            scope_stack_.Add(param);
        }
    }

    if (!blk->Terminator()) {
        AddError(blk) << "block does not end in a terminator instruction";
    }

    // Validate the instructions w.r.t. the parent block
    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "block instruction does not have same block as parent";
            AddNote(blk) << "in block";
        }
    }

    // Enqueue validation of the instructions of the block
    if (!blk->IsEmpty()) {
        QueueInstructions(blk->Instructions());
    }
}

void Structural::EndBlock() {
    scope_stack_.Pop();
    block_stack_.Pop();
}

void Structural::QueueInstructions(const Instruction* inst) {
    if (diag_.ContainsErrors()) {
        return;
    }

    tasks_.Push([this, inst] {
        // Tasks are processed LIFO, so push the next instruction to the stack before checking the
        // current instruction, which may need to add more blocks to the stack itself.
        if (inst->next) {
            QueueInstructions(inst->next);
        }
        CheckInstruction(inst);
    });
}

void Structural::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst) << "destroyed instruction found in instruction list";
        return;
    }

    Capabilities allowed_types{};

    if (auto* call = inst->As<core::ir::CoreBuiltinCall>();
        call && call->Func() == core::BuiltinFn::kBitcast) {
        allowed_types.Add(Capability::kAllow64BitIntegers);
    }

    if (auto* call = inst->As<core::ir::CoreBinary>()) {
        if (call->Op() == core::BinaryOp::kOr || call->Op() == core::BinaryOp::kShiftLeft) {
            allowed_types.Add(Capability::kAllow64BitIntegers);
        }
    }

    if (auto* call = inst->As<core::ir::Convert>()) {
        // This will miss u64 being used if it isn't on the first result, but convert having
        // multiple results is illegal anyway, so will be rejected later in the validator
        if (call->Result(0) && call->Result(0)->Type()->Is<core::type::U64>()) {
            allowed_types.Add(Capability::kAllow64BitIntegers);
        }
    }

    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            continue;
        }

        CheckType(
            res->Type(), [&]() -> diag::Diagnostic& { return AddResultError(inst, i); },
            allowed_types);
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        CheckType(
            op->Type(), [&]() -> diag::Diagnostic& { return AddError(inst, i); }, allowed_types);
    }

    // Push a task to add the results to the scope.
    // This ensures that for control instructions, the results are only added to the scope
    // after their nested blocks have been evaluated (since tasks are processed LIFO).
    tasks_.Push([this, inst] {
        for (auto* result : inst->Results()) {
            if (result) {
                scope_stack_.Add(result);
            }
        }
    });

    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* let) { CheckLet(let); },                            //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Phony* p) { CheckPhony(p); },                            //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Override* o) { CheckOverride(o); },                      //
        [&](const Var* var) { CheckVar(var); },                            //
        [&](const Default) { AddError(inst) << "missing validation"; });
}

void Structural::CheckOverride(const Override* o) {
    // Intentionally not checking operands, since Override may have a null operand
    if (!CheckResults(o, Override::kNumResults)) {
        return;
    }

    if (o->Initializer()) {
        CheckOperand(o, ir::Var::kInitializerOperandOffset);
    } else if (o->Operands().Length() == 0) {
        AddError(o) << "override is malformed, missing initializer operand";
    }
}

void Structural::CheckVar(const Var* var) {
    if (!CheckResultsAndOperands(var, Var::kNumResults, Var::kNumOperands)) {
        return;
    }

    // TODO(516717234): Remove when ValidateShaderIOAnnotations are moved to function validator
    auto* result_type = var->Result()->Type();
    auto* mv = result_type->As<core::type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type " << NameOf(result_type)
                      << " must be a pointer or a reference";
        return;
    }

    if (var->Initializer()) {
        if (!CheckOperand(var, ir::Var::kInitializerOperandOffset)) {
            return;
        }
    }

    // TODO(516717234): Move to functional validator
    CheckBindingPoint(var, var->Result(0)->Type(), var->Attributes(),
                      ShaderIOKind::kModuleScopeVar);

    // TODO(516717234): Move to functional validator
    if (var->Block() == ir_.root_block) {
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut) {
            ValidateShaderIOAnnotations(var, var->Result()->Type(), var->BindingPoint(),
                                        var->Attributes(), ShaderIOKind::kModuleScopeVar);
        }
    }
}

void Structural::CheckBlendSrc(BlendSrcContext& ctx,
                               const CastableBase* target,
                               const core::type::Type* ty,
                               const IOAttributes& attr) {
    if (attr.blend_src.has_value()) {
        if (!capabilities_.Contains(Capability::kLoosenValidationForShaderIO)) {
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
    if (!capabilities_.Contains(Capability::kLoosenValidationForShaderIO)) {
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

void Structural::CheckBlendSrcImpl(BlendSrcContext& ctx,
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

void Structural::CheckLocation(Hashmap<uint32_t, const CastableBase*, 4>& locations,
                               const CastableBase* target,
                               const IOAttributes& attr,
                               const Function::PipelineStage stage,
                               const core::type::Type* type,
                               const IODirection dir) {
    struct WalkContext {
        Structural* validator;
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

void Structural::CheckInterpolation(const CastableBase* anchor,
                                    const core::type::Type* ty,
                                    const IOAttributes& attr,
                                    const Function::PipelineStage stage,
                                    const IODirection dir) {
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
                has_location |= (capabilities_.Contains(Capability::kLoosenValidationForShaderIO) &&
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
                    if (!capabilities_.Contains(Capability::kLoosenValidationForShaderIO)) {
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

void Structural::CheckBindingPoint(const CastableBase* anchor,
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

void Structural::ValidateShaderIOAnnotations(const CastableBase* msg_anchor,
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
                            e, [&](const core::type::Struct* s) { result = is_numeric(s); },
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
            if (!(capabilities_.Contains(Capability::kAllowUnannotatedModuleIOVariables) &&
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

bool Structural::CheckStructMemberAttributes(const core::type::StructMember* member,
                                             std::function<diag::Diagnostic&()> make_diag) {
    const auto checkers = IOAttributeCheckersFor(member->Attributes(), /*skip_builtins*/ false);
    for (const auto* checker : checkers) {
        auto res = checker->check(member->Type(), member->Attributes(), ir_.properties,
                                  IOAttributeUsage::kUndefinedUsage);
        if (res != Success) {
            make_diag() << res.Failure();
            return false;
        }
        if (!checker->type_check(member->Type(), ir_.properties)) {
            make_diag() << ToString(checker->kind) << " " << checker->type_error;
            return false;
        }
    }

    if (member->Attributes().location.has_value()) {
        if (ir_.properties.Contains(Property::kAllowLocationForNumericComposites)) {
            if (!member->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector() &&
                !member->Type()->UnwrapPtrOrRef()->Is<core::type::Struct>()) {
                make_diag() << "struct member with a location attribute must be a numeric scalar, "
                               "a numeric vector or a struct, but has type "
                            << member->Type()->FriendlyName();
                return false;
            }
        } else {
            if (!member->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                make_diag() << "struct member with a location attribute must be "
                               "a numeric scalar or vector, but has type "
                            << member->Type()->FriendlyName();
                return false;
            }
        }
    }
    return true;
}
void Structural::CheckLet(const Let* l) {
    CheckResultsAndOperands(l, Let::kNumResults, Let::kNumOperands);
}

void Structural::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                            //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },              //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },  //
        [&](const Construct* c) { CheckConstruct(c); },                  //
        [&](const Convert* c) { CheckConvert(c); },                      //
        [&](const Discard* d) {                                          //
            stage_restricted_instructions_.Add(
                d, SupportedStages{Function::PipelineStage::kFragment});        //
            CheckDiscard(d);                                                    //
        },                                                                      //
        [&](const UserCall* c) {                                                //
            if (c->Target()) {                                                  //
                auto calls =                                                    //
                    user_func_calls_.GetOr(c->Target(),                         //
                                           Hashset<const ir::UserCall*, 4>{});  //
                calls.Add(c);                                                   //
                user_func_calls_.Replace(c->Target(), calls);                   //
            }
            CheckUserCall(c);
        },
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Structural::CheckBuiltinCall(const BuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, BuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

    // CheckOperands above ensures that all args are non-null and have a valid type
    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });

    intrinsic::Context context{
        call->TableData(),
        type_mgr_,
        symbols_,
    };

    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             call->ExplicitTemplateParams(), args,
                                             core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    TINT_ASSERT(builtin->return_type);

    if (builtin->return_type != call->Result()->Type()) {
        AddError(call) << "call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(builtin->return_type);
        return;
    }

    // Check evaluation stage of parameters that are required to be const-expressions.
    for (uint32_t i = 0; i < builtin->parameters.Length(); i++) {
        const auto& p = builtin->parameters[i];
        const auto* arg = call->Args()[i];
        if (p.is_const && !arg->Is<Constant>()) {
            AddError(call, BuiltinCall::kArgsOperandOffset + i)
                << "the " << style::Variable(p.usage) << " argument must be a constant";
            return;
        }
    }

    if (auto* bc = call->As<CoreBuiltinCall>()) {
        CheckCoreBuiltinCall(bc, builtin.Get());
    }

    // Track the stages that this builtin call is limited to, so that we can check them against the
    // entry points that they are used from.
    SupportedStages stages;
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsComputePipeline)) {
        stages.Add(Function::PipelineStage::kCompute);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsFragmentPipeline)) {
        stages.Add(Function::PipelineStage::kFragment);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsVertexPipeline)) {
        stages.Add(Function::PipelineStage::kVertex);
    }
    stage_restricted_instructions_.Add(call, stages);
}

void Structural::CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                                      const core::intrinsic::Overload& overload) {
    auto idx_for_usage = [&](core::ParameterUsage usage) -> std::optional<uint32_t> {
        for (uint32_t i = 0; i < overload.parameters.Length(); ++i) {
            auto& p = overload.parameters[i];
            if (p.usage == usage) {
                return int32_t(i);
            }
        }
        return std::nullopt;
    };

    auto check_arg_in_range = [&](core::ParameterUsage usage, int32_t min, int32_t max) {
        auto idx_opt = idx_for_usage(usage);
        if (!idx_opt.has_value()) {
            return;
        }
        uint32_t idx = idx_opt.value();
        TINT_ASSERT(idx < call->Args().size());

        auto* val = call->Args()[idx];
        auto* const_val = val->As<ir::Constant>();
        TINT_ASSERT(const_val);
        auto* cnst = const_val->Value();

        if (val->Type()->Is<core::type::Vector>()) {
            for (size_t i = 0; i < cnst->NumElements(); i++) {
                auto value = cnst->Index(i)->ValueAs<int32_t>();
                if (value < min || value > max) {
                    AddError(call, idx)
                        << value << " outside range of [" << min << ", " << max << "]";
                    return;
                }
            }
        } else {
            auto value = cnst->ValueAs<int32_t>();
            if (value < min || value > max) {
                AddError(call, idx) << value << " outside range of [" << min << ", " << max << "]";
                return;
            }
        }
    };

    if (core::IsTexture(call->Func())) {
        check_arg_in_range(core::ParameterUsage::kComponent, 0, 3);
        check_arg_in_range(core::ParameterUsage::kOffset, -8, 7);
    }

    if (ir_.properties.Contains(Property::kDisallowVectorMinMaxClamp)) {
        switch (call->Func()) {
            case core::BuiltinFn::kClamp:
            case core::BuiltinFn::kMax:
            case core::BuiltinFn::kMin:
                if (call->Result()->Type()->Is<core::type::Vector>()) {
                    AddError(call) << "vector " << call->FriendlyName()
                                   << " disallowed by the DisallowVectorMinMaxClamp property";
                }
                break;
            default:
                break;
        }
    }

    if (call->Func() == core::BuiltinFn::kSubgroupMatrixLoad ||
        call->Func() == core::BuiltinFn::kSubgroupMatrixStore) {
        CheckSubgroupMatrixOpOffset(call);
    }
}

void Structural::CheckSubgroupMatrixOpOffset(const CoreBuiltinCall* call) {
    auto* p_arg = call->Args()[0];
    auto* offset_arg = call->Args()[1];

    auto* ptr_ty = p_arg->Type()->As<core::type::Pointer>();
    TINT_ASSERT(ptr_ty);

    auto* arr_ty = ptr_ty->StoreType()->As<core::type::Array>();
    TINT_ASSERT(arr_ty);

    auto const_count = arr_ty->ConstantCount();
    if (!const_count.has_value()) {
        return;
    }

    const core::type::SubgroupMatrix* mat_ty = nullptr;
    if (call->Func() == core::BuiltinFn::kSubgroupMatrixLoad) {
        mat_ty = call->Result()->Type()->As<core::type::SubgroupMatrix>();
    } else if (call->Func() == core::BuiltinFn::kSubgroupMatrixStore) {
        mat_ty = call->Args()[2]->Type()->As<core::type::SubgroupMatrix>();
    }
    TINT_ASSERT(mat_ty);

    auto arr_elem_size = arr_ty->ElemType()->Size();
    auto mat_comp_size = mat_ty->Type()->Size();
    TINT_ASSERT(mat_comp_size > 0);

    auto limit = (const_count.value() * arr_elem_size) / mat_comp_size;

    if (auto* offset_const = offset_arg->As<ir::Constant>()) {
        auto* offset_val = offset_const->Value();
        uint32_t offset = 0;
        if (offset_arg->Type()->IsUnsignedIntegerScalar()) {
            offset = offset_val->ValueAs<u32>();
        } else if (offset_arg->Type()->IsSignedIntegerScalar()) {
            auto ival = offset_val->ValueAs<i32>();
            if (ival < 0) {
                AddError(call, 1) << "the offset argument of " << call->Func()
                                  << " must be non-negative";
                return;
            }
            offset = static_cast<uint32_t>(ival);
        }

        if (offset >= limit) {
            AddError(call, 1) << "the offset argument of " << call->Func() << " (" << offset
                              << ") is out of bounds of the array type of size " << limit;
        }
    }
}

void Structural::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, MemberBuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

    auto args = Vector<const core::type::Type*, 8>({call->Object()->Type()});
    for (auto* arg : call->Args()) {
        args.Push(arg->Type());
    }
    intrinsic::Context context{
        call->TableData(),
        type_mgr_,
        symbols_,
    };

    auto result = core::intrinsic::LookupMemberFn(context, call->FriendlyName().c_str(),
                                                  call->FuncId(), call->ExplicitTemplateParams(),
                                                  std::move(args), core::EvaluationStage::kRuntime);
    if (result != Success) {
        AddError(call) << result.Failure();
        return;
    }

    if (result->return_type != call->Result()->Type()) {
        // Note: This is not currently tested in core unittests as there are no concrete
        // MemberBuiltinCall implementations in core IR. This is tested by backend-specific
        // (e.g. HLSL) validation tests.
        AddError(call) << "member call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(result->return_type);
    }
}

void Structural::CheckConstruct(const Construct* construct) {
    CheckResultsAndOperandRange(construct, Construct::kNumResults, Construct::kMinOperands);
}

void Structural::CheckConvert(const Convert* convert) {
    if (!CheckResultsAndOperands(convert, Convert::kNumResults, Convert::kNumOperands)) {
        return;
    }

    auto* result_type = convert->Result()->Type();
    auto* value_type = convert->Operand(Convert::kValueOperandOffset)->Type();

    intrinsic::CtorConv conv_ty;
    Vector<const core::type::Type*, 1> template_type;
    tint::Switch(
        result_type,                                                             //
        [&](const core::type::I32*) { conv_ty = intrinsic::CtorConv::kI32; },    //
        [&](const core::type::U32*) { conv_ty = intrinsic::CtorConv::kU32; },    //
        [&](const core::type::U64*) { conv_ty = intrinsic::CtorConv::kU64; },    //
        [&](const core::type::F32*) { conv_ty = intrinsic::CtorConv::kF32; },    //
        [&](const core::type::F16*) { conv_ty = intrinsic::CtorConv::kF16; },    //
        [&](const core::type::Bool*) { conv_ty = intrinsic::CtorConv::kBool; },  //
        [&](const core::type::Vector* v) {
            conv_ty = intrinsic::VectorCtorConv(v->Width());
            template_type.Push(v->Type());
        },
        [&](const core::type::Matrix* m) {
            conv_ty = intrinsic::MatrixCtorConv(m->Columns(), m->Rows());
            template_type.Push(m->Type());
        },
        [&](Default) { conv_ty = intrinsic::CtorConv::kNone; });

    if (conv_ty == intrinsic::CtorConv::kNone) {
        AddError(convert) << "not defined for result type, " << NameOf(result_type);
        return;
    }

    auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
    auto match =
        table.Lookup(conv_ty, template_type, Vector{value_type}, core::EvaluationStage::kOverride);
    if (match != Success || !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConverter)) {
        AddError(convert) << "No defined converter for " << NameOf(value_type) << " -> "
                          << NameOf(result_type);
        return;
    }
}

void Structural::CheckDiscard(const tint::core::ir::Discard* discard) {
    CheckResultsAndOperands(discard, Discard::kNumResults, Discard::kNumOperands);
}

void Structural::CheckUserCall(const UserCall* call) {
    if (!CheckResultsAndOperandRange(call, UserCall::kNumResults, UserCall::kMinOperands)) {
        return;
    }

    if (!call->Target()) {
        AddError(call, UserCall::kFunctionOperandOffset) << "target not defined or not a function";
        return;
    }

    if (call->Target()->IsEntryPoint()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "call target must not have a pipeline stage";
    }

    if (call->Target()->ReturnType() != call->Result()->Type()) {
        AddError(call) << "result type does not match function return type";
        return;
    }

    auto args = call->Args();
    auto params = call->Target()->Params();
    if (args.size() != params.Length()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "function has " << params.Length() << " parameters, but call provides "
            << args.size() << " arguments";
        return;
    }

    for (size_t i = 0; i < args.size(); i++) {
        bool allow_mismatch = false;
        if (auto* arg_buffer_ty = args[i]->Type()->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
            auto* arg_ptr_ty = args[i]->Type()->As<core::type::Pointer>();
            if (auto* param_ptr_ty = params[i]->Type()->As<core::type::Pointer>()) {
                if (auto* param_buffer_ty =
                        param_ptr_ty->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
                    allow_mismatch = arg_ptr_ty->AddressSpace() == param_ptr_ty->AddressSpace() &&
                                     arg_ptr_ty->Access() == param_ptr_ty->Access();
                    const bool both_constant =
                        arg_buffer_ty->Count()->Is<core::type::ConstantArrayCount>() &&
                        param_buffer_ty->Count()->Is<core::type::ConstantArrayCount>();
                    uint32_t arg_size = arg_buffer_ty->ConstantCount().value_or(0);
                    uint32_t param_size = param_buffer_ty->ConstantCount().value_or(0);
                    allow_mismatch &=
                        param_buffer_ty->Count()->Is<core::type::RuntimeArrayCount>() ||
                        (both_constant && param_size < arg_size);
                }
            }
        }
        if (!allow_mismatch && args[i]->Type() != params[i]->Type()) {
            AddError(call, UserCall::kArgsOperandOffset + i)
                << "type " << NameOf(params[i]->Type()) << " of function parameter " << i
                << " does not match argument type " << NameOf(args[i]->Type());
        }
    }
}

void Structural::CheckAccess(const Access* a) {
    CheckResultsAndOperandRange(a, Access::kNumResults, Access::kMinNumOperands);
}

void Structural::CheckBinary(const Binary* b) {
    if (!CheckResultsAndOperands(b, Binary::kNumResults, Binary::kNumOperands)) {
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalAnd) {
        AddError(b) << "logical-and is not valid in the IR";
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalOr) {
        AddError(b) << "logical-or is not valid in the IR";
        return;
    }
}

void Structural::CheckUnary(const Unary* u) {
    if (!CheckResultsAndOperands(u, Unary::kNumResults, Unary::kNumOperands)) {
        return;
    }

    if (u->Val()) {
        intrinsic::Context context{u->TableData(), type_mgr_, symbols_};

        auto overload = core::intrinsic::LookupUnary(context, u->Op(), u->Val()->Type(),
                                                     core::EvaluationStage::kRuntime);
        if (overload != Success) {
            AddError(u) << overload.Failure();
            return;
        }

        if (auto* result = u->Result(0)) {
            if (overload->return_type != result->Type()) {
                AddError(u) << "result value type " << NameOf(result->Type()) << " does not match "
                            << style::Instruction(Disassemble().NameOf(u->Op())) << " result type "
                            << NameOf(overload->return_type);
            }
        }
    }
}

void Structural::CheckIf(const If* if_) {
    CheckResults(if_);
    CheckOperands(if_, If::kNumOperands);

    if (if_->False() && if_->False()->Is<core::ir::MultiInBlock>()) {
        AddError(if_) << "if false block must be a block";
    }
    if (if_->True() && if_->True()->Is<core::ir::MultiInBlock>()) {
        AddError(if_) << "if true block must be a block";
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    if (!if_->False()->IsEmpty()) {
        QueueBlock(if_->False());
    }

    QueueBlock(if_->True());

    tasks_.Push([this, if_] { control_stack_.Push(if_); });
}

void Structural::CheckLoop(const Loop* l) {
    CheckResults(l);
    CheckOperands(l, 0);

    if (l->Initializer()->Is<core::ir::MultiInBlock>()) {
        AddError(l->Initializer()) << "loop initializer must be a block";
    }

    if (!l->Initializer()->IsEmpty()) {
        if (!l->Initializer()->Terminator() ||
            !l->Initializer()->Terminator()->Is<core::ir::NextIteration>()) {
            AddError(l->Initializer()) << "loop initializer must have a NextIteration terminator";
        }
    }

    // Note: Tasks are queued in reverse order of their execution
    tasks_.Push([this] { control_stack_.Pop(); });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this] { EndBlock(); });
    }
    tasks_.Push([this] { EndBlock(); });
    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this, l] {
            if (!l->Continuing()->Terminator()->IsAnyOf<NextIteration, BreakIf>()) {
                AddError(l->Continuing())
                    << "loop continuing terminator can only be next_iteration or break_if";
            }
            EndBlock();
        });
    }

    // ⎡Initializer              ⎤
    // ⎢    ⎡Body               ⎤⎥
    // ⎣    ⎣    [Continuing ]  ⎦⎦

    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this, l] { BeginBlock(l->Continuing()); });
    }

    tasks_.Push([this, l] { BeginBlock(l->Body()); });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this, l] { BeginBlock(l->Initializer()); });
    }
    tasks_.Push([this, l] { control_stack_.Push(l); });
}

void Structural::CheckSwitch(const Switch* s) {
    CheckResults(s);
    CheckOperands(s, Switch::kNumOperands);

    if (s->Condition() && !s->Condition()->Type()->IsIntegerScalar()) {
        auto* cond_ty = s->Condition() ? s->Condition()->Type() : nullptr;
        AddError(s, Switch::kConditionOperandOffset)
            << "condition type " << NameOf(cond_ty) << " must be an integer scalar";
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    bool found_default = false;
    for (auto& cse : s->Cases()) {
        if (cse.block->Is<core::ir::MultiInBlock>()) {
            AddError(s) << "case block must be a block";
        }

        if (cse.selectors.IsEmpty()) {
            AddError(s) << "case does not have any selectors";
        }

        QueueBlock(cse.block);
        for (const auto& sel : cse.selectors) {
            if (sel.IsDefault()) {
                if (found_default) {
                    AddError(s) << "multiple default selectors in switch";
                }
                found_default = true;
            } else if (!sel.val->Type()->IsIntegerScalar()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must be an integer scalar";
            } else if (s->Condition() && sel.val->Type() != s->Condition()->Type()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must match the switch condition type "
                            << NameOf(s->Condition()->Type());
            }
        }
    }

    if (!found_default) {
        AddError(s) << "missing default case for switch";
    }

    tasks_.Push([this, s] { control_stack_.Push(s); });
}

void Structural::CheckSwizzle(const Swizzle* s) {
    if (!CheckResultsAndOperands(s, Swizzle::kNumResults, Swizzle::kNumOperands)) {
        return;
    }

    auto* src_vec = s->Object()->Type()->As<core::type::Vector>();
    if (!src_vec) {
        AddError(s) << "object of swizzle, " << NameOf(s->Object()) << ", is not a vector, "
                    << NameOf(s->Object()->Type());
        return;
    }

    auto indices = s->Indices();
    if (indices.Length() < Swizzle::kMinNumIndices) {
        AddError(s) << "expected at least " << Swizzle::kMinNumIndices << " indices";
        return;
    }

    if (indices.Length() > Swizzle::kMaxNumIndices) {
        AddError(s) << "expected at most " << Swizzle::kMaxNumIndices << " indices";
        return;
    }

    auto elem_count = src_vec->Elements().count;
    for (auto& idx : indices) {
        if (idx > Swizzle::kMaxIndexValue || idx >= elem_count) {
            AddError(s) << "invalid index value";
            return;
        }
    }

    auto* elem_ty = src_vec->Elements().type;
    auto* expected_ty = type_mgr_.MatchWidth(elem_ty, indices.Length());
    auto* result_ty = s->Result()->Type();
    if (result_ty != expected_ty) {
        AddError(s) << "result type " << NameOf(result_ty) << " does not match expected type, "
                    << NameOf(expected_ty);
        return;
    }
}

void Structural::CheckTerminator(const Terminator* b) {
    // All terminators should have zero results
    if (!CheckResults(b, 0)) {
        return;
    }

    // Operands must be alive and in scope if they are not nullptr.
    if (!CheckOperands(b)) {
        return;
    }

    tint::Switch(
        b,                                                           //
        [&](const ir::BreakIf* i) { CheckBreakIf(i); },              //
        [&](const ir::Continue* c) { CheckContinue(c); },            //
        [&](const ir::Exit* e) { CheckExit(e); },                    //
        [&](const ir::NextIteration* n) { CheckNextIteration(n); },  //
        [&](const ir::Return* ret) { CheckReturn(ret); },            //
        [&](const ir::TerminateInvocation*) {},                      //
        [&](const ir::Unreachable* u) { CheckUnreachable(u); },      //
        [&](Default) { AddError(b) << "missing validation"; });

    if (b->next) {
        AddError(b) << "must be the last instruction in the block";
    }
}

void Structural::CheckBreakIf(const BreakIf* b) {
    if (b->Condition() == nullptr) {
        AddError(b) << "break_if condition cannot be nullptr";
        return;
    }

    if (!b->Condition()->Type() || !b->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(b) << "condition must be a 'bool'";
        return;
    }

    auto* loop = b->Loop();
    if (loop == nullptr) {
        AddError(b) << "has no associated loop";
        return;
    }

    if (loop->Continuing() != b->Block()) {
        AddError(b) << "must only be called directly from loop continuing";
    }

    auto next_iter_values = b->NextIterValues();
    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(b, b->ArgsOperandOffset(), next_iter_values.size(), body,
                                 body->Params());
    }

    auto exit_values = b->ExitValues();
    CheckOperandsMatchTarget(b, b->ArgsOperandOffset() + next_iter_values.size(),
                             exit_values.size(), loop, loop->Results());
}

void Structural::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    if (loop == nullptr) {
        AddError(c) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Body(), c)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(c) << "must only be called from loop body";
        } else {
            AddError(c) << "called outside of associated loop";
        }
    }

    if (auto* cont = loop->Continuing()) {
        CheckOperandsMatchTarget(c, Continue::kArgsOperandOffset, c->Args().size(), cont,
                                 cont->Params());
    }
}

void Structural::CheckExit(const Exit* e) {
    if (e->ControlInstruction() == nullptr) {
        AddError(e) << "has no parent control instruction";
        return;
    }

    if (control_stack_.IsEmpty()) {
        AddError(e) << "found outside all control instructions";
        return;
    }

    auto args = e->Args();
    CheckOperandsMatchTarget(e, e->ArgsOperandOffset(), args.size(), e->ControlInstruction(),
                             e->ControlInstruction()->Results());

    tint::Switch(
        e,                                                     //
        [&](const ir::ExitIf* i) { CheckExitIf(i); },          //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },      //
        [&](const ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
        [&](Default) { AddError(e) << "missing validation"; });
}

void Structural::CheckNextIteration(const NextIteration* n) {
    auto* loop = n->Loop();
    if (loop == nullptr) {
        AddError(n) << "has no associated loop";
        return;
    }
    if (loop->Initializer() != n->Block() && loop->Continuing() != n->Block()) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(n) << "must only be called directly from loop initializer or continuing";
        } else {
            AddError(n) << "called outside of associated loop";
        }
    }

    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(n, NextIteration::kArgsOperandOffset, n->Args().size(), body,
                                 body->Params());
    }
}

void Structural::CheckExitIf(const ExitIf* e) {
    if (control_stack_.Back() != e->If()) {
        AddError(e) << "if target jumps over other control instructions";
        AddNote(control_stack_.Back()) << "first control instruction jumped";
    }
}

void Structural::CheckReturn(const Return* ret) {
    if (!CheckOperands(ret, Return::kMinOperands, Return::kMaxOperands)) {
        return;
    }

    auto* func = ret->Func();
    if (func == nullptr) {
        // Func() returning nullptr after CheckResultsAndOperandRange is due to the first
        // operand being not a function
        AddError(ret) << "expected function for first operand";
        return;
    }

    if (func != ContainingFunction(ret)) {
        AddError(ret) << "function operand does not match containing function";
        return;
    }

    if (func->ReturnType()->Is<core::type::Void>()) {
        if (ret->HasValue()) {
            AddError(ret) << "unexpected return value";
        }
    } else {
        if (!ret->Value()) {
            AddError(ret) << "expected return value";
        } else if (ret->Value()->Type() != func->ReturnType()) {
            AddError(ret) << "return value type " << NameOf(ret->Value()->Type())
                          << " does not match function return type " << NameOf(func->ReturnType());
        }
    }
}

void Structural::CheckUnreachable(const Unreachable* u) {
    CheckResultsAndOperands(u, Unreachable::kNumResults, Unreachable::kNumOperands);
}

void Structural::CheckControlsAllowingIf(const Exit* exit, const Instruction* control) {
    bool found = false;
    for (auto ctrl : tint::Reverse(control_stack_)) {
        if (ctrl == control) {
            found = true;
            break;
        }
        // A exit switch can step over if instructions, but no others.
        if (!ctrl->Is<ir::If>()) {
            AddError(exit) << control->FriendlyName()
                           << " target jumps over other control instructions";
            AddNote(ctrl) << "first control instruction jumped";
            return;
        }
    }
    if (!found) {
        AddError(exit) << control->FriendlyName() << " not found in parent control instructions";
    }
}

void Structural::CheckExitSwitch(const ExitSwitch* s) {
    CheckControlsAllowingIf(s, s->ControlInstruction());
}

void Structural::CheckExitLoop(const ExitLoop* l) {
    CheckControlsAllowingIf(l, l->ControlInstruction());

    const Instruction* inst = l;
    const Loop* control = l->Loop();
    while (inst) {
        // Found parent loop
        if (inst->Block()->Parent() == control) {
            if (inst->Block() == control->Continuing()) {
                AddError(l) << "loop exit jumps out of continuing block";
                if (control->Continuing() != l->Block()) {
                    AddNote(control->Continuing()) << "in continuing block";
                }
            } else if (inst->Block() == control->Initializer()) {
                AddError(l) << "loop exit not permitted in loop initializer";
                if (control->Initializer() != l->Block()) {
                    AddNote(control->Initializer()) << "in initializer block";
                }
            }
            break;
        }
        inst = inst->Block()->Parent();
    }
}

void Structural::CheckLoad(const Load* l) {
    CheckResultsAndOperands(l, Load::kNumResults, Load::kNumOperands);
}

void Structural::CheckStore(const Store* s) {
    if (!CheckResultsAndOperands(s, Store::kNumResults, Store::kNumOperands)) {
        return;
    }

    if (auto* from = s->From()) {
        if (auto* to = s->To()) {
            auto* mv = As<core::type::MemoryView>(to->Type());
            if (!mv) {
                AddError(s, Store::kToOperandOffset)
                    << "store target operand " << NameOf(to->Type()) << " is not a memory view";
                return;
            }

            if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
                AddError(s, Store::kToOperandOffset)
                    << "store target operand has a non-writeable access type, "
                    << style::Literal(ToString(mv->Access()));
                return;
            }

            auto* value_type = from->Type();
            auto* store_type = mv->StoreType();
            if (value_type != store_type) {
                AddError(s, Store::kFromOperandOffset)
                    << "value type " << NameOf(value_type) << " does not match store type "
                    << NameOf(store_type);
                return;
            }

            if (!store_type->IsConstructible()) {
                AddError(s) << "store type " << NameOf(store_type) << " is not constructible";
                return;
            }
        }
    }
}

void Structural::CheckLoadVectorElement(const LoadVectorElement* l) {
    CheckResultsAndOperands(l, LoadVectorElement::kNumResults, LoadVectorElement::kNumOperands);
}

void Structural::CheckStoreVectorElement(const StoreVectorElement* s) {
    CheckResultsAndOperands(s, StoreVectorElement::kNumResults, StoreVectorElement::kNumOperands);
}

void Structural::CheckPhony(const Phony* p) {
    if (!ir_.properties.Contains(Property::kAllowPhonyInstructions)) {
        AddError(p) << "missing property 'kAllowPhonyInstructions'";
        return;
    }

    if (!CheckResultsAndOperands(p, Phony::kNumResults, Phony::kNumOperands)) {
        return;
    }
}

void Structural::CheckOperandsMatchTarget(const Instruction* source_inst,
                                          size_t source_operand_offset,
                                          size_t source_operand_count,
                                          const CastableBase* target,
                                          VectorRef<const Value*> target_values) {
    if (source_operand_count != target_values.Length()) {
        auto values = [&](size_t n) { return n == 1 ? " value" : " values"; };
        AddError(source_inst) << "provides " << source_operand_count << values(source_operand_count)
                              << " but " << NameOf(target) << " expects " << target_values.Length()
                              << values(target_values.Length());
        AddDeclarationNote(target);
    }
    size_t count = std::min(source_operand_count, target_values.Length());
    for (size_t i = 0; i < count; i++) {
        auto* source_value = source_inst->Operand(source_operand_offset + i);
        auto* target_value = target_values[i];
        if (!source_value || !target_value) {
            continue;  // Caller should be checking operands are not null
        }
        auto* source_type = source_value->Type();
        auto* target_type = target_value->Type();
        if (source_type != target_type) {
            AddError(source_inst, source_operand_offset + i)
                << "operand with type " << NameOf(source_type) << " does not match "
                << NameOf(target) << " target type " << NameOf(target_type);
            AddDeclarationNote(target_value);
        }
    }
}

}  // namespace tint::core::ir::validator
