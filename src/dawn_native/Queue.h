// Copyright 2017 The Dawn Authors
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

#ifndef DAWNNATIVE_QUEUE_H_
#define DAWNNATIVE_QUEUE_H_

#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    class QueueBase : public ObjectBase {
      public:
        QueueBase(DeviceBase* device);

        static QueueBase* MakeError(DeviceBase* device);

        // Dawn API
        void Submit(uint32_t commandCount, CommandBufferBase* const* commands);
        void Signal(Fence* fence, uint64_t signalValue);
        Fence* CreateFence(const FenceDescriptor* descriptor);

      private:
        QueueBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        virtual MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands);

        MaybeError ValidateSubmit(uint32_t commandCount, CommandBufferBase* const* commands);
        MaybeError ValidateSignal(const Fence* fence, uint64_t signalValue);
        MaybeError ValidateCreateFence(const FenceDescriptor* descriptor);
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_QUEUE_H_
