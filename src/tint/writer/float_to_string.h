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

#ifndef SRC_TINT_WRITER_FLOAT_TO_STRING_H_
#define SRC_TINT_WRITER_FLOAT_TO_STRING_H_

#include <string>

namespace tint::writer {

/// Converts the float `f` to a string using fixed-point notation (not
/// scientific). The float will be printed with the full precision required to
/// describe the float. All trailing `0`s will be omitted after the last
/// non-zero fractional number, unless the fractional is zero, in which case the
/// number will end with `.0`.
/// @return the float f formatted to a string
std::string FloatToString(float f);

/// Converts the double `f` to a string using fixed-point notation (not
/// scientific). The double will be printed with the full precision required to
/// describe the double. All trailing `0`s will be omitted after the last
/// non-zero fractional number, unless the fractional is zero, in which case the
/// number will end with `.0`.
/// @return the double f formatted to a string
std::string DoubleToString(double f);

/// Converts the float `f` to a string, using hex float notation for infinities,
/// NaNs, or subnormal numbers. Otherwise behaves as FloatToString.
/// @return the float f formatted to a string
std::string FloatToBitPreservingString(float f);

/// Converts the double `f` to a string, using hex double notation for infinities,
/// NaNs, or subnormal numbers. Otherwise behaves as FloatToString.
/// @return the double f formatted to a string
std::string DoubleToBitPreservingString(double f);

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_FLOAT_TO_STRING_H_
