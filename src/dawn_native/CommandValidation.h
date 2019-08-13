// Copyright 2019 The Dawn Authors
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

#ifndef DAWNNATIVE_COMMANDVALIDATION_H_
#define DAWNNATIVE_COMMANDVALIDATION_H_

#include "dawn_native/CommandAllocator.h"
#include "dawn_native/Error.h"

#include <vector>

namespace dawn_native {

    class AttachmentState;
    struct BeginRenderPassCmd;
    struct PassResourceUsage;

    MaybeError ValidateRenderBundle(CommandIterator* commands,
                                    const AttachmentState* attachmentState,
                                    PassResourceUsage* resourceUsage);
    MaybeError ValidateRenderPass(CommandIterator* commands,
                                  BeginRenderPassCmd* renderPass,
                                  std::vector<PassResourceUsage>* perPassResourceUsages);
    MaybeError ValidateComputePass(CommandIterator* commands,
                                   std::vector<PassResourceUsage>* perPassResourceUsages);

}  // namespace dawn_native

#endif  // DAWNNATIVE_COMMANDVALIDATION_H_
