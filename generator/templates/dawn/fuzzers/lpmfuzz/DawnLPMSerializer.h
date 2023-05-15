// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_FUZZERS_DAWNLPMSERIALIZER_H_
#define SRC_DAWN_FUZZERS_DAWNLPMSERIALIZER_H_

#include "dawn/fuzzers/lpmfuzz/dawn_lpm_autogen.pb.h"
#include "dawn/wire/ChunkedCommandSerializer.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireResult.h"

namespace dawn::wire {

class DawnLPMObjectIdProvider : public ObjectIdProvider {
  private:

    // Implementation of the ObjectIdProvider interface
    {% for type in by_category["object"] %}
        WireResult GetId({{as_cType(type.name)}} object, ObjectId* out) const final {
            *out = reinterpret_cast<uintptr_t>(object);
            return WireResult::Success;
        }
        WireResult GetOptionalId({{as_cType(type.name)}} object, ObjectId* out) const final {
            *out = reinterpret_cast<uintptr_t>(object);
            return WireResult::Success;
        }
    {% endfor %}

};

WireResult SerializedData(const fuzzing::Program& program,
                       dawn::wire::ChunkedCommandSerializer serializer);

}  // namespace dawn::wire

#endif  // SRC_DAWN_FUZZERS_DAWNLPMSERIALIZER_H_
