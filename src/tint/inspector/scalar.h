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

#ifndef SRC_TINT_INSPECTOR_SCALAR_H_
#define SRC_TINT_INSPECTOR_SCALAR_H_

#include <cstdint>

namespace tint::inspector {

/// Contains a literal scalar value
class Scalar {
  public:
    /// Null Constructor
    Scalar();
    /// @param val literal scalar value to contain
    explicit Scalar(bool val);
    /// @param val literal scalar value to contain
    explicit Scalar(uint32_t val);
    /// @param val literal scalar value to contain
    explicit Scalar(int32_t val);
    /// @param val literal scalar value to contain
    explicit Scalar(float val);

    /// @returns true if this is a null
    bool IsNull() const;
    /// @returns true if this is a bool
    bool IsBool() const;
    /// @returns true if this is a unsigned integer.
    bool IsU32() const;
    /// @returns true if this is a signed integer.
    bool IsI32() const;
    /// @returns true if this is a float.
    bool IsFloat() const;

    /// @returns scalar value if bool, otherwise undefined behaviour.
    bool AsBool() const;
    /// @returns scalar value if unsigned integer, otherwise undefined behaviour.
    uint32_t AsU32() const;
    /// @returns scalar value if signed integer, otherwise undefined behaviour.
    int32_t AsI32() const;
    /// @returns scalar value if float, otherwise undefined behaviour.
    float AsFloat() const;

  private:
    typedef enum {
        kNull,
        kBool,
        kU32,
        kI32,
        kFloat,
    } Type;

    typedef union {
        bool b;
        uint32_t u;
        int32_t i;
        float f;
    } Value;

    Type type_;
    Value value_;
};

}  // namespace tint::inspector

#endif  // SRC_TINT_INSPECTOR_SCALAR_H_
