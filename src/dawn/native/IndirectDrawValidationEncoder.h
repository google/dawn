// Copyright 2021 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_INDIRECTDRAWVALIDATIONENCODER_H_
#define SRC_DAWN_NATIVE_INDIRECTDRAWVALIDATIONENCODER_H_

#include "dawn/native/Error.h"
#include "dawn/native/IndirectDrawMetadata.h"

namespace dawn::native {

class CommandEncoder;
struct CombinedLimits;
class DeviceBase;
class RenderPassResourceUsageTracker;

// The maximum number of draws call we can fit into a single validation batch. This is
// essentially limited by the number of indirect parameter blocks that can fit into the maximum
// allowed storage binding size (with the base limits, it is about 6.7M).
uint32_t ComputeMaxDrawCallsPerIndirectValidationBatch(const CombinedLimits& limits);

MaybeError EncodeIndirectDrawValidationCommands(DeviceBase* device,
                                                CommandEncoder* commandEncoder,
                                                RenderPassResourceUsageTracker* usageTracker,
                                                IndirectDrawMetadata* indirectDrawMetadata);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INDIRECTDRAWVALIDATIONENCODER_H_
