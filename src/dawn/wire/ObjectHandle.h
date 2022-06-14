// Copyright 2022 The Dawn Authors
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

#ifndef DAWN_WIRE_OBJECTHANDLE_H_
#define DAWN_WIRE_OBJECTHANDLE_H_

#include <cstdint>

namespace dawn::wire {

using ObjectId = uint32_t;
using ObjectGeneration = uint32_t;
struct ObjectHandle {
    ObjectId id;
    ObjectGeneration generation;

    ObjectHandle();
    ObjectHandle(ObjectId id, ObjectGeneration generation);

    explicit ObjectHandle(const volatile ObjectHandle& rhs);
    ObjectHandle& operator=(const volatile ObjectHandle& rhs);

    ObjectHandle(const ObjectHandle& rhs);
    ObjectHandle& operator=(const ObjectHandle& rhs);

    // MSVC has a bug where it thinks the volatile copy assignment is a duplicate.
    // Workaround this by forwarding to a different function AssignFrom.
    template <typename T>
    ObjectHandle& operator=(const T& rhs) {
        return AssignFrom(rhs);
    }
    ObjectHandle& AssignFrom(const ObjectHandle& rhs);
    ObjectHandle& AssignFrom(const volatile ObjectHandle& rhs);
};

}  // namespace dawn::wire

#endif  // DAWN_WIRE_OBJECTHANDLE_H_
