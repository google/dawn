// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_IMMEDIATECONSTANTSLAYOUT_H_
#define SRC_DAWN_NATIVE_IMMEDIATECONSTANTSLAYOUT_H_

#include <unordered_map>

#include "dawn/common/ityp_bitset.h"
#include "dawn/native/IntegerTypes.h"

namespace dawn::native {

// Define common immediate data layout. Append members to expand layouts.
// NOTE: 'offsetof' doesn't support non-standard-layout structs. So use
// aggregate instead of inheritance for RenderImmediateConstants and
// ComputeImmediateConstants.
struct UserImmediateConstants {
    uint32_t userImmediateData[kMaxExternalImmediateConstantsPerPipeline];
};

struct ClampFragDepthArgs {
    float minClampFragDepth;
    float maxClampFragDepth;
};

// Define render pipeline immediate data layout. Append members to
// expand the layout.
struct RenderImmediateConstants {
    UserImmediateConstants userConstants;

    ClampFragDepthArgs clampFragDepth;

    // first index offset
    uint32_t firstVertex;
    uint32_t firstInstance;
};
static_assert(sizeof(RenderImmediateConstants) == 32u);

struct NumWorkgroupsDimensions {
    uint32_t numWorkgroupsX;
    uint32_t numWorkgroupsY;
    uint32_t numWorkgroupsZ;
};

// Define compute pipeline immediate data layout. Append members to
// expand the layout.
struct ComputeImmediateConstants {
    UserImmediateConstants userConstants;

    NumWorkgroupsDimensions numWorkgroups;
};
static_assert(sizeof(ComputeImmediateConstants) == 28u);

// Convert byte sizes and offsets into immediate constant indices and offsets
// (dividing everything by kImmediateConstantElementByteSize)
ImmediateConstantMask GetImmediateConstantBlockBits(size_t byteOffset, size_t byteSize);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_IMMEDIATECONSTANTSLAYOUT_H_
