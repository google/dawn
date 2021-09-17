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

#define LIMITS(X)                                           \
    X(Higher, maxTextureDimension1D, 8192)                  \
    X(Higher, maxTextureDimension2D, 8192)                  \
    X(Higher, maxTextureDimension3D, 2048)                  \
    X(Higher, maxTextureArrayLayers, 256)                   \
    X(Higher, maxBindGroups, 4)                             \
    X(Higher, maxDynamicUniformBuffersPerPipelineLayout, 8) \
    X(Higher, maxDynamicStorageBuffersPerPipelineLayout, 4) \
    X(Higher, maxSampledTexturesPerShaderStage, 16)         \
    X(Higher, maxSamplersPerShaderStage, 16)                \
    X(Higher, maxStorageBuffersPerShaderStage, 8)           \
    X(Higher, maxStorageTexturesPerShaderStage, 4)          \
    X(Higher, maxUniformBuffersPerShaderStage, 12)          \
    X(Higher, maxUniformBufferBindingSize, 16384)           \
    X(Higher, maxStorageBufferBindingSize, 134217728)       \
    X(Lower, minUniformBufferOffsetAlignment, 256)          \
    X(Lower, minStorageBufferOffsetAlignment, 256)          \
    X(Higher, maxVertexBuffers, 8)                          \
    X(Higher, maxVertexAttributes, 16)                      \
    X(Higher, maxVertexBufferArrayStride, 2048)             \
    X(Higher, maxInterStageShaderComponents, 60)            \
    X(Higher, maxComputeWorkgroupStorageSize, 16352)        \
    X(Higher, maxComputeInvocationsPerWorkgroup, 256)       \
    X(Higher, maxComputeWorkgroupSizeX, 256)                \
    X(Higher, maxComputeWorkgroupSizeY, 256)                \
    X(Higher, maxComputeWorkgroupSizeZ, 64)                 \
    X(Higher, maxComputeWorkgroupsPerDimension, 65535)

namespace dawn_native {
    namespace {
        enum class LimitBetterDirection {
            Lower,
            Higher,
        };

        template <LimitBetterDirection Better>
        struct CheckLimit;

        template <>
        struct CheckLimit<LimitBetterDirection::Lower> {
            template <typename T>
            static MaybeError Invoke(T supported, T required) {
                if (required < supported) {
                    return DAWN_VALIDATION_ERROR("requiredLimit lower than supported limit");
                }
                return {};
            }
        };

        template <>
        struct CheckLimit<LimitBetterDirection::Higher> {
            template <typename T>
            static MaybeError Invoke(T supported, T required) {
                if (required > supported) {
                    return DAWN_VALIDATION_ERROR("requiredLimit greater than supported limit");
                }
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
#define X(Better, limitName, defaultValue) limits->limitName = defaultValue;
        LIMITS(X)
#undef X
    }

    Limits ReifyDefaultLimits(const Limits& limits) {
        Limits out;
#define X(Better, limitName, defaultValue)     \
    if (!IsLimitUndefined(limits.limitName)) { \
        out.limitName = limits.limitName;      \
    } else {                                   \
        out.limitName = defaultValue;          \
    }
        LIMITS(X)
#undef X
        return out;
    }

    MaybeError ValidateLimits(const Limits& supportedLimits, const Limits& requiredLimits) {
#define X(Better, limitName, defaultValue)                                                    \
    if (!IsLimitUndefined(requiredLimits.limitName)) {                                        \
        DAWN_TRY(CheckLimit<LimitBetterDirection::Better>::Invoke(supportedLimits.limitName,  \
                                                                  requiredLimits.limitName)); \
    }
        LIMITS(X)
#undef X
        return {};
    }

}  // namespace dawn_native
