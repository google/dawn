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

#include "common/Assert.h"
#include "dawn_wire/server/Server.h"

namespace dawn_wire { namespace server {

    bool Server::PostHandleQueueSignal(const QueueSignalCmd& cmd) {
        if (cmd.fence == nullptr) {
            return false;
        }
        ObjectId fenceId = mFenceIdTable.Get(cmd.fence);
        ASSERT(fenceId != 0);
        auto* fence = mKnownFence.Get(fenceId);
        ASSERT(fence != nullptr);

        auto* data = new FenceCompletionUserdata;
        data->server = this;
        data->fence = ObjectHandle{fenceId, fence->serial};
        data->value = cmd.signalValue;

        auto userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data));
        mProcs.fenceOnCompletion(cmd.fence, cmd.signalValue, ForwardFenceCompletedValue, userdata);
        return true;
    }

}}  // namespace dawn_wire::server
