// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_API_OPTIONS_PIXEL_LOCAL_H_
#define SRC_TINT_API_OPTIONS_PIXEL_LOCAL_H_

#include <unordered_map>

#include "src/tint/utils/reflection/reflection.h"

namespace tint {

/// Options used to specify pixel local mappings
struct PixelLocalOptions {
    /// Index of pixel_local structure member index to attachment index
    std::unordered_map<uint32_t, uint32_t> attachments;

    /// The supported pixel local storage attachment format
    enum class TexelFormat : uint8_t {
        kR32Sint,
        kR32Uint,
        kR32Float,
        kUndefined,
    };
    /// Index of pixel_local structure member index to pixel local storage attachment format
    std::unordered_map<uint32_t, TexelFormat> attachment_formats;

    /// The bind group index of all pixel local storage attachments
    uint32_t pixel_local_group_index = 0;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(attachments, attachment_formats, pixel_local_group_index);
};

/// Reflect valid value ranges for the PixelLocalOptions::TexelFormat enum.
TINT_REFLECT_ENUM_RANGE(PixelLocalOptions::TexelFormat, kR32Sint, kR32Float);

}  // namespace tint

#endif  // SRC_TINT_API_OPTIONS_PIXEL_LOCAL_H_
