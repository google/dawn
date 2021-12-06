// Copyright 2021 The Dawn Authors
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

#include "dawn_native/Limits.h"

#include "common/Assert.h"

#include <array>

// clang-format off
// TODO(crbug.com/dawn/685):
// For now, only expose these tiers until metrics can determine better ones.
#define LIMITS_WORKGROUP_STORAGE_SIZE(X)                                  \
    X(Higher, maxComputeWorkgroupStorageSize, 16352, 32768, 49152, 65536)

#define LIMITS_STORAGE_BUFFER_BINDING_SIZE(X)                                             \
    X(Higher, maxStorageBufferBindingSize, 134217728, 1073741824, 2147483647, 4294967295)

// TODO(crbug.com/dawn/685):
// These limits don't have tiers yet. Define two tiers with the same values since the macros
// in this file expect more than one tier.
#define LIMITS_OTHER(X)                                                \
    X(Higher,                     maxTextureDimension1D,  8192,  8192) \
    X(Higher,                     maxTextureDimension2D,  8192,  8192) \
    X(Higher,                     maxTextureDimension3D,  2048,  2048) \
    X(Higher,                     maxTextureArrayLayers,   256,   256) \
    X(Higher,                             maxBindGroups,     4,     4) \
    X(Higher, maxDynamicUniformBuffersPerPipelineLayout,     8,     8) \
    X(Higher, maxDynamicStorageBuffersPerPipelineLayout,     4,     4) \
    X(Higher,          maxSampledTexturesPerShaderStage,    16,    16) \
    X(Higher,                 maxSamplersPerShaderStage,    16,    16) \
    X(Higher,           maxStorageBuffersPerShaderStage,     8,     8) \
    X(Higher,          maxStorageTexturesPerShaderStage,     4,     4) \
    X(Higher,           maxUniformBuffersPerShaderStage,    12,    12) \
    X(Higher,               maxUniformBufferBindingSize, 16384, 16384) \
    X( Lower,           minUniformBufferOffsetAlignment,   256,   256) \
    X( Lower,           minStorageBufferOffsetAlignment,   256,   256) \
    X(Higher,                          maxVertexBuffers,     8,     8) \
    X(Higher,                       maxVertexAttributes,    16,    16) \
    X(Higher,                maxVertexBufferArrayStride,  2048,  2048) \
    X(Higher,             maxInterStageShaderComponents,    60,    60) \
    X(Higher,         maxComputeInvocationsPerWorkgroup,   256,   256) \
    X(Higher,                  maxComputeWorkgroupSizeX,   256,   256) \
    X(Higher,                  maxComputeWorkgroupSizeY,   256,   256) \
    X(Higher,                  maxComputeWorkgroupSizeZ,    64,    64) \
    X(Higher,          maxComputeWorkgroupsPerDimension, 65535, 65535)
// clang-format on

#define LIMITS_EACH_GROUP(X)              \
    X(LIMITS_WORKGROUP_STORAGE_SIZE)      \
    X(LIMITS_STORAGE_BUFFER_BINDING_SIZE) \
    X(LIMITS_OTHER)

#define LIMITS(X)                         \
    LIMITS_WORKGROUP_STORAGE_SIZE(X)      \
    LIMITS_STORAGE_BUFFER_BINDING_SIZE(X) \
    LIMITS_OTHER(X)

namespace dawn_native {
    namespace {
        template <uint32_t A, uint32_t B>
        constexpr void StaticAssertSame() {
            static_assert(A == B, "Mismatching tier count in limit group.");
        }

        template <uint32_t I, uint32_t... Is>
        constexpr uint32_t ReduceSameValue(std::integer_sequence<uint32_t, I, Is...>) {
            int unused[] = {0, (StaticAssertSame<I, Is>(), 0)...};
            DAWN_UNUSED(unused);
            return I;
        }

        enum class LimitBetterDirection {
            Lower,
            Higher,
        };

        template <LimitBetterDirection Better>
        struct CheckLimit;

        template <>
        struct CheckLimit<LimitBetterDirection::Lower> {
            template <typename T>
            static bool IsBetter(T lhs, T rhs) {
                return lhs < rhs;
            }

