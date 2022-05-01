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

#include "src/tint/inspector/scalar.h"

namespace tint::inspector {

Scalar::Scalar() : type_(kNull) {}

Scalar::Scalar(bool val) : type_(kBool) {
    value_.b = val;
}

Scalar::Scalar(uint32_t val) : type_(kU32) {
    value_.u = val;
}

Scalar::Scalar(int32_t val) : type_(kI32) {
    value_.i = val;
}

Scalar::Scalar(float val) : type_(kFloat) {
    value_.f = val;
}

bool Scalar::IsNull() const {
    return type_ == kNull;
}

bool Scalar::IsBool() const {
    return type_ == kBool;
}

bool Scalar::IsU32() const {
    return type_ == kU32;
}

bool Scalar::IsI32() const {
    return type_ == kI32;
}

bool Scalar::IsFloat() const {
    return type_ == kFloat;
}

bool Scalar::AsBool() const {
    return value_.b;
}

uint32_t Scalar::AsU32() const {
    return value_.u;
}

int32_t Scalar::AsI32() const {
    return value_.i;
}

float Scalar::AsFloat() const {
    return value_.f;
}

}  // namespace tint::inspector
