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

#ifndef DAWNWIRE_CLIENT_FENCE_H_
#define DAWNWIRE_CLIENT_FENCE_H_

#include <dawn/webgpu.h>

#include "common/SerialMap.h"
#include "dawn_wire/client/ObjectBase.h"

namespace dawn_wire { namespace client {

    class Queue;
    class Fence final : public ObjectBase {
      public:
        using ObjectBase::ObjectBase;
        ~Fence();
        void Initialize(const WGPUFenceDescriptor* descriptor);

        void CheckPassedFences();
        void OnCompletion(uint64_t value, WGPUFenceOnCompletionCallback callback, void* userdata);
        void OnUpdateCompletedValueCallback(uint64_t value);
        bool OnCompletionCallback(uint64_t requestSerial, WGPUFenceCompletionStatus status);

        uint64_t GetCompletedValue() const;

      private:
        void CancelCallbacksForDisconnect() override;

        struct OnCompletionData {
            WGPUFenceOnCompletionCallback callback = nullptr;
            void* userdata = nullptr;
        };
        uint64_t mCompletedValue = 0;
        uint64_t mOnCompletionRequestSerial = 0;
        std::map<uint64_t, OnCompletionData> mOnCompletionRequests;
    };

}}  // namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_FENCE_H_
