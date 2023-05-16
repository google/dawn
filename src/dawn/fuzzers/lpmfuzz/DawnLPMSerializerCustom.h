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

#ifndef SRC_DAWN_FUZZERS_DAWNLPMCUSTOMSERIALIZER_H_
#define SRC_DAWN_FUZZERS_DAWNLPMCUSTOMSERIALIZER_H_

#include "dawn/fuzzers/lpmfuzz/DawnLPMObjectStore.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.h"
#include "dawn/wire/ChunkedCommandSerializer.h"
#include "dawn/wire/ObjectType_autogen.h"

namespace dawn::wire {
void GetCustomSerializedData(
    const fuzzing::Command& command,
    dawn::wire::ChunkedCommandSerializer serializer,
    ityp::array<dawn::wire::ObjectType, dawn::wire::DawnLPMObjectStore, 24>& gObjectStores,
    DawnLPMObjectIdProvider& provider);
}

#endif  // SRC_DAWN_FUZZERS_DAWNLPMCUSTOMSERIALIZER_H_
