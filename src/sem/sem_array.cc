// Copyright 2021 The Tint Authors.
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

#include "src/sem/array.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Array);

namespace tint {
namespace sem {

Array::Array(type::Array* type, uint32_t align, uint32_t size, uint32_t stride)
    : type_(type), align_(align), size_(size), stride_(stride) {}

}  // namespace sem
}  // namespace tint
