// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_WRITER_GLSL_VERSION_H_
#define SRC_TINT_WRITER_GLSL_VERSION_H_

#include <cstdint>

namespace tint::writer::glsl {

/// A structure representing the version of GLSL to be generated.
struct Version {
    /// Is this version desktop GLSL, or GLSL ES?
    enum class Standard {
        kDesktop,
        kES,
    };

    /// Constructor
    /// @param standard_ Desktop or ES
    /// @param major_ the major version
    /// @param minor_ the minor version
    Version(Standard standard_, uint32_t major_, uint32_t minor_)
        : standard(standard_), major_version(major_), minor_version(minor_) {}

    /// Default constructor (see default values below)
    Version() = default;

    /// @returns true if this version is GLSL ES
    bool IsES() const { return standard == Standard::kES; }

    /// @returns true if this version is Desktop GLSL
    bool IsDesktop() const { return standard == Standard::kDesktop; }

    /// Desktop or ES
    Standard standard = Standard::kES;

    /// Major GLSL version
    uint32_t major_version = 3;

    /// Minor GLSL version
    uint32_t minor_version = 1;
};

}  // namespace tint::writer::glsl

#endif  // SRC_TINT_WRITER_GLSL_VERSION_H_
