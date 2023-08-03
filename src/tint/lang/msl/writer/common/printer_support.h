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

#ifndef SRC_TINT_LANG_MSL_WRITER_COMMON_PRINTER_SUPPORT_H_
#define SRC_TINT_LANG_MSL_WRITER_COMMON_PRINTER_SUPPORT_H_

#include <cstdint>
#include <string>

#include "src/tint/lang/core/builtin/builtin_value.h"
#include "src/tint/lang/core/builtin/interpolation.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::msl::writer {

/// A pair of byte size and alignment `uint32_t`s.
struct SizeAndAlign {
    /// The size
    uint32_t size;
    /// The alignment
    uint32_t align;
};

/// @param ty the type to generate size and align for
/// @returns the MSL packed type size and alignment in bytes for the given type.
SizeAndAlign MslPackedTypeSizeAndAlign(const type::Type* ty);

/// Converts a builtin to an attribute name
/// @param builtin the builtin to convert
/// @returns the string name of the builtin or blank on error
std::string BuiltinToAttribute(builtin::BuiltinValue builtin);

/// Converts interpolation attributes to an MSL attribute
/// @param type the interpolation type
/// @param sampling the interpolation sampling
/// @returns the string name of the attribute or blank on error
std::string InterpolationToAttribute(builtin::InterpolationType type,
                                     builtin::InterpolationSampling sampling);

/// Prints a float32 to the output stream
/// @param out the stream to write too
/// @param value the float32 value
void PrintF32(StringStream& out, float value);

/// Prints a float16 to the output stream
/// @param out the stream to write too
/// @param value the float16 value
void PrintF16(StringStream& out, float value);

/// Prints an int32 to the output stream
/// @param out the stream to write too
/// @param value the int32 value
void PrintI32(StringStream& out, int32_t value);

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_COMMON_PRINTER_SUPPORT_H_