            template <typename T>
            static MaybeError Validate(T supported, T required) {
                DAWN_INVALID_IF(IsBetter(required, supported),
                                "Required limit (%u) is lower than the supported limit (%u).",
                                required, supported);
                return {};
            }
        };

        template <>
        struct CheckLimit<LimitBetterDirection::Higher> {
            template <typename T>
            static bool IsBetter(T lhs, T rhs) {
                return lhs > rhs;
            }

            template <typename T>
            static MaybeError Validate(T supported, T required) {
                DAWN_INVALID_IF(IsBetter(required, supported),
                                "Required limit (%u) is greater than the supported limit (%u).",
                                required, supported);
                return {};
            }
        };

        template <typename T>
        bool IsLimitUndefined(T value) {
            static_assert(sizeof(T) != sizeof(T), "IsLimitUndefined not implemented for this type");
            return false;
        }

        template <>
        bool IsLimitUndefined<uint32_t>(uint32_t value) {
            return value == wgpu::kLimitU32Undefined;
        }

        template <>
        bool IsLimitUndefined<uint64_t>(uint64_t value) {
            return value == wgpu::kLimitU64Undefined;
        }

    }  // namespace

    void GetDefaultLimits(Limits* limits) {
        ASSERT(limits != nullptr);
#define X(Better, limitName, base, ...) limits->limitName = base;
        LIMITS(X)
#undef X
    }

    Limits ReifyDefaultLimits(const Limits& limits) {
        Limits out;
#define X(Better, limitName, base, ...)                                           \
    if (IsLimitUndefined(limits.limitName) ||                                     \
        CheckLimit<LimitBetterDirection::Better>::IsBetter(                       \
            static_cast<decltype(limits.limitName)>(base), limits.limitName)) {   \
        /* If the limit is undefined or the default is better, use the default */ \
        out.limitName = base;                                                     \
    } else {                                                                      \
        out.limitName = limits.limitName;                                         \
    }
        LIMITS(X)
#undef X
        return out;
    }

    MaybeError ValidateLimits(const Limits& supportedLimits, const Limits& requiredLimits) {
#define X(Better, limitName, ...)                                                  \
    if (!IsLimitUndefined(requiredLimits.limitName)) {                             \
        DAWN_TRY_CONTEXT(CheckLimit<LimitBetterDirection::Better>::Validate(       \
                             supportedLimits.limitName, requiredLimits.limitName), \
                         "validating " #limitName);                                \
    }
        LIMITS(X)
#undef X
        return {};
    }

    Limits ApplyLimitTiers(Limits limits) {
#define X_TIER_COUNT(Better, limitName, ...) , std::integer_sequence<uint64_t, __VA_ARGS__>{}.size()
#define GET_TIER_COUNT(LIMIT_GROUP) \
    ReduceSameValue(std::integer_sequence<uint32_t LIMIT_GROUP(X_TIER_COUNT)>{})

#define X_EACH_GROUP(LIMIT_GROUP)                                    \
    {                                                                \
        constexpr uint32_t kTierCount = GET_TIER_COUNT(LIMIT_GROUP); \
        for (uint32_t i = kTierCount; i != 0; --i) {                 \
            LIMIT_GROUP(X_CHECK_BETTER_AND_CLAMP)                    \
            /* Limits fit in tier and have been clamped. Break. */   \
            break;                                                   \
        }                                                            \
    }

#define X_CHECK_BETTER_AND_CLAMP(Better, limitName, ...)                                       \
    {                                                                                          \
        constexpr std::array<decltype(Limits::limitName), kTierCount> tiers{__VA_ARGS__};      \
        decltype(Limits::limitName) tierValue = tiers[i - 1];                                  \
        if (CheckLimit<LimitBetterDirection::Better>::IsBetter(tierValue, limits.limitName)) { \
            /* The tier is better. Go to the next tier. */                                     \
            continue;                                                                          \
        } else if (tierValue != limits.limitName) {                                            \
            /* Better than the tier. Degrade |limits| to the tier. */                          \
            limits.limitName = tiers[i - 1];                                                   \
        }                                                                                      \
    }

        LIMITS_EACH_GROUP(X_EACH_GROUP)
#undef X_CHECK_BETTER
#undef X_EACH_GROUP
#undef GET_TIER_COUNT
#undef X_TIER_COUNT
        return limits;
    }

}  // namespace dawn_native
